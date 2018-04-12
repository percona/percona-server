
handler *ha_tokupart::get_file_handler(TABLE_SHARE *share,
                                       MEM_ROOT *alloc,
                                       handlerton *db_type) {
  DBUG_ASSERT(db_type == tokudb_hton);
  ha_tokudb *file = new(alloc) ha_tokudb(db_type, share);
  file->init();
  return file;
}


/**
  Clone the open and locked partitioning handler.

  @param  mem_root  MEM_ROOT to use.

  @return Pointer to the successfully created clone or NULL

  @details
  This function creates a new Partition_base handler as a clone/copy. The
  original (this) must already be opened and locked. The clone will use
  the originals m_part_info.
  It also allocates memory for ref + ref_dup.
  In Partition_base::open() it will clone its original handlers partitions
  which will allocate then on the correct MEM_ROOT and also open them.
*/

handler *ha_tokupart::clone(const char *name, MEM_ROOT *mem_root)
{
  ha_tokupart *new_handler;

  DBUG_ENTER("Partition_base::clone");

  /* If this->table == NULL, then the current handler has been created but not
  opened. Prohibit cloning such handler. */
  if (!table)
    DBUG_RETURN(NULL);

  new_handler= new (mem_root) ha_tokupart(ht, table_share, m_part_info,
                                           this, mem_root);
  if (!new_handler)
    DBUG_RETURN(NULL);

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
  if (!(new_handler->ref= (uchar*) alloc_root(mem_root,
                                              ALIGN_SIZE(ref_length)*2)))
    goto err;

  if (new_handler->ha_open(table, name,
                           table->db_stat,
                           HA_OPEN_IGNORE_IF_LOCKED | HA_OPEN_NO_PSI_CALL))
    goto err;

  DBUG_RETURN((handler*) new_handler);

err:
  delete new_handler;
  DBUG_RETURN(NULL);
}

ulong ha_tokupart::index_flags(uint idx, uint part, bool all_parts) const {
    TOKUDB_HANDLER_DBUG_ENTER("");
    assert_always(table_share);
    ulong flags = (HA_READ_NEXT | HA_READ_PREV | HA_READ_ORDER |
        HA_KEYREAD_ONLY | HA_READ_RANGE | HA_DO_INDEX_COND_PUSHDOWN);
    if (key_is_clustering(&table_share->key_info[idx])) {
        flags |= HA_CLUSTERED_INDEX;
    }
    DBUG_RETURN(flags);
}


