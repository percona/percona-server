/* Copyright (C) 2018 Percona

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
   USA */

handler *ha_tokupart::get_file_handler(TABLE_SHARE *share,
                                       MEM_ROOT *alloc) const {
  ha_tokudb *file = new (alloc) ha_tokudb(tokudb_hton, share);
  file->init();
  return file;
}

/**
  Clone the open and locked partitioning handler.

  @param  mem_root  MEM_ROOT to use.

  @return Pointer to the successfully created clone or nullptr

  @details

  This function creates a new native_part::Partition_base handler as a
  clone/copy. The clone will use the original m_part_info. It also allocates
  memory for ref + ref_dup. In native_part::Partition_base::open()
  a new partition handlers are created (on the passed mem_root)
  for each partition and are also opened.
*/

handler *ha_tokupart::clone(const char *name, MEM_ROOT *mem_root) {
  ha_tokupart *new_handler;

  DBUG_ENTER("ha_tokupart::clone");

  /* If this->table == nullptr, then the current handler has been created but
  not opened. Prohibit cloning such handler. */
  if (!table) DBUG_RETURN(nullptr);

  new_handler =
      new (mem_root) ha_tokupart(ht, table_share, m_part_info, this, mem_root);

  if (!new_handler) DBUG_RETURN(nullptr);

  /*
    Allocate new_handler->ref here because otherwise ha_open will allocate it
    on this->table->mem_root and we will not be able to reclaim that memory
    when the clone handler object is destroyed.
  */
  if (!(new_handler->ref =
            (uchar *)alloc_root(mem_root, ALIGN_SIZE(ref_length) * 2)))
    goto err;

  /* We will not use clone() interface to clone individual partition
  handlers. This is because tokudb_create_handler() gives ha_tokupart handler
  instead of ha_tokudb handlers. This happens because of presence of parition
  info in TABLE_SHARE. New partition handlers are created for each partiton
  in native_part::Partition_base::open() */
  if (new_handler->ha_open(table, name, table->db_stat,
                           HA_OPEN_IGNORE_IF_LOCKED, nullptr))
    goto err;

  DBUG_RETURN((handler *)new_handler);

err:
  delete new_handler;
  DBUG_RETURN(nullptr);
}

ulong ha_tokupart::index_flags(uint idx, TOKUDB_UNUSED(uint part),
                               TOKUDB_UNUSED(bool all_parts)) const {
  TOKUDB_HANDLER_DBUG_ENTER("");
  assert_always(table_share);
  DBUG_RETURN(::index_flags(&table_share->key_info[idx]));
}

#if defined(TOKU_INCLUDE_RFR) && TOKU_INCLUDE_RFR
/*
  Check whether we need to perform row lookup when executing
  Update_rows_log_event or Delete_rows_log_event. Use the 1st
  partition is enough, see @c ha_tokudb::rpl_lookup_rows().
*/
bool ha_tokupart::rpl_lookup_rows() {
  return (static_cast<ha_tokudb *>(m_file[0]))->rpl_lookup_rows();
}
#else   // defined(TOKU_INCLUDE_RFR) && TOKU_INCLUDE_RFR
bool ha_tokupart::rpl_lookup_rows() { return true; }
#endif  // defined(TOKU_INCLUDE_RFR) && TOKU_INCLUDE_RFR
