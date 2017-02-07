/*****************************************************************************

Copyright (c) 2013, 2014, Oracle and/or its affiliates. All Rights Reserved.

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

/**************************************************//**
@file include/sess0sess.h
InnoDB session state tracker.
Multi file, shared, system tablespace implementation.

Created 2014-04-30 by Krunal Bauskar
*******************************************************/

#ifndef sess0sess_h
#define sess0sess_h

#include "univ.i"
#include "dict0mem.h"
#include "ut0new.h"

#include <map>

class dict_intrinsic_table_t {

public:
	/** Constructor
	@param[in,out]	handler		table handler. */
	dict_intrinsic_table_t(dict_table_t*	handler)
		:
		m_handler(handler)
	{
		/* Do nothing. */
	}

	/** Destructor */
	~dict_intrinsic_table_t()
	{
		m_handler = NULL;
	}

public:

	/* Table Handler holding other metadata information commonly needed
	for any table. */
	dict_table_t*				m_handler;
};

/** InnoDB private data that is cached in THD */
typedef std::map<
	std::string,
	dict_intrinsic_table_t*,
	std::less<std::string>,
	ut_allocator<std::pair<const std::string, dict_intrinsic_table_t*> > >
	table_cache_t;

class innodb_session_t {
	friend class innodb_session_dict_mutex_guard_t;
public:
	/** Constructor */
	innodb_session_t()
		: m_trx(),
		  m_open_tables(),
		  m_dict_mutex_locked(0)
	{
		/* Do nothing. */
	}

	/** Destructor */
	~innodb_session_t()
	{
		m_trx = NULL;

		for (table_cache_t::iterator it = m_open_tables.begin();
		     it != m_open_tables.end();
		     ++it) {
			delete(it->second);
		}
	}

	/** Cache table handler.
	@param[in]	table_name	name of the table
	@param[in,out]	table		table handler to register */
	void register_table_handler(
		const char*	table_name,
		dict_table_t*	table)
	{
		ut_ad(lookup_table_handler(table_name) == NULL);
		m_open_tables.insert(table_cache_t::value_type(
			table_name, new dict_intrinsic_table_t(table)));
	}

	/** Lookup for table handler given table_name.
	@param[in]	table_name	name of the table to lookup */
	dict_table_t* lookup_table_handler(
		const char*	table_name)
	{
		table_cache_t::iterator it = m_open_tables.find(table_name);
		return((it == m_open_tables.end())
		       ? NULL : it->second->m_handler);
	}

	/** Remove table handler entry.
	@param[in]	table_name	name of the table to remove */
	void unregister_table_handler(
		const char*	table_name)
	{
		table_cache_t::iterator it = m_open_tables.find(table_name);
		if (it == m_open_tables.end()) {
			return;
		}

		delete(it->second);
		m_open_tables.erase(table_name);
	}

	/** Count of register table handler.
	@return number of register table handlers */
	uint count_register_table_handler() const
	{
		return(static_cast<uint>(m_open_tables.size()));
	}

	/** Checks the state of the dict_sys mutex.
	@return true, if dict_sys mutex is locked */
	bool is_dict_mutex_locked() const
	{
		return m_dict_mutex_locked != 0;
	}
public:

	/** transaction handler. */
	trx_t*		m_trx;

	/** Handler of tables that are created or open but not added
	to InnoDB dictionary as they are session specific.
	Currently, limited to intrinsic temporary tables only. */
	table_cache_t	m_open_tables;

private:
	/** This counter is used by
	ha_innobase::update_field_defs_with_zip_dict_info() to determine
	whether it needs to acquire dict_sys mutex or not. Non-zero value
	means that this mutex has already been locked by one of the purge
	threads just before calling handler::my_prepare_gcolumn_template() /
	handler::my_eval_gcolumn_expr_with_open() and therefore it must not
	be touched to avoid recursive locking. */
	uint		m_dict_mutex_locked;
};

/** A guard class which sets dict_mutex locked flag for the provided innodb
session object in constructor and unset it in destructor. */
class innodb_session_dict_mutex_guard_t {
public:
	/** Constructor
	@param[in,out]	session	innodb session object. */
	innodb_session_dict_mutex_guard_t(innodb_session_t& session)
		: m_session(session)
	{
		++m_session.m_dict_mutex_locked;
	}

	/** Destructor */
	~innodb_session_dict_mutex_guard_t()
	{
		--m_session.m_dict_mutex_locked;
	}

private:
	/* noncopyable */
	innodb_session_dict_mutex_guard_t(
		const innodb_session_dict_mutex_guard_t&);
	innodb_session_dict_mutex_guard_t& operator = (
		const innodb_session_dict_mutex_guard_t&);

	/** Reference to the innodb session object */
	innodb_session_t& m_session;
};


#endif /* sess0sess_h */
