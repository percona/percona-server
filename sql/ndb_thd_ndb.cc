/*
   Copyright (c) 2011, 2021, Oracle and/or its affiliates.

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

#include "ndb_thd_ndb.h"
#include "mysql/plugin.h"          // thd_get_thread_id

/*
  Default value for max number of transactions createable against NDB from
  the handler. Should really be 2 but there is a transaction to much allocated
  when lock table is used, and one extra to used for global schema lock.
*/
static const int MAX_TRANSACTIONS= 4;


Thd_ndb*
Thd_ndb::seize(THD* thd)
{
  DBUG_ENTER("seize_thd_ndb");

  Thd_ndb* thd_ndb= new Thd_ndb(thd);
  if (thd_ndb == NULL)
    DBUG_RETURN(NULL);

  if (thd_ndb->ndb->init(MAX_TRANSACTIONS) != 0)
  {
    DBUG_PRINT("error", ("Ndb::init failed, error: %d  message: %s",
                         thd_ndb->ndb->getNdbError().code,
                         thd_ndb->ndb->getNdbError().message));
    
    delete thd_ndb;
    thd_ndb= NULL;
  }
  else
  {
    thd_ndb->ndb->setCustomData64(thd_get_thread_id(thd));
  }
  DBUG_RETURN(thd_ndb);
}


void
Thd_ndb::release(Thd_ndb* thd_ndb)
{
  DBUG_ENTER("release_thd_ndb");
  delete thd_ndb;
  DBUG_VOID_RETURN;
}


bool
Thd_ndb::recycle_ndb(void)
{
  DBUG_ENTER("recycle_ndb");
  DBUG_PRINT("enter", ("ndb: 0x%lx", (long)ndb));

  assert(global_schema_lock_trans == NULL);
  assert(trans == NULL);

  delete ndb;
  if ((ndb= new Ndb(connection, "")) == NULL)
  {
    DBUG_PRINT("error",("failed to allocate Ndb object"));
    DBUG_RETURN(false);
  }

  if (ndb->init(MAX_TRANSACTIONS) != 0)
  {
    delete ndb;
    ndb= NULL;
    DBUG_PRINT("error", ("Ndb::init failed, %d  message: %s",
                         ndb->getNdbError().code,
                         ndb->getNdbError().message));
    DBUG_RETURN(false);
  }
  else
  {
    ndb->setCustomData64(thd_get_thread_id(m_thd));
  }

  /* Reset last commit epoch for this 'session'. */
  m_last_commit_epoch_session = 0;

  DBUG_RETURN(true);
}


bool
Thd_ndb::valid_ndb(void) const
{
  // The ndb object should be valid as long as a
  // global schema lock transaction is ongoing
  if (global_schema_lock_trans)
    return true;

  // The ndb object should be valid as long as a
  // transaction is ongoing
  if (trans)
    return true;

  if (unlikely(m_connect_count != connection->get_connect_count()))
    return false;

  return true;
}


void
Thd_ndb::init_open_tables()
{
  count= 0;
  m_error= FALSE;
  my_hash_reset(&open_tables);
}


/*
  Used for every additional row operation, to update the guesstimate
  of pending bytes to send, and to check if it is now time to flush a batch.
*/

bool
Thd_ndb::add_row_check_if_batch_full(uint size)
{
  if (m_unsent_bytes == 0)
    free_root(&m_batch_mem_root, MY_MARK_BLOCKS_FREE);

  uint unsent= m_unsent_bytes;
  unsent+= size;
  m_unsent_bytes= unsent;
  return unsent >= m_batch_size;
}
