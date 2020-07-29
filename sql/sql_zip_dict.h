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

#include "lex_string.h"
#include "my_inttypes.h"
#include "sql/dd/types/table.h"  // dd::enum_column_types
#include "sql/mdl.h"

class THD;
struct TABLE_SHARE;

namespace compression_dict {

#ifndef DBUG_OFF
/** Skip creating compression dictionary tables during bootstrap.
This is used simulate upgrade from mysql datadir to percona server
datadir and verify if compression dictionary tables are created
successfully on startup */
extern bool skip_bootstrap;
#endif

/** Create compression dictionary tables
@param[in,out]   thd   Session context
@return false on success, true on failure */
bool bootstrap(THD *thd);

/** During upgrade from 5.7 to 8.0, transfer compression dicitonary
data from 5.7 SYS_ZIP_DICT to 8.0 mysql.compression_dictionary table
@param[in]   thd   Session context
@return false on success, true on failure */
bool upgrade_transfer_compression_dict_data(THD *thd);

/** @return true if the table is a hardcoded compression dictionary table
@param[in] schema     schema name
@param[in] table_name table name */
inline bool is_hardcoded(const dd::String_type &schema,
                         const dd::String_type &table_name) {
  return (schema == "mysql" && (table_name == "compression_dictionary" ||
                                table_name == "compression_dictionary_cols"));
}

/** @return true if the table is a hardcoded compression dictionary table
@param[in]  name  name in "db/table" format */
inline bool is_hardcoded(const char *name) {
  return (strcmp(name, "mysql/compression_dictionary") == 0 ||
          strcmp(name, "mysql/compression_dictionary_cols") == 0);
}

/**
  Creates a new compression dictionary with the specified data.

  @param thd                        thread descriptor.
  @param name                       compression dictionary name
  @param name_len                   compression dictionary name length
  @param data                       compression dictionary data
  @param data_len                   compression dictionary data length
  @param if_not_exists              "IF NOT EXISTS" flag
  @param is_upgrade                 true for upgrade mode, else false

  @return Completion status
  @retval 0                                       Success
  @retval ER_ILLEGAL_HA_CREATE_OPTION             SE does not support
                                                  compression dictionaries
  @retval ER_COMPRESSION_DICTIONARY_NAME_TOO_LONG Dictionary name is too long
  @retval ER_COMPRESSION_DICTIONARY_DATA_TOO_LONG Dictionary data is too long
  @retval ER_COMPRESSION_DICTIONARY_EXISTS        Dictionary with such name
                                                  already exists
  @retval ER_READ_ONLY_MODE                       Forbidden in read-only mode
  @retval ER_UNKNOWN_ERROR                        Unknown error
*/
int create_zip_dict(THD *thd, const char *name, ulong name_len,
                    const char *data, ulong data_len, bool if_not_exists,
                    bool is_upgrade);

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
  @retval ER_UNKNOWN_ERROR                         Unknown error
*/
int drop_zip_dict(THD *thd, const char *name, ulong name_len, bool if_exists);

/* Get dict_id for a given dict_name from mysql.compression_dictionary table
An attachable RO trx is used to do the read
@param[in,out]  thd             Session context
@param[in]      zip_dict_name   dictionary entry name
@return 0 for failure, else dictionary id */
uint64 get_id_for_name(THD *thd, const LEX_CSTRING &zip_dict_name);

/** Get dict_name and dict_data from mysql.compression_dictionary table
for a given dict_id. Memory for dict_name and dict_data is allocated
from table_share mem_root
@param[in,out]     thd            Session context
@param[in]         zip_dict_id    dictionary id
@param[in,out]     share          TABLE_SHARE object
@param[in,out]     zip_dict_name  dictionary name for a given dict_id
@return false on success, true on failure */
bool get_name_for_id(THD *thd, uint64 zip_dict_id, TABLE_SHARE *share,
                     LEX_CSTRING *zip_dict_name, LEX_CSTRING *zip_dict_data);

/** Acquire MDL on mysql.compression_dictionary table
@param[in,out]  thd         Session object
@param[in]      mdl_type    MDL type (like MDL_SHARED_READ etc)
@return true on failure, false on success */
bool acquire_dict_mdl(THD *thd, enum_mdl_type mdl_type);

/** Insert a entry into mysql.compression_dictionary_cols table.
@param[in,out]  thd           Session context
@param[in]	table 	      dictionary table object (dd::Table)
@return false on success, true on failure */
bool cols_table_insert(THD *thd, const dd::Table &table);

/** Delete an entry from mysql.compression_dictionary_cols table
@param[in,out]  thd           Session context
@param[in]	table 	      dictionary table object (dd::Table)
@return false on success, true on failure */
bool cols_table_delete(THD *thd, const dd::Table &table);

}  // namespace compression_dict

#endif /* SQL_ZIP_DICT_INCLUDED */
