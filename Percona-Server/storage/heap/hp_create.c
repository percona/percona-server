/* Copyright (c) 2000, 2011, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "heapdef.h"
#include <mysql_com.h>
#include <mysqld_error.h>

static int keys_compare(heap_rb_param *param, uchar *key1, uchar *key2);
static void init_block(HP_BLOCK *block,uint chunk_length, ulong min_records,
		       ulong max_records);

#define FIXED_REC_OVERHEAD (sizeof(uchar))
#define VARIABLE_REC_OVERHEAD (sizeof(uchar **) + sizeof(uchar))

/* Minimum size that a chunk can take, 12 bytes on 32bit, 24 bytes on 64bit */
#define VARIABLE_MIN_CHUNK_SIZE                                         \
  ((sizeof(uchar **) + VARIABLE_REC_OVERHEAD + sizeof(uchar **) - 1) &  \
   ~(sizeof(uchar **) - 1))

/* Create a heap table */

int heap_create(const char *name, HP_CREATE_INFO *create_info,
                HP_SHARE **res, my_bool *created_new_share)
{
  uint i, j, key_segs, max_length, length;
  HP_SHARE *share= 0;
  HA_KEYSEG *keyseg;
  HP_KEYDEF *keydef= create_info->keydef;
  uint reclength= create_info->reclength;
  uint keys= create_info->keys;
  ulong min_records= create_info->min_records;
  ulong max_records= create_info->max_records;
  ulong max_rows_for_stated_memory;
  DBUG_ENTER("heap_create");

  if (!create_info->internal_table)
  {
    mysql_mutex_lock(&THR_LOCK_heap);
    share= hp_find_named_heap(name);
    if (share && share->open_count == 0)
    {
      hp_free(share);
      share= 0;
    }
  }
  *created_new_share= (share == NULL);

  if (!share)
  {
    uint chunk_dataspace_length, chunk_length, is_variable_size;
    uint fixed_data_length, fixed_column_count;
    HP_KEYDEF *keyinfo;
    DBUG_PRINT("info",("Initializing new table"));

    if (create_info->max_chunk_size)
    {
      uint configured_chunk_size= create_info->max_chunk_size;

      /* User requested variable-size records, let's see if they're possible */

      if (configured_chunk_size < create_info->fixed_data_size)
      {
        /*
          The resulting chunk_size cannot be smaller than fixed data part
          at the start of the first chunk which allows faster copying
          with a single memcpy().
        */
        my_error(ER_CANT_USE_OPTION_HERE, MYF(0), "key_block_size");
        goto err;
      }

      if (reclength > configured_chunk_size + VARIABLE_REC_OVERHEAD ||
	  create_info->blobs > 0)
      {
        /*
          Allow variable size only if we're saving some space, i.e.
          if a fixed-size record would take more space than variable-size
          one plus the variable-size overhead.
          There has to be at least one field after indexed fields.
          Note that NULL bits are already included in key_part_size.
        */
        is_variable_size= 1;
        chunk_dataspace_length= configured_chunk_size;
      }
      else
      {
        /* max_chunk_size is near the full reclength, let's use fixed size */
        is_variable_size= 0;
        chunk_dataspace_length= reclength;
      }
    }
    else if ((create_info->is_dynamic && reclength >
              256 + VARIABLE_REC_OVERHEAD)
             || create_info->blobs > 0)
    {
      /*
        User asked for dynamic records - use 256 as the chunk size, if that
        will may save some memory. Otherwise revert to fixed size format.
      */
      if ((create_info->fixed_data_size + VARIABLE_REC_OVERHEAD) > 256)
        chunk_dataspace_length= create_info->fixed_data_size;
      else
        chunk_dataspace_length= 256 - VARIABLE_REC_OVERHEAD;

      is_variable_size= 1;
    }
    else
    {
      /*
        If max_chunk_size is not specified, put the whole record in one chunk
      */
      is_variable_size= 0;
      chunk_dataspace_length= reclength;
    }

    if (is_variable_size)
    {
      /* Check whether we have any variable size records past key data */
      uint has_variable_fields= 0;

      fixed_data_length= create_info->fixed_data_size;
      fixed_column_count= create_info->fixed_key_fieldnr;

      for (i= create_info->fixed_key_fieldnr; i < create_info->columns; i++)
      {
        HP_COLUMNDEF *column= create_info->columndef + i;
	if ((column->type == MYSQL_TYPE_VARCHAR &&
	     (column->length - column->length_bytes) >= 32) ||
	    column->type == MYSQL_TYPE_BLOB)
        {
            /*
              The field has to be either blob or >= 5.0.3 true VARCHAR
              and have substantial length.
              TODO: do we want to calculate minimum length?
            */
            has_variable_fields= 1;
            break;
        }

        if (has_variable_fields)
        {
          break;
        }

        if ((column->offset + column->length) <= chunk_dataspace_length)
        {
          /* Still no variable-size columns, add one fixed-length */
          fixed_column_count= i + 1;
          fixed_data_length= column->offset + column->length;
        }
      }

      if (!has_variable_fields && create_info->blobs == 0)
      {
        /*
          There is no need to use variable-size records without variable-size
          columns.
          Reset sizes if it's not variable size anymore.
        */
        is_variable_size= 0;
        chunk_dataspace_length= reclength;
        fixed_data_length= reclength;
        fixed_column_count= create_info->columns;
      }
    }
    else
    {
      fixed_data_length= reclength;
      fixed_column_count= create_info->columns;
    }

    /*
      We store uchar* del_link inside the data area of deleted records,
      so the data length should be at least sizeof(uchar*)
    */
    set_if_bigger(chunk_dataspace_length, sizeof (uchar **));

    if (is_variable_size)
    {
      chunk_length= chunk_dataspace_length + VARIABLE_REC_OVERHEAD;
    }
    else
    {
      chunk_length= chunk_dataspace_length + FIXED_REC_OVERHEAD;
    }

    /* Align chunk length to the next pointer */
    chunk_length= (uint) (chunk_length + sizeof(uchar **) - 1) &
      ~(sizeof(uchar **) - 1);

    for (i= key_segs= max_length= 0, keyinfo= keydef; i < keys; i++, keyinfo++)
    {
      memset(&keyinfo->block, 0, sizeof(keyinfo->block));
      memset(&keyinfo->rb_tree, 0, sizeof(keyinfo->rb_tree));
      for (j= length= 0; j < keyinfo->keysegs; j++)
      {
	length+= keyinfo->seg[j].length;
	if (keyinfo->seg[j].null_bit)
	{
	  length++;
	  if (!(keyinfo->flag & HA_NULL_ARE_EQUAL))
	    keyinfo->flag|= HA_NULL_PART_KEY;
	  if (keyinfo->algorithm == HA_KEY_ALG_BTREE)
	    keyinfo->rb_tree.size_of_element++;
	}
	switch (keyinfo->seg[j].type) {
        case HA_KEYTYPE_VARBINARY1:
        case HA_KEYTYPE_VARTEXT1:
        case HA_KEYTYPE_VARBINARY2:
        case HA_KEYTYPE_VARTEXT2:
          length+= 2;
          break;
	default:
	  break;
	}
      }
      keyinfo->length= length;
      length+= keyinfo->rb_tree.size_of_element + 
	       ((keyinfo->algorithm == HA_KEY_ALG_BTREE) ? sizeof(uchar*) : 0);
      if (length > max_length)
	max_length= length;
      key_segs+= keyinfo->keysegs;
      if (keyinfo->algorithm == HA_KEY_ALG_BTREE)
      {
        key_segs++; /* additional HA_KEYTYPE_END segment */
        if (keyinfo->flag & HA_VAR_LENGTH_KEY)
          keyinfo->get_key_length= hp_rb_var_key_length;
        else if (keyinfo->flag & HA_NULL_PART_KEY)
          keyinfo->get_key_length= hp_rb_null_key_length;
        else
          keyinfo->get_key_length= hp_rb_key_length;
      }
    }
    if (!(share= (HP_SHARE*) my_malloc((uint) sizeof(HP_SHARE)+
				       keys*sizeof(HP_KEYDEF)+
                                       (create_info->columns *
                                        sizeof(HP_COLUMNDEF)) +
				       key_segs*sizeof(HA_KEYSEG),
				       MYF(MY_ZEROFILL))))
      goto err;

    /*
      Max_records is used for estimating block sizes and for enforcement.
      Calculate the very maximum number of rows (if everything was one chunk)
      and then take either that value or configured max_records (pick smallest
      one).
    */
    max_rows_for_stated_memory= (ha_rows) (create_info->max_table_size /
                                           (create_info->keys_memory_size +
                                            chunk_length));
    max_records = ((max_records && max_records < max_rows_for_stated_memory) ?
                   max_records : max_rows_for_stated_memory);

    share->column_defs= (HP_COLUMNDEF*) (share + 1);
    memcpy(share->column_defs, create_info->columndef,
           (size_t) (sizeof(create_info->columndef[0]) *
                     create_info->columns));

    share->keydef= (HP_KEYDEF*) (share->column_defs + create_info->columns);
    share->key_stat_version= 1;
    keyseg= (HA_KEYSEG*) (share->keydef + keys);
    init_block(&share->recordspace.block, chunk_length, min_records,
               max_records);
	/* Fix keys */
    memcpy(share->keydef, keydef, (size_t) (sizeof(keydef[0]) * keys));
    for (i= 0, keyinfo= share->keydef; i < keys; i++, keyinfo++)
    {
      keyinfo->seg= keyseg;
      memcpy(keyseg, keydef[i].seg,
	     (size_t) (sizeof(keyseg[0]) * keydef[i].keysegs));
      keyseg+= keydef[i].keysegs;

      if (keydef[i].algorithm == HA_KEY_ALG_BTREE)
      {
	/* additional HA_KEYTYPE_END keyseg */
	keyseg->type=     HA_KEYTYPE_END;
	keyseg->length=   sizeof(uchar*);
	keyseg->flag=     0;
	keyseg->null_bit= 0;
	keyseg++;

	init_tree(&keyinfo->rb_tree, 0, 0, sizeof(uchar*),
		  (qsort_cmp2)keys_compare, 1, NULL, NULL);
	keyinfo->delete_key= hp_rb_delete_key;
	keyinfo->write_key= hp_rb_write_key;
      }
      else
      {
	init_block(&keyinfo->block, sizeof(HASH_INFO), min_records,
		   max_records);
	keyinfo->delete_key= hp_delete_key;
	keyinfo->write_key= hp_write_key;
        keyinfo->hash_buckets= 0;
      }
      if ((keyinfo->flag & HA_AUTO_KEY) && create_info->with_auto_increment)
        share->auto_key= i + 1;
    }
    share->min_records= min_records;
    share->max_records= max_records;
    share->max_table_size= create_info->max_table_size;
    share->index_length= 0;
    share->blength= 1;
    share->keys= keys;
    share->max_key_length= max_length;
    share->column_count= create_info->columns;
    share->changed= 0;
    share->auto_key= create_info->auto_key;
    share->auto_key_type= create_info->auto_key_type;
    share->auto_increment= create_info->auto_increment;
    share->create_time= (long) time((time_t*) 0);

    share->fixed_data_length= fixed_data_length;
    share->fixed_column_count= fixed_column_count;
    share->blobs= create_info->blobs;

    share->recordspace.chunk_length= chunk_length;
    share->recordspace.chunk_dataspace_length= chunk_dataspace_length;
    share->recordspace.is_variable_size= is_variable_size;
    share->recordspace.total_data_length= 0;

    if (is_variable_size) {
      share->recordspace.offset_link= chunk_dataspace_length;
      share->recordspace.offset_status= share->recordspace.offset_link +
        sizeof(uchar **);
    } else {
      /* Make it likely to fail if anyone uses this offset */
      share->recordspace.offset_link= 1 << 22;
      share->recordspace.offset_status= chunk_dataspace_length;
    }

    /* Must be allocated separately for rename to work */
    if (!(share->name= my_strdup(name,MYF(0))))
    {
      my_free(share);
      goto err;
    }
    thr_lock_init(&share->lock);
    mysql_mutex_init(hp_key_mutex_HP_SHARE_intern_lock,
                     &share->intern_lock, MY_MUTEX_INIT_FAST);
    if (!create_info->internal_table)
    {
      share->open_list.data= (void*) share;
      heap_share_list= list_add(heap_share_list,&share->open_list);
    }
    else
      share->delete_on_close= 1;
  }
  if (!create_info->internal_table)
  {
    if (create_info->pin_share)
      ++share->open_count;
    mysql_mutex_unlock(&THR_LOCK_heap);
  }

  *res= share;
  DBUG_RETURN(0);

err:
  if (!create_info->internal_table)
    mysql_mutex_unlock(&THR_LOCK_heap);
  DBUG_RETURN(1);
} /* heap_create */


static int keys_compare(heap_rb_param *param, uchar *key1, uchar *key2)
{
  uint not_used[2];
  return ha_key_cmp(param->keyseg, key1, key2, param->key_length, 
		    param->search_flag, not_used);
}

static void init_block(HP_BLOCK *block, uint chunk_length, ulong min_records,
		       ulong max_records)
{
  uint i,recbuffer,records_in_block;

  max_records= MY_MAX(min_records, max_records);
  if (!max_records)
    max_records= 1000;			/* As good as quess as anything */
  /*
    We want to start each chunk at 8 bytes boundary, round recbuffer to the
    next 8.
  */
  recbuffer= (uint) (chunk_length + sizeof(uchar**) - 1) &
    ~(sizeof(uchar**) - 1);
  records_in_block= max_records / 10;
  if (records_in_block < 10 && max_records)
    records_in_block= 10;
  if (!records_in_block || records_in_block*recbuffer >
      (my_default_record_cache_size-sizeof(HP_PTRS)*HP_MAX_LEVELS))
    records_in_block= (my_default_record_cache_size - sizeof(HP_PTRS) *
		      HP_MAX_LEVELS) / recbuffer + 1;
  block->records_in_block= records_in_block;
  block->recbuffer= recbuffer;
  block->last_allocated= 0L;

  for (i= 0; i <= HP_MAX_LEVELS; i++)
    block->level_info[i].records_under_level=
      (!i ? 1 : i == 1 ? records_in_block :
       HP_PTRS_IN_NOD * block->level_info[i - 1].records_under_level);
}


static inline void heap_try_free(HP_SHARE *share)
{
  if (share->open_count == 0)
    hp_free(share);
  else
    share->delete_on_close= 1;
}


int heap_delete_table(const char *name)
{
  int result;
  reg1 HP_SHARE *share;
  DBUG_ENTER("heap_delete_table");

  mysql_mutex_lock(&THR_LOCK_heap);
  if ((share= hp_find_named_heap(name)))
  {
    heap_try_free(share);
    result= 0;
  }
  else
  {
    result= my_errno=ENOENT;
  }
  mysql_mutex_unlock(&THR_LOCK_heap);
  DBUG_RETURN(result);
}


void heap_drop_table(HP_INFO *info)
{
  DBUG_ENTER("heap_drop_table");
  mysql_mutex_lock(&THR_LOCK_heap);
  heap_try_free(info->s);
  mysql_mutex_unlock(&THR_LOCK_heap);
  DBUG_VOID_RETURN;
}


void hp_free(HP_SHARE *share)
{
  if (share->open_list.data)                    /* If not internal table */
    heap_share_list= list_delete(heap_share_list, &share->open_list);
  hp_clear(share);			/* Remove blocks from memory */
  thr_lock_delete(&share->lock);
  mysql_mutex_destroy(&share->intern_lock);
  my_free(share->name);
  my_free(share);
  return;
}
