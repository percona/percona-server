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

#include <my_global.h>
#include "keys_container.h"
#include "system_keys_container.h"

namespace keyring {

extern PSI_memory_key key_memory_KEYRING;

uchar *get_hash_key(const uchar *key, size_t *length,
                    my_bool not_used MY_ATTRIBUTE((unused)))
{
  std::string *key_signature= reinterpret_cast<const IKey *>(key)->get_key_signature();
  *length= key_signature->length();
  return reinterpret_cast<uchar *>(const_cast<char*>(key_signature->c_str()));
}


void free_hash_key(void* key)
{
  IKey *key_to_free= reinterpret_cast<IKey*>(key);
  delete key_to_free;
}

Keys_container::Keys_container(ILogger *logger)
 : keys_hash(new HASH)
 , logger(logger)
 , keyring_io(NULL)
{
  my_hash_clear(keys_hash);
  system_keys_container.reset(new System_keys_container(logger));
}

Keys_container::~Keys_container()
{
  free_keys_hash();
  delete keys_hash;
  if (keyring_io != NULL)
    delete keyring_io;
}

my_bool Keys_container::init(IKeyring_io* keyring_io, std::string keyring_storage_url)
{
  this->keyring_io= keyring_io;
  this->keyring_storage_url= keyring_storage_url;
  if (my_hash_init(keys_hash, system_charset_info, 0x100, 0, 0,
                   (my_hash_get_key) get_hash_key, free_hash_key, HASH_UNIQUE,
                   key_memory_KEYRING) ||
      keyring_io->init(&this->keyring_storage_url) ||
      load_keys_from_keyring_storage())
  {
    free_keys_hash();
    return TRUE;
  }
  return FALSE;
}

std::string Keys_container::get_keyring_storage_url()
{
  return keyring_storage_url;
}

void Keys_container::store_keys_metadata(IKey *key)
{
  /* if key metadata not present store it */
  Key_metadata km(key->get_key_id(), key->get_user_id());
  keys_metadata.push_back(km);
}

my_bool Keys_container::store_key_in_hash(IKey *key)
{
  if (my_hash_insert(keys_hash, (uchar *) key))
    return TRUE;
  store_keys_metadata(key);
  return FALSE;
}

my_bool Keys_container::store_key(IKey* key)
{
  if (system_keys_container->rotate_key_id_if_system_key_without_version(key) ||
      flush_to_backup() ||
      store_key_in_hash(key))
    return TRUE;
  if (flush_to_storage(key, STORE_KEY))
  {
    remove_key_from_hash(key);
    return TRUE;
  }
  system_keys_container->store_or_update_if_system_key_with_version(key);
  return FALSE;
}

IKey* Keys_container::get_key_from_hash(IKey *key)
{
  IKey* system_key = system_keys_container->get_latest_key_if_system_key_without_version(key);

  return system_key ? system_key
    : reinterpret_cast<IKey*>(my_hash_search(keys_hash,
        reinterpret_cast<const uchar*>(key->get_key_signature()->c_str()),
                                       key->get_key_signature()->length()));
}

void Keys_container::allocate_and_set_data_for_key(IKey *key,
                                                   std::string *source_key_type,
                                                   uchar *source_key_data,
                                                   size_t source_key_data_size)
{
  key->set_key_type(source_key_type);
  uchar *key_data= keyring_malloc<uchar*>(source_key_data_size);
  memcpy(key_data, source_key_data, source_key_data_size);
  key->set_key_data(key_data, source_key_data_size);
}

IKey*Keys_container::fetch_key(IKey *key)
{
  assert(key->get_key_data() == NULL);
  assert(key->get_key_type()->empty());

  IKey *fetched_key= get_key_from_hash(key);

  if (fetched_key == NULL)
    return NULL;

  if (fetched_key->get_key_type()->empty())
    return NULL;

  allocate_and_set_data_for_key(key, fetched_key->get_key_type(),
                                fetched_key->get_key_data(),
                                fetched_key->get_key_data_size());
  return key;
}

bool Keys_container::remove_keys_metadata(IKey *key)
{
  Key_metadata src(key->get_key_id(), key->get_user_id());
  std::vector<Key_metadata>::iterator it= keys_metadata.begin();
  while(it != keys_metadata.end())
  {
    if (src.id == it->id && src.user == it->user)
    {
      keys_metadata.erase(it);
      return false;
    }
    ++it;
  }
  return true;
}

my_bool Keys_container::remove_key_from_hash(IKey *key)
{
  my_bool retVal= TRUE;
  keys_hash->free= NULL; //Prevent my_hash_delete from removing key from memory
  retVal= my_hash_delete(keys_hash, reinterpret_cast<uchar*>(key));
  keys_hash->free= free_hash_key;
  remove_keys_metadata(key);
  return retVal;
}

my_bool Keys_container::remove_key(IKey *key)
{
  IKey* fetched_key_to_delete= get_key_from_hash(key);
  // removing system keys is forbidden
  if (fetched_key_to_delete == NULL ||
      system_keys_container->is_system_key(fetched_key_to_delete) ||
      flush_to_backup() ||
      remove_key_from_hash(fetched_key_to_delete))
    return TRUE;
  if (flush_to_storage(fetched_key_to_delete, REMOVE_KEY))
  {
    //reinsert the key
    store_key_in_hash(fetched_key_to_delete);
    return TRUE;
  }
  //successfully removed the key from hash and flushed to disk, safely remove
  //the key
  delete fetched_key_to_delete;

  return FALSE;
}

void Keys_container::free_keys_hash()
{
  if (my_hash_inited(keys_hash))
    my_hash_free(keys_hash);
}

my_bool Keys_container::load_keys_from_keyring_storage()
{
  my_bool was_error= FALSE;
  ISerialized_object *serialized_keys= NULL;
  was_error= keyring_io->get_serialized_object(&serialized_keys);
  while(was_error == FALSE && serialized_keys != NULL)
  {
    IKey *key_loaded= NULL;
    while(serialized_keys->has_next_key())
    {
      if (serialized_keys->get_next_key(&key_loaded) || key_loaded == NULL ||
          key_loaded->is_key_valid() == FALSE || store_key_in_hash(key_loaded))
      {
        was_error=TRUE;
        delete key_loaded;
        break;
      }
      system_keys_container->store_or_update_if_system_key_with_version(key_loaded);
      key_loaded=NULL;
    }
    delete serialized_keys;
    serialized_keys= NULL;
    if (was_error == FALSE && keyring_io->has_next_serialized_object())
      was_error= keyring_io->get_serialized_object(&serialized_keys);
  }
  if(was_error)
    logger->log(MY_ERROR_LEVEL, "Error while loading keyring content. "
                                "The keyring might be malformed");
  return was_error;
}

my_bool Keys_container::flush_to_storage(IKey *key, Key_operation operation)
{
  ISerialized_object *serialized_object=
    keyring_io->get_serializer()->serialize(keys_hash, key, operation);

  if (serialized_object == NULL || keyring_io->flush_to_storage(serialized_object))
  {
    logger->log(MY_ERROR_LEVEL, "Could not flush keys to keyring");
    delete serialized_object;
    return TRUE;
  }
  delete serialized_object;
  return FALSE;
}

my_bool Keys_container::flush_to_backup()
{
  ISerialized_object *serialized_object=
    keyring_io->get_serializer()->serialize(keys_hash, NULL, NONE);

  if (serialized_object == NULL || keyring_io->flush_to_backup(serialized_object))
  {
    logger->log(MY_ERROR_LEVEL, "Could not flush keys to keyring's backup");
    delete serialized_object;
    return TRUE;
  }
  delete serialized_object;
  return FALSE;
}

} //namespace keyring
