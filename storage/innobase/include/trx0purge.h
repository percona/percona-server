/*****************************************************************************

Copyright (c) 1996, 2017, Oracle and/or its affiliates. All Rights Reserved.

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
@file include/trx0purge.h
Purge old versions

Created 3/26/1996 Heikki Tuuri
*******************************************************/

#ifndef trx0purge_h
#define trx0purge_h

#include "univ.i"
#include "trx0types.h"
#include "mtr0mtr.h"
#include "trx0sys.h"
#include "que0types.h"
#include "page0page.h"
#include "usr0sess.h"
#include "fil0fil.h"
#include "read0types.h"
#include "srv0start.h"
#include "ut0new.h"

/** The global data structure coordinating a purge */
extern trx_purge_t*	purge_sys;

/** A dummy undo record used as a return value when we have a whole undo log
which needs no purge */
extern trx_undo_rec_t	trx_purge_dummy_rec;

/********************************************************************//**
Calculates the file address of an undo log header when we have the file
address of its history list node.
@return file address of the log */
UNIV_INLINE
fil_addr_t
trx_purge_get_log_from_hist(
/*========================*/
	fil_addr_t	node_addr);	/*!< in: file address of the history
					list node of the log */
/********************************************************************//**
Creates the global purge system control structure and inits the history
mutex. */
void
trx_purge_sys_create(
/*=================*/
	ulint		n_purge_threads,/*!< in: number of purge threads */
	purge_pq_t*	purge_queue);	/*!< in/own: UNDO log min binary heap*/
/********************************************************************//**
Frees the global purge system control structure. */
void
trx_purge_sys_close(void);
/*======================*/
/************************************************************************
Adds the update undo log as the first log in the history list. Removes the
update undo log segment from the rseg slot if it is too big for reuse. */
void
trx_purge_add_update_undo_to_history(
/*=================================*/
	trx_t*		trx,		/*!< in: transaction */
	trx_undo_ptr_t*	undo_ptr,	/*!< in: update undo log. */
	page_t*		undo_page,	/*!< in: update undo log header page,
					x-latched */
	bool		update_rseg_history_len,
					/*!< in: if true: update rseg history
					len else skip updating it. */
	ulint		n_added_logs,	/*!< in: number of logs added */
	mtr_t*		mtr);		/*!< in: mtr */
/*******************************************************************//**
This function runs a purge batch.
@return number of undo log pages handled in the batch */
ulint
trx_purge(
/*======*/
	ulint	n_purge_threads,	/*!< in: number of purge tasks to
					submit to task queue. */
	ulint	limit,			/*!< in: the maximum number of
					records to purge in one batch */
	bool	truncate);		/*!< in: truncate history if true */
/*******************************************************************//**
Stop purge and wait for it to stop, move to PURGE_STATE_STOP. */
void
trx_purge_stop(void);
/*================*/
/*******************************************************************//**
Resume purge, move to PURGE_STATE_RUN. */
void
trx_purge_run(void);
/*================*/

/** Purge states */
enum purge_state_t {
	PURGE_STATE_INIT,		/*!< Purge instance created */
	PURGE_STATE_RUN,		/*!< Purge should be running */
	PURGE_STATE_STOP,		/*!< Purge should be stopped */
	PURGE_STATE_EXIT,		/*!< Purge has been shutdown */
	PURGE_STATE_DISABLED		/*!< Purge was never started */
};

/*******************************************************************//**
Get the purge state.
@return purge state. */
purge_state_t
trx_purge_state(void);
/*=================*/

// Forward declaration
struct TrxUndoRsegsIterator;

/** This is the purge pointer/iterator. We need both the undo no and the
transaction no up to which purge has parsed and applied the records. */
struct purge_iter_t {
	purge_iter_t()
		:
		trx_no(),
		undo_no(),
		undo_rseg_space(ULINT_UNDEFINED)
	{
		// Do nothing
	}

	trx_id_t	trx_no;		/*!< Purge has advanced past all
					transactions whose number is less
					than this */
	undo_no_t	undo_no;	/*!< Purge has advanced past all records
					whose undo number is less than this */
	ulint		undo_rseg_space;
					/*!< Last undo record resided in this
					space id. */
};

/* Namespace to hold all the related functions and variables needed
to truncate an undo tablespace. */
namespace undo {

	typedef std::vector<ulint>		undo_spaces_t;
	typedef	std::vector<trx_rseg_t*>	rseg_for_trunc_t;

	/** Magic Number to indicate truncate action is complete. */
	const ib_uint32_t			s_magic = 76845412;

	/** Truncate Log file Prefix. */
	const char* const			s_log_prefix = "undo_";

	/** Truncate Log file Extension. */
	const char* const			s_log_ext = "trunc.log";

	/** Check if the space_id is an undo space ID in the reserved range.
	@param[in]	space_id	undo tablespace ID
	@return true if it is in the reserved undo space ID range. */
	inline bool is_reserved(space_id_t space_id)
	{
		return(space_id >= dict_sys_t::s_min_undo_space_id
		       && space_id <= dict_sys_t::s_max_undo_space_id);
	}

	/** Convert an undo space number (from 1 to 127) into an undo space_id.
	@param[in]	space_num	undo tablespace number
	@return space_id of the undo tablespace */
	inline space_id_t num2id(space_id_t space_num)
	{
		ut_ad(space_num > 0);
		ut_ad(space_num <= FSP_MAX_UNDO_TABLESPACES);

		return(static_cast<space_id_t>(
				dict_sys_t::s_log_space_first_id - space_num));
	}

	/** Convert an undo space ID into an undo space number.
	NOTE: This may be an undo space_id from a pre-exisiting 5.7
	database which used space_ids from 1 to 128.  If so, the
	space_id is the space_num.
	@param[in]	space_id	undo tablespace ID
	@return space number of the undo tablespace */
	inline space_id_t id2num(space_id_t space_id)
	{
		if (!is_reserved(space_id)) {
			return(space_id);
		}

		return(dict_sys_t::s_log_space_first_id - space_id);
	}

	/** An undo::Tablespace object is used to easily convert between
	undo_space_id and undo_space_num and to create the automatic file_name
	and space name.  In addition, it is used in undo::Tablespaces to track
	the trx_rseg_t objects in an Rsegs vector. So we do not allocate the
	Rsegs vector for each object, only when requested by the constructor. */
	struct Tablespace
	{
		/** Constructor
		@param[in]	id		tablespace id
		@param[in]	use_rsegs	Tue if rsegs will be tracked */
		explicit Tablespace(space_id_t id, bool use_rsegs = false)
			:
			m_id(id),
			m_num(undo::id2num(id)),
			m_space_name(),
			m_file_name(),
			m_log_file_name(),
			m_rsegs()
		{
			/* This object is used to track rollback segments
			only in the global undo::Tablespaces object. */
			if (use_rsegs) {
				ut_ad(id == 0 || is_reserved(id));

				m_rsegs = UT_NEW_NOKEY(TrxUndoRsegs());
			}
		}

		/** Destructor */
		~Tablespace()
		{
			if (m_space_name != NULL) {
				ut_free(m_space_name);
				m_space_name = NULL;
			}

			if (m_file_name != NULL) {
				ut_free(m_file_name);
				m_file_name = NULL;
			}

			if (m_log_file_name != NULL) {
				ut_free(m_log_file_name);
				m_log_file_name = NULL;
			}

			/* Clear the cached rollback segments.  */
			if (m_rsegs != NULL) {
				UT_DELETE(m_rsegs);
				m_rsegs = NULL;
			}
		};

		/** Build a standard undo tablespace name from a space_id.
		@param[in]	space_id	id of the undo tablespace.
		@return tablespace name of the undo tablespace file */
		char* make_space_name(space_id_t space_id);

		/** Get the undo tablespace name. Make it if not yet made.
		NOTE: This is only called from stack objects so there is no
		race condition. If it is ever called from a shared object
		like undo::spaces, then it must be protected by the caller.
		@return tablespace name created from the space_id */
		char* space_name()
		{
			if (m_space_name == NULL) {
				m_space_name = make_space_name(m_id);
			}

			return(m_space_name);
		}

		/** Build a standard undo tablespace file name from a space_id.
		@param[in]	space_id	id of the undo tablespace.
		@return file_name of the undo tablespace file */
		char* make_file_name(space_id_t space_id);

		/** Get the undo space filename. Make it if not yet made.
		NOTE: This is only called from stack objects so there is no
		race condition. If it is ever called from a shared object
		like undo::spaces, then it must be protected by the caller.
		@return tablespace filename created from the space_id */
		char* file_name()
		{
			if (m_file_name == NULL) {
				m_file_name = make_file_name(m_id);
			}

			return(m_file_name);
		}

		/** Build a log file name based on space_id
		@param[in]	space_id	id of the undo tablespace.
		@return DB_SUCCESS or error code */
		char* make_log_file_name(space_id_t space_id);

		/** Get the undo log filename. Make it if not yet made.
		NOTE: This is only called from stack objects so there is no
		race condition. If it is ever called from a shared object
		like undo::spaces, then it must be protected by the caller.
		@return tablespace filename created from the space_id */
		char* log_file_name()
		{
			if (m_log_file_name == NULL) {
				m_log_file_name = make_log_file_name(m_id);
			}

			return(m_log_file_name);
		}

		/** Get the undo tablespace ID.
		@return tablespace ID */
		space_id_t id()
		{
			return(m_id);
		}

		/** Get the undo tablespace number.  This is the same as m_id
		if m_id is 0 or this is a v5.6-5.7 undo tablespace. v8+ undo
		tablespaces use a space_id from the reserved range.
		@return undo tablespace number */
		space_id_t num()
		{
			ut_ad(m_num < FSP_MAX_ROLLBACK_SEGMENTS);

			return(m_num);
		}

		/** Get a reference to the List of rollback segments within
		this undo tablespace.
		@return a reference to the Rsegs vector. */
		TrxUndoRsegs* rsegs()
		{
			return(m_rsegs);
		}

	private:
		/** Undo Tablespace ID. */
		space_id_t	m_id;

		/** Undo Tablespace number, from 1 to 127. This is the
		7-bit number that is used in a rollback pointer.
		Use id2num() to get this number from a space_id. */
		space_id_t	m_num;

		/** The tablespace name, auto-generated when needed from
		the space number. */
		char*		m_space_name;

		/** The tablespace file name, auto-generated when needed
		from the space number. */
		char*		m_file_name;

		/** The tablespace log file name, auto-generated when needed
		from the space number. */
		char*		m_log_file_name;

		/** List of rollback segments within this tablespace.
		This is not always used. Must call init_rsegs to use it. */
		TrxUndoRsegs*		m_rsegs;
	};

	typedef std::vector<Tablespace*, ut_allocator<Tablespace*> > Tablespaces_Vector;

	/** List of undo tablespaces, each containing a list of
	rollback segments. */
	class Tablespaces
	{
	public:
		Tablespaces_Vector	m_spaces;

		Tablespaces()
		{
			init();
		}

		~Tablespaces()
		{
			deinit();
		}

		/** Initialize */
		void init();

		/** De-initialize */
		void deinit();

		/** Clear the contents of the list of Tablespace objects.
		This does not deallocate any memory. */
		void clear() {
			// for (Tablespace* undo_space : m_spaces) {
			for (Tablespaces_Vector::iterator undo_space_ptr =
				m_spaces.begin();
				undo_space_ptr != m_spaces.end();
				++undo_space_ptr)
			{
				Tablespace* undo_space = *undo_space_ptr;
				UT_DELETE(undo_space);
			}
			m_spaces.clear();
		}

		/** Get the number of tablespaces tracked by this object. */
		ulint size() {
			return(m_spaces.size());
		}

		/** See if the list of tablespaces is empty. */
		bool empty() {
			return(m_spaces.empty());
		}

		/** Get the Tablespace tracked at a position. */
		Tablespace* at(size_t pos) {
			return(m_spaces.at(pos));
		}

		/** Add a new space_id to the back of the vector.
		The vector has been pre-allocated to 128 so read threads will
		not loose what is pointed to.
		@param[in]	id	tablespace ID */
		void add(space_id_t id);

		/** Check if the given space_is is in the vector.
		@return true if space_id is found, else false */
		bool contains(space_id_t id) {
			return(find(id) != NULL);
		}

		/** Find the given space_id in the vector.
		@return pointer to an undo::Tablespace struct */
		Tablespace* find(space_id_t id) {
			if (m_spaces.empty()) {
				return(NULL);
			}

			/* The sort method above puts this vector in order by
			Tablespace::num. If there are no gaps, then we should
			be able to find it quickly. */
			space_id_t	slot = id2num(id) - 1;
			if (slot < m_spaces.size()) {
				Tablespace* undo_space = m_spaces.at(slot);
				if (undo_space->id() == id) {
					return(undo_space);
				}
			}

			/* If there are gaps in the numbering, do a search. */
			//for (Tablespace* undo_space : m_spaces) {
			for (Tablespaces_Vector::iterator undo_space_ptr =
				m_spaces.begin();
				undo_space_ptr != m_spaces.end();
				++undo_space_ptr)
			{
				Tablespace* undo_space = *undo_space_ptr;
				if (undo_space->id() == id) {
					return(undo_space);
				}
			}

			return(NULL);
		}

#ifdef UNIV_DEBUG
		/** Determine if this thread owns a lock on m_latch. */
		bool own_latch()
		{
			return(rw_lock_own(m_latch, RW_LOCK_X)
			       || rw_lock_own(m_latch, RW_LOCK_S));
		}
#endif /* UNIV_DEBUG */

		/** Get a shared lock on m_spaces. */
		void s_lock()
		{
			rw_lock_s_lock(m_latch);
		}

		/** Release a shared lock on m_spaces. */
		void s_unlock()
		{
			rw_lock_s_unlock(m_latch);
		}

		/** Get an exclusive lock on m_spaces. */
		void x_lock()
		{
			rw_lock_x_lock(m_latch);
		}

		/** Release an exclusive lock on m_spaces. */
		void x_unlock()
		{
			rw_lock_x_unlock(m_latch);
		}

	private:
		/** RW lock to protect m_spaces.
		x for adding elements, s for scanning, size() etc. */
		rw_lock_t*	m_latch;
	};

	/** A global object that contains a vector of undo::Tablespace structs. */
	extern Tablespaces*	spaces;

	/** Create the truncate log file.
	@param[in]	space_id	id of the undo tablespace to truncate.
	@return DB_SUCCESS or error code. */
	dberr_t start_logging(space_id_t space_id);

	/** Mark completion of undo truncate action by writing magic number
	to the log file and then removing it from the disk.
	If we are going to remove it from disk then why write magic number?
	This is to safeguard from unlink (file-system) anomalies that will
	keep the link to the file even after unlink action is successfull
	and ref-count = 0.
	@param[in]	space_id	ID of the undo tablespace to truncate.*/
	void done_logging(space_id_t space_id);

	/** Check if TRUNCATE_DDL_LOG file exist.
	@param[in]	space_id	ID of the undo tablespace.
	@return true if exist else false. */
	bool is_active_truncate_log_present(space_id_t space_id);

	/** list of undo tablespaces that need header pages and rollback
	segments written to them at startup.  This can be because they are
	newly initialized, were being truncated and the system crashed, or
	they were an old format at startup and were replaced when they were
	opened. Old format undo tablespaces do not have space_ids between
	dict_sys_t::s_min_undo_space_id and dict_sys_t::s_max_undo_space_id
	and they do not contain an RSEG_ARRAY page. */
	extern Space_Ids	s_under_construction;

	/** Add undo tablespace to s_under_construction vector.
	@param[in]	space_id	space id of tablespace to
	truncate */
	void add_space_to_construction_list(space_id_t space_id);

	/** Clear the s_under_construction vector. */
	void clear_construction_list();

	/** Is an undo tablespace under constuction at the moment.
	@param[in]	space_id	space id to check
	@return true if marked for truncate, else false. */
	bool is_under_construction(space_id_t space_id);

	/* Return whether the undo tablespace is active.
	@return true if active */
	bool is_active(space_id_t space_id);

	/* Return whether the undo tablespace is inactive.
	@return true if inactive */
	inline bool is_inactive(space_id_t space_id) {
		return(!is_active(space_id));
	}

	/** Populate log file name based on space_id
	@param[in]	space_id	id of the undo tablespace.
	@return DB_SUCCESS or error code */
	dberr_t populate_log_file_name(
		ulint	space_id,
		char*&	log_file_name);

	/** Create the truncate log file.
	@param[in]	space_id	id of the undo tablespace to truncate.
	@return DB_SUCCESS or error code. */
	dberr_t init(ulint space_id);

	/** Mark completion of undo truncate action by writing magic number to
	the log file and then removing it from the disk.
	If we are going to remove it from disk then why write magic number ?
	This is to safeguard from unlink (file-system) anomalies that will keep
	the link to the file even after unlink action is successfull and
	ref-count = 0.
	@param[in]	space_id	id of the undo tablespace to truncate.*/
	void done(ulint	space_id);

	/** Check if TRUNCATE_DDL_LOG file exist.
	@param[in]	space_id	id of the undo tablespace.
	@return true if exist else false. */
	bool is_log_present(ulint space_id);

	/** Track UNDO tablespace mark for truncate. */
	class Truncate {
	public:

		Truncate()
			:
			m_undo_for_trunc(ULINT_UNDEFINED),
			m_rseg_for_trunc(),
			m_scan_start(srv_undo_space_id_start),
			m_purge_rseg_truncate_frequency(
				static_cast<ulint>(
				srv_purge_rseg_truncate_frequency))
		{
			/* Do Nothing. */
		}

		/** Clear the cached rollback segment. Normally done
		when purge is about to shutdown. */
		void clear()
		{
			reset();
			rseg_for_trunc_t	temp;
			m_rseg_for_trunc.swap(temp);
		}

		/** Is tablespace selected for truncate.
		@return true if undo tablespace is marked for truncate */
		bool is_marked() const
		{
			return(!(m_undo_for_trunc == ULINT_UNDEFINED));
		}

		/** Mark the tablespace for truncate.
		@param[in]	undo_id		tablespace for truncate. */
		void mark(ulint undo_id)
		{
			m_undo_for_trunc = undo_id;

			/* We found an UNDO-tablespace to truncate so set the
			local purge rseg truncate frequency to 1. This will help
			accelerate the purge action and in turn truncate. */
			m_purge_rseg_truncate_frequency = 1;
		}

		/** Get the tablespace marked for truncate.
		@return tablespace id marked for truncate. */
		ulint get_marked_space_id() const
		{
			return(m_undo_for_trunc);
		}

		/** Add rseg to truncate vector.
		@param[in,out]	rseg	rseg for truncate */
		void add_rseg_to_trunc(trx_rseg_t* rseg)
		{
			m_rseg_for_trunc.push_back(rseg);
		}

		/** Get number of rsegs registered for truncate.
		@return return number of rseg that belongs to tablespace mark
		for truncate. */
		ulint rsegs_size() const
		{
			return(m_rseg_for_trunc.size());
		}

		/** Get ith registered rseg.
		@param[in]	id	index of rseg to get.
		@return reference to registered rseg. */
		trx_rseg_t* get_ith_rseg(ulint id)
		{
			ut_ad(id < m_rseg_for_trunc.size());
			return(m_rseg_for_trunc.at(id));
		}

		/** Reset for next rseg truncate. */
		void reset()
		{
			m_undo_for_trunc = ULINT_UNDEFINED;
			m_rseg_for_trunc.clear();

			/* Sync with global value as we are done with
			truncate now. */
			m_purge_rseg_truncate_frequency = static_cast<ulint>(
				srv_purge_rseg_truncate_frequency);
		}

		/** Get the tablespace id to start scanning from.
		@return	id of UNDO tablespace to start scanning from. */
		ulint get_scan_start() const
		{
			return(m_scan_start);
		}

		/** Set the tablespace id to start scanning from. */
		void set_scan_start(ulint	space_id)
		{
			m_scan_start = space_id;
		}

		/** Check if the tablespace needs fix-up (based on presence of
		DDL truncate log)
		@param	space_id	space id of the undo tablespace to check
		@return true if fix up is needed else false */
		bool needs_fix_up(ulint	space_id) const
		{
			return(is_log_present(space_id));
		}

		/** Add undo tablespace to truncate vector.
		@param[in]	space_id	space id of tablespace to
						truncate */
		static void add_space_to_trunc_list(ulint space_id)
		{
			s_spaces_to_truncate.push_back(space_id);
		}

		/** Clear the truncate vector. */
		static void clear_trunc_list()
		{
			s_spaces_to_truncate.clear();
		}

		/** Is tablespace marked for truncate.
		@param[in]	space_id	space id to check
		@return true if marked for truncate, else false. */
		static bool is_tablespace_truncated(ulint space_id)
		{
			return(std::find(s_spaces_to_truncate.begin(),
					 s_spaces_to_truncate.end(), space_id)
			       != s_spaces_to_truncate.end());
		}

		/** Was a tablespace truncated at startup
		@param[in]	space_id	space id to check
		@return whether space_id was truncated at startup */
		static bool was_tablespace_truncated(ulint space_id)
		{
			return(std::find(s_fix_up_spaces.begin(),
					 s_fix_up_spaces.end(),
					 space_id)
			       != s_fix_up_spaces.end());
		}

		/** Get local rseg purge truncate frequency
		@return rseg purge truncate frequency. */
		ulint get_rseg_truncate_frequency() const
		{
			return(m_purge_rseg_truncate_frequency);
		}

		/* Start writing log information to a special file.
		On successfull completion, file is removed.
		On crash, file is used to complete the truncate action.
		@param	space_id	space id of undo tablespace
		@return DB_SUCCESS or error code. */
		dberr_t start_logging(ulint space_id)
		{
			return(init(space_id));
		}

		/* Mark completion of logging./
		@param	space_id	space id of undo tablespace */
		void done_logging(ulint space_id)
		{
			return(done(space_id));
		}

	private:
		/** UNDO tablespace is mark for truncate. */
		ulint			m_undo_for_trunc;

		/** rseg that resides in UNDO tablespace is marked for
		truncate. */
		rseg_for_trunc_t	m_rseg_for_trunc;

		/** Start scanning for UNDO tablespace from this space_id.
		This is to avoid bias selection of one tablespace always. */
		ulint			m_scan_start;

		/** Rollback segment(s) purge frequency. This is local
		value maintained along with global value. It is set to global
		value on start but when tablespace is marked for truncate it
		is updated to 1 and then minimum value among 2 is used by
		purge action. */
		ulint			m_purge_rseg_truncate_frequency;

		/** List of UNDO tablespace(s) to truncate. */
		static undo_spaces_t	s_spaces_to_truncate;
	public:
		/** Undo tablespaces that were truncated at startup */
		static undo_spaces_t	s_fix_up_spaces;
	};	/* class Truncate */

};	/* namespace undo */

/** The control structure used in the purge operation */
struct trx_purge_t{
	sess_t*		sess;		/*!< System session running the purge
					query */
	trx_t*		trx;		/*!< System transaction running the
					purge query: this trx is not in the
					trx list of the trx system and it
					never ends */
	rw_lock_t	latch;		/*!< The latch protecting the purge
					view. A purge operation must acquire an
					x-latch here for the instant at which
					it changes the purge view: an undo
					log operation can prevent this by
					obtaining an s-latch here. It also
					protects state and running */
	os_event_t	event;		/*!< State signal event */
	ulint		n_stop;		/*!< Counter to track number stops */
	volatile bool	running;	/*!< true, if purge is active,
					we check this without the latch too */
	volatile purge_state_t	state;	/*!< Purge coordinator thread states,
					we check this in several places
					without holding the latch. */
	que_t*		query;		/*!< The query graph which will do the
					parallelized purge operation */
	ReadView	view;		/*!< The purge will not remove undo logs
					which are >= this view (purge view) */
	bool		view_active;	/*!< true if view is active */
	volatile ulint	n_submitted;	/*!< Count of total tasks submitted
					to the task queue */
	volatile ulint	n_completed;	/*!< Count of total tasks completed */

	/*------------------------------*/
	/* The following two fields form the 'purge pointer' which advances
	during a purge, and which is used in history list truncation */

	purge_iter_t	iter;		/* Limit up to which we have read and
					parsed the UNDO log records.  Not
					necessarily purged from the indexes.
					Note that this can never be less than
					the limit below, we check for this
					invariant in trx0purge.cc */
	purge_iter_t	limit;		/* The 'purge pointer' which advances
					during a purge, and which is used in
					history list truncation */
#ifdef UNIV_DEBUG
	purge_iter_t	done;		/* Indicate 'purge pointer' which have
					purged already accurately. */
#endif /* UNIV_DEBUG */
	/*-----------------------------*/
	ibool		next_stored;	/*!< TRUE if the info of the next record
					to purge is stored below: if yes, then
					the transaction number and the undo
					number of the record are stored in
					purge_trx_no and purge_undo_no above */
	trx_rseg_t*	rseg;		/*!< Rollback segment for the next undo
					record to purge */
	ulint		page_no;	/*!< Page number for the next undo
					record to purge, page number of the
					log header, if dummy record */
	ulint		offset;		/*!< Page offset for the next undo
					record to purge, 0 if the dummy
					record */
	ulint		hdr_page_no;	/*!< Header page of the undo log where
					the next record to purge belongs */
	ulint		hdr_offset;	/*!< Header byte offset on the page */


	TrxUndoRsegsIterator*
			rseg_iter;	/*!< Iterator to get the next rseg
					to process */

	purge_pq_t*	purge_queue;	/*!< Binary min-heap, ordered on
					TrxUndoRsegs::trx_no. It is protected
					by the pq_mutex */
	PQMutex		pq_mutex;	/*!< Mutex protecting purge_queue */

	undo::Truncate	undo_trunc;	/*!< Track UNDO tablespace marked
					for truncate. */
};

/** Info required to purge a record */
struct trx_purge_rec_t {
	trx_undo_rec_t*	undo_rec;	/*!< Record to purge */
	roll_ptr_t	roll_ptr;	/*!< File pointr to UNDO record */
};

/**
Chooses the rollback segment with the smallest trx_no. */
struct TrxUndoRsegsIterator {

	/** Constructor */
	TrxUndoRsegsIterator(trx_purge_t* purge_sys);

	/** Sets the next rseg to purge in m_purge_sys.
	@return page size of the table for which the log is.
	NOTE: if rseg is NULL when this function returns this means that
	there are no rollback segments to purge and then the returned page
	size object should not be used. */
	const page_size_t set_next();

private:
	// Disable copying
	TrxUndoRsegsIterator(const TrxUndoRsegsIterator&);
	TrxUndoRsegsIterator& operator=(const TrxUndoRsegsIterator&);

	/** The purge system pointer */
	trx_purge_t*			m_purge_sys;

	/** The current element to process */
	TrxUndoRsegs			m_trx_undo_rsegs;

	/** Track the current element in m_trx_undo_rseg */
	TrxUndoRsegs::iterator		m_iter;

	/** Sentinel value */
	static const TrxUndoRsegs	NullElement;
};

#ifndef UNIV_NONINL
#include "trx0purge.ic"
#endif /* UNIV_NOINL */

#endif /* trx0purge_h */
