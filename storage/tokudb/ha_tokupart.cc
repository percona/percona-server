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
  This function creates a new Partition_base handler as a clone/copy. The
  original (this) must already be opened and locked. The clone will use
  the originals m_part_info.
  It also allocates memory for ref + ref_dup.
  In Partition_base::open() it will clone its original handlers partitions
  which will allocate then on the correct MEM_ROOT and also open them.
*/

handler *ha_tokupart::clone(const char *name, MEM_ROOT *mem_root) {
  ha_tokupart *new_handler;

  DBUG_ENTER("Partition_base::clone");

  /* If this->table == nullptr, then the current handler has been created but
  not opened. Prohibit cloning such handler. */
  if (!table) DBUG_RETURN(nullptr);

  new_handler =
      new (mem_root) ha_tokupart(ht, table_share, m_part_info, this, mem_root);
  if (!new_handler) DBUG_RETURN(nullptr);

  /*
    We will not clone each partition's handler here, it will be done in
    Partition_base::open() for clones. Also set_ha_share_ref is not needed
    here, since 1) ha_share is copied in the constructor used above
    2) each partition's cloned handler will set it from its original.
  */

  /*
    Allocate new_handler->ref here because otherwise ha_open will allocate it
    on this->table->mem_root and we will not be able to reclaim that memory
    when the clone handler object is destroyed.
  */
  if (!(new_handler->ref =
            (uchar *)alloc_root(mem_root, ALIGN_SIZE(ref_length) * 2)))
    goto err;

  // TODO: fix the last argument of ha_open
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
