/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#ifndef MASKING_FUNCTIONS_DICTIONARY_FLUSHER_THREAD_HPP
#define MASKING_FUNCTIONS_DICTIONARY_FLUSHER_THREAD_HPP

#include "masking_functions/dictionary_flusher_thread_fwd.hpp"  // IWYU pragma: export

#include <atomic>
#include <cstdint>
#include <memory>

#include "masking_functions/term_cache_core_fwd.hpp"

namespace masking_functions {

// Facts that impacted 'dictionary_flusher_thread' design.
// - 'mysql_command_xxx' services cannot be used inside component 'init()' /
//   'deinit()' handlers as that may mess up with the 'THD' object of the
//   connection that executes 'INSTALL COMPONENT' / 'UNINSTALL COMPONENT'.
//   Therefore, 'INSTALL COMPONENT' cannot immediately reload dictionary
//   cache. It can only spawn a background thread that will do this later.
//   The cache will be loaded either during the very first call to one of the
//   dictionary-related UDFs or by the flusher thread, whichever happens
//   first.
//
// - MySQL internal connection (an instance of the 'sql_context' class,
//   which in turn uses 'mysql_command_xxx' services) must not be created
//   in the component 'init()' handler but inside the background thread.
//   The main reason for this is that it needs to have its own THD
//   object associated with it. Internally it is done by seting
//   'MYSQL_COMMAND_LOCAL_THD_HANDLE' option for the internal connection.
//
// - MySQL Service registry is locked inside component 'init()' / 'deinit()'
//   handlers. In other words, we cannot instruct component 'init()' handler
//   to wait for background thread to initiate the connection as this will
//   result in a deadlock.
//
// - Similarly, we cannot instruct component 'deinit()' handler to wait for
//   internal connection to be closed using regular means. However, if we
//   set the 'MYSQL_NO_LOCK_REGISTRY' for this internal connection, it will
//   be closed without trying to lock MySQL Service registry (which is
//   already locked by the 'UNINSTALL COMPONENT' logic).
//
// - During startup when server installs components that are marked for
//   loading in the Data Dictionary, Session Server is not yet available
//   ('srv_session_server_is_available()' returns false) and
//   'mysql_command_xxx' cannot be used immediately. Therefore, the
//   first step the background thread needs to do is to wait until the
//   Session Server becomes available (until
//   'srv_session_server_is_available()' returns true) and only then
//   initiate an internal connection.
//
// - During shutdown MySQL server before uninstalling components tries
//   to gracefully close all remaining sessions (those registered in the
//   session manager). Our background thread is also in this list as it
//   sets 'MYSQL_COMMAND_LOCAL_THD_HANDLE' option for the internal
//   connection. Therefore, our background thread needs to respond to
//   'KILL CONNECTION' statement (to setting 'thd->is_killed') because
//   otherwise the thread will be killed by force (via 'ptheread_cancel()'
//   or similar) which most probably will result in server crash.
//
// - In rare cases when 'UNINSTALL COMPONENT' is called immediately after
//   'INSTALL COMPONENT' (and when background thread has been spawned but
//   not yet initialized the connection), the only safe way to avoid
//   deadlocks is to let 'UNINSTALL COMPONENT' ('deinit()' handler) to fail
//   earlier (without waiting for the background thread to join) by just
//   requesting it to stop later (by setting the state to 'stopped').
//   Performing several attempts to 'UNINSTALL COMPONENT' should eventually
//   succeed.
class dictionary_flusher_thread {
 public:
  dictionary_flusher_thread(const term_cache_core_ptr &cache_core,
                            std::uint64_t flush_interval_seconds);
  dictionary_flusher_thread(const dictionary_flusher_thread &other) = delete;
  dictionary_flusher_thread(dictionary_flusher_thread &&other) = delete;
  dictionary_flusher_thread &operator=(const dictionary_flusher_thread &other) =
      delete;
  dictionary_flusher_thread &operator=(dictionary_flusher_thread &&other) =
      delete;
  ~dictionary_flusher_thread();

 private:
  term_cache_core_ptr cache_core_;
  std::uint64_t flush_interval_seconds_;

  enum class state_type : std::uint8_t;
  using atomic_state_type = std::atomic<state_type>;
  atomic_state_type state_;

  struct jthread_deleter {
    void operator()(void *ptr) const noexcept;
  };
  using jthread_ptr = std::unique_ptr<void, jthread_deleter>;
  jthread_ptr thread_impl_;

  void do_periodic_reload();
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_DICTIONARY_FLUSHER_THREAD_HPP
