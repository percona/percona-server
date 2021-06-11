/* Copyright (c) 2016, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef BUFFEREDFILEIO_INCLUDED
#define BUFFEREDFILEIO_INCLUDED

#include <my_global.h>
#include <mysql/plugin.h>
#include "i_keyring_io.h"
#include "keyring.h"
#include "logger.h"
#include "keyring_memory.h"
#include "buffer.h"
#include "hash_to_buffer_serializer.h"
#include "keyring_stat.h"
#include "file_io.h"

namespace keyring {

class Buffered_file_io : public IKeyring_io
{
public:
  Buffered_file_io(ILogger *logger)
    : eofTAG("EOF")
    , file_version("Keyring file version:1.0")
    , logger(logger)
    , backup_exists(FALSE)
    , memory_needed_for_buffer(0)
    , file_io(logger)
    , keyring_file(-1)
  {
  }

  my_bool init(const std::string *keyring_filename);

  my_bool flush_to_backup(ISerialized_object *serialized_object);
  my_bool flush_to_storage(ISerialized_object *serialized_object);

  ISerializer* get_serializer();
  my_bool get_serialized_object(ISerialized_object **serialized_object);
  my_bool has_next_serialized_object();
protected:
  virtual my_bool remove_backup(myf myFlags);
  virtual my_bool read_keyring_stat(File file);
  virtual my_bool check_keyring_file_stat(File file);
private:
  my_bool recreate_keyring_from_backup_if_backup_exists();

  std::string* get_backup_filename();
  my_bool open_backup_file(File *backup_file);
  my_bool load_file_into_buffer(File file, Buffer *buffer);
  my_bool flush_buffer_to_storage(Buffer *buffer, File file);
  my_bool flush_buffer_to_file(Buffer *buffer, File file);
  inline my_bool check_file_structure(File file, size_t file_size);
  my_bool check_if_keyring_file_can_be_opened_or_created();
  my_bool is_file_tag_correct(File file);
  my_bool is_file_version_correct(File file);

  Keyring_stat saved_keyring_stat;
  std::string keyring_filename;
  std::string backup_filename;
  const std::string eofTAG;
  const std::string file_version;
  ILogger *logger;
  my_bool backup_exists;
  Hash_to_buffer_serializer hash_to_buffer_serializer;
  size_t memory_needed_for_buffer;
  File_io file_io;
  File keyring_file;
};

}//namespace keyring

#endif //BUFFEREDFILEIO_INCLUDED
