/*****************************************************************************

Copyright (c) 2016, Percona Inc. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

/* create and drop zip_dicts */

#include "sql_zip_dict.h"
#include "sql_table.h"                          // write_bin_log
#include "sql_class.h"                          // THD

/**
Creates a new compression dictionary with the specified data.

@param thd                        thread descriptor.
@param name                       compression dictionary name
@param name_len                   compression dictionary name length
@param data                       compression dictionary data
@param data_len                   compression dictionary data length
@param if_not_exists              "IF NOT EXISTS" flag

@return Completion status
@retval 0                                       Success
@retval ER_ILLEGAL_HA_CREATE_OPTION             SE does not support
                                                compression dictionaries
@retval ER_COMPRESSION_DICTIONARY_NAME_TOO_LONG Dictionary name is too long
@retval ER_COMPRESSION_DICTIONARY_DATA_TOO_LONG Dictionary data is too long
@retval ER_COMPRESSION_DICTIONARY_EXISTS        Dictionary with such name
                                                already exists
@retval ER_READ_ONLY_MODE                       Forbidden in read-only mode
@retval ER_OUT_OF_RESOURCES                     Out of memory
@retval ER_RECORD_FILE_FULL                     Out of disk space
@retval ER_TOO_MANY_CONCURRENT_TRXS             Too many concurrent
                                                transactions
@retval ER_UNKNOWN_ERROR                        Unknown error
*/
int mysql_create_zip_dict(THD* thd, const char* name, ulong name_len,
  const char* data, ulong data_len, bool if_not_exists)
{
  int error= 0;

  DBUG_ENTER("mysql_create_zip_dict");
  handlerton *hton= ha_default_handlerton(thd);

  if (!hton->create_zip_dict)
  {
    error= ER_ILLEGAL_HA_CREATE_OPTION;
    my_error(error, MYF(0),
      ha_resolve_storage_engine_name(hton), "COMPRESSED COLUMNS");
    DBUG_RETURN(error);
  }

  ulong local_name_len= name_len;
  ulong local_data_len= data_len;
  handler_create_zip_dict_result create_result=
    hton->create_zip_dict(hton, thd, name, &local_name_len,
                          data, &local_data_len);
  if (create_result != HA_CREATE_ZIP_DICT_OK)
  {
    switch (create_result)
    {
      case HA_CREATE_ZIP_DICT_NAME_TOO_LONG:
        error= ER_COMPRESSION_DICTIONARY_NAME_TOO_LONG;
        my_error(error, MYF(0), name, local_name_len);
        break;
      case HA_CREATE_ZIP_DICT_DATA_TOO_LONG:
        error= ER_COMPRESSION_DICTIONARY_DATA_TOO_LONG;
        my_error(error, MYF(0), name, local_data_len);
        break;
      case HA_CREATE_ZIP_DICT_ALREADY_EXISTS:
        if (if_not_exists)
        {
          push_warning_printf(thd, Sql_condition::WARN_LEVEL_NOTE,
                              ER_COMPRESSION_DICTIONARY_EXISTS,
                              ER(ER_COMPRESSION_DICTIONARY_EXISTS),
                              name);
        }
        else
        {
          error= ER_COMPRESSION_DICTIONARY_EXISTS;
          my_error(error, MYF(0), name);
        }
        break;
      case HA_CREATE_ZIP_DICT_READ_ONLY:
        error= ER_READ_ONLY_MODE;
        my_error(error, MYF(0));
        break;
      case HA_CREATE_ZIP_DICT_OUT_OF_MEMORY:
        error= ER_OUT_OF_RESOURCES;
        my_error(error, MYF(0));
        break;
      case HA_CREATE_ZIP_DICT_OUT_OF_FILE_SPACE:
        error= ER_RECORD_FILE_FULL;
        my_error(error, MYF(0), "SYS_ZIP_DICT");
        break;
      case HA_CREATE_ZIP_DICT_TOO_MANY_CONCURRENT_TRXS:
        error= ER_TOO_MANY_CONCURRENT_TRXS;
        my_error(error, MYF(0));
        break;
      default:
        DBUG_ASSERT(0);
        error= ER_UNKNOWN_ERROR;
        my_error(error, MYF(0));
    }
  }

  if (error == 0)
    error= write_bin_log(thd, FALSE, thd->query(), thd->query_length());
  DBUG_RETURN(error);
}

/**
Deletes a compression dictionary.

@param thd                        thread descriptor.
@param name                       compression dictionary name
@param name_len                   compression dictionary name length
@param if_exists                  "IF EXISTS" flag

@return Completion status
@retval 0                                        Success
@retval ER_ILLEGAL_HA_CREATE_OPTION              SE does not support
                                                 compression dictionaries
@retval ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST Dictionary with such name
                                                 does not exist
@retval ER_COMPRESSION_DICTIONARY_IS_REFERENCED  Dictictionary is still in
                                                 use
@retval ER_READ_ONLY_MODE                        Forbidden in read-only mode
@retval ER_OUT_OF_RESOURCES                      Out of memory
@retval ER_RECORD_FILE_FULL                      Out of disk space
@retval ER_TOO_MANY_CONCURRENT_TRXS              Too many concurrent
                                                 transactions
@retval ER_UNKNOWN_ERROR                         Unknown error
*/
int mysql_drop_zip_dict(THD* thd, const char* name, ulong name_len,
  bool if_exists)
{
  int error= 0;

  DBUG_ENTER("mysql_drop_zip_dict");
  handlerton *hton= ha_default_handlerton(thd);

  if (!hton->drop_zip_dict)
  {
    error= ER_ILLEGAL_HA_CREATE_OPTION;
    my_error(error, MYF(0),
      ha_resolve_storage_engine_name(hton), "COMPRESSED COLUMNS");
    DBUG_RETURN(error);
  }

  ulong local_name_len= name_len;
  handler_drop_zip_dict_result drop_result=
    hton->drop_zip_dict(hton, thd, name, &local_name_len);
  if (drop_result != HA_DROP_ZIP_DICT_OK)
  {
    switch (drop_result)
    {
      case HA_DROP_ZIP_DICT_DOES_NOT_EXIST:
        if (if_exists)
        {
          push_warning_printf(thd, Sql_condition::WARN_LEVEL_NOTE,
                              ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST,
                              ER(ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST),
                              name);
        }
        else
        {
          error= ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST;
          my_error(error, MYF(0), name);
        }
        break;
      case HA_DROP_ZIP_DICT_IS_REFERENCED:
        error= ER_COMPRESSION_DICTIONARY_IS_REFERENCED;
        my_error(error, MYF(0), name);
        break;
      case HA_DROP_ZIP_DICT_READ_ONLY:
        error= ER_READ_ONLY_MODE;
        my_error(error, MYF(0));
        break;
      case HA_DROP_ZIP_DICT_OUT_OF_MEMORY:
        error= ER_OUT_OF_RESOURCES;
        my_error(error, MYF(0));
        break;
      case HA_DROP_ZIP_DICT_OUT_OF_FILE_SPACE:
        error= ER_RECORD_FILE_FULL;
        my_error(error, MYF(0), "SYS_ZIP_DICT");
        break;
      case HA_DROP_ZIP_DICT_TOO_MANY_CONCURRENT_TRXS:
        error= ER_TOO_MANY_CONCURRENT_TRXS;
        my_error(error, MYF(0));
        break;
      default:
        DBUG_ASSERT(0);
        error= ER_UNKNOWN_ERROR;
        my_error(error, MYF(0));
    }
  }

  if (error == 0)
    error= write_bin_log(thd, FALSE, thd->query(), thd->query_length());
  DBUG_RETURN(error);
}
