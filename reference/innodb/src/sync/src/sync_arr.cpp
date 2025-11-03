// Copyright (c) 2025, Fabio N. Filasieno and Roberto Boati
// Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.
// Copyright (c) 2008, Google Inc.
// 
// Portions of this file contain modifications contributed and copyrighted by
// Google, Inc. Those modifications are gratefully acknowledged and are described
// briefly in the InnoDB documentation. The contributions by Google are
// incorporated with their permission, and subject to the conditions contained in
// the file COPYING.Google.
// 
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; version 2 of the License.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA

/// \file sync_arr.c
/// \brief The wait array used in synchronization primitives
/// \details Created 9/5/1995 Heikki Tuuri
/// \author Fabio N. Filasieno
/// \date 2025-10-15

#include "sync_arr.hpp"

#ifdef IB_DO_NOT_INLINE
	#include "sync_arr.inl"
#endif

#include "sync_sync.hpp"
#include "sync_rw.hpp"
#include "os_sync.hpp"
#include "os_file.hpp"
#include "srv_srv.hpp"

#ifdef IB_SYNC_DEBUG

	/// \brief Detect a dead lock in debug
	/// \details This function is called only in the debug version. Detects a deadlock of one or more threads because of waits of semaphores.
	/// \param arr The wait array
	/// \param start The cell where recursive search started
	/// \param cell The cell to search
	/// \param depth The recursion depth
	/// \return	TRUE if a deadlock detected
	static ibool sync_array_detect_deadlock(sync_array_t* arr, sync_cell_t* start, sync_cell_t* cell, ulint depth);	

#endif // IB_SYNC_DEBUG

/// \brief Gets the nth cell in array.
/// \param arr The sync array
/// \param n The index
/// \return The cell
static sync_cell_t* sync_array_get_nth_cell(sync_array_t* arr, ulint n);
{
	ut_a(arr != nullptr);
	ut_a(n < arr->n_cells);
	return arr->array + n;
}

/// \brief Reserves the mutex semaphore protecting a sync array.
/// \param arr The wait array
/// \return The cell
static void sync_array_enter(sync_array_t* arr)
{
	ut_a(arr != nullptr);
	ulint protection = arr->protection;
	if (protection == SYNC_ARRAY_OS_MUTEX) {
		os_mutex_enter(arr->os_mutex);
	} else if (protection == SYNC_ARRAY_MUTEX) {
		mutex_enter(&(arr->mutex));
	} else {
		UT_ERROR;
	}
}
/// \brief Releases the mutex semaphore protecting a sync array
/// \param arr The wait array
static void sync_array_exit(sync_array_t* arr)
{
	ut_a(arr != nullptr);
	ulint protection = arr->protection;
	if (protection == SYNC_ARRAY_OS_MUTEX) {
		os_mutex_exit(arr->os_mutex);
	} else if (protection == SYNC_ARRAY_MUTEX) {
		mutex_exit(&(arr->mutex));
	} else {
		UT_ERROR;
	}
}


IB_INTERN sync_array_t* sync_array_create(ulint n_cells, ulint protection)
	ut_a(n_cells > 0);
	sync_array_t* arr = ut_malloc(sizeof(sync_array_t));
	memset(arr, 0x0, sizeof(*arr));
	ulint sz = sizeof(sync_cell_t) * n_cells;
	arr->array = ut_malloc(sz);
	memset(arr->array, 0x0, sz);
	arr->n_cells = n_cells;
	arr->protection = protection;
	// Then create the mutex to protect the wait array complex
	if (protection == SYNC_ARRAY_OS_MUTEX) {
		arr->os_mutex = os_mutex_create(NULL);
	} else if (protection == SYNC_ARRAY_MUTEX) {
		mutex_create(&arr->mutex, SYNC_NO_ORDER_CHECK);
	} else {
		UT_ERROR;
	}
	return(arr);
}

IB_INTERN void sync_array_free(sync_array_t* arr)	
{
	ut_a(arr->n_reserved == 0);
	sync_array_validate(arr);
	ulint protection = arr->protection;
	// Release the mutex protecting the wait array complex
	if (protection == SYNC_ARRAY_OS_MUTEX) {
		os_mutex_free(arr->os_mutex);
	} else if (protection == SYNC_ARRAY_MUTEX) {
		mutex_free(&(arr->mutex));
	} else {
		UT_ERROR;
	}
	ut_free(arr->array);
	ut_free(arr);
}

IB_INTERN void sync_array_validate(sync_array_t* arr)
{
	ulint count = 0;
	sync_array_enter(arr);
	for (ulint i = 0; i < arr->n_cells; i++) {
		sync_cell_t* cell = sync_array_get_nth_cell(arr, i);
		if (cell->wait_object != NULL) {
			count++;
		}
	}
	ut_a(count == arr->n_reserved);
	sync_array_exit(arr);
}


/// \brief Returns the event that the thread owning the cell waits for.
/// \param [in] cell non-empty sync array cell
static os_event_t sync_cell_get_event(sync_cell_t* cell)
{
	ulint type = cell->request_type;
	switch (type) {
	case SYNC_MUTEX:
		return ((mutex_t *) cell->wait_object)->event;
	case RW_LOCK_WAIT_EX:
		return ((rw_lock_t *) cell->wait_object)->wait_ex_event;
	default:
		// RW_LOCK_SHARED && RW_LOCK_EX wait on the same event
		return ((rw_lock_t *) cell->wait_object)->event;
	}
}

IB_INTERN void sync_array_reserve_cell(sync_array_t* arr, void* object, ulint type, const char*	file, ulint line, ulint* index)
{
	ut_a(object != nullptr);
	ut_a(index != nullptr);
	ut_a(arr != nullptr);
	sync_array_enter(arr);
	arr->res_count++;

	// Reserve a new cell
	for (ulint i = 0; i < arr->n_cells; i++) {
		sync_cell_t* cell = sync_array_get_nth_cell(arr, i);
		if (cell->wait_object == nullptr) {
			cell->waiting = FALSE;
			cell->wait_object = object;
			if (type == SYNC_MUTEX) {
				cell->old_wait_mutex = object;
			} else {
				cell->old_wait_rw_lock = object;
			}
			cell->request_type = type;
			cell->file = file;
			cell->line = line;
			arr->n_reserved++;
			*index = i;
			sync_array_exit(arr);

			// Make sure the event is reset and also store the value of signal_count at which the event was reset
			os_event_t event = sync_cell_get_event(cell);
			cell->signal_count = os_event_reset(event);
			cell->reservation_time = time(NULL);
			cell->thread = os_thread_get_curr_id();
			return;
		}
	}

	UT_ERROR; // No free cell found
	return;
}


/// \brief Wait for an event 
/// \details This function should be called when a thread starts to wait on
/// a wait array cell. In the debug version this function checks
/// if the wait for a semaphore will result in a deadlock, in which
/// case prints info and asserts.
/// TODO: 
/// \param [in] arr wait array
/// \param [in] index index of the reserved cell
IB_INTERN void sync_array_wait_event(sync_array_t* arr, ulint index)
{
	ut_a(arr);

	sync_array_enter(arr);
	{
		sync_cell_t* cell = sync_array_get_nth_cell(arr, index);
		ut_a(cell->wait_object != nullptr);
		ut_a(!cell->waiting);
		ut_ad(os_thread_get_curr_id() == cell->thread);
	
		os_event_t event = sync_cell_get_event(cell);
		cell->waiting = TRUE;
	
		if constexpr (IB_SYNC_DEBUG) {
			// We use simple enter to the mutex below, because if we cannot acquire it at once, 
			// mutex_enter would call recursively sync_array routines, leading to trouble. 
			// `rw_lock_debug_mutex` freezes the debug lists.
			rw_lock_debug_mutex_enter();
			{
				if (sync_array_detect_deadlock(arr, cell, cell, 0) == TRUE) {
					ib_log(state, "Deadlock detected");
					UT_ERROR;
				}
			}
			rw_lock_debug_mutex_exit();
		}
	
	}	
	sync_array_exit(arr);

	os_event_wait_low(event, cell->signal_count);
	sync_array_free_cell(arr, index);
}

/// \brief Reports info of a wait array cell
/// \param [in] state The state
/// \param [in] cell The cell
static void sync_array_cell_print(innodb_t* state, sync_cell_t* cell)
{
	ut_a(state != nullptr);
	ut_a(cell != nullptr);
	ulint type = cell->request_type;
	ib_log(state, "Thread %lu has waited at %s line %lu for %.2f seconds the semaphore:\n", (ulong) os_thread_pf(cell->thread), cell->file, (ulong) cell->line, difftime(time(NULL), cell->reservation_time));
	if (type == SYNC_MUTEX) {
		// We use old_wait_mutex in case the cell has already been freed meanwhile
		mutex_t* mutex = cell->old_wait_mutex;
		if constexpr (IB_SYNC_DEBUG) {
			ib_log(
				state, 
				"Mutex at %p created file %s line %lu, lock var %lu; Last time reserved in file %s line %lu, waiters flag %lu\n",
				(void*) mutex, 
				mutex->cfile_name, 
				(ulong) mutex->cline,
				(ulong) mutex->lock_word,	
				mutex->file_name, 
				(ulong) mutex->line,
				(ulong) mutex->waiters);
		} else {
			ib_log(state,
				"Mutex at %p created file %s line %lu, lock var %lu; waiters flag %lu\n",
				(void*) mutex, mutex->cfile_name, (ulong) mutex->cline,
				(ulong) mutex->lock_word,
				(ulong) mutex->waiters);
		}			
	} else if (type == RW_LOCK_EX || type == RW_LOCK_WAIT_EX || type == RW_LOCK_SHARED) {
		ib_log(state, "%s", type == RW_LOCK_EX ? "X-lock on" : "S-lock on");
		rw_lock_t* rwlock = cell->old_wait_rw_lock;
		ib_log(state,"RW-latch at %p created in file %s line %lu\n", (void*) rwlock, rwlock->cfile_name, (ulong) rwlock->cline);
		ulint writer = rw_lock_get_writer(rwlock);
		if (writer != RW_LOCK_NOT_LOCKED) {
			ib_log(state, "a writer (thread id %lu) has reserved it in mode %s", (ulong) os_thread_pf(rwlock->writer_thread), writer == RW_LOCK_EX ? " exclusive\n" : " wait exclusive\n");
		}
		ib_log(state, 
			"number of readers %lu, waiters flag %lu, lock_word: %lx; Last time read locked in file %s line %lu; Last time write locked in file %s line %lu\n", 
			(ulong) rw_lock_get_reader_count(rwlock), 
			(ulong) rwlock->waiters, rwlock->lock_word,
			rwlock->last_s_file_name,
			(ulong) rwlock->last_s_line,
			rwlock->last_x_file_name,
			(ulong) rwlock->last_x_line);
	} else {
		UT_ERROR;
	}

	if (!cell->waiting) {
		ib_log(state, "wait has ended\n");
	}
}

#ifdef IB_SYNC_DEBUG

	/// \brief Looks for a cell with the given thread id.
	/// \param [in] arr The wait array
	/// \param [in] thread The thread id
	/// \return Pointer to cell or NULL if not found
	static sync_cell_t* sync_array_find_thread(sync_array_t* arr, os_thread_id_t thread)
	{
		for (ulint i = 0; i < arr->n_cells; i++) {
			sync_cell_t* cell = sync_array_get_nth_cell(arr, i);
			if (cell->wait_object != NULL && os_thread_eq(cell->thread, thread)) {
				return cell; /// found
			}
		}
		return nullptr; /// not found
	}

	/// \brief Recursion step for deadlock detection.
	/// \param [in] arr The wait array
	/// \param [in] start The cell where recursive search started
	/// \param [in] thread The thread to look at
	/// \param [in] pass The pass value
	/// \param [in] depth The recursion depth
	/// \return TRUE if deadlock detected
	static ibool sync_array_deadlock_step(sync_array_t* arr, sync_cell_t* start, os_thread_id_t thread, ulint pass, ulint depth)
	{
		depth++;
		if (pass != 0) {
			// If pass != 0, then we do not know which threads are responsible of releasing the lock, and no deadlock can be detected.
			return FALSE;
		}
		sync_cell_t* new_cell = sync_array_find_thread(arr, thread);
		if (new_cell == start) {
			// Stop running of other threads
			ut_dbg_stop_threads = TRUE;
			ib_log(state, "Deadlock of threads detected!\n");
			return TRUE ;
		} else if (new_cell != nullptr) {
			ibool ret = sync_array_detect_deadlock(arr, start, new, depth);
			if (ret) {
				return TRUE;
			}
		}
		return FALSE;
	}



	/// \brief This function is called only in the debug version. Detects a deadlock of one or more threads because of waits of semaphores.
	/// \param [in] arr The wait array; note: the caller must own the mutex to array
	/// \param [in] start The cell where recursive search started
	/// \param [in] cell The cell to search
	/// \param [in] depth The recursion depth
	/// \return	TRUE if deadlock detected
	static ibool sync_array_detect_deadlock(innodb_t* state, sync_array_t* arr, sync_cell_t* start, sync_cell_t* cell, ulint depth)
	{
		ut_a(arr);
		ut_a(start);
		ut_a(cell);
		ut_ad(cell->wait_object);
		ut_ad(os_thread_get_curr_id() == start->thread);
		ut_ad(depth < 100);

		depth++;

		if (!cell->waiting) {
			// No deadlock here
			return FALSE ; 
		}

		// Sync mutex
		if (cell->request_type == SYNC_MUTEX) {
			mutex_t* mutex = cell->wait_object;
			if (mutex_get_lock_word(mutex) != 0) {
				os_thread_id_t thread = mutex->thread_id;
				// Note that mutex->thread_id above may be also OS_THREAD_ID_UNDEFINED, because the thread which held the mutex maybe has not
				// yet updated the value, or it has already released the mutex: in this case no deadlock can occur, as the wait array cannot contain
				// a thread with ID_UNDEFINED value.
				ibool res = sync_array_deadlock_step(state, arr, start, thread, 0, depth);
				if (res) {
					ib_log(state, "Mutex %p owned by thread %lu file %s line %lu\n", mutex, (ulong) os_thread_pf(mutex->thread_id), mutex->file_name, (ulong) mutex->line);
					sync_array_cell_print(state->stream, cell);
					return TRUE;
				}
			}
			// No deadlock
			return FALSE;
		} 
		
		// RW_LOCK_EX || RW_LOCK_WAIT_EX
		if (cell->request_type == RW_LOCK_EX || cell->request_type == RW_LOCK_WAIT_EX) {
			rw_lock_t* lock = cell->wait_object;
			rw_lock_debug_t* debug = UT_LIST_GET_FIRST(lock->debug_list);
			while (debug != NULL) {
				os_thread_id_t thread = debug->thread_id;
				if (((debug->lock_type == RW_LOCK_EX) && !os_thread_eq(thread, cell->thread)) || ((debug->lock_type == RW_LOCK_WAIT_EX) && !os_thread_eq(thread, cell->thread)) || (debug->lock_type == RW_LOCK_SHARED)) {
					// The (wait) x-lock request can block infinitely only if someone (can be also cel thread) is holding s-lock, or someone
					// (cannot be cell thread) (wait) x-lock, and he is blocked by start thread
					ibool ret = sync_array_deadlock_step( arr, start, thread, debug->pass, depth);
					if (ret) {
						ib_log(state, "rw-lock %p ", (void*) lock);
						sync_array_cell_print(state->stream, cell);
						rw_lock_debug_print(debug);
						return TRUE;	
					}
				}
				debug = UT_LIST_GET_NEXT(list, debug);
			}
			return FALSE;

		} 
		
		// RW_LOCK_SHARED
		if (cell->request_type == RW_LOCK_SHARED) {
			rw_lock_t* lock = cell->wait_object;
			rw_lock_debug_t* debug = UT_LIST_GET_FIRST(lock->debug_list);
			while (debug != NULL) {
				os_thread_id_t thread = debug->thread_id;
				if ((debug->lock_type == RW_LOCK_EX) || (debug->lock_type == RW_LOCK_WAIT_EX)) {
					// The s-lock request can block infinitely only if someone (can also be cell thread) is
					// holding (wait) x-lock, and he is blocked by start thread
					ret = sync_array_deadlock_step(arr, start, thread, debug->pass, depth);
					if (ret) {
						ib_log(state, "rw-lock %p ", (void*) lock);
						sync_array_cell_print(state->stream, cell);
						rw_lock_debug_print(debug);
						return TRUE;	
					}
				}
				debug = UT_LIST_GET_NEXT(list, debug);
			}
			return FALSE ;
		} 
		
		// fail
		UT_ERROR;
	}

#endif // IB_SYNC_DEBUG

/// \brief Determines if we can wake up the thread waiting for a sempahore.
/// \param cell The cell to search
/// \return TRUE if we can wake up the thread
static ibool sync_arr_cell_can_wake_up(sync_cell_t* cell)
{
	if (cell->request_type == SYNC_MUTEX) {
		mutex_t* mutex = cell->wait_object;
		if (mutex_get_lock_word(mutex) == 0) {
			return TRUE;
		}
	} else if (cell->request_type == RW_LOCK_EX) {
		rw_lock_t* lock = cell->wait_object;
		if (lock->lock_word > 0) {
		    // Either unlocked or only read locked.
			return TRUE;
		}
	} else if (cell->request_type == RW_LOCK_WAIT_EX) {
		rw_lock_t* lock = cell->wait_object;
		// lock_word == 0 means all readers have left
		if (lock->lock_word == 0) {
			return TRUE;
		}
	} else if (cell->request_type == RW_LOCK_SHARED) {
		rw_lock_t* lock = cell->wait_object;
		// lock_word > 0 means no writer or reserved writer
		if (lock->lock_word > 0) {
			return TRUE;
		}
	}
	return FALSE;
}


/// \brief Frees the cell. 
/// \note sync_array_wait_event frees the cell automatically!
/// \param arr The wait array
/// \param index The index of the cell in array
IB_INTERN void sync_array_free_cell(sync_array_t* arr, ulint index)
{
	sync_array_enter(arr);
	sync_cell_t* cell = sync_array_get_nth_cell(arr, index);
	ut_a(cell->wait_object != NULL);
	cell->waiting = FALSE;
	cell->wait_object =  NULL;
	cell->signal_count = 0;
	ut_a(arr->n_reserved > 0);
	arr->n_reserved--;
	sync_array_exit(arr);
}

/// \brief Increments the signalled count.
/// \param arr The wait array
IB_INTERN void sync_array_object_signalled(sync_array_t* arr)
{
#ifdef IB_HAVE_ATOMIC_BUILTINS
	(void) os_atomic_increment_ulint(&arr->sg_count, 1);
#else
	sync_array_enter(arr);
	arr->sg_count++;
	sync_array_exit(arr);
#endif
}

// If the wakeup algorithm does not work perfectly at semaphore relases,
// this function will do the waking (see the comment in mutex_exit). This
// function should be called about every 1 second in the server.
// 
// Note that there's a race condition between this thread and mutex_exit
// changing the lock_word and calling signal_object, so sometimes this finds
// threads to wake up even when nothing has gone wrong. */
IB_INTERN void sync_arr_wake_threads_if_sema_free(void)
{
	sync_array_t* arr = sync_primary_wait_array;
	ulint i = 0;
	ulint count = 0;
	os_event_t event;

	sync_array_enter(arr);	
	while (count < arr->n_reserved) {
		sync_cell_t* cell = sync_array_get_nth_cell(arr, i);
		i++;
		if (cell->wait_object == NULL) {
			continue;
		}
		count++;
		if (sync_arr_cell_can_wake_up(cell)) {
			event = sync_cell_get_event(cell);
			os_event_set(event);
		}
	}
	sync_array_exit(arr);
}


/// \brief Prints warnings of long semaphore waits to state->stream.
/// \param state The state
/// \return TRUE if fatal semaphore wait threshold was exceeded
IB_INTERN ibool sync_array_print_long_waits(innodb_t* state)
{
	ulint fatal_timeout = srv_fatal_semaphore_wait_threshold;
	ibool fatal = FALSE;
	ibool noticed = FALSE;
	for (ulint i = 0; i < sync_primary_wait_array->n_cells; ++i) {
		sync_cell_t* cell = sync_array_get_nth_cell(sync_primary_wait_array, i);
		if (cell->wait_object != NULL && cell->waiting && difftime(time(NULL), cell->reservation_time) > 240) {
			ib_log(state, "InnoDB: Warning: a long semaphore wait:\n");
			sync_array_cell_print(state->stream, cell);
			noticed = TRUE;
		}
		if (cell->wait_object != NULL && cell->waiting && difftime(time(NULL), cell->reservation_time) > fatal_timeout) {
			fatal = TRUE;
		}
	}

	if (noticed) {
		ib_log(state, "InnoDB: ###### Starts InnoDB Monitor for 30 secs to print diagnostic info:\n");
		ibool old_val = srv_print_innodb_monitor;
		// If some crucial semaphore is reserved, then also the InnoDB Monitor can hang, and we do not get diagnostics. 
		// Since in many cases an InnoDB hang is caused by a pwrite() or a pread() call hanging inside the operating system, let us print right now the values of pending calls of these.
		ib_log(state, "InnoDB: Pending preads %lu, pwrites %lu\n", (ulong)os_file_n_pending_preads, (ulong)os_file_n_pending_pwrites);
		srv_print_innodb_monitor = TRUE;
		os_event_set(srv_lock_timeout_thread_event);
		os_thread_sleep(30000000);
		srv_print_innodb_monitor = old_val;
		ib_log(state,"InnoDB: ###### Diagnostic info printed to the standard error stream\n");
	}

	return(fatal);
}

/// \brief Prints info of the wait array.
/// \param state The state
/// \param arr The wait array
static void sync_array_output_info(innodb_t* state, sync_array_t* arr)
{
	ib_log(state,"OS WAIT ARRAY INFO: reservation count %ld, signal count %ld\n", (long) arr->res_count, (long) arr->sg_count);
	ulint i = 0;
	ulint count = 0;
	while (count < arr->n_reserved) {
		sync_cell_t* cell = sync_array_get_nth_cell(arr, i);
		if (cell->wait_object != NULL) {
			count++;
			sync_array_cell_print(state->stream, cell);
		}
		i++;
	}
}

/// \brief Prints info of the wait array.
/// \param state The state
/// \param arr The wait array
IB_INTERN void sync_array_print_info(innodb_t* state, sync_array_t* arr)
{
	sync_array_enter(arr);
	sync_array_output_info(state->stream, arr);
	sync_array_exit(arr);
}
