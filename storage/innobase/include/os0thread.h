/*****************************************************************************

Copyright (c) 1995, 2021, Oracle and/or its affiliates.

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
#include <cstring>
#include <functional>
#include <future>
#include <sstream>
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

/** Returns the string representation of the thread ID supplied. It uses the
 only standard-compliant way of printing the thread ID.
 @param thread_id The thread ID to convert to string.
 @param hex_value If true, the conversion will be asked to output in
 hexadecimal format. The support for it is OS-implementation-dependent and may
 be ignored.
*/
std::string to_string(std::thread::id thread_id, bool hex_value = false);

#ifdef _WIN32
using os_tid_t = int;
#else
/** An alias for pid_t on Linux, where setpriority() accepts thread id
of this type and not pthread_t */
using os_tid_t = pid_t;
#endif

/** A class to allow any trivially copyable object to be XOR'ed. Trivially
copyable according to
https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable means we can
copy the underlying representation to array of chars, and back and consider
it a valid copy. It is thread-safe when changing, but no modifications must be
assured during reading the stored value. */
template <typename T_thing, typename T_digit>
class Atomic_xor_of_things {
 public:
  Atomic_xor_of_things() {
    /* We could just default-initialize the acc with acc{}, if not the
     SunStudio. */
    for (size_t i = 0; i < digits_count; i++) {
      acc[i].store(0);
    }
  }

  void xor_thing(T_thing id) {
    /* A buffer filled with zeros to pad the thing to next sizeof(T_digit)
     bytes. It is thread-safe. */
    char buff[sizeof(T_digit) * digits_count]{};
    memcpy(buff, &id, sizeof(T_thing));
    for (size_t i = 0; i < digits_count; i++) {
      T_digit x;
      memcpy(&x, buff + i * sizeof(T_digit), sizeof(T_digit));
      acc[i].fetch_xor(x);
    }
  }

  /** Returns an object that was XOR'ed odd number of times. This function
   assumes there is exactly one such object, and caller must assure this. This
   method is not thread-safe and caller must ensure no other thread is trying
   to modify the value. */
  T_thing recover_if_single() {
    T_thing res;
    char buff[sizeof(T_digit) * digits_count];
    for (size_t i = 0; i < acc.size(); ++i) {
      T_digit x = acc[i].load(std::memory_order_acquire);
      memcpy(buff + i * sizeof(T_digit), &x, sizeof(T_digit));
    }
    memcpy(&res, buff, sizeof(T_thing));
    return res;
  }

 private:
  static constexpr size_t digits_count =
      (sizeof(T_thing) + sizeof(T_digit) - 1) / sizeof(T_digit);
  /* initial value must be all zeros, as opposed to the representation of
  thing{}, because we care about "neutral element of the XOR operation", and not
  "default value of a thing". */
  std::array<std::atomic<T_digit>, digits_count> acc;
};

/** A type for std::thread::id digit to store XOR efficiently. This will make
 the compiler to optimize the operations hopefully to single instruction. */
using Xor_digit_for_thread_id =
    std::conditional<sizeof(std::thread::id) >= sizeof(uint64_t), uint64_t,
                     uint32_t>::type;

/** A type to store XORed objects of type std::thread::id */
using Atomic_xor_of_thread_id =
    Atomic_xor_of_things<std::thread::id, Xor_digit_for_thread_id>;

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
