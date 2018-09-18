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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
   USA */

/*
  Implements various base record-related functions, such as encode and decode
  into chunks.
*/

#include <mysql_com.h>
#include "heapdef.h"
#include "my_byteorder.h"
#include "mysql/service_mysql_alloc.h"

/**
  Calculate size of the record for the purpose of storing in chunks

  Walk through the fields of the record and calculates the exact space
  needed in chunks as well the the total chunk count

  @param       info         the hosting table
  @param       record       the record in standard unpacked format
  @param[out]  chunk_count  the number of chunks needed for this record

  @return The size of the required storage in bytes
*/

uint hp_get_encoded_data_length(const HP_SHARE &info, const uchar *record,
                                uint *chunk_count) noexcept {
  uint dst_offset = info.fixed_data_length;

  if (!info.recordspace.is_variable_size) {
    /* Nothing more to copy */
    *chunk_count = 1;
    return dst_offset;
  }

  for (uint i = info.fixed_column_count; i < info.column_count; i++) {
    HP_COLUMNDEF *column = info.column_defs + i;

    if (column->null_bit) {
      if (record[column->null_pos] & column->null_bit) {
        /* Skip all NULL values */
        continue;
      }
    }

    uint src_offset = column->offset;
    uint length;
    if (column->type == MYSQL_TYPE_VARCHAR) {
      uint pack_length;

      /* >= 5.0.3 true VARCHAR */

      pack_length = column->length_bytes;
      length = pack_length + (pack_length == 1
                                  ? (uint) * (const uchar *)(record + src_offset)
                                  : uint2korr(record + src_offset));
    } else if (is_blob_column(column)) {
      uint pack_length = column->length_bytes;

      length =
          pack_length + hp_calc_blob_length(pack_length, record + src_offset);
    } else {
      length = column->length;
    }

    dst_offset += length;
  }

  *chunk_count = get_chunk_count(&info.recordspace, dst_offset);

  return dst_offset;
}

#if !defined(DBUG_OFF) && defined(EXTRA_HEAP_DEBUG)
static void dump_chunk(const HP_SHARE &info, const uchar *curr_chunk) noexcept {
  fprintf(stdout, "Chunk dump at 0x%lx: ", (long)curr_chunk);
  for (uint i = 0; i < info.recordspace.chunk_dataspace_length; i++) {
    const uint b = *((uchar *)(curr_chunk + i));
    if (b < 0x10) {
      fprintf(stdout, "0");
    }
    fprintf(stdout, "%lx ", (long)b);
  }
  fprintf(stdout, ". Next = 0x%lx, Status = %d\n",
          (long)(*((uchar **)(curr_chunk + info.recordspace.offset_link))),
          (uint)(*((uchar *)(curr_chunk + info.recordspace.offset_status))));
}
#endif

/**
  Stores data from packed field into the preallocated chunkset,
  or performs data comparison

  @param  info         the hosting table
  @param  data         the field data in packed format
  @param  length       the field data length
  @param  pos_ptr      the target chunkset
  @param  off_ptr      the pointer to the offset within the current chunkset
  @param  is_compare   flag indicating whether we should compare data or store
                       it

  @return  Status of comparison
    @retval  true      if comparison found data differences
    @retval  false     otherwise
*/

static inline bool hp_process_field_data_to_chunkset(
    const HP_SHARE &info, const uchar *data, uint length, uchar **pos_ptr,
    uint *off_ptr, bool is_compare) noexcept {
  uchar *curr_chunk = *pos_ptr;
  uint dst_offset = *off_ptr;
  bool rc = true;

  while (length > 0) {
    uint to_copy = info.recordspace.chunk_dataspace_length - dst_offset;
    if (to_copy == 0) {
    /* Jump to the next chunk */
#if !defined(DBUG_OFF) && defined(EXTRA_HEAP_DEBUG)
      dump_chunk(info, curr_chunk);
#endif
      memcpy(&curr_chunk, curr_chunk + info.recordspace.offset_link,
             sizeof(uchar *));
      dst_offset = 0;
      continue;
    }

    to_copy = std::min(length, to_copy);

    if (is_compare) {
      if (memcmp(curr_chunk + dst_offset, data, (size_t)to_copy)) {
        goto end;
      }
    } else {
      memcpy(curr_chunk + dst_offset, data, (size_t)to_copy);
    }

    data += to_copy;
    dst_offset += to_copy;
    length -= to_copy;
  }

  rc = false;

end:
  *pos_ptr = curr_chunk;
  *off_ptr = dst_offset;

  return rc;
}

/**
  Encodes or compares record

  Copies data from original unpacked record into the preallocated chunkset,
  or performs data comparison

  @param  info         the hosting table
  @param  record       the record in standard unpacked format
  @param  pos          the target chunkset
  @param  is_compare   flag indicating whether we should compare data or store
                       it

  @return  Status of comparison
    @retval  true      if comparison fond data differences
    @retval  false     otherwise
*/

bool hp_process_record_data_to_chunkset(const HP_SHARE &info,
                                        const uchar *record, uchar *pos,
                                        bool is_compare) noexcept {
  uchar *curr_chunk = pos;

  if (is_compare) {
    if (memcmp(curr_chunk, record, (size_t)info.fixed_data_length)) {
      return true;
    }
  } else {
    memcpy(curr_chunk, record, (size_t)info.fixed_data_length);
  }

  if (!info.recordspace.is_variable_size) {
    /* Nothing more to copy */
    return false;
  }

  uint dst_offset = info.fixed_data_length;

  for (uint i = info.fixed_column_count; i < info.column_count; i++) {
    HP_COLUMNDEF *column = info.column_defs + i;

    if (column->null_bit) {
      if (record[column->null_pos] & column->null_bit) {
        /* Skip all NULL values */
        continue;
      }
    }

    const uchar *data = record + column->offset;
    uint length;
    if (column->type == MYSQL_TYPE_VARCHAR) {
      uint pack_length;

      /* >= 5.0.3 true VARCHAR */

      /* Make sure to copy length indicator and actuals string bytes */
      pack_length = column->length_bytes;
      length = pack_length + (pack_length == 1 ? (uint)*data : uint2korr(data));
    } else if (is_blob_column(column)) {
      uint pack_length;

      pack_length = column->length_bytes;
      /* Just want to store the length, so not interested in the return code */
      (void)hp_process_field_data_to_chunkset(info, data, pack_length,
                                              &curr_chunk, &dst_offset, 0);
      length = hp_calc_blob_length(pack_length, data);
      memcpy(&data, data + pack_length, sizeof(char *));
    } else {
      length = column->length;
    }

    if (hp_process_field_data_to_chunkset(info, data, length, &curr_chunk,
                                          &dst_offset, is_compare)) {
      return true;
    }
  }

#if !defined(DBUG_OFF) && defined(EXTRA_HEAP_DEBUG)
  dump_chunk(info, curr_chunk);
#endif

  return false;
}

/**
  Stores record in the heap table chunks

  Copies data from original unpacked record into the preallocated chunkset

  @param  info         the hosting table
  @param  record       the record in standard unpacked format
  @param  pos          the target chunkset
*/

void hp_copy_record_data_to_chunkset(const HP_SHARE &info, const uchar *record,
                                     uchar *pos) noexcept {
  DBUG_ENTER("hp_copy_record_data_to_chunks");

  hp_process_record_data_to_chunkset(info, record, pos, 0);

  DBUG_VOID_RETURN;
}

/*
  Function to switch curr_chunk to the next chunk in the chunkset and reset
  src_offset.
*/
static inline void switch_to_next_chunk_for_read(const HP_SHARE &share,
                                                 const uchar *&curr_chunk,
                                                 uint &src_offset) {
  memcpy(&curr_chunk, curr_chunk + share.recordspace.offset_link,
         sizeof(uchar *));
  src_offset = 0;
#if !defined(DBUG_OFF) && defined(EXTRA_HEAP_DEBUG)
  dump_chunk(share, curr_chunk);
#endif
}

/**
  Copies record data from storage to unpacked record format

  Copies data from chunkset into its original unpacked record

  @param       info         the hosting table
  @param[out]  record       the target record in standard unpacked format
  @param       pos          the source chunkset

  @return                   Status of conversion
    @retval   false         success
    @retval   true          out of memory
*/

bool hp_extract_record(HP_INFO *info, uchar *record,
                       const uchar *pos) noexcept {
  const HP_SHARE *const share = info->s;
  uint *rec_offsets = nullptr;
  uint *buf_offsets = nullptr;
  uint nblobs = 0;

  DBUG_ENTER("hp_extract_record");

#if !defined(DBUG_OFF) && defined(EXTRA_HEAP_DEBUG)
  if (share->recordspace.is_variable_size) {
    dump_chunk(share, curr_chunk);
  }
#endif

  const uchar *curr_chunk = pos;
  memcpy(record, curr_chunk, (size_t)share->fixed_data_length);

  if (!share->recordspace.is_variable_size) {
    /* Nothing more to copy */
    DBUG_RETURN(0);
  }

  /* Reserve space for rec_offsets and buf_offsets.*/
  const uint init_offset = share->blobs * sizeof(uint) * 2;
  info->blob_offset = init_offset;
  uint src_offset = share->fixed_data_length;

  for (uint i = share->fixed_column_count; i < share->column_count; i++) {
    uint length;
    uchar *to;

    HP_COLUMNDEF *column = share->column_defs + i;

    bool is_null = false;
    if (column->null_bit) {
      if (record[column->null_pos] & column->null_bit) {
        is_null = true;
      }
    }

    if (is_null) {
      /* TODO: is memset really needed? */
      memset(record + column->offset, 0, column->length);
      continue;
    }

    to = record + column->offset;
    if (column->type == MYSQL_TYPE_VARCHAR || is_blob_column(column)) {
      uint pack_length, i;
      uchar *tmp = to;

      pack_length = column->length_bytes;

      for (i = 0; i < pack_length; i++) {
        if (src_offset == share->recordspace.chunk_dataspace_length) {
          switch_to_next_chunk_for_read(*share, curr_chunk, src_offset);
        }
        *to++ = curr_chunk[src_offset++];
      }
      /*
        We copy byte-by-byte and then use hp_calc_blob_length to combine bytes
        in the right order.
      */
      length = hp_calc_blob_length(pack_length, tmp);

      if (is_blob_column(column) && length == 0) {
        /*
          Store a zero pointer for zero-length BLOBs because the server
          relies on that (see Field_blob::val_*().
        */
        memset(reinterpret_cast<uchar **>(to), 0, sizeof(uchar *));
      } else if (is_blob_column(column) && length > 0) {
        uint newsize = info->blob_offset + length;

        assert(share->blobs > 0);
        /*
          Make sure we have enough space in blob_buffer and store the pointer
          to this blob in record.
        */
        if (info->blob_size < newsize) {
          uchar *ptr;
          ptr = static_cast<uchar *>(my_realloc(hp_key_memory_HP_INFO,
                                                info->blob_buffer, newsize,
                                                MYF(MY_ALLOW_ZERO_PTR)));
          if (ptr == nullptr) {
            DBUG_RETURN(1);
          }

          if (info->blob_buffer == nullptr) {
            memset(ptr, 0, init_offset);
          }
          info->blob_buffer = ptr;
          info->blob_size = newsize;
        }

        rec_offsets = (uint *)info->blob_buffer;
        buf_offsets = rec_offsets + share->blobs;

        rec_offsets[nblobs] = (uint)(to - record);
        buf_offsets[nblobs] = info->blob_offset;
        nblobs++;

        /* Change 'to' so blob data is copied into blob_buffer */
        to = info->blob_buffer + info->blob_offset;
        info->blob_offset = newsize;
      }
    } else {
      length = column->length;
    }

    while (length > 0) {
      uint to_copy;

      to_copy = share->recordspace.chunk_dataspace_length - src_offset;
      if (to_copy == 0) {
        switch_to_next_chunk_for_read(*share, curr_chunk, src_offset);
        to_copy = share->recordspace.chunk_dataspace_length;
      }

      to_copy = std::min(length, to_copy);

      memcpy(to, curr_chunk + src_offset, (size_t)to_copy);
      src_offset += to_copy;
      to += to_copy;
      length -= to_copy;
    }
  }

  /* Store pointers to blob data in record */
  for (uint i = 0; i < nblobs; i++) {
    uchar *record_off = info->blob_buffer + buf_offsets[i];
    memcpy(record + rec_offsets[i], &record_off, sizeof(uchar *));
  }

  DBUG_RETURN(0);
}
