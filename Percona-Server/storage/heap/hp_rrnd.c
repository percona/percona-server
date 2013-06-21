/* Copyright (C) 2000-2002, 2004, 2006 MySQL AB

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

/* Read a record from a random position */

#include "heapdef.h"

/*
	   Returns one of following values:
	   0 = Ok.
	   HA_ERR_RECORD_DELETED = Record is deleted.
	   HA_ERR_END_OF_FILE = EOF.
*/

int heap_rrnd(register HP_INFO *info, uchar *record, uchar *pos)
{
  HP_SHARE *share=info->s;
  DBUG_ENTER("heap_rrnd");
  DBUG_PRINT("enter",("info: 0x%lx  pos: %lx",(long) info, (long) pos));

  info->lastinx= -1;
  if (!(info->current_ptr= pos))
  {
    info->update= 0;
    DBUG_RETURN(my_errno= HA_ERR_END_OF_FILE);
  }
  if (get_chunk_status(&share->recordspace, info->current_ptr) !=
      CHUNK_STATUS_ACTIVE)
  {
    /* treat deleted and linked chunks as deleted */
    info->update= HA_STATE_PREV_FOUND | HA_STATE_NEXT_FOUND;
    DBUG_RETURN(my_errno=HA_ERR_RECORD_DELETED);
  }
  info->update=HA_STATE_PREV_FOUND | HA_STATE_NEXT_FOUND | HA_STATE_AKTIV;
  if (hp_extract_record(info, record, info->current_ptr))
  {
    DBUG_RETURN(my_errno);
  }
  DBUG_PRINT("exit", ("found record at 0x%lx", (long) info->current_ptr));
  info->current_hash_ptr=0;			/* Can't use rnext */
  DBUG_RETURN(0);
} /* heap_rrnd */


#ifdef WANT_OLD_HEAP_VERSION

/*
	   If pos == -1 then read next record
	   Returns one of following values:
	   0 = Ok.
	   HA_ERR_RECORD_DELETED = Record is deleted.
	   HA_ERR_END_OF_FILE = EOF.
*/

int heap_rrnd_old(register HP_INFO *info, uchar *record, ulong pos)
{
  HP_SHARE *share=info->s;
  DBUG_ENTER("heap_rrnd");
  DBUG_PRINT("enter",("info: 0x%lx  pos: %ld",info,pos));

  info->lastinx= -1;
  if (pos == (ulong) -1)
  {
    pos= ++info->current_record;
    if (pos % share->block.records_in_block &&	/* Quick next record */
        pos < share->used_chunk_count + share->deleted_chunk_count &&
        (info->update & HA_STATE_PREV_FOUND))
    {
      info->current_ptr+= share->block.recbufferlen;
      goto end;
    }
  }
  else
    info->current_record=pos;

  if (pos >= share->used_chunk_count + share->deleted_chunk_count)
  {
    info->update= 0;
    DBUG_RETURN(my_errno= HA_ERR_END_OF_FILE);
  }

	/* Find record number pos */
  hp_find_record(info, pos);

end:
  if (GET_CHUNK_STATUS(info, info->current_ptr) != CHUNK_STATUS_ACTIVE)
  {
    /* treat deleted and linked chunks as deleted */
    info->update= HA_STATE_PREV_FOUND | HA_STATE_NEXT_FOUND;
    DBUG_RETURN(my_errno=HA_ERR_RECORD_DELETED);
  }
  info->update=HA_STATE_PREV_FOUND | HA_STATE_NEXT_FOUND | HA_STATE_AKTIV;
  if (hp_extract_record(info, record, info->current_ptr))
  {
    DBUG_RETURN(my_errno);
  }
  DBUG_PRINT("exit",("found record at 0x%lx",info->current_ptr));
  info->current_hash_ptr=0;			/* Can't use rnext */
  DBUG_RETURN(0);
} /* heap_rrnd */

#endif /* WANT_OLD_HEAP_VERSION */
