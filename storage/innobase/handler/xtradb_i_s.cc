/*****************************************************************************

Copyright (c) 2007, 2012, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2010-2012, Percona Inc. All Rights Reserved.

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

#include "xtradb_i_s.h"

#include "mysqld_error.h"
#include "sql/auth/sql_acl.h"  // PROCESS_ACL

#include "i_s.h"
#include "m_ctype.h"
#include "my_sys.h"
#include "myisampack.h"
#include "mysql/innodb_priv.h"
#include "mysys_err.h"
#include "sql/sql_plugin.h"

#include "btr0pcur.h"  /* btr_pcur_t */
#include "btr0sea.h"   /* btr_search_sys */
#include "dict0crea.h" /* for ZIP_DICT_MAX_* constants */
#include "fil0fil.h"
#include "log0recv.h" /* recv_sys */
#include "read0i_s.h"
#include "sql/auth/auth_acls.h"
#include "sql/field.h"
#include "sql/table.h"
#include "srv0start.h" /* for srv_was_started */
#include "trx0i_s.h"

#define PLUGIN_AUTHOR "Percona Inc."

#define OK(expr)     \
  if ((expr) != 0) { \
    DBUG_RETURN(1);  \
  }

#if !defined __STRICT_ANSI__ && defined __GNUC__ && (__GNUC__) > 2 && \
    !defined __INTEL_COMPILER && !defined __clang__
#define STRUCT_FLD(name, value) \
  name:                         \
  value
#else
#define STRUCT_FLD(name, value) value
#endif

#define END_OF_ST_FIELD_INFO                                           \
  {                                                                    \
    STRUCT_FLD(field_name, nullptr), STRUCT_FLD(field_length, 0),      \
        STRUCT_FLD(field_type, MYSQL_TYPE_NULL), STRUCT_FLD(value, 0), \
        STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),          \
        STRUCT_FLD(open_method, 0)                                     \
  }

/** Auxiliary function to store char* value in MYSQL_TYPE_STRING field.
@return	0 on success */
static int field_store_string(
    Field *field,    /*!< in/out: target field for storage */
    const char *str) /*!< in: NUL-terminated utf-8 string,
                     or NULL */
{
  int ret;

  if (str != nullptr) {
    ret = field->store(str, strlen(str), system_charset_info);
    field->set_notnull();
  } else {
    ret = 0; /* success */
    field->set_null();
  }

  return (ret);
}

static int i_s_common_deinit(void *p) /*!< in/out: table schema object */
{
  DBUG_ENTER("i_s_common_deinit");

  /* Do nothing */

  DBUG_RETURN(0);
}

static ST_FIELD_INFO xtradb_read_view_fields_info[] = {
#define READ_VIEW_LOW_LIMIT_NUMBER 0
    {STRUCT_FLD(field_name, "READ_VIEW_LOW_LIMIT_TRX_NUMBER"),
     STRUCT_FLD(field_length, TRX_ID_MAX_LEN + 1),
     STRUCT_FLD(field_type, MYSQL_TYPE_STRING), STRUCT_FLD(value, 0),
     STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),
     STRUCT_FLD(open_method, 0)},

#define READ_VIEW_UPPER_LIMIT_ID 1
    {STRUCT_FLD(field_name, "READ_VIEW_UPPER_LIMIT_TRX_ID"),
     STRUCT_FLD(field_length, TRX_ID_MAX_LEN + 1),
     STRUCT_FLD(field_type, MYSQL_TYPE_STRING), STRUCT_FLD(value, 0),
     STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),
     STRUCT_FLD(open_method, 0)},

#define READ_VIEW_LOW_LIMIT_ID 2
    {STRUCT_FLD(field_name, "READ_VIEW_LOW_LIMIT_TRX_ID"),

     STRUCT_FLD(field_length, TRX_ID_MAX_LEN + 1),
     STRUCT_FLD(field_type, MYSQL_TYPE_STRING), STRUCT_FLD(value, 0),
     STRUCT_FLD(field_flags, 0), STRUCT_FLD(old_name, ""),
     STRUCT_FLD(open_method, 0)},

    END_OF_ST_FIELD_INFO};

static int xtradb_read_view_fill_table(THD *thd, TABLE_LIST *tables, Item *) {
  DBUG_ENTER("xtradb_read_view_fill_table");

  /* deny access to non-superusers */
  if (check_global_access(thd, PROCESS_ACL)) {
    DBUG_RETURN(0);
  }

  i_s_xtradb_read_view_t read_view;
  if (read_fill_i_s_xtradb_read_view(&read_view) == nullptr) DBUG_RETURN(0);

  TABLE *table = tables->table;
  Field **fields = table->field;

  char trx_id[TRX_ID_MAX_LEN + 1];
  snprintf(trx_id, sizeof(trx_id), TRX_ID_FMT, read_view.low_limit_no);
  OK(field_store_string(fields[READ_VIEW_LOW_LIMIT_NUMBER], trx_id));

  snprintf(trx_id, sizeof(trx_id), TRX_ID_FMT, read_view.up_limit_id);
  OK(field_store_string(fields[READ_VIEW_UPPER_LIMIT_ID], trx_id));

  snprintf(trx_id, sizeof(trx_id), TRX_ID_FMT, read_view.low_limit_id);
  OK(field_store_string(fields[READ_VIEW_LOW_LIMIT_ID], trx_id));

  OK(schema_table_store_record(thd, table));

  DBUG_RETURN(0);
}

static int xtradb_read_view_init(void *p) {
  DBUG_ENTER("xtradb_read_view_init");

  ST_SCHEMA_TABLE *schema = (ST_SCHEMA_TABLE *)p;

  schema->fields_info = xtradb_read_view_fields_info;
  schema->fill_table = xtradb_read_view_fill_table;

  DBUG_RETURN(0);
}

static struct st_mysql_information_schema i_s_info = {
    MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION};

struct st_mysql_plugin i_s_xtradb_read_view = {
    STRUCT_FLD(type, MYSQL_INFORMATION_SCHEMA_PLUGIN),
    STRUCT_FLD(info, &i_s_info),
    STRUCT_FLD(name, "XTRADB_READ_VIEW"),
    STRUCT_FLD(author, PLUGIN_AUTHOR),
    STRUCT_FLD(descr, "InnoDB Read View information"),
    STRUCT_FLD(license, PLUGIN_LICENSE_GPL),
    STRUCT_FLD(init, xtradb_read_view_init),
    nullptr,
    STRUCT_FLD(deinit, i_s_common_deinit),
    STRUCT_FLD(version, INNODB_VERSION_SHORT),
    STRUCT_FLD(status_vars, nullptr),
    STRUCT_FLD(system_vars, nullptr),
    STRUCT_FLD(__reserved1, nullptr),
    STRUCT_FLD(flags, 0UL),
};
