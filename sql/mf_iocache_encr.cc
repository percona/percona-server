/*
   Copyright (c) 2018, Percona and/or its affiliates. All rights reserved.
   Copyright (c) 2010, 2015, MariaDB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/*************************************************************************
  Limitation of encrypted IO_CACHEs
  1. Designed to support temporary files only (open_cached_file, fd=-1)
  2. Created with WRITE_CACHE, later can be reinit_io_cache'ed to
     READ_CACHE and WRITE_CACHE in any order arbitrary number of times.
  3. no seeks for writes, but reinit_io_cache(WRITE_CACHE, seek_offset)
     is allowed (there's a special hack in reinit_io_cache() for that)
*/

#include "my_global.h"
#include "my_sys.h"
#include "my_aes.h"
#include "my_rnd.h"
#include "log.h"
#include "mysql/psi/mysql_file.h"

/*
The following three constants must be in sync: 
1. if 'io_cache_aes_mode' is defined as 'my_aes_<nnn>_cbc', then
'io_cache_nonce_aes_mode' must be defined as 'my_aes_<nnn>_ecb' with the same
<nnn>.
2. if 'io_cache_aes_mode' is defined as 'my_aes_<nnn>_cbc', then
'io_cache_aes_key_length_bytes' must be equal to (<nnn> / 8).
*/
#define IO_CACHE_AES_KEY_LENGTH_BITS 256
#define PP_CAT(A, B) PP_CAT_I(A, B)
#define PP_CAT_I(A, B) A ## B

#define IO_CACHE_AES_MODE(BITS, MODE) my_aes_ ## BITS ## MODE
static const enum my_aes_opmode io_cache_aes_mode= 
  PP_CAT(PP_CAT(my_aes_, IO_CACHE_AES_KEY_LENGTH_BITS), _cbc);
static const enum my_aes_opmode io_cache_nonce_aes_mode= 
  PP_CAT(PP_CAT(my_aes_, IO_CACHE_AES_KEY_LENGTH_BITS), _ecb);
static const size_t io_cache_aes_key_length_bytes=
  IO_CACHE_AES_KEY_LENGTH_BITS / 8;

typedef uchar io_cache_aes_key_buffer[io_cache_aes_key_length_bytes];
typedef uchar io_cache_aes_block_buffer[MY_AES_BLOCK_SIZE];

struct io_cache_crypt {
  ulonglong counter;
  uint block_length, last_block_length;
  io_cache_aes_key_buffer key;
  ulonglong inbuf_counter;
};

static void set_iv_from_nonces(io_cache_aes_block_buffer& iv,
                               my_off_t position, ulonglong counter,
                               const io_cache_aes_key_buffer& key)
{
  compile_time_assert(sizeof(iv) == MY_AES_BLOCK_SIZE);
  compile_time_assert(sizeof(iv) >= sizeof(position) + sizeof(counter));

  io_cache_aes_block_buffer nonce_block;
  compile_time_assert(sizeof(nonce_block) == MY_AES_BLOCK_SIZE);

  memset(nonce_block, 0, sizeof(nonce_block));
  memcpy(nonce_block, &position, sizeof(position));
  memcpy(&nonce_block[sizeof(position)], &counter, sizeof(counter));

  int encryption_result MY_ATTRIBUTE((unused))= my_aes_encrypt(nonce_block,
    sizeof(nonce_block), iv, key, sizeof(key), io_cache_nonce_aes_mode,
    NULL, FALSE);
  assert(encryption_result == sizeof(nonce_block));
}

static int my_b_encr_read(IO_CACHE *info, uchar *Buffer, size_t Count)
{
  my_off_t pos_in_file= info->pos_in_file + (info->read_end - info->buffer);
  my_off_t old_pos_in_file= pos_in_file, pos_offset= 0;
  io_cache_crypt *crypt_data= reinterpret_cast<io_cache_crypt *>(
    info->buffer + info->buffer_length + MY_AES_BLOCK_SIZE);
  uchar *wbuffer= reinterpret_cast<uchar*>(&(crypt_data->inbuf_counter));
  uchar *ebuffer= reinterpret_cast<uchar*>(crypt_data + 1);
  DBUG_ENTER("my_b_encr_read");

  if (pos_in_file == info->end_of_file)
  {
    /*  reading past EOF should not empty the cache */
    info->read_pos= info->read_end;
    info->error= 0;
    DBUG_RETURN(MY_TEST(Count));
  }

  if (info->seek_not_done)
  {
    size_t wpos;

    pos_offset= pos_in_file % info->buffer_length;
    pos_in_file-= pos_offset;

    wpos= pos_in_file / info->buffer_length * crypt_data->block_length;

    if (mysql_file_seek(info->file, wpos, MY_SEEK_SET, MYF(0)) ==
        MY_FILEPOS_ERROR)
    {
      info->error= -1;
      DBUG_RETURN(1);
    }
    info->seek_not_done= 0;
  }

  do
  {
    size_t copied;
    uint elength, wlength;
    int length;
    io_cache_aes_block_buffer iv= {0};

    assert(pos_in_file % info->buffer_length == 0);

    if (info->end_of_file - pos_in_file >= info->buffer_length)
      wlength= crypt_data->block_length;
    else
      wlength= crypt_data->last_block_length;

    if (mysql_file_read(info->file, wbuffer, wlength,
                        info->myflags | MY_NABP))
    {
      info->error= -1;
      DBUG_RETURN(1);
    }

    elength= wlength - (ebuffer - wbuffer);
    set_iv_from_nonces(iv, pos_in_file, crypt_data->inbuf_counter,
                       crypt_data->key);

    length= my_aes_decrypt(ebuffer, elength, info->buffer,
      crypt_data->key, sizeof(crypt_data->key),
      io_cache_aes_mode, iv, TRUE);
    if (length == MY_AES_BAD_DATA)
    {
      set_my_errno(1);
      info->error= -1;
      DBUG_RETURN(1);
    }

    assert(static_cast<uint>(length) <= info->buffer_length);

    copied= MY_MIN(Count, length - pos_offset);

    if (copied) {
      assert(Buffer != NULL);
      memcpy(Buffer, info->buffer + pos_offset, copied);
      Count-= copied;
      Buffer+= copied;
    }

    info->read_pos= info->buffer + pos_offset + copied;
    info->read_end= info->buffer + length;
    info->pos_in_file= pos_in_file;
    pos_in_file+= length;
    pos_offset= 0;

    if (wlength < crypt_data->block_length && pos_in_file < info->end_of_file)
    {
      info->error= pos_in_file - old_pos_in_file;
      DBUG_RETURN(1);
    }
  } while (Count);

  DBUG_RETURN(0);
}

static int my_b_encr_write(IO_CACHE *info, const uchar *Buffer, size_t Count)
{
  io_cache_crypt *crypt_data= reinterpret_cast<io_cache_crypt *>(
    info->buffer + info->buffer_length + MY_AES_BLOCK_SIZE);
  uchar *wbuffer= reinterpret_cast<uchar*>(&(crypt_data->inbuf_counter));
  uchar *ebuffer= reinterpret_cast<uchar*>(crypt_data + 1);
  DBUG_ENTER("my_b_encr_write");

  if (Buffer != info->write_buffer)
  {
    Count-= Count % info->buffer_length;
    if (!Count)
      DBUG_RETURN(0);
  }

#ifndef NDEBUG
  if (info->pos_in_file == 0) crypt_data->block_length= 0;
#endif

  if (info->seek_not_done)
  {
    assert(info->pos_in_file % info->buffer_length == 0);
    size_t wpos=
      info->pos_in_file / info->buffer_length * crypt_data->block_length;

    if (mysql_file_seek(info->file, wpos, MY_SEEK_SET, MYF(0)) ==
        MY_FILEPOS_ERROR)
    {
      info->error= -1;
      DBUG_RETURN(1);
    }
    info->seek_not_done= 0;
  }

  if (info->pos_in_file == 0)
  {
    if (my_rand_buffer(crypt_data->key, sizeof(crypt_data->key)))
    {
      set_my_errno(1);
      info->error= -1;
      DBUG_RETURN(1);
    }
    crypt_data->counter= 0;
  }

  do
  {
    size_t length= MY_MIN(info->buffer_length, Count);
    int elength;
    uint wlength;
    io_cache_aes_block_buffer iv= {0};

    crypt_data->inbuf_counter= crypt_data->counter;
    set_iv_from_nonces(iv, info->pos_in_file, crypt_data->inbuf_counter,
                       crypt_data->key);

    elength= my_aes_encrypt(Buffer, length, ebuffer,
      crypt_data->key, sizeof(crypt_data->key),
      io_cache_aes_mode, iv, TRUE);
    if (elength == MY_AES_BAD_DATA)
    {
      set_my_errno(1);
      info->error= -1;
      DBUG_RETURN(1);
    }
    wlength= elength + ebuffer - wbuffer;

    if (length == info->buffer_length)
    {
      /*
        block_length should be always the same. that is, encrypting
        buffer_length bytes should *always* produce block_length bytes
      */
      assert(crypt_data->block_length == 0 ||
                  crypt_data->block_length == wlength);
      assert(static_cast<uint>(elength) <= length + MY_AES_BLOCK_SIZE);
      crypt_data->block_length= wlength;
    }
    else
    {
      /* if we write a partial block, it *must* be the last write */
#ifndef NDEBUG
      info->write_function= 0;
#endif
      crypt_data->last_block_length= wlength;
    }

    if (mysql_file_write(info->file, wbuffer, wlength,
        info->myflags | MY_NABP))
    {
      info->error= -1;
      DBUG_RETURN(1);
    }

    Buffer+= length;
    Count-= length;
    info->pos_in_file+= length;
    crypt_data->counter++;
  } while (Count);
  DBUG_RETURN(0);
}

/**
  Initialize IO_CACHE encryption subsystem
*/
void init_io_cache_encryption(my_bool enable)
{
  if (enable)
  {
    sql_print_information("Using encryption for temporary files");
    init_io_cache_encryption_ext(&my_b_encr_read, &my_b_encr_write,
                                 MY_AES_BLOCK_SIZE, sizeof(io_cache_crypt));
  }
  else
  {
    init_io_cache_encryption_ext(NULL, NULL, 0, 0);
  }
}
