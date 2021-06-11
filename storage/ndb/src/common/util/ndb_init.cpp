/*
   Copyright (c) 2004, 2021, Oracle and/or its affiliates.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <ndb_global.h>
#include <my_sys.h>
#include <NdbMutex.h>

class EventLogger *g_eventLogger = NULL;

NdbMutex *g_ndb_connection_mutex = NULL;

extern class EventLogger * create_event_logger();
extern void destroy_event_logger(class EventLogger ** g_eventLogger);

static int ndb_init_called = 0;

extern "C" void NdbMutex_SysInit();
extern "C" void NdbMutex_SysEnd();
extern "C" void NdbCondition_initialize();
extern "C" int NdbThread_Init();
extern "C" void NdbThread_End();
extern "C" void Ndb_GetRUsage_Init();
extern "C" void Ndb_GetRUsage_End();
extern "C" int NdbLockCpu_Init();
extern "C" void NdbLockCpu_End();

extern void NdbTick_Init();
extern void NdbOut_Init();

extern "C"
{

void
ndb_init_internal()
{
  NdbOut_Init();
  NdbMutex_SysInit();
  if (!g_ndb_connection_mutex)
    g_ndb_connection_mutex = NdbMutex_Create();
  if (!g_eventLogger)
    g_eventLogger = create_event_logger();
  if ((g_ndb_connection_mutex == NULL) || (g_eventLogger == NULL))
  {
    {
      const char* err = "ndb_init() failed - exit\n";
      int res = (int)write(2, err, (unsigned)strlen(err));
      (void)res;
      exit(1);
    }
  }
  NdbTick_Init();
  NdbCondition_initialize();
  NdbThread_Init();
  Ndb_GetRUsage_Init();
  NdbLockCpu_Init();
}

int
ndb_init()
{
  if (ndb_init_called == 0)
  {
    ndb_init_called = 1;
    if (my_init())
    {
      const char* err = "my_init() failed - exit\n";
      int res = (int)write(2, err, (unsigned)strlen(err));
      (void)res;
      exit(1);
    }
    /*
      Initialize time conversion information
       - the "time conversion information" structures are primarily
         used by localtime_r() when converting epoch time into
         broken-down local time representation.
    */
    tzset();

    ndb_init_internal();
  }
  return 0;
}

void
ndb_end_internal()
{
  if (g_ndb_connection_mutex) 
  {
    NdbMutex_Destroy(g_ndb_connection_mutex);
    g_ndb_connection_mutex=NULL;
  }
  if (g_eventLogger)
    destroy_event_logger(&g_eventLogger);

  Ndb_GetRUsage_End();
  NdbLockCpu_End();
  NdbThread_End();
  NdbMutex_SysEnd();
}

void
ndb_end(int flags)
{
  if (ndb_init_called == 1)
  {
    my_end(flags);
    ndb_end_internal();
    ndb_init_called = 0;
  }
}

} /* extern "C" */
