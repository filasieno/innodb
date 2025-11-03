// Copyright (c) 1995, 2010, Innobase Oy. All Rights Reserved.
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

/// \file mtr_mtr.inl
/// \brief Mini-transaction buffer
/// \details Originally created by Heikki Tuuri in 11/26/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#ifndef IB_HOTBACKUP
# include "sync0sync.h"
# include "sync0rw.h"
#endif /* !IB_HOTBACKUP */
#include "mach_data.hpp"

/// \brief Starts a mini-transaction and creates a mini-transaction handle
/// \details and a buffer in the memory buffer given by the caller.
/// \param [in] mtr memory buffer for the mtr buffer
/// \return mtr buffer which also acts as the mtr handle
IB_INLINE mtr_t* mtr_start(mtr_t* mtr)
{
	dyn_array_create(&(mtr->memo));
	dyn_array_create(&(mtr->log));

	mtr->log_mode = MTR_LOG_ALL;
	mtr->modifications = FALSE;
	mtr->n_log_recs = 0;

	ut_d(mtr->state = MTR_ACTIVE);
	ut_d(mtr->magic_n = MTR_MAGIC_N);

	return(mtr);
}

/// \brief Pushes an object to an mtr memo stack.
/// \param [in] mtr mtr
/// \param [in] object object
/// \param [in] type object type: MTR_MEMO_S_LOCK, ...
IB_INLINE void mtr_memo_push(mtr_t* mtr, void* object, ulint type)
{
	dyn_array_t*		memo;
	mtr_memo_slot_t*	slot;

	ut_ad(object);
	ut_ad(type >= MTR_MEMO_PAGE_S_FIX);
	ut_ad(type <= MTR_MEMO_X_LOCK);
	ut_ad(mtr);
	ut_ad(mtr->magic_n == MTR_MAGIC_N);
	ut_ad(mtr->state == MTR_ACTIVE);

	memo = &(mtr->memo);

	slot = (mtr_memo_slot_t*) dyn_array_push(memo, sizeof *slot);

	slot->object = object;
	slot->type = type;
}

/// \brief Sets and returns a savepoint in mtr.
/// \param [in] mtr mtr
/// \return savepoint
IB_INLINE ulint mtr_set_savepoint(mtr_t* mtr)
{
	dyn_array_t*	memo;

	ut_ad(mtr);
	ut_ad(mtr->magic_n == MTR_MAGIC_N);
	ut_ad(mtr->state == MTR_ACTIVE);

	memo = &(mtr->memo);

	return(dyn_array_get_data_size(memo));
}

#ifndef IB_HOTBACKUP
/// \brief Releases the (index tree) s-latch stored in an mtr memo after a savepoint.
/// \param [in] mtr mtr
/// \param [in] savepoint savepoint
/// \param [in] lock latch to release
IB_INLINE void mtr_release_s_latch_at_savepoint(mtr_t* mtr, ulint savepoint, rw_lock_t* lock)
{
	mtr_memo_slot_t* slot;
	dyn_array_t*	memo;

	ut_ad(mtr);
	ut_ad(mtr->magic_n == MTR_MAGIC_N);
	ut_ad(mtr->state == MTR_ACTIVE);

	memo = &(mtr->memo);

	ut_ad(dyn_array_get_data_size(memo) > savepoint);

	slot = (mtr_memo_slot_t*) dyn_array_get_element(memo, savepoint);

	ut_ad(slot->object == lock);
	ut_ad(slot->type == MTR_MEMO_S_LOCK);

	rw_lock_s_unlock(lock);

	slot->object = NULL;
}

# ifdef IB_DEBUG
/// \brief Checks if memo contains the given item.
/// \param [in] mtr mtr
/// \param [in] object object to search
/// \param [in] type type of object
/// \return TRUE if contains
IB_INLINE ibool mtr_memo_contains(mtr_t* mtr, const void* object, ulint type)
{
	mtr_memo_slot_t* slot;
	dyn_array_t*	memo;
	ulint		offset;

	ut_ad(mtr);
	ut_ad(mtr->magic_n == MTR_MAGIC_N);
	ut_ad(mtr->state == MTR_ACTIVE || mtr->state == MTR_COMMITTING);

	memo = &(mtr->memo);

	offset = dyn_array_get_data_size(memo);

	while (offset > 0) {
		offset -= sizeof(mtr_memo_slot_t);

		slot = dyn_array_get_element(memo, offset);

		if ((object == slot->object) && (type == slot->type)) {

			return(TRUE);
		}
	}

	return(FALSE);
}
# endif /* IB_DEBUG */
#endif /* !IB_HOTBACKUP */

/// \brief Gets the logging mode of a mini-transaction.
/// \param [in] mtr mtr
/// \return logging mode: MTR_LOG_NONE, ...
IB_INLINE ulint mtr_get_log_mode(mtr_t* mtr)
{
	ut_ad(mtr);
	ut_ad(mtr->log_mode >= MTR_LOG_ALL);
	ut_ad(mtr->log_mode <= MTR_LOG_SHORT_INSERTS);

	return(mtr->log_mode);
}

/// \brief Changes the logging mode of a mini-transaction.
/// \param [in] mtr mtr
/// \param [in] mode logging mode: MTR_LOG_NONE, ...
/// \return old mode
IB_INLINE ulint mtr_set_log_mode(mtr_t* mtr, ulint mode)
{
	ulint	old_mode;

	ut_ad(mtr);
	ut_ad(mode >= MTR_LOG_ALL);
	ut_ad(mode <= MTR_LOG_SHORT_INSERTS);

	old_mode = mtr->log_mode;

	if ((mode == MTR_LOG_SHORT_INSERTS) && (old_mode == MTR_LOG_NONE)) {
		/* Do nothing */
	} else {
		mtr->log_mode = mode;
	}

	ut_ad(old_mode >= MTR_LOG_ALL);
	ut_ad(old_mode <= MTR_LOG_SHORT_INSERTS);

	return(old_mode);
}

#ifndef IB_HOTBACKUP
/// \brief Locks a lock in s-mode.
/// \param [in] lock rw-lock
/// \param [in] file file name
/// \param [in] line line number
/// \param [in] mtr mtr
IB_INLINE void mtr_s_lock_func(rw_lock_t* lock, const char* file, ulint line, mtr_t* mtr)
{
	ut_ad(mtr);
	ut_ad(lock);

	rw_lock_s_lock_func(lock, 0, file, line);

	mtr_memo_push(mtr, lock, MTR_MEMO_S_LOCK);
}

/// \brief Locks a lock in x-mode.
/// \param [in] lock rw-lock
/// \param [in] file file name
/// \param [in] line line number
/// \param [in] mtr mtr
IB_INLINE void mtr_x_lock_func(rw_lock_t* lock, const char* file, ulint line, mtr_t* mtr)
{
	ut_ad(mtr);
	ut_ad(lock);

	rw_lock_x_lock_func(lock, 0, file, line);

	mtr_memo_push(mtr, lock, MTR_MEMO_X_LOCK);
}
#endif /* !IB_HOTBACKUP */
