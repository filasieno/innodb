/*****************************************************************************

Copyright (c) 1995, 2025, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/// @file os_sync.cpp
/// \brief The interface to the operating system synchronization primitives
///
/// Created 9/6/1995 Heikki Tuuri

#include "os_sync.hpp"
#ifdef IB_DO_NOT_INLINE
#include "os0sync.inl"
#endif

#include "ut_mem.hpp"
#include "srv_start.hpp"

/* Type definition for an operating system mutex struct */
struct os_mutex_struct{
	os_event_t	event;	/*!< Used by sync0arr.c for queing threads */
	void*		handle;	/*!< OS handle to mutex */
	ulint		count;	/*!< we use this counter to check
				that the same thread does not
				recursively lock the mutex: we
				do not assume that the OS mutex
				supports recursive locking, though
				NT seems to do that */
	UT_LIST_NODE_T(os_mutex_str_t) os_mutex_list;
				/* list of all 'slow' OS mutexes created */
};

/** Mutex protecting counts and the lists of OS mutexes and events */
IB_INTERN os_mutex_t	os_sync_mutex;
/** TRUE if os_sync_mutex has been initialized */
static ibool		os_sync_mutex_inited = FALSE;
/** TRUE when os_sync_free() is being executed */
static ibool		os_sync_free_called	 = FALSE;

/** This is incremented by 1 in os_thread_create and decremented by 1 in
os_thread_exit */
IB_INTERN ulint	state->os.thread_count		= 0;

/** The list of all events created */
static UT_LIST_BASE_NODE_T(os_event_struct_t)	os_event_list;

/** The list of all OS 'slow' mutexes */
static UT_LIST_BASE_NODE_T(os_mutex_str_t)	os_mutex_list;

IB_INTERN ulint	state->os.event_count		= 0;
IB_INTERN ulint	state->os.mutex_count		= 0;
IB_INTERN ulint	state->os.fast_mutex_count	= 0;

/* Because a mutex is embedded inside an event and there is an
event embedded inside a mutex, on free, this generates a recursive call.
This version of the free event function doesn't acquire the global lock */
static void os_event_free_internal(os_event_t	event);

/// \brief Reset the variables.
IB_INTERN
void os_sync_var_init(void)
{
	os_sync_mutex = NULL;
	os_sync_mutex_inited = FALSE;
	os_sync_free_called = FALSE;
	state->os.thread_count = 0;

	memset(&os_event_list, 0x0, sizeof(os_event_list));
	memset(&os_mutex_list, 0x0, sizeof(os_mutex_list));

	state->os.event_count		= 0;
	state->os.mutex_count		= 0;
	state->os.fast_mutex_count	= 0;
}

/// \brief Initializes global event and OS 'slow' mutex lists.
IB_INTERN
void os_sync_init(void)
{
	UT_LIST_INIT(os_event_list);
	UT_LIST_INIT(os_mutex_list);

	os_sync_mutex = NULL;
	os_sync_mutex_inited = FALSE;

	os_sync_mutex = os_mutex_create(NULL);

	os_sync_mutex_inited = TRUE;
}

/// \brief Frees created events and OS 'slow' mutexes
IB_INTERN void os_sync_free(void)
{
	os_event_t	event;
	os_mutex_t	mutex;
	os_sync_free_called = TRUE;
	event = UT_LIST_GET_FIRST(os_event_list);
	while (event) {
		os_event_free(event);
		event = UT_LIST_GET_FIRST(os_event_list);
	}
	mutex = UT_LIST_GET_FIRST(os_mutex_list);
	while (mutex) {
		if (mutex == os_sync_mutex) {
			/* Set the flag to FALSE so that we do not try to reserve os_sync_mutex any more in remaining freeing operations in shutdown */
			os_sync_mutex_inited = FALSE;
		}
		os_mutex_free(mutex);
		mutex = UT_LIST_GET_FIRST(os_mutex_list);
	}
	os_sync_free_called = FALSE;
}

/// \brief Creates an event semaphore, i.e., a semaphore which may just have two states: signaled and nonsignaled. 
/// The created event is manual reset: it must be reset explicitly by calling sync_os_reset_event.
/// \param [in] name The name of the event, if NULL the event is created without a name
/// \return	the event handle
IB_INTERN os_event_t os_event_create(const char* name)
{
	os_event_t	event;
	UT_NOT_USED(name);
	event = ut_malloc(sizeof(struct os_event_struct));
	os_fast_mutex_init(&(event->os_mutex));
	ut_a(0 == pthread_cond_init(&(event->cond_var), NULL));
	event->is_set = FALSE;
	// We return this value in os_event_reset(), which can then be be used to pass to the os_event_wait_low(). The value of zero is reserved in os_event_wait_low() for the case when the
	// caller does not want to pass any signal_count value. To distinguish between the two cases we initialize signal_count to 1 here.
	event->signal_count = 1;
	// The os_sync_mutex can be NULL because during startup an event
	// can be created [ because it's embedded in the mutex/rwlock ] before
	// this module has been initialized
	if (os_sync_mutex != NULL) {
		os_mutex_enter(os_sync_mutex);
	}
	// Put to the list of events
	UT_LIST_ADD_FIRST(os_event_list, os_event_list, event);
	state->os.event_count++;
	if (os_sync_mutex != NULL) {
		os_mutex_exit(os_sync_mutex);
	}
	return event;
}


/// \brief Sets an event semaphore to the signaled state: lets waiting threads proceed
IB_INTERN void os_event_set(os_event_t	event)	/*!< in: event to set */
{
	ut_a(event);
	os_fast_mutex_lock(&(event->os_mutex));
	if (event->is_set) {
		/* Do nothing */
	} else {
		event->is_set = TRUE;
		event->signal_count += 1;
		ut_a(0 == pthread_cond_broadcast(&(event->cond_var)));
	}
	os_fast_mutex_unlock(&(event->os_mutex));
}
/**
Resets an event semaphore to the nonsignaled state. Waiting threads will
stop to wait for the event.
The return value should be passed to os_even_wait_low() if it is desired
that this thread should not wait in case of an intervening call to
os_event_set() between this os_event_reset() and the
os_event_wait_low() call. See comments for os_event_wait_low().
@return	current signal_count. */
IB_INTERN ib_int64_t os_event_reset(os_event_t event)	/*!< in: event to reset */
{
	ib_int64_t ret = 0;
	ut_a(event);
	os_fast_mutex_lock(&(event->os_mutex));
	if (!event->is_set) {
		/* Do nothing */
	} else {
		event->is_set = FALSE;
	}
	ret = event->signal_count;
	os_fast_mutex_unlock(&(event->os_mutex));
	return(ret);
}

/// \brief Frees an event object, without acquiring the global lock. 
/// \param [in] event The event to free
static void os_event_free_internal(os_event_t event)
{
	ut_a(event);
	// This is to avoid freeing the mutex twice 
	os_fast_mutex_free(&(event->os_mutex));
	ut_a(0 == pthread_cond_destroy(&(event->cond_var)));
	// Remove from the list of events
	UT_LIST_REMOVE(os_event_list, os_event_list, event);
	state->os.event_count--;
	ut_free(event);
}

/// \brief Frees an event object.
/// \param [in] event The event to free
IB_INTERN void os_event_free(os_event_t	event)
{
	ut_a(event);
	os_fast_mutex_free(&(event->os_mutex));
	ut_a(0 == pthread_cond_destroy(&(event->cond_var)));
	// Remove from the list of events
	os_mutex_enter(os_sync_mutex);
	UT_LIST_REMOVE(os_event_list, os_event_list, event);
	state->os.event_count--;
	os_mutex_exit(os_sync_mutex);
	ut_free(event);
}

/// \brief Waits for an event object until it is in the signaled state.
///	Waits for an event object until it is in the signaled state. If
///	srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS this also exits the
///	waiting thread when the event becomes signaled (or immediately if the
///	event is already in the signaled state).
///	Typically, if the event has been signalled after the os_event_reset()
///	we'll return immediately because event->is_set == TRUE.
///	There are, however, situations (e.g.: sync_array code) where we may
///	lose this information. For example:
///	thread A calls os_event_reset()
///	thread B calls os_event_set()   [event->is_set == TRUE]
///	thread C calls os_event_reset() [event->is_set == FALSE]
///	thread A calls os_event_wait()  [infinite wait!]
///	thread C calls os_event_wait()  [infinite wait!]
///	Where such a scenario is possible, to avoid infinite wait, the
///	value returned by os_event_reset() should be passed in as
///	reset_sig_count. 
/// \param [in] event The event to wait
/// \param [in] reset_sig_count The value returned by previous call of os_event_reset(), zero or the value returned by previous call of os_event_reset()
IB_INTERN void os_event_wait_low(os_event_t event, ib_int64_t reset_sig_count)
{
	ib_int64_t	old_signal_count;
	os_fast_mutex_lock(&(event->os_mutex));
	if (reset_sig_count) {
		old_signal_count = reset_sig_count;
	} else {
		old_signal_count = event->signal_count;
	}
	for (;;) {
		if (event->is_set == TRUE || event->signal_count != old_signal_count) {
			os_fast_mutex_unlock(&(event->os_mutex));
			if (srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS) {
				os_thread_exit(NULL);
			}
			// Ok, we may return 
			return;
		}
		pthread_cond_wait(&(event->cond_var), &(event->os_mutex));
		// Solaris manual said that spurious wakeups may occur: we
		// have to check if the event really has been signaled after
		// we came here to wait
	}
}


/// \brief Creates an operating system mutex semaphore. 
/// \details Because these are slow, the mutex semaphore of InnoDB itself (ib_mutex_t) should be used where possible.
/// \param [in] name The name of the mutex, if NULL the mutex is created without a name
/// \return	the mutex handle
IB_INTERN os_mutex_t os_mutex_create()
{
	os_fast_mutex_t* mutex = ut_malloc(sizeof(os_fast_mutex_t));
	os_fast_mutex_init(mutex);
	os_mutex_t* mutex_str = ut_malloc(sizeof(os_mutex_str_t));
	mutex_str->handle = mutex;
	mutex_str->count = 0;
	mutex_str->event = os_event_create(NULL);
	if (IB_LIKELY(os_sync_mutex_inited)) {
		// When creating os_sync_mutex itself we cannot reserve it
		os_mutex_enter(os_sync_mutex);
	}
	UT_LIST_ADD_FIRST(os_mutex_list, os_mutex_list, mutex_str);
	state->os.mutex_count++;
	if (IB_LIKELY(os_sync_mutex_inited)) {
		os_mutex_exit(os_sync_mutex);
	}
	return mutex_str;
}

/// \brief Acquires ownership of a mutex semaphore. 
/// \param [in] mutex The mutex to acquire
IB_INTERN void os_mutex_enter(os_mutex_t mutex)
{
	os_fast_mutex_lock(mutex->handle);
	mutex->count++;
	ut_a(mutex->count == 1);
}

/// \brief Releases ownership of a mutex. 
/// \param [in] mutex The mutex to release
IB_INTERN void os_mutex_exit(os_mutex_t mutex)
{
	ut_a(mutex);
	ut_a(mutex->count == 1);
	mutex->count--;
	os_fast_mutex_unlock(mutex->handle);
}

/// \brief Frees a mutex object. 
/// \param [in] mutex The mutex to free
IB_INTERN void os_mutex_free(os_mutex_t mutex)
{
	ut_a(mutex);
	if (IB_LIKELY(!os_sync_free_called)) {
		os_event_free_internal(mutex->event);
	}
	if (IB_LIKELY(os_sync_mutex_inited)) {
		os_mutex_enter(os_sync_mutex);
	}
	UT_LIST_REMOVE(os_mutex_list, os_mutex_list, mutex);
	state->os.mutex_count--;
	if (IB_LIKELY(os_sync_mutex_inited)) {
		os_mutex_exit(os_sync_mutex);
	}
	os_fast_mutex_free(mutex->handle);
	ut_free(mutex->handle);
	ut_free(mutex);
}

/// \brief Initializes an operating system fast mutex semaphore. 
/// \param [in] fast_mutex The fast mutex to initialize
IB_INTERN void os_fast_mutex_init(os_fast_mutex_t* fast_mutex)
{
	ut_a(pthread_mutex_init(fast_mutex, NULL) == 0);
	if (IB_LIKELY(os_sync_mutex_inited)) {
		// When creating os_sync_mutex itself (in Unix) we cannot reserve it
		os_mutex_enter(os_sync_mutex);
	}
	state->os.fast_mutex_count++;
	if (IB_LIKELY(os_sync_mutex_inited)) {
		os_mutex_exit(os_sync_mutex);
	}
}

/// \brief Acquires ownership of a fast mutex. 
/// \param [in] fast_mutex The fast mutex to acquire
IB_INTERN void os_fast_mutex_lock(os_fast_mutex_t* fast_mutex)
{
	pthread_mutex_lock(fast_mutex);
}

/// \brief Releases ownership of a fast mutex. 
/// \param [in] fast_mutex The fast mutex to release
IB_INTERN void os_fast_mutex_unlock(os_fast_mutex_t* fast_mutex)
{
	pthread_mutex_unlock(fast_mutex);
}


IB_INTERN void os_fast_mutex_free(innodb_state* state, os_fast_mutex_t* fast_mutex)
{
	int ret = pthread_mutex_destroy(fast_mutex);
	if (IB_UNLIKELY(ret != 0)) {
		ut_print_timestamp(state->stream);
		ib_log(state, "  InnoDB: error: return value %lu when calling pthread_mutex_destroy()\n", , (ulint)ret);
		ib_log(state, "InnoDB: Byte contents of the pthread mutex at %p:\n", (void*) fast_mutex);
		ut_print_buf(state->stream, fast_mutex, sizeof(os_fast_mutex_t));
		ib_log(state, "\n");
	}
	if (IB_LIKELY(os_sync_mutex_inited)) {
		// When freeing the last mutexes, we have already freed os_sync_mutex
		os_mutex_enter(os_sync_mutex);
	}
	ut_ad(state->os.fast_mutex_count > 0);
	state->os.fast_mutex_count--;
	if (IB_LIKELY(os_sync_mutex_inited)) {
		os_mutex_exit(os_sync_mutex);
	}
}
