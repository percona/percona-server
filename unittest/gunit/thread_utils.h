/* Copyright (c) 2009, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef SQL_THREAD_INCLUDED
#define SQL_THREAD_INCLUDED

#include <my_global.h>
#include <mysql/psi/mysql_thread.h>

#include <sys/time.h>
#include <sys/resource.h>

namespace thread {

/*
  An abstract class for creating/running/joining threads.
  Thread::start() will create a new pthread, and execute the run() function.
*/
class Thread
{
public:
  Thread()
  {}
  virtual ~Thread()
  {}

  /*
    Will create a new pthread, and invoke run();
    Returns the value from my_thread_create().
  */
  int start();

  /*
    You may invoke this to wait for the thread to finish.
    You should probalbly join() a thread before deleting it.
  */
  void join();

  // The handle of the thread (valid only if it is actually running).
  my_thread_handle thread_handle() const { return m_thread_handle; }

  /*
    A wrapper for the run() function.
    Users should *not* call this function directly, they should rather
    invoke the start() function.
  */
  static void run_wrapper(Thread*);

protected:
  /*
    Define this function in derived classes.
    Users should *not* call this function directly, they should rather
    invoke the start() function.
  */
  virtual void run() = 0;

private:
  my_thread_handle m_thread_handle;

  Thread(const Thread&);                        /* Not copyable. */
  void operator=(const Thread&);                /* Not assignable. */
};


// A barrier which can be used for one-time synchronization between threads.
class Notification
{
public:
  Notification();
  ~Notification();

  bool has_been_notified();
  void wait_for_notification();
  void notify();
private:
  bool            m_notified;
  mysql_cond_t    m_cond;
  mysql_mutex_t   m_mutex;

  Notification(const Notification&);            /* Not copyable. */
  void operator=(const Notification&);          /* Not assignable. */
};

}  // namespace thread

class disable_core_dumps_in_scope
{
private:
  disable_core_dumps_in_scope(const disable_core_dumps_in_scope &);
  disable_core_dumps_in_scope& operator = (const disable_core_dumps_in_scope &);

  struct rlimit saved_core_rlimit;

public:
  disable_core_dumps_in_scope()
  {
    int ret= getrlimit(RLIMIT_CORE, &saved_core_rlimit);
    if (ret != 0)
    {
      perror("getrlimit(RLIMIT_CORE) failed");
      assert(ret == 0);
    }

    struct rlimit disabled_core_rlimit= saved_core_rlimit;
    disabled_core_rlimit.rlim_cur= 0;

    ret= setrlimit(RLIMIT_CORE, &disabled_core_rlimit);
    if (ret != 0)
    {
      perror("setrlimit(RLIMIT_CORE) failed");
      assert(ret == 0);
    }
  }

  ~disable_core_dumps_in_scope()
  {
    int ret= setrlimit(RLIMIT_CORE, &saved_core_rlimit);
    if (ret != 0)
    {
      perror("setrlimit(RLIMIT_CORE) failed");
      assert(ret == 0);
    }
  }
};

#define MY_EXPECT_DEATH_IF_SUPPORTED(statement, regex)  \
  do {  \
    disable_core_dumps_in_scope disable_core_dumps; \
    EXPECT_DEATH_IF_SUPPORTED(statement, regex); \
  } while (0)

#define MY_EXPECT_DEATH(statement, regex) \
  do { \
    disable_core_dumps_in_scope disable_core_dumps; \
    EXPECT_DEATH(statement, regex); \
  } while (0)

#endif  // SQL_THREAD_INCLUDED
