/*****************************************************************************
Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.

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
@file include/os0event.h
The interface to the operating system condition variables

Created 2012-09-23 Sunny Bains (split from os0sync.h)
*******************************************************/

#ifndef os0event_h
#define os0event_h

#include "univ.i"

#ifndef UNIV_INNOCHECKSUM

#include "sync0types.h"

typedef OSMutex EventMutex;

/** InnoDB condition variable. */
struct os_event {

	os_event(void) UNIV_NOTHROW;

	~os_event() UNIV_NOTHROW;

	friend void os_event_global_init();
	friend void os_event_global_destroy();

	/**
	Destroys a condition variable */
	void destroy() UNIV_NOTHROW
	{
#ifndef _WIN32
		int	ret = pthread_cond_destroy(&cond_var);
		ut_a(ret == 0);
#endif /* !_WIN32 */

		mutex.destroy();
	}

	/** Set the event */
	void set() UNIV_NOTHROW
	{
		mutex.enter();

		if (UNIV_LIKELY(!is_set())) {
			broadcast();
		}

		mutex.exit();
	}

	int64_t signal_count() const UNIV_NOTHROW
	{
		return (count_and_set & count_mask);
	}

	int64_t reset() UNIV_NOTHROW
	{
		mutex.enter();

		if (UNIV_LIKELY(is_set())) {
			count_and_set &= count_mask;
		}

		int64_t	ret = signal_count();

		mutex.exit();

		return(ret);
	}

	/**
	Waits for an event object until it is in the signaled state.

	Typically, if the event has been signalled after the os_event_reset()
	we'll return immediately because event->m_set == true.
	There are, however, situations (e.g.: sync_array code) where we may
	lose this information. For example:

	thread A calls os_event_reset()
	thread B calls os_event_set()   [event->m_set == true]
	thread C calls os_event_reset() [event->m_set == false]
	thread A calls os_event_wait()  [infinite wait!]
	thread C calls os_event_wait()  [infinite wait!]

	Where such a scenario is possible, to avoid infinite wait, the
	value returned by reset() should be passed in as
	reset_sig_count. */
	void wait_low(int64_t reset_sig_count) UNIV_NOTHROW;

	/**
	Waits for an event object until it is in the signaled state or
	a timeout is exceeded.
	@param time_in_usec - timeout in microseconds,
	or OS_SYNC_INFINITE_TIME
	@param reset_sig_count- zero or the value returned by
	previous call of os_event_reset().
	@return	0 if success, OS_SYNC_TIME_EXCEEDED if timeout was exceeded */
	ulint wait_time_low(
		ulint		time_in_usec,
		int64_t		reset_sig_count) UNIV_NOTHROW;

	/** @return true if the event is in the signalled state. */
	bool is_set() const UNIV_NOTHROW
	{
		return(count_and_set & set_mask);
	}

private:
	/**
	Initialize a condition variable */
	void init() UNIV_NOTHROW
	{

		mutex.init();

#ifdef _WIN32
		InitializeConditionVariable(&cond_var);
#else
		{
			int	ret;

			ret = pthread_cond_init(&cond_var, &cond_attr);
			ut_a(ret == 0);
		}
#endif /* _WIN32 */
	}

	/**
	Wait on condition variable */
	void wait() UNIV_NOTHROW
	{
#ifdef _WIN32
		if (!SleepConditionVariableCS(&cond_var, mutex, INFINITE)) {
			ut_error;
		}
#else
		{
			int	ret;

			ret = pthread_cond_wait(&cond_var, mutex);
			ut_a(ret == 0);
		}
#endif /* _WIN32 */
	}

	/**
	Wakes all threads waiting for condition variable */
	void broadcast() UNIV_NOTHROW
	{
		count_and_set |= set_mask;
		count_and_set++;

#ifdef _WIN32
		WakeAllConditionVariable(&cond_var);
#else
		{
			int	ret;

			ret = pthread_cond_broadcast(&cond_var);
			ut_a(ret == 0);
		}
#endif /* _WIN32 */
	}

	/**
	Wakes one thread waiting for condition variable */
	void signal() UNIV_NOTHROW
	{
#ifdef _WIN32
		WakeConditionVariable(&cond_var);
#else
		{
			int	ret;

			ret = pthread_cond_signal(&cond_var);
			ut_a(ret == 0);
		}
#endif /* _WIN32 */
	}

	/**
	Do a timed wait on condition variable.
	@param abstime - timeout
	@param time_in_ms - timeout in milliseconds.
	@return true if timed out, false otherwise */
	bool timed_wait(
#ifndef _WIN32
		const timespec*	abstime
#else
		DWORD		time_in_ms
#endif /* !_WIN32 */
			);
#ifndef _WIN32
	/** Returns absolute time until which we should wait if
	we wanted to wait for time_in_usec microseconds since now. */
	struct timespec get_wait_timelimit(ulint time_in_usec);
#endif /* !_WIN32 */

private:

	/** Masks for the event signal count and set flag in the count_and_set
	field */
	enum { count_mask = 0x7fffffffffffffffULL,
	       set_mask   = 0x8000000000000000ULL};

	/** The MSB is set whenever when the event is in the signaled state,
	i.e. a thread does not stop if it tries to wait for this event.
	bits are incremented each time the event becomes signaled. */
	uint64_t		count_and_set;

	EventMutex		mutex;		/*!< this mutex protects
						the next fields */

#ifdef _WIN32
/** Native condition variable. */
	typedef CONDITION_VARIABLE	os_cond_t;
#else
/** Native condition variable */
	typedef pthread_cond_t		os_cond_t;
#endif /* _WIN32 */

	os_cond_t		cond_var;	/*!< condition variable is
						used in waiting for the event */
#ifndef _WIN32
	/** Attributes object passed to pthread_cond_* functions.
	Defines usage of the monotonic clock if it's available.
	Initialized once, in the os_event::global_init(), and
	destroyed in the os_event::global_destroy(). */
	static pthread_condattr_t cond_attr;

	/** True iff usage of the monotonic clock has been successfuly
	enabled for the cond_attr object. */
	static bool cond_attr_has_monotonic_clock;
#endif /* !_WIN32 */
	static lock_word_t global_initialized;

	// Disable copy constructor
	os_event(const os_event&);
};
#endif

typedef struct os_event* os_event_t;

/** Denotes an infinite delay for os_event_wait_time() */
#define OS_SYNC_INFINITE_TIME   ULINT_UNDEFINED

/** Return value of os_event_wait_time() when the time is exceeded */
#define OS_SYNC_TIME_EXCEEDED   1

/**
Creates an event semaphore, i.e., a semaphore which may just have two states:
signaled and nonsignaled. The created event is manual reset: it must be reset
explicitly by calling os_event_reset().
@return	the event handle */
os_event_t
os_event_create(
/*============*/
	const char*	name);	/*!< in: the name of the event, if NULL
				the event is created without a name */

/**
Sets an event semaphore to the signaled state: lets waiting threads
proceed. */
void
os_event_set(
/*=========*/
	os_event_t	event);	/*!< in/out: event to set */

/**
Check if the event is set.
@return true if set */
bool
os_event_is_set(
/*============*/
	const os_event_t	event);	/*!< in: event to set */

/**
Resets an event semaphore to the nonsignaled state. Waiting threads will
stop to wait for the event.
The return value should be passed to os_even_wait_low() if it is desired
that this thread should not wait in case of an intervening call to
os_event_set() between this os_event_reset() and the
os_event_wait_low() call. See comments for os_event_wait_low(). */
int64_t
os_event_reset(
/*===========*/
	os_event_t	event);	/*!< in/out: event to reset */

/**
Frees an event object. */
void
os_event_destroy(
/*=============*/
	os_event_t&	event);	/*!< in/own: event to free */

/**
Waits for an event object until it is in the signaled state.

Typically, if the event has been signalled after the os_event_reset()
we'll return immediately because event->is_set == TRUE.
There are, however, situations (e.g.: sync_array code) where we may
lose this information. For example:

thread A calls os_event_reset()
thread B calls os_event_set()   [event->is_set == TRUE]
thread C calls os_event_reset() [event->is_set == FALSE]
thread A calls os_event_wait()  [infinite wait!]
thread C calls os_event_wait()  [infinite wait!]

Where such a scenario is possible, to avoid infinite wait, the
value returned by os_event_reset() should be passed in as
reset_sig_count. */
void
os_event_wait_low(
/*==============*/
	os_event_t	event,		/*!< in/out: event to wait */
	int64_t		reset_sig_count);/*!< in: zero or the value
					returned by previous call of
					os_event_reset(). */

/** Blocking infinite wait on an event, until signealled.
@param e - event to wait on. */
#define os_event_wait(e) os_event_wait_low((e), 0)

/**
Waits for an event object until it is in the signaled state or
a timeout is exceeded. In Unix the timeout is always infinite.
@return 0 if success, OS_SYNC_TIME_EXCEEDED if timeout was exceeded */
ulint
os_event_wait_time_low(
/*===================*/
	os_event_t	event,			/*!< in/out: event to wait */
	ulint		time_in_usec,		/*!< in: timeout in
						microseconds, or
						OS_SYNC_INFINITE_TIME */
	int64_t		reset_sig_count);	/*!< in: zero or the value
						returned by previous call of
						os_event_reset(). */
/** Initializes support for os_event objects. Must be called once,
 and before any os_event object is created. */
void os_event_global_init(void);

/** Deinitializes support for os_event objects. Must be called once,
 and after all os_event objects are destroyed. After it is called, no
new os_event is allowed to be created. */
void os_event_global_destroy(void);

/** Blocking timed wait on an event.
@param e - event to wait on.
@param t - timeout in microseconds */
#define os_event_wait_time(e, t) os_event_wait_time_low((e), (t), 0)

#endif /* !os0event_h */
