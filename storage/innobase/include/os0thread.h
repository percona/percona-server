/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/os0thread.h
 The interface to the operating system
 process and thread control primitives

 Created 9/8/1995 Heikki Tuuri
 *******************************************************/

#ifndef os0thread_h
#define os0thread_h

#include <atomic>
#include <future>
#include <thread>
#ifdef UNIV_LINUX
#include <sys/types.h>
#endif

#include "my_compiler.h"

class IB_thread {
 public:
  enum class State { INVALID, NOT_STARTED, ALLOWED_TO_START, STARTED, STOPPED };

  State state() const {
    return (m_state == nullptr ? State::INVALID : m_state->load());
  }

  void start();
  void wait(State state_to_wait_for = State::STOPPED);
  void join();

 private:
  std::shared_future<void> m_shared_future;
  std::shared_ptr<std::atomic<State>> m_state;

  friend class Runnable;

  void init(std::promise<void> &promise);
  void set_state(State state);
};

/** Operating system thread native handle */
using os_thread_id_t = std::thread::native_handle_type;

#ifdef UNIV_LINUX
/** An alias for pid_t on Linux, where setpriority() accepts thread id
of this type and not pthread_t */
using os_tid_t = pid_t;
#else
using os_tid_t = os_thread_id_t;
#endif

/** Returns the thread identifier of current thread. Currently the thread
identifier in Unix is the thread handle itself.
@return current thread native handle */
os_thread_id_t os_thread_get_curr_id();

bool os_thread_set_priority(int priority);

void os_thread_set_priority(int priority, const char *thread_name);

/** Return the thread handle. The purpose of this function is to cast the
native handle to an integer type for consistency
@return the current thread ID cast to an uint64_t */
#define os_thread_handle() ((uint64_t)(os_thread_get_curr_id()))

/** Compares two thread ids for equality.
@param[in]	lhs	OS thread or thread id
@param[in]	rhs	OS thread or thread id
return true if equal */
#define os_thread_eq(lhs, rhs) ((lhs) == (rhs))

/** Advises the OS to give up remainder of the thread's time slice. */
#define os_thread_yield()      \
  do {                         \
    std::this_thread::yield(); \
  } while (false)

/** The thread sleeps at least the time given in microseconds.
@param[in]	usecs		time in microseconds */
#define os_thread_sleep(usecs)                                     \
  do {                                                             \
    std::this_thread::sleep_for(std::chrono::microseconds(usecs)); \
  } while (false)

/** Returns the system-specific thread identifier of current
thread. On Linux, returns tid. On other systems currently returns
os_thread_get_curr_id().
@return current thread identifier */
MY_NODISCARD os_tid_t os_thread_get_tid() noexcept;

/** Set relative scheduling priority for a given thread on
Linux. Currently a no-op on other systems.
@param[in]	thread_id	thread id
@param[in]	relative_priority	system-specific priority value
@return An actual thread priority after the update  */
MY_NODISCARD
unsigned long int os_thread_set_priority(
    os_tid_t thread_id, unsigned long int relative_priority) noexcept;

#endif /* !os0thread_h */
