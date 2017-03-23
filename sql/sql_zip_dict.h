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

#ifndef SQL_ZIP_DICT_INCLUDED
#define SQL_ZIP_DICT_INCLUDED

#include "my_global.h"

class THD;

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
  @retval ER_ILLEGAL_HA                           Forbidden when fake
                                                  changes enabled
  @retval ER_UNKNOWN_ERROR                        Unknown error
*/
int mysql_create_zip_dict(THD* thd, const char* name, ulong name_len,
  const char* data, ulong data_len, bool if_not_exists);

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
  @retval ER_READ_ONLY_MODE                        Forbidden in read-only
                                                   mode
  @retval ER_ILLEGAL_HA                            Forbidden when fake
                                                   changes enabled
  @retval ER_UNKNOWN_ERROR                         Unknown error
*/
int mysql_drop_zip_dict(THD* thd, const char* name, ulong name_len,
  bool if_exists);

#endif /* SQL_ZIP_DICT_INCLUDED */
