/* Copyright (c) 2000, 2024, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* Scan through all rows */

#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "storage/heap/heapdef.h"

/*
           Returns one of following values:
           0 = Ok.
           HA_ERR_RECORD_DELETED = Record is deleted.
           HA_ERR_END_OF_FILE = EOF.
*/

int heap_scan_init(HP_INFO *info) {
  DBUG_TRACE;
  info->lastinx = -1;
  info->current_record = (ulong)~0L; /* No current record */
  info->update = 0;
  //info->next_block = 0;
  return 0;
}

int heap_scan(HP_INFO *info, uchar *record) {
  HP_SHARE *share = info->s;
  ulong pos;
  DBUG_TRACE;

  pos = ++info->current_record;
  if (pos >= share->recordspace.chunk_count) {
    info->update = 0;
    set_my_errno(HA_ERR_END_OF_FILE);
    return HA_ERR_END_OF_FILE;
  }

  hp_find_record(info, pos);

  if (get_chunk_status(&share->recordspace, info->current_ptr) !=
      CHUNK_STATUS_ACTIVE) {
    DBUG_PRINT("warning", ("Found deleted record or secondary chunk"));
    info->update = HA_STATE_PREV_FOUND | HA_STATE_NEXT_FOUND;
    set_my_errno(HA_ERR_RECORD_DELETED);
    return HA_ERR_RECORD_DELETED;
  }
  info->update = HA_STATE_PREV_FOUND | HA_STATE_NEXT_FOUND | HA_STATE_AKTIV;
  if (hp_extract_record(info, record, info->current_ptr)) {
    return my_errno();
  }
  info->current_hash_ptr = nullptr; /* Can't use read_next */
  return 0;
} /* heap_scan */
