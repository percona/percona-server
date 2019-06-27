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

#include "sql/sql_zip_dict.h"

#include <iostream>
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // DD_bootstrap_ctx
#include "sql/dd/impl/bootstrap/bootstrapper.h"
#include "sql/dd/impl/transaction_impl.h"
#include "sql/dd/impl/utils.h"                 // execute_query
#include "sql/dd/types/column.h"               // dd::enum_column_types
#include "sql/dd/types/column_type_element.h"  // dd::Column_type_element
#include "sql/dd/types/table.h"                // dd::enum_column_types
#include "sql/derror.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_table.h"  // write_bin_log
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"

namespace compression_dict {

/** Fields of mysql.compression_dictionary table */
static const constexpr uint32_t COMPRESSION_DICTIONARY_FIELD_ID = 0;
static const constexpr uint32_t COMPRESSION_DICTIONARY_FIELD_VERSION = 1;
static const constexpr uint32_t COMPRESSION_DICTIONARY_FIELD_NAME = 2;
static const constexpr uint32_t COMPRESSION_DICTIONARY_FIELD_DATA = 3;

/** Index of compression_dictionary table */
static const constexpr uint32_t COMPRESSION_DICTIONARY_NAME_INDEX = 1;
static const constexpr uint32_t COMPRESSION_DICTIONARY_VERSION = 1;

static const constexpr char COMPRESSION_DICTIONARY_DB[] = "mysql";
static const constexpr char COMPRESSION_DICTIONARY_TABLE[] =
    "compression_dictionary";

/** Fields of mysql.compression_dictionary_cols table */
static const constexpr uint32_t COMPRESSION_DICTIONARY_COLS_FIELD_TABLE_ID = 0;
static const constexpr uint32_t COMPRESSION_DICTIONARY_COLS_FIELD_COLUMN_ID = 1;
static const constexpr uint32_t COMPRESSION_DICTIONARY_COLS_FIELD_DICT_ID = 2;

/** Index of compression_dictionary_cols table */
static const constexpr uint32_t COMPRESSION_DICTIONARY_COLS_TABLE_INDEX = 0;

static const constexpr char COMPRESSION_DICTIONARY_COLS_DB[] = "mysql";
static const constexpr char COMPRESSION_DICTIONARY_COLS_TABLE[] =
    "compression_dictionary_cols";

/** Max window size (2^15) minus 262 */
static const constexpr uint32_t MAX_DATA_LENGTH = 32506;

/** Schema definition of mysql.compression_dictionary */
static const dd::String_type dict_table_str =
    "CREATE TABLE IF NOT EXISTS mysql.compression_dictionary(id "
    "BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, version "
    "INT UNSIGNED NOT NULL, name VARCHAR(64) NOT NULL, data BLOB NOT "
    "NULL, UNIQUE KEY name_idx(name)) ENGINE=InnoDB CHARACTER "
    "SET=utf8mb4 COLLATE=utf8mb4_general_ci STATS_PERSISTENT=0 "
    "TABLESPACE=mysql";

/** Schema definition of mysql.compression_dictionary_cols */
static const dd::String_type dict_cols_table_str =
    "CREATE TABLE IF NOT EXISTS "
    "mysql.compression_dictionary_cols(table_id BIGINT UNSIGNED "
    "NOT NULL, column_id BIGINT UNSIGNED NOT NULL, dict_id "
    "BIGINT UNSIGNED NOT NULL, FOREIGN KEY (dict_id) REFERENCES "
    "compression_dictionary(id) ON DELETE RESTRICT ON UPDATE "
    "RESTRICT, UNIQUE KEY table_idx(table_id, column_id)) "
    "ENGINE=InnoDB CHARACTER SET=utf8mb4 COLLATE=utf8mb4_general_ci "
    "STATS_PERSISTENT=0 TABLESPACE=mysql";

#ifndef DBUG_OFF
/** Skip creating compression dictionary tables during bootstrap.
This is used simulate upgrade from mysql datadir to percona server
datadir and verify if compression dictionary tables are created
successfully on startup */
bool skip_bootstrap = false;
#endif

/** Create compression dictionary tables
@param[in,out]   thd   Session context
@return false on success, true on failure */
bool bootstrap(THD *thd) {
  DBUG_EXECUTE_IF("skip_compression_dict_create", skip_bootstrap = true;
                  return false;);

  dd::String_type dict_table_str_enc{dict_table_str};

  if (dd::bootstrap::DD_bootstrap_ctx::instance().is_dd_encrypted()) {
    dict_table_str_enc += " ENCRYPTION='Y'";
  }

  // Create mysql.compression_dictionary table
  if (execute_query(thd, dict_table_str_enc)) {
    return true;
  }

  dd::String_type dict_cols_table_str_enc{dict_cols_table_str};

  if (dd::bootstrap::DD_bootstrap_ctx::instance().is_dd_encrypted()) {
    dict_cols_table_str_enc += " ENCRYPTION='Y'";
  }

  // Create mysql.compression_dictionary_cols table
  if (execute_query(thd, dict_cols_table_str_enc)) {
    return true;
  }

  return false;
}

/** During upgrade from 5.7 to 8.0, transfer compression dicitonary
data from 5.7 SYS_ZIP_DICT to 8.0 mysql.compression_dictionary table
@param[in]   thd   Session context
@return false on success, true on failure */
bool upgrade_transfer_compression_dict_data(THD *thd) {
  compression_dict_data_vec_t zip_dict_vec;
  handlerton *hton = ha_resolve_by_legacy_type(thd, DB_TYPE_INNODB);
  hton->upgrade_get_compression_dict_data(thd, zip_dict_vec);

  for (const auto &elem : zip_dict_vec) {
    const auto &name = elem.first;
    const auto &data = elem.second;

    int ret = create_zip_dict(thd, name.c_str(), name.length(), data.c_str(),
                              data.length(), false, true);

    if (ret != 0) {
      return (true);
    }

    DBUG_LOG("zip_dict", "Compression dictionary Name is: "
                             << name << " Data is: " << data);
  }
  return (false);
}

/** Acquire MDL on mysql.compression_dictionary table
@param[in,out]  thd         Session object
@param[in]      mdl_type    MDL type (like MDL_SHARED_READ etc)
@return true on failure, false on success */
bool acquire_dict_mdl(THD *thd, enum_mdl_type mdl_type) {
  MDL_request *mdl_request = new (thd->mem_root) MDL_request;
  MDL_REQUEST_INIT(mdl_request, MDL_key::TABLE, COMPRESSION_DICTIONARY_DB,
                   COMPRESSION_DICTIONARY_TABLE, mdl_type, MDL_TRANSACTION);
  /*
    Acquire the lock request created above, and check if
    acquisition fails (e.g. timeout or deadlock).
  */
  if (thd->mdl_context.acquire_lock(mdl_request,
                                    thd->variables.lock_wait_timeout)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    my_error(ER_LOCK_WAIT_TIMEOUT, MYF(0));
    DBUG_LOG("zip_dict",
             "MDL acquisition on compression_dictionary table failed "
             " for query: "
                 << thd->query().str << " MDL_request: " << mdl_type
                 << " thd->err: " << thd->get_stmt_da()->mysql_errno()
                 << " thd->err_str: " << thd->get_stmt_da()->message_text());
    return (true);
  }

  return (false);
}

/** Open mysql.compression_dictionary table for modification (insert, update,
delete)
@param[in,out] thd  Session object
@return TABLE* on success else nullptr */
static TABLE *open_dictionary_table_write(THD *thd) {
  TABLE_LIST tablelist(C_STRING_WITH_LEN(COMPRESSION_DICTIONARY_DB),
                       C_STRING_WITH_LEN(COMPRESSION_DICTIONARY_TABLE),
                       COMPRESSION_DICTIONARY_TABLE, TL_WRITE,
                       MDL_SHARED_NO_READ_WRITE);
  tablelist.next_local = tablelist.next_global = nullptr;

  const uint flags = (MYSQL_LOCK_IGNORE_TIMEOUT | MYSQL_OPEN_IGNORE_KILLED |
                      MYSQL_OPEN_IGNORE_FLUSH);

  if (open_and_lock_tables(thd, &tablelist, flags)) {
    DBUG_LOG("zip_dict",
             "open_and_lock_tables() on compression_dictionary table in write"
             " mode failed for query: "
                 << thd->query().str
                 << " thd->err: " << thd->get_stmt_da()->mysql_errno()
                 << " thd->err_str: " << thd->get_stmt_da()->message_text());
    return (nullptr);
  }

  return (tablelist.table);
}

/** Open mysql.compression_dictionary table for only reads
delete). An RO attachable transaction must be used. RO attachable trx
uses Read Committed Isolation and does non-locking reads
@param[in,out] thd  Session object
@return TABLE* on success else nullptr */
static TABLE *open_dictionary_table_read(THD *thd) {
  DBUG_ASSERT(!thd->is_attachable_ro_transaction_active());
  thd->begin_attachable_ro_transaction();

  TABLE_LIST tablelist(C_STRING_WITH_LEN(COMPRESSION_DICTIONARY_DB),
                       C_STRING_WITH_LEN(COMPRESSION_DICTIONARY_TABLE),
                       COMPRESSION_DICTIONARY_TABLE, TL_READ);
  tablelist.next_local = tablelist.next_global = nullptr;

  uint flags = (MYSQL_LOCK_IGNORE_TIMEOUT | MYSQL_OPEN_IGNORE_KILLED |
                MYSQL_OPEN_IGNORE_FLUSH);

  uint counter;
  TABLE_LIST *table_list = &tablelist;
  if (::open_tables(thd, &table_list, &counter, flags)) {
    DBUG_LOG("zip_dict",
             "open_tables() on compression_dictionary table in read"
             " mode failed for query: "
                 << thd->query().str
                 << " thd->err: " << thd->get_stmt_da()->mysql_errno()
                 << " thd->err_str: " << thd->get_stmt_da()->message_text());
    return (nullptr);
  }

  if (lock_tables(thd, table_list, counter, flags)) {
    DBUG_LOG("zip_dict",
             "lock_tables() on compression_dictionary table in read"
             " mode failed for query: "
                 << thd->query().str
                 << " thd->err: " << thd->get_stmt_da()->mysql_errno()
                 << " thd->err_str: " << thd->get_stmt_da()->message_text());

    return (nullptr);
  }

  return (tablelist.table);
}

/** Close mysql.compression_dictionary table and end attachable trx which
restores saved session state
@param[in,out]  thd  Session object */
static void close_dictionary_table_read(THD *thd) {
  thd->end_attachable_transaction();
}

/** Open mysql.compression_dictionary_cols table for writes. An entry is
inserted/deleted into this table at the end of successful DDL. The main
objective for writing to this table is to prevent dropping of compression
dictionary when tables are using a compression dictionary.
mysql.compression_dictionary_cols table has an FK relationship with the parent
table mysql.compression_dictionary
@param[in,out] thd Session object
@return on success, a TABLE* object else nullptr on failure */

static TABLE *open_dictionary_cols_table_write(THD *thd) {
  TABLE_LIST tablelist(C_STRING_WITH_LEN(COMPRESSION_DICTIONARY_COLS_DB),
                       C_STRING_WITH_LEN(COMPRESSION_DICTIONARY_COLS_TABLE),
                       COMPRESSION_DICTIONARY_COLS_TABLE,
                       TL_WRITE_CONCURRENT_DEFAULT, MDL_SHARED_WRITE);
  tablelist.next_local = tablelist.next_global = nullptr;

  const uint flags = (MYSQL_LOCK_IGNORE_TIMEOUT | MYSQL_OPEN_IGNORE_KILLED |
                      MYSQL_OPEN_IGNORE_FLUSH);

  if (open_and_lock_tables(thd, &tablelist, flags)) {
    DBUG_LOG(
        "zip_dict",
        "open_and_lock_tables() on compression_dictionary_cols table in write"
        " mode failed for query: "
            << thd->query().str
            << " thd->err: " << thd->get_stmt_da()->mysql_errno()
            << " thd->err_str: " << thd->get_stmt_da()->message_text());

    return (nullptr);
  }

  return (tablelist.table);
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
@retval ER_COMPRESSION_DICTIONARY_EXISTS        Dictionary with such name
                                                already exists
@retval ER_READ_ONLY_MODE                       Forbidden in read-only mode
@retval ER_OUT_OF_RESOURCES                     Out of memory
@retval ER_RECORD_FILE_FULL                     Out of disk space
@retval ER_TOO_MANY_CONCURRENT_TRXS             Too many concurrent
                                                transactions
@retval ER_UNKNOWN_ERROR                        Unknown error
*/
int create_zip_dict(THD *thd, const char *name, ulong name_len,
                    const char *data, ulong data_len, bool if_not_exists,
                    bool is_upgrade) {
  DBUG_ENTER("mysql_create_zip_dict");
  handlerton *hton = ha_default_handlerton(thd);

#ifndef DBUG_OFF
  const std::string name_str(name, name_len);
  const std::string data_str(data, data_len);
  const std::string query_str(to_string(thd->query()));
  DBUG_LOG("zip_dict", "thd->query: " << query_str << " dict_name: " << name_str
                                      << " dict_name_len: " << name_len
                                      << " data: " << data_str
                                      << " data_len: " << data_len
                                      << " if_not_exists: " << if_not_exists);
#endif
  int error;

  /* thd is from bootstrap thread and the default storage engine
  is not yet set. So don't do the hton storage engine flag check */
  if (!is_upgrade &&
      !ha_check_storage_engine_flag(hton, HTON_SUPPORTS_COMPRESSED_COLUMNS)) {
    error = ER_ILLEGAL_HA_CREATE_OPTION;
    my_error(error, MYF(0), ha_resolve_storage_engine_name(hton),
             "COMPRESSED COLUMNS");
    DBUG_RETURN(error);
  }

  if (data_len > MAX_DATA_LENGTH) {
    error = ER_COMPRESSION_DICTIONARY_DATA_TOO_LONG;
    my_error(error, MYF(0), name, MAX_DATA_LENGTH);
    DBUG_RETURN(error);
  }

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  TABLE *table = open_dictionary_table_write(thd);

  if (table == nullptr) {
    DBUG_ASSERT(thd->is_error());
    DBUG_ASSERT(thd->get_stmt_da()->mysql_errno() != 0);

    if (thd->get_stmt_da()->mysql_errno() == ER_CANT_LOCK) {
      error = ER_READ_ONLY_MODE;
      thd->clear_error();
      my_error(error, MYF(0));
    } else {
      error = thd->get_stmt_da()->mysql_errno();
    }

    DBUG_LOG("zip_dict",
             "thd->query: " << thd->query().str
                            << "Unable to open"
                               "  mysql.compression_dictionary table "
                            << " error: " << error << " error_msg: "
                            << thd->get_stmt_da()->message_text());

    DBUG_RETURN(error);
  }

  table->use_all_columns();

  sql_mode_t saved_mode = thd->variables.sql_mode;
  thd->variables.sql_mode = 0;

  TABLE_SHARE *ts = table->s;
  if (table->next_number_field == nullptr && ts->found_next_number_field) {
    table->next_number_field = table->found_next_number_field =
        table->field[(uint)(ts->found_next_number_field - ts->field)];
  }

  table->next_number_field->set_null();
  table->auto_increment_field_not_null = true;
  table->record[0][0] = ts->default_values[0];
  table->file->ha_start_bulk_insert(1);  // 1 is the estimated rows to insert

  table->field[COMPRESSION_DICTIONARY_FIELD_ID]->set_null();

  type_conversion_status rc =
      table->field[COMPRESSION_DICTIONARY_FIELD_NAME]->store(
          name, name_len, system_charset_info);

  if (rc != TYPE_OK) {
    error = ER_COMPRESSION_DICTIONARY_NAME_TOO_LONG;
    my_error(error, MYF(0), name, 64);
    table->file->ha_release_auto_increment();
    table->file->ha_end_bulk_insert();
    table->auto_increment_field_not_null = false;
    close_thread_tables(thd);
    thd->mdl_context.release_transactional_locks();
    DBUG_RETURN(error);
  }

  table->field[COMPRESSION_DICTIONARY_FIELD_VERSION]->store(
      COMPRESSION_DICTIONARY_VERSION, true);
  table->field[COMPRESSION_DICTIONARY_FIELD_DATA]->store(data, data_len,
                                                         system_charset_info);
  error = table->file->ha_write_row(table->record[0]);

  thd->variables.sql_mode = saved_mode;

  if (error != 0) {
    switch (error) {
      case HA_ERR_FOUND_DUPP_KEY:
        if (if_not_exists) {
          push_warning_printf(
              thd, Sql_condition::SL_NOTE, ER_COMPRESSION_DICTIONARY_EXISTS,
              ER_THD(thd, ER_COMPRESSION_DICTIONARY_EXISTS), name);
          /* Claim success */
          error = 0;
        } else {
          error = ER_COMPRESSION_DICTIONARY_EXISTS;
          my_error(error, MYF(0), name);
        }
        break;
      case HA_ERR_RECORD_FILE_FULL:
        error = ER_RECORD_FILE_FULL;
        my_error(error, MYF(0), COMPRESSION_DICTIONARY_TABLE);
        break;
      case HA_ERR_TOO_MANY_CONCURRENT_TRXS:
        error = ER_TOO_MANY_CONCURRENT_TRXS;
        my_error(error, MYF(0));
        break;
      case HA_ERR_TABLE_READONLY:
        error = ER_OPEN_AS_READONLY;
        my_error(error, MYF(0), COMPRESSION_DICTIONARY_TABLE);
        break;
      case HA_ERR_INNODB_FORCED_RECOVERY:
        error = ER_INNODB_FORCED_RECOVERY;
        my_error(error, MYF(0));
        break;
      default:
        DBUG_ASSERT(0);
        error = ER_UNKNOWN_ERROR;
        my_error(error, MYF(0));
    }
  }

  bool failure =
      error != 0 || thd->is_error() || thd->transaction_rollback_request;

  if (!failure && write_bin_log(thd, false, thd->query().str,
                                thd->query().length, true) != 0) {
    failure = true;
    error = thd->get_stmt_da()->mysql_errno();
    DBUG_ASSERT(thd->is_error());
  }

  if (failure) {
    DBUG_ASSERT(error != 0);
    trans_rollback_stmt(thd);
    trans_rollback_implicit(thd);
  } else {
    trans_commit_stmt(thd);
    trans_commit_implicit(thd);
  }

  table->file->ha_release_auto_increment();
  table->file->ha_end_bulk_insert();
  table->auto_increment_field_not_null = false;

  close_thread_tables(thd);
  thd->mdl_context.release_transactional_locks();
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
int drop_zip_dict(THD *thd, const char *name, ulong name_len, bool if_exists) {
  DBUG_ENTER("mysql_drop_zip_dict");
  handlerton *hton = ha_default_handlerton(thd);

#ifndef DBUG_OFF
  const std::string name_str(name, name_len);
  DBUG_LOG("zip_dict", "thd->query: " << thd->query().str
                                      << " dict_name: " << name_str
                                      << " dict_name_len: " << name_len
                                      << " if_exists: " << if_exists);
#endif

  int error;
  if (!ha_check_storage_engine_flag(hton, HTON_SUPPORTS_COMPRESSED_COLUMNS)) {
    error = ER_ILLEGAL_HA_CREATE_OPTION;
    my_error(error, MYF(0), ha_resolve_storage_engine_name(hton),
             "COMPRESSED COLUMNS");
    DBUG_RETURN(error);
  }

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  TABLE *table = open_dictionary_table_write(thd);

  if (table == nullptr) {
    DBUG_ASSERT(thd->is_error());
    if (thd->get_stmt_da()->mysql_errno() == ER_CANT_LOCK) {
      thd->clear_error();
      my_error(ER_READ_ONLY_MODE, MYF(0));
    }
    DBUG_RETURN(true);
  }

  table->use_all_columns();
  type_conversion_status rc =
      table->field[COMPRESSION_DICTIONARY_FIELD_NAME]->store(
          name, name_len, system_charset_info);

  if (rc != TYPE_OK) {
    error = ER_COMPRESSION_DICTIONARY_NAME_TOO_LONG;
    my_error(error, MYF(0), name, 64);
    close_thread_tables(thd);
    thd->mdl_context.release_transactional_locks();
    DBUG_RETURN(error);
  }

  uchar user_key[MAX_KEY_LENGTH];
  KEY *name_idx_key = table->key_info + COMPRESSION_DICTIONARY_NAME_INDEX;
  key_copy(user_key, table->record[0], name_idx_key, name_idx_key->key_length);

  error = table->file->ha_index_read_idx_map(
      table->record[0], COMPRESSION_DICTIONARY_NAME_INDEX, user_key,
      HA_WHOLE_KEY, HA_READ_KEY_EXACT);

  if (error == 0) {
    error = table->file->ha_delete_row(table->record[0]);
  }

  switch (error) {
    case 0:
      /* Do nothing. Success */
      break;
    case HA_ERR_KEY_NOT_FOUND:
      if (!if_exists) {
        error = ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST;
        my_error(error, MYF(0), name);
      } else {
        /* Claim success */
        push_warning_printf(
            thd, Sql_condition::SL_NOTE,
            ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST,
            ER_THD(thd, ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST), name);

        error = 0;
      }
      break;
    case HA_ERR_ROW_IS_REFERENCED:
      error = ER_COMPRESSION_DICTIONARY_IS_REFERENCED;
      my_error(error, MYF(0), name);
      break;
    case HA_ERR_TOO_MANY_CONCURRENT_TRXS:
      error = ER_TOO_MANY_CONCURRENT_TRXS;
      my_error(error, MYF(0));
      break;
    case HA_ERR_TABLE_READONLY:
      error = ER_OPEN_AS_READONLY;
      my_error(error, MYF(0), COMPRESSION_DICTIONARY_TABLE);
      break;
    case HA_ERR_INNODB_FORCED_RECOVERY:
      error = ER_INNODB_FORCED_RECOVERY;
      my_error(error, MYF(0));
      break;
    default:
      DBUG_ASSERT(0);
      error = ER_UNKNOWN_ERROR;
      my_error(error, MYF(0));
      break;
  }

  bool failure =
      error != 0 || thd->is_error() || thd->transaction_rollback_request;

  if (!failure && write_bin_log(thd, false, thd->query().str,
                                thd->query().length, true) != 0) {
    failure = true;
    error = thd->get_stmt_da()->mysql_errno();
    DBUG_ASSERT(thd->is_error());
  }

  if (failure) {
    DBUG_ASSERT(error != 0);
    trans_rollback_stmt(thd);
    trans_rollback_implicit(thd);
  } else {
    trans_commit_stmt(thd);
    trans_commit_implicit(thd);
  }

  close_thread_tables(thd);
  thd->mdl_context.release_transactional_locks();
  DBUG_RETURN(error);
}

/* Get dict_id for a given dict_name from mysql.compression_dictionary table
An attachable RO trx is used to do the read
@param[in,out]  thd             Session context
@param[in]      zip_dict_name   dictionary entry name
@return 0 for failure, else dictionary id */
uint64 get_id_for_name(THD *thd, const LEX_CSTRING &zip_dict_name) {
  DBUG_ASSERT(zip_dict_name.str != nullptr);
  DBUG_ASSERT(zip_dict_name.length != 0);

  TABLE *table = open_dictionary_table_read(thd);

  if (table == nullptr) {
    DBUG_ASSERT(thd->is_error());
    if (thd->get_stmt_da()->mysql_errno() == ER_CANT_LOCK) {
      thd->clear_error();
      my_error(ER_READ_ONLY_MODE, MYF(0));
    }
    close_dictionary_table_read(thd);
    return (0);
  }

  table->use_all_columns();
  table->field[COMPRESSION_DICTIONARY_FIELD_NAME]->store(
      zip_dict_name.str, zip_dict_name.length, system_charset_info);

  uchar user_key[MAX_KEY_LENGTH];
  KEY *name_idx_key = table->key_info + COMPRESSION_DICTIONARY_NAME_INDEX;
  key_copy(user_key, table->record[0], name_idx_key, name_idx_key->key_length);

  int ret = table->file->ha_index_read_idx_map(
      table->record[0], COMPRESSION_DICTIONARY_NAME_INDEX, user_key,
      HA_WHOLE_KEY, HA_READ_KEY_EXACT);

  const auto zip_dict_id =
      (ret == 0) ? table->field[COMPRESSION_DICTIONARY_FIELD_ID]->val_int() : 0;

  close_dictionary_table_read(thd);
  return (zip_dict_id);
}

/** Get dict_name and dict_data from mysql.compression_dictionary table
for a given dict_id. Memory for dict_name and dict_data is allocated
from table_share mem_root
@param[in,out]     thd            Session context
@param[in]         zip_dict_id    dictionary id
@param[in,out]     share          TABLE_SHARE object
@param[in,out]     zip_dict_name  dictionary name for a given dict_id
@return false on success, true on failure */
bool get_name_for_id(THD *thd, uint64 zip_dict_id, TABLE_SHARE *share,
                     LEX_CSTRING *zip_dict_name, LEX_CSTRING *zip_dict_data) {
  DBUG_ASSERT(zip_dict_id != 0);

  TABLE *table = open_dictionary_table_read(thd);

  if (table == nullptr) {
    DBUG_ASSERT(thd->is_error());
    if (thd->get_stmt_da()->mysql_errno() == ER_CANT_LOCK) {
      thd->clear_error();
      my_error(ER_READ_ONLY_MODE, MYF(0));
    }
    close_dictionary_table_read(thd);
    return (true);
  }

  table->use_all_columns();
  table->field[COMPRESSION_DICTIONARY_FIELD_ID]->store(zip_dict_id, true);

  uchar user_key[MAX_KEY_LENGTH];
  KEY *pk_key = table->key_info;
  key_copy(user_key, table->record[0], pk_key, pk_key->key_length);

  int ret = table->file->ha_index_read_idx_map(table->record[0], 0, user_key,
                                               HA_WHOLE_KEY, HA_READ_KEY_EXACT);

  bool failure = true;
  if (ret == 0) {
    char buff[MAX_FIELD_WIDTH];
    String val(buff, sizeof(buff), &my_charset_bin);

    table->field[COMPRESSION_DICTIONARY_FIELD_NAME]->val_str(&val);
    zip_dict_name->str =
        strmake_root(&share->mem_root, val.c_ptr_safe(), val.length());
    zip_dict_name->length = val.length();

    memset(buff, 0, MAX_FIELD_WIDTH);
    String val2(buff, sizeof(buff), &my_charset_bin);

    table->field[COMPRESSION_DICTIONARY_FIELD_DATA]->val_str(&val2);
    zip_dict_data->str =
        strmake_root(&share->mem_root, val2.c_ptr_safe(), val2.length());
    zip_dict_data->length = val2.length();
    failure = false;
  }

  close_dictionary_table_read(thd);
  return (failure);
}

/** Insert a entry into mysql.compression_dictionary_cols table.
@param[in]    table           TABLE* object
@param[in]    table_id        table_id of table in mysql.tables table
@param[in]    column_id       column_id of table in mysql.columns table
@param[in]    dict_id         dictionary id in mysql.compression_dictionary
                              table
@return false on success, true on failure */
static bool cols_table_insert_low(TABLE *table, uint64 table_id,
                                  uint64 column_id, uint64 dict_id) {
  table->use_all_columns();

  table->field[COMPRESSION_DICTIONARY_COLS_FIELD_TABLE_ID]->store(table_id,
                                                                  true);
  table->field[COMPRESSION_DICTIONARY_COLS_FIELD_COLUMN_ID]->store(column_id,
                                                                   true);
  table->field[COMPRESSION_DICTIONARY_COLS_FIELD_DICT_ID]->store(dict_id, true);

  int ret = table->file->ha_write_row(table->record[0]);

  if (ret != 0) {
    DBUG_ASSERT(0);
    int error = ER_UNKNOWN_ERROR;
    my_error(error, MYF(0));
    return (true);
  }

  return (false);
}

/** Delete an entry from mysql.compression_dictionary_cols table
@param[in]   table      TABLE* object
@param[in]   table_id   table_id of table in mysql.tables
@param[in]   column_id  column_id of a column in mysql.columns
@return false on success, true on failure */
static bool cols_table_delete_low(TABLE *table, uint64 table_id,
                                  uint64 column_id) {
  table->use_all_columns();

  table->field[COMPRESSION_DICTIONARY_COLS_FIELD_TABLE_ID]->store(table_id,
                                                                  true);

  table->field[COMPRESSION_DICTIONARY_COLS_FIELD_COLUMN_ID]->store(column_id,
                                                                   true);

  uchar user_key[MAX_KEY_LENGTH];
  KEY *table_idx_key =
      table->key_info + COMPRESSION_DICTIONARY_COLS_TABLE_INDEX;
  key_copy(user_key, table->record[0], table_idx_key,
           table_idx_key->key_length);

  int ret = table->file->ha_index_read_idx_map(
      table->record[0], COMPRESSION_DICTIONARY_COLS_TABLE_INDEX, user_key,
      HA_WHOLE_KEY, HA_READ_KEY_EXACT);

  if (ret == 0) {
    ret = table->file->ha_delete_row(table->record[0]);
  }

  switch (ret) {
    case 0:
      return (false);
    case HA_ERR_INNODB_FORCED_RECOVERY:
      my_error(ER_INNODB_FORCED_RECOVERY, MYF(0));
      return (true);
    default:
      DBUG_ASSERT(0);
      my_error(ER_UNKNOWN_ERROR, MYF(0));
      return (true);
  }
}

/** Insert a entry into mysql.compression_dictionary_cols table.
@param[in,out]  thd           Session context
@param[in]	table 	      dictionary table object (dd::Table)
@return false on success, true on failure */
bool cols_table_insert(THD *thd, const dd::Table &table) {
  TABLE *cols_table = nullptr;

  dd::Update_dictionary_tables_ctx ctx(thd);

  for (const dd::Column *col_obj : table.columns()) {
    const dd::Properties &column_options = col_obj->options();

    if (!column_options.exists("zip_dict_id")) {
      continue;
    }
    uint64 zip_dict_id;
    column_options.get("zip_dict_id", &zip_dict_id);
    DBUG_ASSERT(zip_dict_id != 0);
    uint64 table_id = table.id();
    uint64 column_id = col_obj->id();

    if (cols_table == nullptr) {
      cols_table = open_dictionary_cols_table_write(thd);
      if (cols_table == nullptr) {
        DBUG_ASSERT(thd->is_error());
        if (thd->get_stmt_da()->mysql_errno() == ER_CANT_LOCK) {
          thd->clear_error();
          my_error(ER_READ_ONLY_MODE, MYF(0));
        }
        return (true);
      }
    }

    bool failure =
        cols_table_insert_low(cols_table, table_id, column_id, zip_dict_id);
    if (failure) {
      return (true);
    }
  }

  return (false);
}

/** Delete an entry from mysql.compression_dictionary_cols table
@param[in,out]  thd           Session context
@param[in]	table 	      dictionary table object (dd::Table)
@return false on success, true on failure */
bool cols_table_delete(THD *thd, const dd::Table &table) {
  TABLE *cols_table = nullptr;
  dd::Update_dictionary_tables_ctx ctx(thd);

  for (const dd::Column *col_obj : table.columns()) {
    const dd::Properties &column_options = col_obj->options();

    if (!column_options.exists("zip_dict_id")) {
      continue;
    }

    uint64 table_id = table.id();
    uint64 column_id = col_obj->id();
    if (cols_table == nullptr) {
      cols_table = open_dictionary_cols_table_write(thd);
      if (cols_table == nullptr) {
        DBUG_ASSERT(thd->is_error());
        if (thd->get_stmt_da()->mysql_errno() == ER_CANT_LOCK) {
          thd->clear_error();
          my_error(ER_READ_ONLY_MODE, MYF(0));
        }
        return (true);
      }
    }

    bool failure = cols_table_delete_low(cols_table, table_id, column_id);

    if (failure) {
      int error = ER_UNKNOWN_ERROR;
      my_error(error, MYF(0));
      return (true);
    }
  }
  return (false);
}

}  // namespace compression_dict
