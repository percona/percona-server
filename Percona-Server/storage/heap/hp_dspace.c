/* Copyright (C) 2000-2002 MySQL AB
   Copyright (C) 2008 eBay, Inc

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*
  Implements various base dataspace-related functions - allocate, free, clear
*/

#include "heapdef.h"


/*
  MySQL Heap tables keep data in arrays of fixed-size chunks.
  These chunks are organized into two groups of HP_BLOCK structures:
    - group1 contains indexes, with one HP_BLOCK per key
      (part of HP_KEYDEF)
    - group2 contains record data, with single HP_BLOCK
      for all records, referenced by HP_SHARE.recordspace.block

  While columns used in index are usually small, other columns
  in the table may need to accomodate larger data. Typically,
  larger data is placed into VARCHAR or BLOB columns. With actual
  sizes varying, Heap Engine has to support variable-sized records
  in memory. Heap Engine implements the concept of dataspace
  (HP_DATASPACE), which incorporates HP_BLOCK for the record data,
  and adds more information for managing variable-sized records.

  Variable-size records are stored in multiple "chunks",
  which means that a single record of data (database "row") can
  consist of multiple chunks organized into one "set". HP_BLOCK
  contains chunks. In variable-size format, one record
  is represented as one or many chunks, depending on the actual
  data, while in fixed-size mode, one record is always represented
  as one chunk. The index structures would always point to the first
  chunk in the chunkset.

  At the time of table creation, Heap Engine attempts to find out if
  variable-size records are desired. A user can request
  variable-size records by providing either row_type=dynamic or
  key_block_size=NNN table create option. Heap Engine will check
  whether key_block_size provides enough space in the first chunk
  to keep all null bits and columns that are used in indexes.
  If key_block_size is too small, table creation will be aborted
  with an error. Heap Engine will revert to fixed-size allocation
  mode if key_block_size provides no memory benefits (if the
  fixed-size record would always be shorter then the first chunk
  in the chunkset with the specified key_block_size).

  In order to improve index search performance, Heap Engine needs
  to keep all null flags and all columns used as keys inside
  the first chunk of a chunkset. In particular, this means that
  all columns used as keys should be defined first in the table
  creation SQL. The length of data used by null bits and key columns
  is stored as fixed_data_length inside HP_SHARE. fixed_data_length
  will extend past last key column if more fixed-length fields can
  fit into the first chunk.

  Variable-size records are necessary only in the presence of
  variable-size columns. Heap Engine will be looking for BLOB
  columns or VARCHAR columns, which declare length of 32 or more. If
  no such columns are found, table will be switched to fixed-size
  format. You should always try to put such columns at the end of
  the table definition.

  Whenever data is being inserted or updated in the table
  Heap Engine will calculate how many chunks are necessary.
  For insert operations, Heap Engine allocates new chunkset in
  the recordspace. For update operations it will modify length of
  the existing chunkset, unlinking unnecessary chunks at the end,
  or allocating and adding more if larger length is necessary.

  When writing data to chunks or copying data back to record,
  fixed-size columns are copied in their full format. VARCHARs and
  BLOBs are copied based on their actual length. Any NULL values
  after fixed_data_length are skipped.

  The allocation and contents of the actual chunks varies between
  fixed and variable-size modes. Total chunk length is always
  aligned to the next sizeof(uchar*). Here is the format of
  fixed-size chunk:
      uchar[] - sizeof=chunk_dataspace_length, but at least
                sizeof(uchar*) bytes. Keeps actual data or pointer
                to the next deleted chunk.
                chunk_dataspace_length equals to full record length
      uchar   - status field (1 means "in use", 0 means "deleted")

  Variable-size chunk uses different format:
      uchar[] - sizeof=chunk_dataspace_length, but at least
                sizeof(uchar*) bytes. Keeps actual data or pointer
                to the next deleted chunk.
                chunk_dataspace_length is set according to table
                setup (key_block_size)
      uchar*  - pointer to the next chunk in this chunkset,
                or NULL for the last chunk
      uchar   - status field (1 means "first", 0 means "deleted",
                2 means "linked")

  When allocating a new chunkset of N chunks, Heap Engine will try
  to allocate chunks one-by-one, linking them as they become
  allocated. Allocation of a single chunk will attempt to reuse
  a deleted (freed) chunk. If no free chunks are available,
  it will attempt to allocate a new area inside HP_BLOCK.
  Freeing chunks will place them at the front of free list
  referenced by del_link in HP_DATASPACE. The newly freed chunk
  will contain reference to the previously freed chunk in its first
  sizeof(uchar*) of the payload space.

  Here is open issues:
    -  It is not very nice to require people to keep key columns
       at the beginning of the table creation SQL. There are three
       proposed resolutions:
       a. Leave it as is. It's a reasonable limitation
       b. Add new HA_KEEP_KEY_COLUMNS_TO_FRONT flag to handler.h and
          make table.cpp align columns when it creates the table
       c. Make HeapEngine reorder columns in the chunk data, so that
          key columns go first. Add parallel HA_KEYSEG structures
          to distinguish positions in record vs. positions in
          the first chunk. Copy all data field-by-field rather than
          using single memcpy unless DBA kept key columns to
          the beginning.
    -  heap_check_heap needs verify linked chunks, looking for
       issues such as orphans, cycles, and bad links. However,
       Heap Engine today does not do similar things even for
       free list.
    -  In a more sophisticated implementation, some space can
       be saved even with all fixed-size columns if many of them
       have NULL value, as long as these columns are not used
       in indexes
    -  In variable-size format status should be moved to lower
       bits of the "next" pointer. Pointer is always aligned
       to sizeof(byte*), which is at least 4, leaving 2 lower
       bits free. This will save 8 bytes per chunk
       on 64-bit platform.
    -  As we do not want to modify FRM format or to add new SQL
       keywords, KEY_BLOCK_SIZE option of "CREATE TABLE" is reused
       to specify block size for Heap Engine tables.
    -  since all key columns must fit in the first chunk, having keys
       on BLOB columns is currently impossible. This limitation is
       relatively easiy to remove in future.
*/

static uchar *hp_allocate_one_chunk(HP_DATASPACE *info);


/**
  Clear a dataspace

  Frees memory and zeros-out any relevant counters in the dataspace

  @param  info  the dataspace to clear
*/

void hp_clear_dataspace(HP_DATASPACE *info)
{
  if (info->block.levels)
  {
    hp_free_level(&info->block,info->block.levels,info->block.root,
                  (uchar *) 0);
  }
  info->block.levels= 0;
  info->del_chunk_count= info->chunk_count= 0;
  info->del_link= 0;
  info->total_data_length= 0;
}


/**
  Allocate or reallocate a chunkset in the dataspace

  Attempts to allocate a new chunkset or change the size of an existing chunkset

  @param  info            the hosting dataspace
  @param  chunk_count     the number of chunks that we expect as the result
  @param  existing_set    non-null value asks function to resize existing
                          chunkset, return value would point to this set

  @return  Pointer to the first chunk in the new or updated chunkset, or NULL
           if unsuccessful
*/

static uchar *hp_allocate_variable_chunkset(HP_DATASPACE *info,
                                           uint chunk_count,
                                           uchar *existing_set)
{
  int alloc_count= chunk_count, i;
  uchar *first_chunk= 0, *curr_chunk= 0, *prev_chunk= 0;
  uchar  *last_existing_chunk= 0;

  DBUG_ASSERT(alloc_count);

  if (existing_set)
  {
    first_chunk= existing_set;

    curr_chunk= existing_set;
    while (curr_chunk && alloc_count)
    {
      prev_chunk= curr_chunk;
      curr_chunk= *((uchar **) (curr_chunk + info->offset_link));
      alloc_count--;
    }

    if (!alloc_count)
    {
      if (curr_chunk)
      {
        /*
          We came through all chunks and there is more left, let's truncate the
          list.
        */
        *((uchar **) (prev_chunk + info->offset_link))= NULL;
        hp_free_chunks(info, curr_chunk);
      }

      return first_chunk;
    }

    last_existing_chunk= prev_chunk;
  }

  /*
    We can reach this point only if we're allocating new chunkset or more chunks
    in existing set.
  */

  for (i= 0; i < alloc_count; i++)
  {
    curr_chunk= hp_allocate_one_chunk(info);
    if (!curr_chunk)
    {
      /* no space in the current block */

      if (last_existing_chunk)
      {
        /* Truncate whatever was added at the end of the existing chunkset */
        prev_chunk= last_existing_chunk;
        curr_chunk= *((uchar **)(prev_chunk + info->offset_link));
        *((uchar **)(prev_chunk + info->offset_link))= NULL;
        hp_free_chunks(info, curr_chunk);
      }
      else if (first_chunk)
      {
        /* free any chunks previously allocated */
        hp_free_chunks(info, first_chunk);
      }

      return NULL;
    }

    /* mark as if this chunk is last in the chunkset */
    *((uchar **) (curr_chunk + info->offset_link))= 0;

    if (prev_chunk)
    {
      /* tie them into a linked list */
      *((uchar **) (prev_chunk + info->offset_link))= curr_chunk;
      /* Record linked from active */
      curr_chunk[info->offset_status]= CHUNK_STATUS_LINKED;
    }
    else
    {
      /* Record active */
      curr_chunk[info->offset_status]= CHUNK_STATUS_ACTIVE;
    }

    if (!first_chunk)
    {
      first_chunk= curr_chunk;
    }

    prev_chunk= curr_chunk;
}

  return first_chunk;
}


/**
  Allocate a new chunkset in the dataspace

  Attempts to allocate a new chunkset

  @param  info            the hosting dataspace
  @param  chunk_count     the number of chunks that we expect as the result

  @return  Pointer to the first chunk in the new or updated chunkset, or NULL if
           unsuccessful
*/

uchar *hp_allocate_chunkset(HP_DATASPACE *info, uint chunk_count)
{
  uchar *result;

  DBUG_ENTER("hp_allocate_chunks");

  if (info->is_variable_size)
  {
    result = hp_allocate_variable_chunkset(info, chunk_count, NULL);
  }
  else
  {
    result= hp_allocate_one_chunk(info);
    if (result)
    {
      result[info->offset_status]= CHUNK_STATUS_ACTIVE;
    }

    DBUG_RETURN(result);
  }

  DBUG_RETURN(result);
}


/**
  Reallocate an existing chunkset in the dataspace

  Attempts to change the size of an existing chunkset

  @param  info            the hosting dataspace
  @param  chunk_count     the number of chunks that we expect as the result
  @param  pos             pointer to the existing chunkset

  @return  Error code or zero if successful
*/

int hp_reallocate_chunkset(HP_DATASPACE *info, uint chunk_count, uchar *pos)
{
  DBUG_ENTER("hp_reallocate_chunks");

  if (!info->is_variable_size)
  {
    /* Update should never change chunk_count in fixed-size mode */
    my_errno= HA_ERR_WRONG_COMMAND;
    return my_errno;
  }

  /* Reallocate never moves the first chunk */
  if (!hp_allocate_variable_chunkset(info, chunk_count, pos))
    DBUG_RETURN(my_errno);

  DBUG_RETURN(0);
}


/**
  Allocate a single chunk in the dataspace

  Attempts to allocate a new chunk or reuse one from deleted list

  @param  info            the hosting dataspace

  @return  Pointer to the chunk, or NULL if unsuccessful
*/

static uchar *hp_allocate_one_chunk(HP_DATASPACE *info)
{
  uchar *curr_chunk;
  size_t length;
  ulong block_pos;

  if (info->del_link)
  {
    curr_chunk= info->del_link;
    info->del_link= *((uchar **) curr_chunk);
    info->del_chunk_count--;

    DBUG_PRINT("hp_allocate_one_chunk",
               ("Used old position: 0x%lx",(long) curr_chunk));
    return curr_chunk;
  }

  block_pos= (info->chunk_count % info->block.records_in_block);
  if (!block_pos)
  {
    if (hp_get_new_block(&info->block, &length))
    {
      /* no space in the current block */
      return NULL;
    }

    info->total_data_length+= length;
  }

  info->chunk_count++;
  curr_chunk= ((uchar *) info->block.level_info[0].last_blocks +
               block_pos * info->block.recbuffer);

  DBUG_PRINT("hp_allocate_one_chunk",
             ("Used new position: 0x%lx", (long) curr_chunk));

  return curr_chunk;
}


/**
  Free a list of chunks

  Reclaims all chunks linked by the pointer,
  which could be the whole chunkset or a part of an existing chunkset

  @param  info            the hosting dataspace
  @param  pos             pointer to the head of the chunkset
*/

void hp_free_chunks(HP_DATASPACE *info, uchar *pos)
{
  uchar *curr_chunk= pos;

  while (curr_chunk)
  {
    info->del_chunk_count++;
    *((uchar **) curr_chunk)= info->del_link;
    info->del_link= curr_chunk;

    curr_chunk[info->offset_status]= CHUNK_STATUS_DELETED;

    DBUG_PRINT("hp_free_chunks",("Freed position: 0x%lx", (long) curr_chunk));

    if (!info->is_variable_size)
    {
      break;
    }

    /* Delete next chunk in this chunkset */
    curr_chunk= *((uchar **)(curr_chunk + info->offset_link));
  }
}
