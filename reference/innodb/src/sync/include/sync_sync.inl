/*****************************************************************************

Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.
Copyright (c) 2008, Google Inc.

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

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


/// \file sync0sync.ic
/// \brief Mutex, the basic synchronization primitive
/// \details Originally created 9/5/1995 Heikki Tuuri
/// Created 9/5/1995 Heikki Tuuri

/// Sets the waiters field in a mutex.
/// \param [in,out] mutex Pointer to mutex
/// \param [in] n Value to set
IB_INTERN void mutex_set_waiters(mutex_t* mutex, ulint n);

/** Reserves a mutex for the current thread. If the mutex is reserved, the
function spins a preset time (controlled by state->srv.n_spin_wait_rounds) waiting
for the mutex before suspending the thread. */
IB_INTERN void mutex_spin_wait(
	mutex_t*	mutex,		/*!< in: pointer to mutex */
	const char*	file_name,	/*!< in: file name where mutex
					requested */
	ulint		line);		/*!< in: line where requested */


/// \brief Sets the debug information for a reserved mutex.
/// \param [in,out] mutex Pointer to mutex
/// \param [in] file_name File name where mutex requested
/// \param [in] line Line where mutex requested
IB_INTERN void mutex_set_debug_info(
	mutex_t* mutex,
	const char* file_name,
	ulint line);


/// \brief Releases the threads waiting in the primary wait array for this mutex.
IB_INTERN void mutex_signal_object(
	mutex_t* mutex);

/// \brief Performs an atomic test-and-set instruction to the lock_word field of a mutex.
/// \param [in, out] mutex Pointer to mutex
/// \return the previous value of lock_word: 0 or 1
IB_INLINE ibool mutex_test_and_set( mutex_t* mutex)
{
	if constexpr (IB_HAVE_ATOMIC_BUILTINS) {
		return os_atomic_test_and_set_byte(&mutex->lock_word, 1);
	}

	ibool ret = os_fast_mutex_trylock(&(mutex->os_fast_mutex));
	if (ret == 0) {
		// We check that os_fast_mutex_trylock does not leak and allow race conditions
		ut_a(mutex->lock_word == 0);
		mutex->lock_word = 1;
	}

	return (byte)ret;
}

/// Performs a reset instruction to the lock_word field of a mutex. This
/// instruction also serializes memory operations to the program order. */
/// \param [in, out] mutex Pointer to mutex
IB_INLINE void mutex_reset_lock_word(mutex_t* mutex)
{
#if defined(IB_HAVE_ATOMIC_BUILTINS)
	/* In theory __sync_lock_release should be used to release the lock.
	Unfortunately, it does not work properly alone. The workaround is
	that more conservative __sync_lock_test_and_set is used instead. */
	os_atomic_test_and_set_byte(&mutex->lock_word, 0);
#else
	mutex->lock_word = 0;

	os_fast_mutex_unlock(&(mutex->os_fast_mutex));
#endif
}

/// \brief Gets the value of the lock word. 
/// \param [in] mutex Pointer to mutex
/// \return	value of the lock word
IB_INLINE lock_word_t mutex_get_lock_word( const mutex_t* mutex)
{
	ut_ad(mutex);
	return mutex->lock_word;
}

/// \brief Gets the waiters field in a mutex. 
/// \param [in] mutex Pointer to mutex
/// \return	value to set
IB_INLINE ulint mutex_get_waiters(const mutex_t* mutex)
{
	ut_ad(mutex);
	const volatile ulint* ptr = &(mutex->waiters);
	return *ptr; // Here we assume that the read of a single word from memory is atomic
}


/// \brief Unlocks a mutex owned by the current thread.
/// \param [in, out] mutex Pointer to mutex
IB_INLINE void mutex_exit(mutex_t* mutex)
{
	ut_ad(mutex_own(mutex));
	ut_d(mutex->thread_id = (os_thread_id_t) ULINT_UNDEFINED);

	if constexpr (IB_SYNC_DEBUG) {
		sync_thread_reset_level(mutex);
	}
	
	mutex_reset_lock_word(mutex);

	/// TODO: A problem: we assume that mutex_reset_lock word is a memory barrier, that is when we read the waiters
	/// field next, the read must be serialized in memory after the reset. A speculative processor might
	/// perform the read first, which could leave a waiting thread hanging indefinitely.

	// Our current solution call every second sync_arr_wake_threads_if_sema_free() to wake up possible hanging threads if
	// they are missed in mutex_signal_object.

	if (mutex_get_waiters(mutex) != 0) {
		mutex_signal_object(mutex);
	}

	if constexpr (IB_SYNC_PERF_STAT) {
		mutex_exit_count++;
	}
}

/// \brief Locks a mutex for the current thread. 
/// \details If the mutex is reserved, the function spins a preset time (controlled by state->srv.n_spin_wait_rounds), 
/// waiting for the mutex before suspending the thread.
/// \param [in, out] mutex Pointer to mutex
/// \param [in] file_name File name where locked
/// \param [in] line Line where locked
IB_INLINE void mutex_enter_func( mutex_t* mutex, const char* file_name, ulint line)
{
	ut_ad(mutex_validate(mutex));
	ut_ad(!mutex_own(mutex));

	// Note that we do not peek at the value of lock_word before trying
	// the atomic test_and_set; we could peek, and possibly save time.
	ut_d(mutex->count_using++);
	if (!mutex_test_and_set(mutex)) {
		ut_d(mutex->thread_id = os_thread_get_curr_id());
#ifdef IB_SYNC_DEBUG
		mutex_set_debug_info(mutex, file_name, line);
#endif
		return;	/* Succeeded! */
	}

	mutex_spin_wait(mutex, file_name, line);
}
