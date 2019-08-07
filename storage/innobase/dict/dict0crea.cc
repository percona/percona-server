/*****************************************************************************

Copyright (c) 1996, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file dict/dict0crea.cc
 Database object creation

 Created 1/8/1996 Heikki Tuuri
 *******************************************************/

#include "dict0crea.h"
#include "btr0btr.h"
#include "btr0pcur.h"
#include "dict0boot.h"
#include "dict0dd.h"
#include "dict0dict.h"
#include "dict0priv.h"
#include "dict0stats.h"
#include "fsp0space.h"
#include "fsp0sysspace.h"
#include "fts0priv.h"
#include "ha_prototypes.h"
#include "mach0data.h"

#include "my_dbug.h"

#include "dict0upgrade.h"
#include "page0page.h"
#include "pars0pars.h"
#include "que0que.h"
#include "row0ins.h"
#include "row0mysql.h"
#include "row0sel.h"
#include "srv0start.h"
#include "trx0roll.h"
#include "usr0sess.h"
#include "ut0vec.h"

#include "fil0crypt.h"  //dla FIL_ENCRYPTION_KEY_DEFAULT
#include "fil0fil.h"
#include "sql/sql_zip_dict.h"

/** Build a table definition without updating SYSTEM TABLES
@param[in,out]	table	dict table object
@param[in,out]	trx	transaction instance
@return DB_SUCCESS or error code */
dberr_t dict_build_table_def(
    dict_table_t *table, trx_t *trx, fil_encryption_t mode,
    const KeyringEncryptionKeyIdInfo &keyring_encryption_key_id) {
  char db_buf[NAME_LEN + 1];
  char tbl_buf[NAME_LEN + 1];

  dd_parse_tbl_name(table->name.m_name, db_buf, tbl_buf, nullptr, nullptr,
                    nullptr);

  bool is_dd_table = dd::get_dictionary()->is_dd_table_name(db_buf, tbl_buf);

  /** In-memory counter used for assigning table_id
  of data dictionary table. This counter is only used
  during bootstrap or upgrade */
  static uint32_t dd_table_id = 1;

  /* Treat mysql.compression_dictionary like DD table during bootstrap or
  during upgrade. Only exemption is when this table is created by Percona
  server started on mysql datadir. In that scenario, we should
  use the next available table id */
  if (is_dd_table ||
      (compression_dict::is_hardcoded(db_buf, tbl_buf) && dd_table_id != 1)) {
    table->id = dd_table_id++;
    table->is_dd_table = true;

    ut_ad(compression_dict::is_hardcoded(db_buf, tbl_buf) ||
          strcmp(tbl_buf, innodb_dd_table[table->id - 1].name) == 0);

  } else {
    dict_table_assign_new_id(table, trx);
  }

  dberr_t err = dict_build_tablespace_for_table(table, trx, mode,
                                                keyring_encryption_key_id);

  return (err);
}

/** Build a tablespace to store various objects.
@param[in,out]	trx		DD transaction
@param[in,out]	tablespace	Tablespace object describing what to build.
@param[in]      keyring_encryption_key_id info on keyring encryption key
@return DB_SUCCESS or error code. */
dberr_t dict_build_tablespace(
    trx_t *trx, Tablespace *tablespace, fil_encryption_t mode,
    const KeyringEncryptionKeyIdInfo &keyring_encryption_key_id) {
  dberr_t err = DB_SUCCESS;
  mtr_t mtr;
  space_id_t space = 0;
  ut_d(static uint32_t crash_injection_after_create_counter = 1;);

  ut_ad(mutex_own(&dict_sys->mutex));
  ut_ad(tablespace);

  DBUG_EXECUTE_IF("out_of_tablespace_disk", return (DB_OUT_OF_FILE_SPACE););
  /* Get a new space id. */
  dict_hdr_get_new_id(NULL, NULL, &space, NULL, false);
  if (space == SPACE_UNKNOWN) {
    return (DB_ERROR);
  }
  tablespace->set_space_id(space);

  Datafile *datafile = tablespace->first_datafile();

  /* If file already exists we cannot write delete space to ddl log. */
  os_file_type_t type;
  bool exists;
  if (os_file_status(datafile->filepath(), &exists, &type)) {
    if (exists) {
      return DB_TABLESPACE_EXISTS;
    }
  } else {
    return DB_IO_ERROR;
  }

  err = log_ddl->write_delete_space_log(trx, NULL, space, datafile->filepath(),
                                        false, true);
  if (err != DB_SUCCESS) {
    return err;
  }

  /* We create a new generic empty tablespace.
  We initially let it be 4 pages:
  - page 0 is the fsp header and an extent descriptor page,
  - page 1 is an ibuf bitmap page,
  - page 2 is the first inode page,
  - page 3 will contain the root of the clustered index of the
  first table we create here. */

  err = fil_ibd_create(space, tablespace->name(), datafile->filepath(),
                       tablespace->flags(), FIL_IBD_FILE_INITIAL_SIZE, mode,
                       keyring_encryption_key_id);

  DBUG_INJECT_CRASH("ddl_crash_after_create_tablespace",
                    crash_injection_after_create_counter++);

  if (err != DB_SUCCESS) {
    return (err);
  }

  DBUG_EXECUTE_IF("innodb_fail_to_update_tablespace_dict",
                  return (DB_INTERRUPTED););

  mtr_start(&mtr);

  /* Once we allow temporary general tablespaces, we must do this;
  mtr_set_log_mode(&mtr, MTR_LOG_NO_REDO); */
  ut_a(!FSP_FLAGS_GET_TEMPORARY(tablespace->flags()));

  bool ret = fsp_header_init(space, FIL_IBD_FILE_INITIAL_SIZE, &mtr, false);
  mtr_commit(&mtr);

  DBUG_EXECUTE_IF("fil_ibd_create_log",
                  log_write_up_to(*log_sys, mtr.commit_lsn(), true);
                  DBUG_SUICIDE(););

  if (!ret) {
    return (DB_ERROR);
  }

  return (err);
}

/** Determine the session temporary tablespace for temp or intrinsic tables
@param[in]   innodb_session   InnoDB session context(thd)
@param[in]   is_intrinsic     true if temp table is created by optimizer
@param[in]   is_slave_thd     true if temp table is created by slave thread
                              (happens only with binlog row format)
@return Session temporary tablespace on success, else nullptr on failure */
static ibt::Tablespace *determine_session_temp_tblsp(
    innodb_session_t *innodb_session, bool is_intrinsic, bool is_slave_thd) {
  ibt::Tablespace *tblsp = nullptr;
  bool encrypted = false;
  switch (srv_default_table_encryption) {
    case DEFAULT_TABLE_ENC_ON:
    case DEFAULT_TABLE_ENC_KEYRING_ON:
    case DEFAULT_TABLE_ENC_ONLINE_TO_KEYRING:
      encrypted = true;
      break;
    case DEFAULT_TABLE_ENC_OFF:
    case DEFAULT_TABLE_ENC_ONLINE_FROM_KEYRING_TO_UNENCRYPTED:
      if (srv_tmp_tablespace_encrypt) {
        encrypted = true;
      }
      break;
    default:
      ut_ad(0);
  }

  if (encrypted) {
    if (is_slave_thd) {
      tblsp = ibt::get_enc_rpl_slave_tblsp();
    } else if (is_intrinsic) {
      tblsp = innodb_session->get_enc_instrinsic_temp_tblsp();
    } else {
      tblsp = innodb_session->get_enc_usr_temp_tblsp();
    }

  } else {
    if (is_slave_thd) {
      tblsp = ibt::get_rpl_slave_tblsp();
    } else if (is_intrinsic) {
      tblsp = innodb_session->get_instrinsic_temp_tblsp();
    } else {
      tblsp = innodb_session->get_usr_temp_tblsp();
    }
  }
  return (tblsp);
}

/** Builds a tablespace to contain a table, using file-per-table=1.
@param[in,out]	table	Table to build in its own tablespace.
@param[in,out]	trx	Transaction
@param[in]      keyring_encryption_key_id info on keyring encryption key
@return DB_SUCCESS or error code */
dberr_t dict_build_tablespace_for_table(
    dict_table_t *table, trx_t *trx, fil_encryption_t mode,
    const KeyringEncryptionKeyIdInfo &keyring_encryption_key_id) {
  dberr_t err = DB_SUCCESS;
  mtr_t mtr;
  space_id_t space = 0;
  bool needs_file_per_table;
  char *filepath;
  ut_d(static uint32_t crash_injection_after_create_counter = 1;);

  ut_ad(!mutex_own(&dict_sys->mutex));

  needs_file_per_table =
      DICT_TF2_FLAG_IS_SET(table, DICT_TF2_USE_FILE_PER_TABLE);

  if (mode == FIL_ENCRYPTION_ON ||
      (mode == FIL_ENCRYPTION_DEFAULT &&
       srv_default_table_encryption == DEFAULT_TABLE_ENC_ONLINE_TO_KEYRING)) {
    DICT_TF2_FLAG_SET(table, DICT_TF2_ENCRYPTION_FILE_PER_TABLE);
  }

  if (needs_file_per_table) {
    /* Temporary table would always reside in the same
    shared temp tablespace. */
    ut_ad(!table->is_temporary());
    /* This table will need a new tablespace. */

    ut_ad(DICT_TF_GET_ZIP_SSIZE(table->flags) == 0 ||
          dict_table_has_atomic_blobs(table));

    /* Get a new tablespace ID */
    dict_hdr_get_new_id(NULL, NULL, &space, table, false);

    DBUG_EXECUTE_IF("ib_create_table_fail_out_of_space_ids",
                    space = SPACE_UNKNOWN;);

    if (space == SPACE_UNKNOWN) {
      return (DB_ERROR);
    }
    table->space = space;

    /* Determine the tablespace flags. */
    uint32_t fsp_flags = dict_tf_to_fsp_flags(table->flags);

    /* For file-per-table tablespace, set encryption flag */
    if (DICT_TF2_FLAG_IS_SET(table, DICT_TF2_ENCRYPTION_FILE_PER_TABLE)) {
      fsp_flags_set_encryption(fsp_flags);
    }

    if (DICT_TF_HAS_DATA_DIR(table->flags)) {
      std::string path;

      path = dict_table_get_datadir(table);

      filepath = Fil_path::make(path, table->name.m_name, IBD, true);
    } else {
      filepath = Fil_path::make_ibd_from_table_name(table->name.m_name);
    }

    /* If file already exists we cannot write delete space to ddl log. */
    os_file_type_t type;
    bool exists;
    if (os_file_status(filepath, &exists, &type)) {
      if (exists) {
        ut_free(filepath);
        return DB_TABLESPACE_EXISTS;
      }
    } else {
      ut_free(filepath);
      return DB_IO_ERROR;
    }

    err = log_ddl->write_delete_space_log(trx, table, space, filepath, false,
                                          false);
    if (err != DB_SUCCESS) {
      ut_free(filepath);
      return err;
    }

    /* We create a new single-table tablespace for the table.
    We initially let it be 4 pages:
    - page 0 is the fsp header and an extent descriptor page,
    - page 1 is an ibuf bitmap page,
    - page 2 is the first inode page,
    - page 3 will contain the root of the clustered index of
    the table we create here. */

    std::string tablespace_name;

    dd_filename_to_spacename(table->name.m_name, &tablespace_name);

    err = fil_ibd_create(space, tablespace_name.c_str(), filepath, fsp_flags,
                         FIL_IBD_FILE_INITIAL_SIZE, mode,
                         keyring_encryption_key_id);

    ut_free(filepath);

    DBUG_INJECT_CRASH("ddl_crash_after_create_tablespace",
                      crash_injection_after_create_counter++);

    if (err != DB_SUCCESS) {
      return (err);
    }

    mtr_start(&mtr);

    bool ret =
        fsp_header_init(table->space, FIL_IBD_FILE_INITIAL_SIZE, &mtr, false);
    mtr_commit(&mtr);

    DBUG_EXECUTE_IF("fil_ibd_create_log",
                    log_write_up_to(*log_sys, mtr.commit_lsn(), true);
                    DBUG_SUICIDE(););

    if (!ret) {
      return (DB_ERROR);
    }

    err = btr_sdi_create_index(table->space, false);
    return (err);

  } else {
    /* We do not need to build a tablespace for this table. It
    is already built.  Just find the correct tablespace ID. */

    if (DICT_TF_HAS_SHARED_SPACE(table->flags)) {
      ut_ad(table->tablespace != NULL);

      ut_ad(table->space == fil_space_get_id_by_name(table->tablespace()));
    } else if (table->is_temporary()) {
      /* Use the shared temporary tablespace.
      Note: The temp tablespace supports all non-Compressed
      row formats whereas the system tablespace only
      supports Redundant and Compact */
      ut_ad(dict_tf_get_rec_format(table->flags) != REC_FORMAT_COMPRESSED);

      innodb_session_t *innodb_session = thd_to_innodb_session(trx->mysql_thd);

      bool is_slave_thd = thd_is_replication_slave_thread(trx->mysql_thd);
      bool is_intrinsic = table->is_intrinsic();
      const ibt::Tablespace *tblsp = determine_session_temp_tblsp(
          innodb_session, is_intrinsic, is_slave_thd);

      /* Session temporary tablespace couldn't be allocated. This means,
      we have run out of disk space */
      if (tblsp == nullptr) {
        return (DB_NO_SESSION_TEMP);
      }

      table->space = tblsp->space_id();

    } else {
      /* Create in the system tablespace. */
      ut_ad(table->space == TRX_SYS_SPACE);
    }

    DBUG_EXECUTE_IF("ib_ddl_crash_during_tablespace_alloc", DBUG_SUICIDE(););
  }

  return (DB_SUCCESS);
}

/** Builds an index definition
 @return DB_SUCCESS or error code */
void dict_build_index_def(const dict_table_t *table, /*!< in: table */
                          dict_index_t *index,       /*!< in/out: index */
                          trx_t *trx) /*!< in/out: InnoDB transaction handle */
{
  ut_ad(!mutex_own(&dict_sys->mutex));
  ut_ad((UT_LIST_GET_LEN(table->indexes) > 0) || index->is_clustered());

  if (!table->is_intrinsic()) {
    if (srv_is_upgrade_mode) {
      index->id = dd_upgrade_indexes_num++;
      ut_ad(index->id <=
            dd_get_total_indexes_num() +
                4 /* total indexes from compression dictionary tables */);
    } else {
      dict_hdr_get_new_id(NULL, &index->id, NULL, table, false);
    }

  } else {
    /* Index are re-loaded in process of creation using id.
    If same-id is used for all indexes only first index will always
    be retrieved when expected is iterative return of all indexes*/
    if (UT_LIST_GET_LEN(table->indexes) > 0) {
      index->id = UT_LIST_GET_LAST(table->indexes)->id + 1;
    } else {
      index->id = 1;
    }
  }

  /* Inherit the space id from the table; we store all indexes of a
  table in the same tablespace */

  index->space = table->space;

  /* Note that the index was created by this transaction. */
  index->trx_id = trx->id;
}

/** Creates an index tree for the index if it is not a member of a cluster.
@param[in,out]	index	InnoDB index object
@param[in,out]	trx	transaction
@return DB_SUCCESS or DB_OUT_OF_FILE_SPACE */
dberr_t dict_create_index_tree_in_mem(dict_index_t *index, trx_t *trx) {
  mtr_t mtr;
  ulint page_no = FIL_NULL;

  ut_ad(!mutex_own(&dict_sys->mutex));

  DBUG_EXECUTE_IF("ib_dict_create_index_tree_fail", return (DB_OUT_OF_MEMORY););

  if (index->type == DICT_FTS) {
    /* FTS index does not need an index tree */
    return (DB_SUCCESS);
  }

  const bool unreadable =
      !index->table->is_readable() || dict_table_is_discarded(index->table);

  if (unreadable) {
    index->page = FIL_NULL;
    index->trx_id = trx->id;

    return (DB_SUCCESS);
  }

  mtr_start(&mtr);

  if (index->table->is_temporary()) {
    mtr_set_log_mode(&mtr, MTR_LOG_NO_REDO);
  }

  dberr_t err = DB_SUCCESS;

  page_no =
      btr_create(index->type, index->space, dict_table_page_size(index->table),
                 index->id, index, &mtr);

  index->page = page_no;
  index->trx_id = trx->id;

  mtr_commit(&mtr);
  if (page_no == FIL_NULL) {
    err = DB_OUT_OF_FILE_SPACE;
  } else {
    /* FIXME: Now writing ddl log after the index has been created,
    so if server crashes before the redo log gets persisted,
    there is no way to find the resources(two segments, etc.)
    allocated to this index. Since this is a rare case, living
    with it is acceptable */
    /* FIXME: if it's part of CREATE TABLE, and file_per_table is
    true, skip ddl log, because during rollback, the whole
    tablespace would be dropped */

    /* During upgrade, etc., the log_ddl may haven't been
    initialized and we don't need to write DDL logs too.
    This can only happen for CREATE TABLE. */
    if (log_ddl != nullptr) {
      err = log_ddl->write_free_tree_log(trx, index, false);
    }
  }

  return (err);
}

/** Drop an index tree belonging to a temporary table.
@param[in]	index		index in a temporary table
@param[in]	root_page_no	index root page number */
void dict_drop_temporary_table_index(const dict_index_t *index,
                                     page_no_t root_page_no) {
  ut_ad(mutex_own(&dict_sys->mutex) || index->table->is_intrinsic());
  ut_ad(index->table->is_temporary());
  ut_ad(index->page == FIL_NULL);

  space_id_t space = index->space;
  bool found;
  const page_size_t page_size(fil_space_get_page_size(space, &found));

  /* If tree has already been freed or it is a single table
  tablespace and the .ibd file is missing do nothing,
  else free the all the pages */
  if (root_page_no != FIL_NULL && found) {
    btr_free(page_id_t(space, root_page_no), page_size);
  }
}

/** Check whether a column is in an index by the column name
@param[in]	col_name	column name for the column to be checked
@param[in]	index		the index to be searched
@return	true if this column is in the index, otherwise, false */
static bool dict_index_has_col_by_name(const char *col_name,
                                       const dict_index_t *index) {
  for (ulint i = 0; i < index->n_fields; i++) {
    dict_field_t *field = index->get_field(i);

    if (strcmp(field->name, col_name) == 0) {
      return (true);
    }
  }
  return (false);
}

/** Check whether the foreign constraint could be on a column that is
part of a virtual index (index contains virtual column) in the table
@param[in]	fk_col_name	FK column name to be checked
@param[in]	table		the table
@return	true if this column is indexed with other virtual columns */
bool dict_foreign_has_col_in_v_index(const char *fk_col_name,
                                     const dict_table_t *table) {
  /* virtual column can't be Primary Key, so start with secondary index */
  for (const dict_index_t *index = table->first_index()->next(); index;
       index = index->next()) {
    if (dict_index_has_virtual(index)) {
      if (dict_index_has_col_by_name(fk_col_name, index)) {
        return (true);
      }
    }
  }

  return (false);
}

/** Check whether the foreign constraint could be on a column that is
a base column of some indexed virtual columns.
@param[in]	col_name	column name for the column to be checked
@param[in]	table		the table
@return	true if this column is a base column, otherwise, false */
bool dict_foreign_has_col_as_base_col(const char *col_name,
                                      const dict_table_t *table) {
  /* Loop through each virtual column and check if its base column has
  the same name as the column name being checked */
  for (ulint i = 0; i < table->n_v_cols; i++) {
    dict_v_col_t *v_col = dict_table_get_nth_v_col(table, i);

    /* Only check if the virtual column is indexed */
    if (!v_col->m_col.ord_part) {
      continue;
    }

    for (ulint j = 0; j < v_col->num_base; j++) {
      if (strcmp(col_name, table->get_col_name(v_col->base_col[j]->ind)) == 0) {
        return (true);
      }
    }
  }

  return (false);
}

/** Check if a foreign constraint is on the given column name.
@param[in]	col_name	column name to be searched for fk constraint
@param[in]	table		table to which foreign key constraint belongs
@return true if fk constraint is present on the table, false otherwise. */
static bool dict_foreign_base_for_stored(const char *col_name,
                                         const dict_table_t *table) {
  /* Loop through each stored column and check if its base column has
  the same name as the column name being checked */
  dict_s_col_list::const_iterator it;
  for (it = table->s_cols->begin(); it != table->s_cols->end(); ++it) {
    dict_s_col_t s_col = *it;

    for (ulint j = 0; j < s_col.num_base; j++) {
      /** If the stored column can refer to virtual column
      or stored column then it can points to NULL. */

      if (s_col.base_col[j] == NULL) {
        continue;
      }

      if (strcmp(col_name, table->get_col_name(s_col.base_col[j]->ind)) == 0) {
        return (true);
      }
    }
  }

  return (false);
}

/** Check if a foreign constraint is on columns served as base columns
of any stored column. This is to prevent creating SET NULL or CASCADE
constraint on such columns
@param[in]	local_fk_set	set of foreign key objects, to be added to
the dictionary tables
@param[in]	table		table to which the foreign key objects in
local_fk_set belong to
@return true if yes, otherwise, false */
bool dict_foreigns_has_s_base_col(const dict_foreign_set &local_fk_set,
                                  const dict_table_t *table) {
  dict_foreign_t *foreign;

  if (table->s_cols == NULL) {
    return (false);
  }

  for (dict_foreign_set::const_iterator it = local_fk_set.begin();
       it != local_fk_set.end(); ++it) {
    foreign = *it;
    ulint type = foreign->type;

    type &=
        ~(DICT_FOREIGN_ON_DELETE_NO_ACTION | DICT_FOREIGN_ON_UPDATE_NO_ACTION);

    if (type == 0) {
      continue;
    }

    for (ulint i = 0; i < foreign->n_fields; i++) {
      /* Check if the constraint is on a column that
      is a base column of any stored column */
      if (dict_foreign_base_for_stored(foreign->foreign_col_names[i], table)) {
        return (true);
      }
    }
  }

  return (false);
}

/** Check if a column is in foreign constraint with CASCADE properties or
SET NULL
@param[in]	table		table
@param[in]	col_name	name for the column to be checked
@return true if the column is in foreign constraint, otherwise, false */
bool dict_foreigns_has_this_col(const dict_table_t *table,
                                const char *col_name) {
  dict_foreign_t *foreign;
  const dict_foreign_set *local_fk_set = &table->foreign_set;

  for (dict_foreign_set::const_iterator it = local_fk_set->begin();
       it != local_fk_set->end(); ++it) {
    foreign = *it;
    ut_ad(foreign->id != NULL);
    ulint type = foreign->type;

    type &=
        ~(DICT_FOREIGN_ON_DELETE_NO_ACTION | DICT_FOREIGN_ON_UPDATE_NO_ACTION);

    if (type == 0) {
      continue;
    }

    for (ulint i = 0; i < foreign->n_fields; i++) {
      if (strcmp(foreign->foreign_col_names[i], col_name) == 0) {
        return (true);
      }
    }
  }
  return (false);
}

/** Assign a new table ID and put it into the table cache and the transaction.
@param[in,out]	table	Table that needs an ID
@param[in,out]	trx	Transaction */
void dict_table_assign_new_id(dict_table_t *table, trx_t *trx) {
  if (table->is_intrinsic()) {
    /* There is no significance of this table->id (if table is
    intrinsic) so assign it default instead of something meaningful
    to avoid confusion.*/
    table->id = ULINT_UNDEFINED;
  } else {
    dict_hdr_get_new_id(&table->id, NULL, NULL, table, false);
  }
}

/** Create in-memory tablespace dictionary index & table
@param[in]	space		tablespace id
@param[in]	space_discarded	true if space is discarded
@param[in]	in_flags	space flags to use when space_discarded is true
@param[in]	is_create	true when creating SDI index
@return in-memory index structure for tablespace dictionary or NULL */
dict_index_t *dict_sdi_create_idx_in_mem(space_id_t space, bool space_discarded,
                                         uint32_t in_flags, bool is_create) {
  uint32_t flags = space_discarded ? in_flags : fil_space_get_flags(space);

  /* This means the tablespace is evicted from cache */
  if (flags == UINT32_UNDEFINED) {
    return (NULL);
  }

  ut_ad(fsp_flags_is_valid(flags));

  mutex_exit(&dict_sys->mutex);

  rec_format_t rec_format;

  ulint zip_ssize = FSP_FLAGS_GET_ZIP_SSIZE(flags);
  ulint atomic_blobs = FSP_FLAGS_HAS_ATOMIC_BLOBS(flags);
  bool has_data_dir = FSP_FLAGS_HAS_DATA_DIR(flags);
  bool has_shared_space = FSP_FLAGS_GET_SHARED(flags);

  if (zip_ssize > 0) {
    rec_format = REC_FORMAT_COMPRESSED;
  } else if (atomic_blobs) {
    rec_format = REC_FORMAT_DYNAMIC;
  } else {
    rec_format = REC_FORMAT_COMPACT;
  }

  uint32_t table_flags = 0;
  dict_tf_set(&table_flags, rec_format, zip_ssize, has_data_dir,
              has_shared_space);

  /* 18 = strlen(SDI) + Max digits of 4 byte spaceid (10) + 1 */
  char table_name[18];
  mem_heap_t *heap = mem_heap_create(DICT_HEAP_SIZE);
  snprintf(table_name, sizeof(table_name), "SDI_" SPACE_ID_PF, space);

  dict_table_t *table =
      dict_mem_table_create(table_name, space, 5, 0, table_flags, 0);

  dict_mem_table_add_col(table, heap, "type", DATA_INT,
                         DATA_NOT_NULL | DATA_UNSIGNED, 4);
  dict_mem_table_add_col(table, heap, "id", DATA_INT,
                         DATA_NOT_NULL | DATA_UNSIGNED, 8);
  dict_mem_table_add_col(table, heap, "compressed_len", DATA_INT,
                         DATA_NOT_NULL | DATA_UNSIGNED, 4);
  dict_mem_table_add_col(table, heap, "uncompressed_len", DATA_INT,
                         DATA_NOT_NULL | DATA_UNSIGNED, 4);
  dict_mem_table_add_col(table, heap, "data", DATA_BLOB, DATA_NOT_NULL, 0);

  table->id = dict_sdi_get_table_id(space);

  /* Disable persistent statistics on the table */
  dict_stats_set_persistent(table, false, true);

  dict_table_add_system_columns(table, heap);

  const char *index_name = "CLUST_IND_SDI";

  dict_index_t *temp_index =
      dict_mem_index_create(table_name, index_name, space,
                            DICT_CLUSTERED | DICT_UNIQUE | DICT_SDI, 2);
  ut_ad(temp_index);

  temp_index->add_field("type", 0, true);
  temp_index->add_field("id", 0, true);

  temp_index->table = table;

  /* Disable AHI on SDI tables */
  temp_index->disable_ahi = true;

  page_no_t index_root_page_num;

  /* When we do DISCARD TABLESPACE, there will be no fil_space_t
  for the tablespace. In this case, we should not use fil_space_*()
  methods */
  if (!space_discarded && !is_create) {
    mtr_t mtr;
    mtr.start();

    index_root_page_num =
        fsp_sdi_get_root_page_num(space, page_size_t(flags), &mtr);

    mtr_commit(&mtr);

  } else {
    index_root_page_num = FIL_NULL;
  }

  temp_index->id = dict_sdi_get_index_id();

  dberr_t error =
      dict_index_add_to_cache(table, temp_index, index_root_page_num, false);
  ut_a(error == DB_SUCCESS);

  mutex_enter(&dict_sys->mutex);

  /* After re-acquiring dict_sys mutex, check if there is already
  a table created by other threads. Just keep one copy in memory */
  dict_table_t *exist = dict_table_check_if_in_cache_low(table->name.m_name);
  if (exist != nullptr) {
    dict_index_remove_from_cache(table, table->first_index());
    dict_mem_table_free(table);
    table = exist;
  } else {
    dict_table_add_to_cache(table, TRUE, heap);
  }

  mem_heap_free(heap);
  return (table->first_index());
}

/** Fetch callback, just stores extracted zip_dict id in the external
variable.
@return TRUE if all OK */
static ibool dict_create_extract_int_aux(void *row,      /*!< in: sel_node_t* */
                                         void *user_arg) /*!< in: int32 id */
{
  sel_node_t *node = static_cast<sel_node_t *>(row);
  dfield_t *dfield = que_node_get_val(node->select_list);
  dtype_t *type = dfield_get_type(dfield);
  ulint len = dfield_get_len(dfield);

  ut_a(dtype_get_mtype(type) == DATA_INT);
  ut_a(len == sizeof(ib_uint32_t));

  memcpy(user_arg, dfield_get_data(dfield), sizeof(ib_uint32_t));

  return (TRUE);
}

/** Get a single compression dictionary id for the given
(table id, column pos) pair.
@return	error code or DB_SUCCESS */
dberr_t dict_create_get_zip_dict_id_by_reference(
    table_id_t table_id, /*!< in: table id */
    ulint column_pos,    /*!< in: column position */
    ulint *dict_id,      /*!< out: dict id */
    trx_t *trx)          /*!< in/out: transaction */
{
  ut_ad(dict_id);
  ut_ad(srv_is_upgrade_mode);

  pars_info_t *info = pars_info_create();

  ib_uint32_t dict_id_buf;
  mach_write_to_4(reinterpret_cast<byte *>(&dict_id_buf), UINT32_UNDEFINED);

  pars_info_add_int4_literal(info, "table_id", table_id);
  pars_info_add_int4_literal(info, "column_pos", column_pos);
  pars_info_bind_function(info, "my_func", dict_create_extract_int_aux,
                          &dict_id_buf);

  dberr_t error = que_eval_sql(info,
                               "PROCEDURE P () IS\n"
                               "DECLARE FUNCTION my_func;\n"
                               "DECLARE CURSOR cur IS\n"
                               "  SELECT DICT_ID FROM SYS_ZIP_DICT_COLS\n"
                               "    WHERE TABLE_ID = :table_id AND\n"
                               "          COLUMN_POS = :column_pos;\n"
                               "BEGIN\n"
                               "  OPEN cur;\n"
                               "  FETCH cur INTO my_func();\n"
                               "  CLOSE cur;\n"
                               "END;\n",
                               FALSE, trx);
  if (error == DB_SUCCESS) {
    ib_uint32_t local_dict_id =
        mach_read_from_4(reinterpret_cast<const byte *>(&dict_id_buf));
    if (local_dict_id == UINT32_UNDEFINED)
      error = DB_RECORD_NOT_FOUND;
    else
      *dict_id = local_dict_id;
  }
  return error;
}

/** Auxiliary enum used to indicate zip dict data extraction result code */
enum zip_dict_info_aux_code {
  zip_dict_info_success,        /*!< success */
  zip_dict_info_not_found,      /*!< zip dict record not found */
  zip_dict_info_oom,            /*!< out of memory */
  zip_dict_info_corrupted_name, /*!< corrupted zip dict name */
  zip_dict_info_corrupted_data  /*!< corrupted zip dict data */
};

/** Auxiliary struct used to return zip dict info aling with result code */
struct zip_dict_info_aux {
  LEX_STRING name; /*!< zip dict name */
  LEX_STRING data; /*!< zip dict data */
  int code;        /*!< result code (0 - success) */
};

/** Fetch callback, just stores extracted zip_dict data in the external
variable.
@return always returns TRUE */
static ibool dict_create_get_zip_dict_info_by_id_aux(
    void *row,      /*!< in: sel_node_t* */
    void *user_arg) /*!< in: pointer to zip_dict_info_aux* */
{
  sel_node_t *node = static_cast<sel_node_t *>(row);
  zip_dict_info_aux *result = static_cast<zip_dict_info_aux *>(user_arg);

  result->code = zip_dict_info_success;
  result->name.str = 0;
  result->name.length = 0;
  result->data.str = 0;
  result->data.length = 0;

  /* NAME field */
  que_node_t *exp = node->select_list;
  ut_a(exp != 0);

  dfield_t *dfield = que_node_get_val(exp);
  dtype_t *type = dfield_get_type(dfield);
  ut_a(dtype_get_mtype(type) == DATA_VARCHAR);

  ulint len = dfield_get_len(dfield);
  void *data = dfield_get_data(dfield);

  if (len == UNIV_SQL_NULL) {
    result->code = zip_dict_info_corrupted_name;
  } else {
    result->name.str =
        static_cast<char *>(my_malloc(PSI_INSTRUMENT_ME, len + 1, MYF(0)));
    if (result->name.str == 0) {
      result->code = zip_dict_info_oom;
    } else {
      memcpy(result->name.str, data, len);
      result->name.str[len] = '\0';
      result->name.length = len;
    }
  }

  /* DATA field */
  exp = que_node_get_next(exp);
  ut_a(exp != 0);

  dfield = que_node_get_val(exp);
  type = dfield_get_type(dfield);
  ut_a(dtype_get_mtype(type) == DATA_BLOB);

  len = dfield_get_len(dfield);
  data = dfield_get_data(dfield);

  if (len == UNIV_SQL_NULL) {
    result->code = zip_dict_info_corrupted_data;
  } else {
    result->data.str = static_cast<char *>(
        my_malloc(PSI_INSTRUMENT_ME, len == 0 ? 1 : len, MYF(0)));
    if (result->data.str == 0) {
      result->code = zip_dict_info_oom;
    } else {
      memcpy(result->data.str, data, len);
      result->data.length = len;
    }
  }

  ut_ad(que_node_get_next(exp) == 0);

  if (result->code != zip_dict_info_success) {
    if (result->name.str == 0) {
      my_free(result->name.str);
      result->name.str = 0;
      result->name.length = 0;
    }
    if (result->data.str == 0) {
      my_free(result->data.str);
      result->data.str = 0;
      result->data.length = 0;
    }
  }

  return TRUE;
}

/** Get compression dictionary info (name and data) for the given id.
Allocates memory for name and data on success.
Must be freed with my_free().
@return	error code or DB_SUCCESS */
dberr_t dict_create_get_zip_dict_info_by_id(
    ulint dict_id,   /*!< in: dict id */
    char **name,     /*!< out: dict name */
    ulint *name_len, /*!< out: dict name length*/
    char **data,     /*!< out: dict data */
    ulint *data_len, /*!< out: dict data length*/
    trx_t *trx)      /*!< in/out: transaction */
{
  ut_ad(name);
  ut_ad(data);
  ut_ad(srv_is_upgrade_mode);

  zip_dict_info_aux rec;
  rec.code = zip_dict_info_not_found;
  pars_info_t *info = pars_info_create();

  pars_info_add_int4_literal(info, "id", dict_id);
  pars_info_bind_function(info, "my_func",
                          dict_create_get_zip_dict_info_by_id_aux, &rec);

  dberr_t error = que_eval_sql(info,
                               "PROCEDURE P () IS\n"
                               "DECLARE FUNCTION my_func;\n"
                               "DECLARE CURSOR cur IS\n"
                               "  SELECT NAME, DATA FROM SYS_ZIP_DICT\n"
                               "    WHERE ID = :id;\n"
                               "BEGIN\n"
                               "  OPEN cur;\n"
                               "  FETCH cur INTO my_func();\n"
                               "  CLOSE cur;\n"
                               "END;\n",
                               FALSE, trx);
  if (error == DB_SUCCESS) {
    switch (rec.code) {
      case zip_dict_info_success:
        *name = rec.name.str;
        *name_len = rec.name.length;
        *data = rec.data.str;
        *data_len = rec.data.length;
        break;
      case zip_dict_info_not_found:
        error = DB_RECORD_NOT_FOUND;
        break;
      case zip_dict_info_oom:
        error = DB_OUT_OF_MEMORY;
        break;
      case zip_dict_info_corrupted_name:
      case zip_dict_info_corrupted_data:
        error = DB_INVALID_NULL;
        break;
      default:
        ut_error;
    }
  }
  return error;
}
