/* Copyright (C) 2000-2002, 2004-2005 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA */

/* Update current record in heap-database */

#include "heapdef.h"

int heap_update(HP_INFO *info, const uchar *old_record, const uchar *new_record)
{
  HP_KEYDEF *keydef, *end, *p_lastinx;
  uchar *pos;
  my_bool auto_key_changed= 0;
  HP_SHARE *share= info->s;
  uint old_chunk_count, new_chunk_count;

  DBUG_ENTER("heap_update");

  test_active(info);
  pos=info->current_ptr;

  if (info->opt_flag & READ_CHECK_USED && hp_rectest(info, old_record))
    DBUG_RETURN(my_errno);				/* Record changed */

  hp_get_encoded_data_length(share, old_record, &old_chunk_count);
  hp_get_encoded_data_length(share, new_record, &new_chunk_count);

  if (new_chunk_count > old_chunk_count)
  {
    /* extend the old chunkset size as necessary, but do not shrink yet */
    if (hp_reallocate_chunkset(&share->recordspace, new_chunk_count, pos))
    {
      DBUG_RETURN(my_errno); /* Out of memory or table space */
    }
  }

  if (--(share->records) < share->blength >> 1) share->blength>>= 1;
  share->changed=1;

  p_lastinx= share->keydef + info->lastinx;
  for (keydef= share->keydef, end= keydef + share->keys; keydef < end; keydef++)
  {
    if (hp_rec_key_cmp(keydef, old_record, new_record, 0))
    {
      if ((*keydef->delete_key)(info, keydef, old_record, pos,
                                keydef == p_lastinx) ||
          (*keydef->write_key)(info, keydef, new_record, pos))
        goto err;
      if (share->auto_key == (uint) (keydef - share->keydef + 1))
        auto_key_changed= 1;
    }
  }

  hp_copy_record_data_to_chunkset(share, new_record, pos);
  if (++(share->records) == share->blength) share->blength+= share->blength;

  if (new_chunk_count < old_chunk_count)
  {
    /* Shrink the chunkset to its new size */
    hp_reallocate_chunkset(&share->recordspace, new_chunk_count, pos);
  }

#if !defined(DBUG_OFF) && defined(EXTRA_HEAP_DEBUG)
  DBUG_EXECUTE("check_heap",heap_check_heap(info, 0););
#endif
  if (auto_key_changed)
    heap_update_auto_increment(info, new_record);
  DBUG_RETURN(0);

 err:
  if (my_errno == HA_ERR_FOUND_DUPP_KEY)
  {
    info->errkey = (int) (keydef - share->keydef);
    if (keydef->algorithm == HA_KEY_ALG_BTREE)
    {
      /* we don't need to delete non-inserted key from rb-tree */
      if ((*keydef->write_key)(info, keydef, old_record, pos))
      {
        if (++(share->records) == share->blength)
	  share->blength+= share->blength;
        DBUG_RETURN(my_errno);
      }
      keydef--;
    }
    while (keydef >= share->keydef)
    {
      if (hp_rec_key_cmp(keydef, old_record, new_record, 0))
      {
        if ((*keydef->delete_key)(info, keydef, new_record, pos, 0) ||
            (*keydef->write_key)(info, keydef, old_record, pos))
	  break;
      }
      keydef--;
    }
  }
  if (++(share->records) == share->blength)
    share->blength+= share->blength;

  if (new_chunk_count > old_chunk_count)
  {
    /* Shrink the chunkset to its original size */
    hp_reallocate_chunkset(&share->recordspace, old_chunk_count, pos);
  }

  DBUG_RETURN(my_errno);
} /* heap_update */
