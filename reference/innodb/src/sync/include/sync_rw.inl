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


/// \file sync_rw.inl
/// \brief The read-write lock (for threads)
/// \details Originally created 9/11/1995 Heikki Tuuri


/// \brief Lock an rw-lock in shared mode for the current thread. 
/// \details If the rw-lock is locked in exclusive mode, or there is an exclusive lock request waiting,
/// he function spins a preset time (controlled by state->srv.n_spin_wait_rounds),
/// waiting for the lock before suspending the thread.
/// \param lock Pointer to rw-lock
/// \param pass Pass value; != 0, if the lock will be passed to another thread to unlock
/// \param file_name File name where lock requested
/// \param line Line where requested
IB_INTERN void rw_lock_s_lock_spin(rw_lock_t* lock, ulint pass, const char* file_name, ulint line);


#ifdef IB_SYNC_DEBUG
	/// \brief Inserts the debug information for an rw-lock.
	/// \details Inserts the debug information for an rw-lock.
	/// \param lock Pointer to rw-lock
	/// \param pass Pass value; != 0, if the lock will be passed to another thread to unlock
	/// \param lock_type Lock type
	/// \param file_name File name where lock requested
	/// \param line Line where requested */
	IB_INTERN void rw_lock_add_debug_info(rw_lock_t* lock, ulint pass, ulint lock_type, const char*	file_name, ulint line);


	/// \brief Removes a debug information struct for an rw-lock.
	/// \param lock Pointer to rw-lock
	/// \param pass Pass value; != 0, if the lock will be passed to another thread to unlock
	/// \param lock_type Lock type
	IB_INTERN void rw_lock_remove_debug_info(rw_lock_t* lock, ulint pass, ulint lock_type);
#endif // IB_SYNC_DEBUG


/// \brief Check if there are threads waiting for the rw-lock.
/// \details Check if there are threads waiting for the rw-lock.
/// \param lock Pointer to rw-lock
/// \return 1 if waiters, 0 otherwise
IB_INLINE ulint rw_lock_get_waiters(const rw_lock_t* lock)
{
	return lock->waiters;
}

/// \brief Sets lock->waiters to 1.
/// \details Sets lock->waiters to 1.
/// It is not an error if lock->waiters is already
/// 1. On platforms where ATOMIC builtins are used this function enforces a memory barrier
/// \param [in,out] lock Pointer to rw-lock
IB_INLINE void rw_lock_set_waiter_flag(rw_lock_t* lock)
{
	if constexpr (INNODB_RW_LOCKS_USE_ATOMICS) {
		os_compare_and_swap_ulint(&lock->waiters, 0, 1);
	} else {
		lock->waiters = 1;
	}
}

/// Resets lock->waiters to 0. It is not an error if lock->waiters is already 0. 
/// On platforms where ATOMIC builtins are used this function enforces a memory barrier.
/// \param [in,out] lock Pointer to rw-lock
IB_INLINE void rw_lock_reset_waiter_flag(rw_lock_t* lock)
{
	if constexpr (INNODB_RW_LOCKS_USE_ATOMICS) {
		os_compare_and_swap_ulint(&lock->waiters, 1, 0);
	} else {
		lock->waiters = 0;
	}
}

/// \brief Returns the write-status of the lock - this function made more sense with the old rw_lock implementation.
/// \param [in] lock Pointer to rw-lock
/// \return RW_LOCK_NOT_LOCKED | RW_LOCK_EX | RW_LOCK_WAIT_EX
IB_INLINE ulint rw_lock_get_writer(const rw_lock_t* lock)
{
	lint lock_word = lock->lock_word;
	if (lock_word > 0) {
		// return NOT_LOCKED in s-lock state, like the writer member of the old lock implementation. */
		return RW_LOCK_NOT_LOCKED;
	}
	
	if (((-lock_word) % X_LOCK_DECR) == 0) {
		return RW_LOCK_EX;
	} 
    
	ut_ad(lock_word > -X_LOCK_DECR);
	return RW_LOCK_WAIT_EX;
	
}

/// \brief Returns the number of readers.
/// \param [in] lock Pointer to rw-lock
/// \return number of readers
IB_INLINE ulint rw_lock_get_reader_count(const rw_lock_t* lock)
{
	lint lock_word = lock->lock_word;
	if (lock_word > 0) {
		// s-locked, no x-waiters
		return X_LOCK_DECR - lock_word;
	} 
	
	if (lock_word < 0 && lock_word > -X_LOCK_DECR) {
		// s-locked, with x-waiters
		return (ulint)(-lock_word);
	}
	return 0;
}

#ifndef INNODB_RW_LOCKS_USE_ATOMICS
	IB_INLINE mutex_t* rw_lock_get_mutex(rw_lock_t*	lock)
	{
		return &lock->mutex;
	}
#endif

/// \brief Returns the value of writer_count for the lock. 
/// Does not reserve the lock mutex, so the caller must be sure it is not changed during the call.
/// \param [in] lock Pointer to rw-lock
/// \return	value of writer_count
IB_INLINE ulint rw_lock_get_x_lock_count(const rw_lock_t* lock)
{
	lint lock_copy = lock->lock_word;
	// If there is a reader, lock_word is not divisible by X_LOCK_DECR
	if (lock_copy > 0 || (-lock_copy) % X_LOCK_DECR != 0) {
		return 0;
	}
	return ((-lock_copy) / X_LOCK_DECR) + 1;
}

/// \brief Two different implementations for decrementing the lock_word of a rw_lock:
/// one for systems supporting atomic operations, one for others. 
/// This does not support recusive x-locks: they should be handled by the caller and need not be atomic since they are performed by the current lock holder.
/// \param [in,out] lock Pointer to rw-lock
/// \param [in] amount Amount to decrement
/// \return TRUE if decr occurs
IB_INLINE ibool rw_lock_lock_word_decr(rw_lock_t* lock, ulint amount)
{
	if constexpr (INNODB_RW_LOCKS_USE_ATOMICS) {
        lint local_lock_word = lock->lock_word;
		while (local_lock_word > 0) {
			if (os_compare_and_swap_lint(&lock->lock_word, local_lock_word, local_lock_word - amount)) {
				return TRUE;
			}
			local_lock_word = lock->lock_word;
		}
		return FALSE;
	}


	ibool success = FALSE;
	mutex_enter(&(lock->mutex));
	{
		if (lock->lock_word > 0) {
			lock->lock_word -= amount;
			success = TRUE;
		}
	}
	mutex_exit(&(lock->mutex));
	return successor;
}

/******************************************************************//**
Increments lock_word the specified amount and returns new value.
@return	lock->lock_word after increment */
IB_INLINE lint rw_lock_lock_word_incr(
	rw_lock_t*	lock,		/*!< in/out: rw-lock */
	ulint		amount)		/*!< in: amount of increment */
{
#ifdef INNODB_RW_LOCKS_USE_ATOMICS
	return(os_atomic_increment_lint(&lock->lock_word, amount));
#else /* INNODB_RW_LOCKS_USE_ATOMICS */
	lint		local_lock_word;

	mutex_enter(&(lock->mutex));

	lock->lock_word += amount;
	local_lock_word = lock->lock_word;

	mutex_exit(&(lock->mutex));

        return(local_lock_word);
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
}

/******************************************************************//**
This function sets the lock->writer_thread and lock->recursive fields.
For platforms where we are using atomic builtins instead of lock->mutex
it sets the lock->writer_thread field using atomics to ensure memory
ordering. Note that it is assumed that the caller of this function
effectively owns the lock i.e.: nobody else is allowed to modify
lock->writer_thread at this point in time.
The protocol is that lock->writer_thread MUST be updated BEFORE the
lock->recursive flag is set. */
IB_INLINE void rw_lock_set_writer_id_and_recursion_flag(
	rw_lock_t*	lock,		/*!< in/out: lock to work on */
	ibool		recursive)	/*!< in: TRUE if recursion
					allowed */
{
	os_thread_id_t	curr_thread	= os_thread_get_curr_id();

#ifdef INNODB_RW_LOCKS_USE_ATOMICS
	os_thread_id_t	local_thread;
	ibool		success;
	/* Prevent Valgrind warnings about writer_thread being
	uninitialized.  It does not matter if writer_thread is
	uninitialized, because we are comparing writer_thread against
	itself, and the operation should always succeed. */
	IB_MEM_VALID(&lock->writer_thread, sizeof lock->writer_thread);
	local_thread = lock->writer_thread;
	success = os_compare_and_swap_thread_id(&lock->writer_thread, local_thread, curr_thread);
	ut_a(success);
	lock->recursive = recursive;

#else /* INNODB_RW_LOCKS_USE_ATOMICS */

	mutex_enter(&lock->mutex);
	lock->writer_thread = curr_thread;
	lock->recursive = recursive;
	mutex_exit(&lock->mutex);

#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
}

/// \brief Low-level function which tries to lock an rw-lock in s-mode. 
/// \details Performs no spinning.
/// \param [in,out] lock Pointer to rw-lock
/// \param [in] pass Pass value; != 0, if the lock will be passed to another thread to unlock
/// \param [in] file_name File name where lock requested
/// \param [in] line Line where requested
/// \return TRUE if success
IB_INLINE ibool rw_lock_s_lock_low(rw_lock_t* lock, ulint pass, const char* file_name, ulint line)
{
	IB_UNUSED(pass);

	/// TODO: study performance of IB_LIKELY branch prediction hints.
	if (!rw_lock_lock_word_decr(lock, 1)) {
		// Locking did not succeed
		return FALSE;
	}

	if constexpr (IB_SYNC_DEBUG) {
		rw_lock_add_debug_info(lock, pass, RW_LOCK_SHARED, file_name, line);
	}

	// These debugging values are not set safely: they may be incorrect or even refer to a line that is invalid for the file name.
	lock->last_s_file_name = file_name;
	lock->last_s_line = line;

	return TRUE; // locking succeeded
}

/// \brief Low-level function which locks an rw-lock in s-mode when we know that it is possible and none else is currently accessing the rw-lock structure. Then we can do the locking without reserving the mutex.
/// \param [in,out] lock Pointer to rw-lock
/// \param [in] file_name File name where requested
/// \param [in] line Line where lock requested
IB_INLINE void rw_lock_s_lock_direct(rw_lock_t* lock, const char* file_name, ulint line)
{
	ut_ad(lock->lock_word == X_LOCK_DECR);

	// Indicate there is a new reader by decrementing lock_word
	lock->lock_word--;
	lock->last_s_file_name = file_name;
	lock->last_s_line = line;

	if constexpr (IB_SYNC_DEBUG) {
		rw_lock_add_debug_info(lock, 0, RW_LOCK_SHARED, file_name, line);
	}
}

/// \brief Low-level function which locks an rw-lock in x-mode when we know that it is not locked and none else is currently accessing the rw-lock structure; then we can do the locking without reserving the mutex.
/// \param [in,out] lock Pointer to rw-lock
/// \param [in] file_name File name where requested
/// \param [in] line Line where lock requested
IB_INLINE void rw_lock_x_lock_direct(rw_lock_t* lock, const char* file_name, ulint line)
{
	ut_ad(rw_lock_validate(lock));
	ut_ad(lock->lock_word == X_LOCK_DECR);

	lock->lock_word -= X_LOCK_DECR;
	lock->writer_thread = os_thread_get_curr_id();
	lock->recursive = TRUE;

	lock->last_x_file_name = file_name;
	lock->last_x_line = line;

	if constexpr (IB_SYNC_DEBUG) {
		rw_lock_add_debug_info(lock, 0, RW_LOCK_EX, file_name, line);
	}
}


/// \note: Use the corresponding macro, not directly this function! Lock an
/// rw-lock in shared mode for the current thread. If the rw-lock is locked
/// in exclusive mode, or there is an exclusive lock request waiting, the
/// function spins a preset time (controlled by state->srv.n_spin_wait_rounds), waiting for
/// the lock, before suspending the thread.
/// \param [in,out] lock Pointer to rw-lock
/// \param [in] pass Pass value; != 0, if the lock will be passed to another thread to unlock
/// \param [in] file_name File name where lock requested
/// \param [in] line Line where requested
IB_INLINE void rw_lock_s_lock_func(rw_lock_t* lock, ulint pass, const char*	file_name, ulint line)
{
	// As we do not know the thread ids for threads which have
	// s-locked a latch, and s-lockers will be served only after waiting
	// x-lock requests have been fulfilled, then if this thread already
	// owns an s-lock here, it may end up in a deadlock with another thread
	// which requests an x-lock here. Therefore, we will forbid recursive
	// s-locking of a latch: the following assert will warn the programmer
	// of the possibility of this kind of a deadlock. If we want to implement
	// safe recursive s-locking, we should keep in a list the thread ids of
	// the threads which have s-locked a latch. This would use some CPU time.

	if constexpr (IB_SYNC_DEBUG) {
		ut_ad(!rw_lock_own(lock, RW_LOCK_SHARED)); // see NOTE above
	}

	/// TODO: study performance of IB_LIKELY branch prediction hints.
	if (rw_lock_s_lock_low(lock, pass, file_name, line)) {
		return; // success
	} 
	
	/// did not succeed, try spin wait
	rw_lock_s_lock_spin(lock, pass, file_name, line);	
}

/// \note Use the corresponding macro, not directly this function! Lock an
/// rw-lock in exclusive mode for the current thread if the lock can be
/// obtained immediately.
/// \param [in,out] lock Pointer to rw-lock
/// \param [in] file_name File name where lock requested
/// \param [in] line Line where requested
/// \return TRUE if success
IB_INLINE ibool rw_lock_x_lock_func_nowait(rw_lock_t* lock, const char*	file_name, ulint line)
{
	ibool success;
	os_thread_id_t curr_thread	= os_thread_get_curr_id();
	
	if constexpr (INNODB_RW_LOCKS_USE_ATOMICS) {
		success = os_compare_and_swap_lint(&lock->lock_word, X_LOCK_DECR, 0);
	} else {
		success = FALSE;
		mutex_enter(&(lock->mutex));
		if (lock->lock_word == X_LOCK_DECR) {
			lock->lock_word = 0;
			success = TRUE;
		}
		mutex_exit(&(lock->mutex));
	}

	if (success) {
		rw_lock_set_writer_id_and_recursion_flag(lock, TRUE);
	} else if (lock->recursive && os_thread_eq(lock->writer_thread, curr_thread)) {
		// Relock: this lock_word modification is safe since no other
		// threads can modify (lock, unlock, or reserve) lock_word while
		// there is an exclusive writer and this is the writer thread.
		lock->lock_word -= X_LOCK_DECR;
		ut_ad(((-lock->lock_word) % X_LOCK_DECR) == 0);
	} else {
		// Failure
		return FALSE;
	}

	if constexpr (IB_SYNC_DEBUG) {
		rw_lock_add_debug_info(lock, 0, RW_LOCK_EX, file_name, line);
	}

	lock->last_x_file_name = file_name;
	lock->last_x_line = line;
	ut_ad(rw_lock_validate(lock));
	return TRUE;
}

/**
Releases a shared mode lock. */
IB_INLINE void rw_lock_s_unlock_func(

#ifdef IB_SYNC_DEBUG
	ulint		pass,	/*!< in: pass value; != 0, if the lock may have
				been passed to another thread to unlock */
#endif
	rw_lock_t*	lock)	/*!< in/out: rw-lock */
{
	ut_ad((lock->lock_word % X_LOCK_DECR) != 0);

#ifdef IB_SYNC_DEBUG
	rw_lock_remove_debug_info(lock, pass, RW_LOCK_SHARED);
#endif

	/* Increment lock_word to indicate 1 less reader */
	if (rw_lock_lock_word_incr(lock, 1) == 0) {

		/* wait_ex waiter exists. It may not be asleep, but we signal
                anyway. We do not wake other waiters, because they can't
                exist without wait_ex waiter and wait_ex waiter goes first.*/
		os_event_set(lock->wait_ex_event);
		sync_array_object_signalled(sync_primary_wait_array);

	}

	ut_ad(rw_lock_validate(lock));

#ifdef IB_SYNC_PERF_STAT
	rw_s_exit_count++;
#endif
}

/// Releases a shared mode lock when we know there are no waiters and none else will access the lock during the time this function is executed.
IB_INLINE void rw_lock_s_unlock_direct(
	rw_lock_t*	lock)	/*!< in/out: rw-lock */
{
	ut_ad(lock->lock_word < X_LOCK_DECR);

#ifdef IB_SYNC_DEBUG
	rw_lock_remove_debug_info(lock, 0, RW_LOCK_SHARED);
#endif

	// Decrease reader count by incrementing lock_word
	lock->lock_word++;

	ut_ad(!lock->waiters);
	ut_ad(rw_lock_validate(lock));
#ifdef IB_SYNC_PERF_STAT
	rw_s_exit_count++;
#endif
}

/// \brief Releases an exclusive mode lock.
IB_INLINE void rw_lock_x_unlock_func(
#ifdef IB_SYNC_DEBUG
	ulint		pass,	/*!< in: pass value; != 0, if the lock may have been passed to another thread to unlock */
#endif
	rw_lock_t*	lock)	/*!< in/out: rw-lock */
{
	ut_ad((lock->lock_word % X_LOCK_DECR) == 0);

	// lock->recursive flag also indicates if lock->writer_thread is
	// valid or stale. If we are the last of the recursive callers
	// then we must unset lock->recursive flag to indicate that the
	// lock->writer_thread is now stale.
	// Note that since we still hold the x-lock we can safely read the
	// lock_word. 
	if (lock->lock_word == 0) {
		// Last caller in a possible recursive chain.
		lock->recursive = FALSE;
		IB_MEM_INVALID(&lock->writer_thread, sizeof lock->writer_thread);
	}

#ifdef IB_SYNC_DEBUG
	rw_lock_remove_debug_info(lock, pass, RW_LOCK_EX);
#endif

	if (rw_lock_lock_word_incr(lock, X_LOCK_DECR) == X_LOCK_DECR) {
		// Lock is now free. May have to signal read/write waiters.
        // We do not need to signal wait_ex waiters, since they cannot
		// exist when there is a writer.
		if (lock->waiters) {
			rw_lock_reset_waiter_flag(lock);
			os_event_set(lock->event);
			sync_array_object_signalled(sync_primary_wait_array);
		}
	}
	ut_ad(rw_lock_validate(lock));
#ifdef IB_SYNC_PERF_STAT
	rw_x_exit_count++;
#endif
}

/// \brief Releases an exclusive mode lock when we know there are no waiters, and none else will access the lock during the time this function is executed.
/// \param [in,out] lock Pointer to rw-lock
IB_INLINE void rw_lock_x_unlock_direct(rw_lock_t* lock)
{
	/* Reset the exclusive lock if this thread no longer has an x-mode lock */
	ut_ad((lock->lock_word % X_LOCK_DECR) == 0);
	if constexpr (IB_SYNC_DEBUG) {
		rw_lock_remove_debug_info(lock, 0, RW_LOCK_EX);
	}

	if (lock->lock_word == 0) {
		lock->recursive = FALSE;
		IB_MEM_INVALID(&lock->writer_thread, sizeof lock->writer_thread);
	}
	lock->lock_word += X_LOCK_DECR;
	ut_ad(!lock->waiters);
	ut_ad(rw_lock_validate(lock));

	if constexpr (IB_SYNC_PERF_STAT) {
		rw_x_exit_count++;
	}
}
