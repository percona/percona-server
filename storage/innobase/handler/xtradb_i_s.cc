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

#include <mysqld_error.h>
#include <sql_acl.h>				// PROCESS_ACL

#include <m_ctype.h>
#include <hash.h>
#include <myisampack.h>
#include <mysys_err.h>
#include <my_sys.h>
#include "i_s.h"
#include <sql_plugin.h>
#include <mysql/innodb_priv.h>

#include <read0i_s.h>
#include <trx0i_s.h>
#include "srv0start.h"	/* for srv_was_started */
#include <btr0pcur.h> /* btr_pcur_t */
#include <btr0sea.h> /* btr_search_sys */
#include <log0recv.h> /* recv_sys */
#include <fil0fil.h>
#include <dict0crea.h> /* for ZIP_DICT_MAX_* constants */

/* for XTRADB_RSEG table */
#include "trx0trx.h" /* for TRX_QUE_STATE_STR_MAX_LEN */
#include "trx0rseg.h" /* for trx_rseg_struct */
#include "trx0sys.h" /* for trx_sys */

#define PLUGIN_AUTHOR "Percona Inc."

#define OK(expr)		\
	if ((expr) != 0) {	\
		DBUG_RETURN(1);	\
	}

#if !defined __STRICT_ANSI__ && defined __GNUC__ && (__GNUC__) > 2 &&	\
	!defined __INTEL_COMPILER && !defined __clang__
#define STRUCT_FLD(name, value)	name: value
#else
#define STRUCT_FLD(name, value)	value
#endif

#define END_OF_ST_FIELD_INFO \
	{STRUCT_FLD(field_name,		NULL), \
	 STRUCT_FLD(field_length,	0), \
	 STRUCT_FLD(field_type,		MYSQL_TYPE_NULL), \
	 STRUCT_FLD(value,		0), \
	 STRUCT_FLD(field_flags,	0), \
	 STRUCT_FLD(old_name,		""), \
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)}


/*******************************************************************//**
Auxiliary function to store ulint value in MYSQL_TYPE_LONGLONG field.
If the value is ULINT_UNDEFINED then the field it set to NULL.
@return	0 on success */
static
int
field_store_ulint(
/*==============*/
	Field*	field,	/*!< in/out: target field for storage */
	ulint	n)	/*!< in: value to store */
{
	int	ret;

	if (n != ULINT_UNDEFINED) {

		ret = field->store(n);
		field->set_notnull();
	} else {

		ret = 0; /* success */
		field->set_null();
	}

	return(ret);
}

/*******************************************************************//**
Auxiliary function to store char* value in MYSQL_TYPE_STRING field.
@return	0 on success */
static
int
field_store_string(
/*===============*/
	Field*		field,	/*!< in/out: target field for storage */
	const char*	str)	/*!< in: NUL-terminated utf-8 string,
				or NULL */
{
	int	ret;

	if (str != NULL) {

		ret = field->store(str, strlen(str),
				   system_charset_info);
		field->set_notnull();
	} else {

		ret = 0; /* success */
		field->set_null();
	}

	return(ret);
}
/** Auxiliary function to store (char*, len) value in MYSQL_TYPE_BLOB
field.
@return	0 on success */
static
int
field_store_blob(
	Field*		field,		/*!< in/out: target field for storage */
	const char*	data,		/*!< in: pointer to data, or NULL */
	uint		data_len)	/*!< in: data length */
{
	int	ret;

	if (data != NULL) {
		ret = field->store(data, data_len, system_charset_info);
		field->set_notnull();
	} else {
		ret = 0; /* success */
		field->set_null();
	}

	return(ret);
}

static
int
i_s_common_deinit(
/*==============*/
	void*	p)	/*!< in/out: table schema object */
{
	DBUG_ENTER("i_s_common_deinit");

	/* Do nothing */

	DBUG_RETURN(0);
}

static ST_FIELD_INFO xtradb_read_view_fields_info[] =
{
#define READ_VIEW_LOW_LIMIT_NUMBER	0
	{STRUCT_FLD(field_name,		"READ_VIEW_LOW_LIMIT_TRX_NUMBER"),
	 STRUCT_FLD(field_length,	TRX_ID_MAX_LEN + 1),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_STRING),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	0),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

#define READ_VIEW_UPPER_LIMIT_ID	1
	{STRUCT_FLD(field_name,		"READ_VIEW_UPPER_LIMIT_TRX_ID"),
	 STRUCT_FLD(field_length,	TRX_ID_MAX_LEN + 1),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_STRING),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	0),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

#define READ_VIEW_LOW_LIMIT_ID		2
	{STRUCT_FLD(field_name,		"READ_VIEW_LOW_LIMIT_TRX_ID"),

	 STRUCT_FLD(field_length,	TRX_ID_MAX_LEN + 1),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_STRING),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	0),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	END_OF_ST_FIELD_INFO
};

static int xtradb_read_view_fill_table(THD* thd, TABLE_LIST* tables, Item*)
{
	Field**	fields;
	TABLE* table;
	char		trx_id[TRX_ID_MAX_LEN + 1];


	DBUG_ENTER("xtradb_read_view_fill_table");

	/* deny access to non-superusers */
	if (check_global_access(thd, PROCESS_ACL)) {

		DBUG_RETURN(0);
	}

	table = tables->table;
	fields = table->field;

	i_s_xtradb_read_view_t read_view;

	if (read_fill_i_s_xtradb_read_view(&read_view) == NULL)
		DBUG_RETURN(0);

	ut_snprintf(trx_id, sizeof(trx_id), TRX_ID_FMT, read_view.low_limit_no);
	OK(field_store_string(fields[READ_VIEW_LOW_LIMIT_NUMBER], trx_id));

	ut_snprintf(trx_id, sizeof(trx_id), TRX_ID_FMT, read_view.up_limit_id);
	OK(field_store_string(fields[READ_VIEW_UPPER_LIMIT_ID], trx_id));

	ut_snprintf(trx_id, sizeof(trx_id), TRX_ID_FMT, read_view.low_limit_id);
	OK(field_store_string(fields[READ_VIEW_LOW_LIMIT_ID], trx_id));

	OK(schema_table_store_record(thd, table));

	DBUG_RETURN(0);
}


static int xtradb_read_view_init(void* p)
{
	ST_SCHEMA_TABLE*	schema;

	DBUG_ENTER("xtradb_read_view_init");

	schema = (ST_SCHEMA_TABLE*) p;

	schema->fields_info = xtradb_read_view_fields_info;
	schema->fill_table = xtradb_read_view_fill_table;

	DBUG_RETURN(0);
}

static struct st_mysql_information_schema i_s_info =
{
	MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION
};

struct st_mysql_plugin i_s_xtradb_read_view =
{
	STRUCT_FLD(type, MYSQL_INFORMATION_SCHEMA_PLUGIN),
	STRUCT_FLD(info, &i_s_info),
	STRUCT_FLD(name, "XTRADB_READ_VIEW"),
	STRUCT_FLD(author, PLUGIN_AUTHOR),
	STRUCT_FLD(descr, "InnoDB Read View information"),
	STRUCT_FLD(license, PLUGIN_LICENSE_GPL),
	STRUCT_FLD(init, xtradb_read_view_init),
	STRUCT_FLD(deinit, i_s_common_deinit),
	STRUCT_FLD(version, INNODB_VERSION_SHORT),
	STRUCT_FLD(status_vars, NULL),
	STRUCT_FLD(system_vars, NULL),
	STRUCT_FLD(__reserved1, NULL),
	STRUCT_FLD(flags, 0UL),
};

static ST_FIELD_INFO xtradb_internal_hash_tables_fields_info[] =
{
#define INT_HASH_TABLES_NAME		0
	{STRUCT_FLD(field_name,		"INTERNAL_HASH_TABLE_NAME"),
	 STRUCT_FLD(field_length,	100),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_STRING),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	0),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

#define INT_HASH_TABLES_TOTAL		1
	{STRUCT_FLD(field_name,		"TOTAL_MEMORY"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

#define INT_HASH_TABLES_CONSTANT		2
	{STRUCT_FLD(field_name,		"CONSTANT_MEMORY"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

#define INT_HASH_TABLES_VARIABLE		3
	{STRUCT_FLD(field_name,		"VARIABLE_MEMORY"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	END_OF_ST_FIELD_INFO
};

static int xtradb_internal_hash_tables_fill_table(THD* thd, TABLE_LIST* tables, Item*)
{
	Field**		fields;
	TABLE*		table;

	DBUG_ENTER("xtradb_internal_hash_tables_fill_table");

	/* deny access to non-superusers */
	if (check_global_access(thd, PROCESS_ACL)) {

		DBUG_RETURN(0);
	}

	table = tables->table;
	fields = table->field;

	/* Calculate AHI constant and variable memory allocations */

	ulong btr_search_sys_constant = btr_search_sys_constant_mem;
	os_rmb;
	ulong btr_search_sys_variable = btr_search_sys_variable_mem;
	size_t dict_sys_hash_size = dict_sys->hash_size;
	ulong dict_sys_size = dict_sys->size;

	OK(field_store_string(fields[INT_HASH_TABLES_NAME],
			      "Adaptive hash index"));
	OK(field_store_ulint(fields[INT_HASH_TABLES_TOTAL],
			     btr_search_sys_variable + btr_search_sys_constant));
	OK(field_store_ulint(fields[INT_HASH_TABLES_CONSTANT],
			     btr_search_sys_constant));
	OK(field_store_ulint(fields[INT_HASH_TABLES_VARIABLE],
			     btr_search_sys_variable));
	OK(schema_table_store_record(thd, table));

	{
	  OK(field_store_string(fields[INT_HASH_TABLES_NAME],
				"Page hash (buffer pool 0 only)"));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_TOTAL],
			       (ulong) (buf_pool_from_array(0)->page_hash->n_cells * sizeof(hash_cell_t))));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_CONSTANT],
			       (ulong) (buf_pool_from_array(0)->page_hash->n_cells * sizeof(hash_cell_t))));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_VARIABLE], 0));
	  OK(schema_table_store_record(thd, table));

	}

	OK(field_store_string(fields[INT_HASH_TABLES_NAME],
			      "Dictionary Cache"));
	OK(field_store_ulint(fields[INT_HASH_TABLES_TOTAL],
			     dict_sys_hash_size + dict_sys_size));
	OK(field_store_ulint(fields[INT_HASH_TABLES_CONSTANT],
			     dict_sys_hash_size));
	OK(field_store_ulint(fields[INT_HASH_TABLES_VARIABLE],
			     dict_sys_size));
	OK(schema_table_store_record(thd, table));

	{
	  OK(field_store_string(fields[INT_HASH_TABLES_NAME],
				"File system"));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_TOTAL],
			       (ulong) (fil_system_hash_cells()
					* sizeof(hash_cell_t)
					+ fil_system_hash_nodes())));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_CONSTANT],
			       (ulong) (fil_system_hash_cells() * sizeof(hash_cell_t))));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_VARIABLE],
			       (ulong) fil_system_hash_nodes()));
	  OK(schema_table_store_record(thd, table));

	}

	{
	  ulint lock_sys_constant, lock_sys_variable;

	  trx_i_s_get_lock_sys_memory_usage(&lock_sys_constant,
					    &lock_sys_variable);

	  OK(field_store_string(fields[INT_HASH_TABLES_NAME], "Lock System"));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_TOTAL],
			       lock_sys_constant + lock_sys_variable));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_CONSTANT],
			       lock_sys_constant));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_VARIABLE],
			       lock_sys_variable));
	  OK(schema_table_store_record(thd, table));
	}

	if (recv_sys)
	{
	  ulint recv_sys_subtotal = ((recv_sys && recv_sys->addr_hash)
				     ? mem_heap_get_size(recv_sys->heap) : 0);

	  OK(field_store_string(fields[INT_HASH_TABLES_NAME], "Recovery System"));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_TOTAL],
			       ((recv_sys->addr_hash) ? (recv_sys->addr_hash->n_cells * sizeof(hash_cell_t)) : 0) + recv_sys_subtotal));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_CONSTANT],
			       ((recv_sys->addr_hash) ? (recv_sys->addr_hash->n_cells * sizeof(hash_cell_t)) : 0)));
	  OK(field_store_ulint(fields[INT_HASH_TABLES_VARIABLE],
			       recv_sys_subtotal));
	  OK(schema_table_store_record(thd, table));
	}

	DBUG_RETURN(0);
}

static int xtradb_internal_hash_tables_init(void* p)
{
	ST_SCHEMA_TABLE*	schema;

	DBUG_ENTER("xtradb_internal_hash_tables_init");

	schema = (ST_SCHEMA_TABLE*) p;

	schema->fields_info = xtradb_internal_hash_tables_fields_info;
	schema->fill_table = xtradb_internal_hash_tables_fill_table;

	DBUG_RETURN(0);
}

struct st_mysql_plugin i_s_xtradb_internal_hash_tables =
{
	STRUCT_FLD(type, MYSQL_INFORMATION_SCHEMA_PLUGIN),
	STRUCT_FLD(info, &i_s_info),
	STRUCT_FLD(name, "XTRADB_INTERNAL_HASH_TABLES"),
	STRUCT_FLD(author, PLUGIN_AUTHOR),
	STRUCT_FLD(descr, "InnoDB internal hash tables information"),
	STRUCT_FLD(license, PLUGIN_LICENSE_GPL),
	STRUCT_FLD(init, xtradb_internal_hash_tables_init),
	STRUCT_FLD(deinit, i_s_common_deinit),
	STRUCT_FLD(version, INNODB_VERSION_SHORT),
	STRUCT_FLD(status_vars, NULL),
	STRUCT_FLD(system_vars, NULL),
	STRUCT_FLD(__reserved1, NULL),
	STRUCT_FLD(flags, 0UL),
};


/***********************************************************************
*/
static ST_FIELD_INFO	i_s_xtradb_rseg_fields_info[] =
{
	{STRUCT_FLD(field_name,		"rseg_id"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	{STRUCT_FLD(field_name,		"space_id"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	{STRUCT_FLD(field_name,		"physical_page_size"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	{STRUCT_FLD(field_name,		"logical_page_size"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	{STRUCT_FLD(field_name,		"is_compressed"),
	 STRUCT_FLD(field_length,	1),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_TINY),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	{STRUCT_FLD(field_name,		"page_no"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	{STRUCT_FLD(field_name,		"max_size"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	{STRUCT_FLD(field_name,		"curr_size"),
	 STRUCT_FLD(field_length,	MY_INT64_NUM_DECIMAL_DIGITS),
	 STRUCT_FLD(field_type,		MYSQL_TYPE_LONGLONG),
	 STRUCT_FLD(value,		0),
	 STRUCT_FLD(field_flags,	MY_I_S_UNSIGNED),
	 STRUCT_FLD(old_name,		""),
	 STRUCT_FLD(open_method,	SKIP_OPEN_TABLE)},

	END_OF_ST_FIELD_INFO
};

static
int
i_s_xtradb_rseg_fill(
/*=================*/
	THD*		thd,	/* in: thread */
	TABLE_LIST*	tables,	/* in/out: tables to fill */
	Item*		)	/* in: condition (ignored) */
{
	TABLE*	table	= (TABLE *) tables->table;
	int	status	= 0;
	trx_rseg_t*	rseg;

	DBUG_ENTER("i_s_xtradb_rseg_fill");

	/* deny access to non-superusers */
	if (check_global_access(thd, PROCESS_ACL)) {

		DBUG_RETURN(0);
	}

	for(int i=0; i < TRX_SYS_N_RSEGS; i++)
	{
	  rseg = trx_sys->rseg_array[i];
	  if (!rseg)
	    continue;

	  table->field[0]->store(rseg->id);
	  table->field[1]->store(rseg->space);
	  table->field[2]->store(rseg->page_size.physical());
	  table->field[3]->store(rseg->page_size.logical());
	  table->field[4]->store(rseg->page_size.is_compressed());
	  table->field[5]->store(rseg->page_no);
	  table->field[6]->store(rseg->max_size);
	  table->field[7]->store(rseg->curr_size);

	  if (schema_table_store_record(thd, table)) {
	    status = 1;
	    break;
	  }
	}

	DBUG_RETURN(status);
}

static
int
i_s_xtradb_rseg_init(
/*=================*/
			/* out: 0 on success */
	void*	p)	/* in/out: table schema object */
{
	DBUG_ENTER("i_s_xtradb_rseg_init");
	ST_SCHEMA_TABLE* schema = (ST_SCHEMA_TABLE*) p;

	schema->fields_info = i_s_xtradb_rseg_fields_info;
	schema->fill_table = i_s_xtradb_rseg_fill;

	DBUG_RETURN(0);
}

struct st_mysql_plugin	i_s_xtradb_rseg =
{
	STRUCT_FLD(type, MYSQL_INFORMATION_SCHEMA_PLUGIN),
	STRUCT_FLD(info, &i_s_info),
	STRUCT_FLD(name, "XTRADB_RSEG"),
	STRUCT_FLD(author, PLUGIN_AUTHOR),
	STRUCT_FLD(descr, "InnoDB rollback segment information"),
	STRUCT_FLD(license, PLUGIN_LICENSE_GPL),
	STRUCT_FLD(init, i_s_xtradb_rseg_init),
	STRUCT_FLD(deinit, i_s_common_deinit),
	STRUCT_FLD(version, INNODB_VERSION_SHORT),
	STRUCT_FLD(status_vars, NULL),
	STRUCT_FLD(system_vars, NULL),
	STRUCT_FLD(__reserved1, NULL),
	STRUCT_FLD(flags, 0UL),
};


/************************************************************************/
enum zip_dict_field_type
{
	zip_dict_field_id,
	zip_dict_field_name,
	zip_dict_field_zip_dict
};

static ST_FIELD_INFO xtradb_sys_zip_dict_fields_info[] =
{
	{ STRUCT_FLD(field_name, "id"),
	STRUCT_FLD(field_length, MY_INT64_NUM_DECIMAL_DIGITS),
	STRUCT_FLD(field_type, MYSQL_TYPE_LONGLONG),
	STRUCT_FLD(value, 0),
	STRUCT_FLD(field_flags, MY_I_S_UNSIGNED),
	STRUCT_FLD(old_name, ""),
	STRUCT_FLD(open_method, SKIP_OPEN_TABLE) },

	{ STRUCT_FLD(field_name, "name"),
	STRUCT_FLD(field_length, ZIP_DICT_MAX_NAME_LENGTH),
	STRUCT_FLD(field_type, MYSQL_TYPE_STRING),
	STRUCT_FLD(value, 0),
	STRUCT_FLD(field_flags, 0),
	STRUCT_FLD(old_name, ""),
	STRUCT_FLD(open_method, SKIP_OPEN_TABLE) },

	{ STRUCT_FLD(field_name, "zip_dict"),
	STRUCT_FLD(field_length, ZIP_DICT_MAX_DATA_LENGTH),
	STRUCT_FLD(field_type, MYSQL_TYPE_BLOB),
	STRUCT_FLD(value, 0),
	STRUCT_FLD(field_flags, 0),
	STRUCT_FLD(old_name, ""),
	STRUCT_FLD(open_method, SKIP_OPEN_TABLE) },

	END_OF_ST_FIELD_INFO
};

/** Function to fill INFORMATION_SCHEMA.XTRADB_ZIP_DICT with information
collected by scanning SYS_ZIP_DICT table.
@return 0 on success */
static
int
xtradb_i_s_dict_fill_sys_zip_dict(
	THD*		thd,		/*!< in: thread */
	ulint		id,		/*!< in: dict ID */
	const char*	name,		/*!< in: dict name */
	const char*	data,		/*!< in: dict data */
	ulint		data_len,	/*!< in: dict data length */
	TABLE*		table_to_fill)	/*!< in/out: fill this table */
{
	DBUG_ENTER("xtradb_i_s_dict_fill_sys_zip_dict");

	Field**	fields = table_to_fill->field;

	OK(field_store_ulint(fields[zip_dict_field_id], id));
	OK(field_store_string(fields[zip_dict_field_name], name));
	OK(field_store_blob(fields[zip_dict_field_zip_dict], data,
		data_len));

	OK(schema_table_store_record(thd, table_to_fill));

	DBUG_RETURN(0);
}

/** Function to populate INFORMATION_SCHEMA.XTRADB_ZIP_DICT table.
Loop through each record in SYS_ZIP_DICT, and extract the column
information and fill the INFORMATION_SCHEMA.XTRADB_ZIP_DICT table.
@return 0 on success */
static
int
xtradb_i_s_sys_zip_dict_fill_table(
	THD*		thd,	/*!< in: thread */
	TABLE_LIST*	tables,	/*!< in/out: tables to fill */
	Item*		)	/*!< in: condition (not used) */
{
	btr_pcur_t	pcur;
	const rec_t*	rec;
	mem_heap_t*	heap;
	mtr_t		mtr;

	DBUG_ENTER("xtradb_i_s_sys_zip_dict_fill_table");

	/* deny access to user without SUPER_ACL privilege */
	if (check_global_access(thd, SUPER_ACL)) {
		DBUG_RETURN(0);
	}

	heap = mem_heap_create(1000);
	mutex_enter(&dict_sys->mutex);
	mtr_start(&mtr);

	rec = dict_startscan_system(&pcur, &mtr, SYS_ZIP_DICT);
	page_size_t page_size = dict_table_page_size(
		pcur.btr_cur.index->table);

	while (rec) {
		const char*	err_msg;
		ulint		id;
		const char*	name;
		const char*	data;
		ulint		data_len;

		/* Extract necessary information from a SYS_ZIP_DICT row */
		err_msg = dict_process_sys_zip_dict(
			heap, page_size, rec, &id, &name, &data, &data_len);

		mtr_commit(&mtr);
		mutex_exit(&dict_sys->mutex);

		if (!err_msg) {
			xtradb_i_s_dict_fill_sys_zip_dict(
				thd, id, name, data, data_len,
				tables->table);
		} else {
			push_warning_printf(thd,
				Sql_condition::SL_WARNING,
				ER_CANT_FIND_SYSTEM_REC, "%s", err_msg);
		}

		mem_heap_empty(heap);

		/* Get the next record */
		mutex_enter(&dict_sys->mutex);
		mtr_start(&mtr);
		rec = dict_getnext_system(&pcur, &mtr);
	}

	mtr_commit(&mtr);
	mutex_exit(&dict_sys->mutex);
	mem_heap_free(heap);

	DBUG_RETURN(0);
}

static int i_s_xtradb_zip_dict_init(void* p)
{
	DBUG_ENTER("i_s_xtradb_zip_dict_init");

	ST_SCHEMA_TABLE* schema = static_cast<ST_SCHEMA_TABLE*>(p);

	schema->fields_info = xtradb_sys_zip_dict_fields_info;
	schema->fill_table = xtradb_i_s_sys_zip_dict_fill_table;

	DBUG_RETURN(0);
}

struct st_mysql_plugin	i_s_xtradb_zip_dict =
{
	STRUCT_FLD(type, MYSQL_INFORMATION_SCHEMA_PLUGIN),
	STRUCT_FLD(info, &i_s_info),
	STRUCT_FLD(name, "XTRADB_ZIP_DICT"),
	STRUCT_FLD(author, PLUGIN_AUTHOR),
	STRUCT_FLD(descr, "InnoDB compression dictionaries information"),
	STRUCT_FLD(license, PLUGIN_LICENSE_GPL),
	STRUCT_FLD(init, i_s_xtradb_zip_dict_init),
	STRUCT_FLD(deinit, i_s_common_deinit),
	STRUCT_FLD(version, INNODB_VERSION_SHORT),
	STRUCT_FLD(status_vars, NULL),
	STRUCT_FLD(system_vars, NULL),
	STRUCT_FLD(__reserved1, NULL),
	STRUCT_FLD(flags, 0UL),
};

enum zip_dict_cols_field_type
{
	zip_dict_cols_field_table_id,
	zip_dict_cols_field_column_pos,
	zip_dict_cols_field_dict_id
};

static ST_FIELD_INFO xtradb_sys_zip_dict_cols_fields_info[] =
{
	{ STRUCT_FLD(field_name, "table_id"),
	STRUCT_FLD(field_length, MY_INT64_NUM_DECIMAL_DIGITS),
	STRUCT_FLD(field_type, MYSQL_TYPE_LONGLONG),
	STRUCT_FLD(value, 0),
	STRUCT_FLD(field_flags, MY_I_S_UNSIGNED),
	STRUCT_FLD(old_name, ""),
	STRUCT_FLD(open_method, SKIP_OPEN_TABLE) },

	{ STRUCT_FLD(field_name, "column_pos"),
	STRUCT_FLD(field_length, MY_INT64_NUM_DECIMAL_DIGITS),
	STRUCT_FLD(field_type, MYSQL_TYPE_LONGLONG),
	STRUCT_FLD(value, 0),
	STRUCT_FLD(field_flags, MY_I_S_UNSIGNED),
	STRUCT_FLD(old_name, ""),
	STRUCT_FLD(open_method, SKIP_OPEN_TABLE) },

	{ STRUCT_FLD(field_name, "dict_id"),
	STRUCT_FLD(field_length, MY_INT64_NUM_DECIMAL_DIGITS),
	STRUCT_FLD(field_type, MYSQL_TYPE_LONGLONG),
	STRUCT_FLD(value, 0),
	STRUCT_FLD(field_flags, MY_I_S_UNSIGNED),
	STRUCT_FLD(old_name, ""),
	STRUCT_FLD(open_method, SKIP_OPEN_TABLE) },

	END_OF_ST_FIELD_INFO
};

/** Function to fill INFORMATION_SCHEMA.XTRADB_ZIP_DICT_COLS with information
collected by scanning SYS_ZIP_DICT_COLS table.
@return 0 on success */
static
int
xtradb_i_s_dict_fill_sys_zip_dict_cols(
	THD*		thd,		/*!< in: thread */
	ulint		table_id,	/*!< in: table ID */
	ulint		column_pos,	/*!< in: column position */
	ulint		dict_id,	/*!< in: dict ID */
	TABLE*		table_to_fill)	/*!< in/out: fill this table */
{
	DBUG_ENTER("xtradb_i_s_dict_fill_sys_zip_dict_cols");

	Field**	fields = table_to_fill->field;

	OK(field_store_ulint(fields[zip_dict_cols_field_table_id],
		table_id));
	OK(field_store_ulint(fields[zip_dict_cols_field_column_pos],
		column_pos));
	OK(field_store_ulint(fields[zip_dict_cols_field_dict_id],
		dict_id));

	OK(schema_table_store_record(thd, table_to_fill));

	DBUG_RETURN(0);
}

/** Function to populate INFORMATION_SCHEMA.XTRADB_ZIP_DICT_COLS table.
Loop through each record in SYS_ZIP_DICT_COLS, and extract the column
information and fill the INFORMATION_SCHEMA.XTRADB_ZIP_DICT_COLS table.
@return 0 on success */
static
int
xtradb_i_s_sys_zip_dict_cols_fill_table(
	THD*		thd,	/*!< in: thread */
	TABLE_LIST*	tables,	/*!< in/out: tables to fill */
	Item*		)	/*!< in: condition (not used) */
{
	btr_pcur_t	pcur;
	const rec_t*	rec;
	mem_heap_t*	heap;
	mtr_t		mtr;

	DBUG_ENTER("xtradb_i_s_sys_zip_dict_cols_fill_table");

	/* deny access to user without SUPER_ACL privilege */
	if (check_global_access(thd, SUPER_ACL)) {
		DBUG_RETURN(0);
	}

	heap = mem_heap_create(1000);
	mutex_enter(&dict_sys->mutex);
	mtr_start(&mtr);

	rec = dict_startscan_system(&pcur, &mtr, SYS_ZIP_DICT_COLS);

	while (rec) {
		const char*	err_msg;
		ulint table_id;
		ulint column_pos;
		ulint dict_id;

		/* Extract necessary information from a SYS_ZIP_DICT_COLS
		row */
		err_msg = dict_process_sys_zip_dict_cols(
			heap, rec, &table_id, &column_pos, &dict_id);

		mtr_commit(&mtr);
		mutex_exit(&dict_sys->mutex);

		if (!err_msg) {
			xtradb_i_s_dict_fill_sys_zip_dict_cols(
				thd, table_id, column_pos, dict_id,
				tables->table);
		} else {
			push_warning_printf(thd,
				Sql_condition::SL_WARNING,
				ER_CANT_FIND_SYSTEM_REC, "%s", err_msg);
		}

		mem_heap_empty(heap);

		/* Get the next record */
		mutex_enter(&dict_sys->mutex);
		mtr_start(&mtr);
		rec = dict_getnext_system(&pcur, &mtr);
	}

	mtr_commit(&mtr);
	mutex_exit(&dict_sys->mutex);
	mem_heap_free(heap);

	DBUG_RETURN(0);
}

static int i_s_xtradb_zip_dict_cols_init(void* p)
{
	DBUG_ENTER("i_s_xtradb_zip_dict_cols_init");

	ST_SCHEMA_TABLE* schema = static_cast<ST_SCHEMA_TABLE*>(p);

	schema->fields_info = xtradb_sys_zip_dict_cols_fields_info;
	schema->fill_table = xtradb_i_s_sys_zip_dict_cols_fill_table;

	DBUG_RETURN(0);
}

struct st_mysql_plugin	i_s_xtradb_zip_dict_cols =
{
	STRUCT_FLD(type, MYSQL_INFORMATION_SCHEMA_PLUGIN),
	STRUCT_FLD(info, &i_s_info),
	STRUCT_FLD(name, "XTRADB_ZIP_DICT_COLS"),
	STRUCT_FLD(author, PLUGIN_AUTHOR),
	STRUCT_FLD(descr, "InnoDB compressed columns information"),
	STRUCT_FLD(license, PLUGIN_LICENSE_GPL),
	STRUCT_FLD(init, i_s_xtradb_zip_dict_cols_init),
	STRUCT_FLD(deinit, i_s_common_deinit),
	STRUCT_FLD(version, INNODB_VERSION_SHORT),
	STRUCT_FLD(status_vars, NULL),
	STRUCT_FLD(system_vars, NULL),
	STRUCT_FLD(__reserved1, NULL),
	STRUCT_FLD(flags, 0UL),
};
