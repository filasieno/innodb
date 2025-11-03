// Copyright (c) 1996, 2010, Innobase Oy. All Rights Reserved.
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




#define LOCK_MODULE_IMPLEMENTATION

#include "lock_lock.h"
#include "lock_priv.h"

#ifdef UNIV_NONINL
#include "lock_lock.ic"
#include "lock_priv.ic"
#endif

#include "api_ucode.h"
#include "usr_sess.h"
#include "trx_purge.h"
#include "dict_mem.h"
#include "trx_sys.h"

// -----------------------------------------------------------------------------------------
// type definitions
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

constinit ulint LOCK_MAX_N_STEPS_IN_DEADLOCK_CHECK = 1000000;

constinit ulint LOCK_MAX_DEPTH_IN_DEADLOCK_CHECK = 200;

constinit ulint LOCK_RELEASE_KERNEL_INTERVAL = 1000;

constinit ulint LOCK_PAGE_BITMAP_MARGIN = 64;

#define LOCK_VICTIM_IS_START	1
#define LOCK_VICTIM_IS_OTHER	2
#define LOCK_EXCEED_MAX_DEPTH	3

// -----------------------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------------------

#ifdef IB_DEBUG
IB_INTERN ibool lock_print_waits = FALSE;
#endif // IB_DEBUG 

IB_INTERN lock_sys_t* lock_sys = NULL;

IB_INTERN ibool lock_deadlock_found = FALSE;
IB_INTERN ib_stream_t lock_latest_err_stream;

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

/// \brief Resets the record lock bitmap to zero.
/// \details Does not touch the transaction's wait_lock pointer. Used during
/// lock object creation and resetting.
/// \param [in] lock record lock
static void lock_rec_bitmap_reset(ib_lock_t* lock);

/// \brief Copies a record lock into the given heap.
/// \param [in] lock record lock to copy
/// \param [in] heap destination memory heap
/// \return copy of lock (allocated on heap)
static ib_lock_t* lock_rec_copy(const ib_lock_t* lock, mem_heap_t* heap);

/// \brief Checks if some other transaction has a lock request in the queue.
/// \param [in] mode LOCK_S or LOCK_X
/// \param [in] gap LOCK_GAP to include gap locks, or 0 to ignore
/// \param [in] wait LOCK_WAIT to include waiting locks, or 0 to ignore
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of the record
/// \param [in] trx transaction to exclude, or NULL to consider all
/// \return matching lock or NULL
static ib_lock_t* lock_rec_other_has_expl_req(enum lock_mode mode, ulint gap, ulint wait, const buf_bib_lock_t* block, ulint heap_no, const trx_t* trx);

/// \brief Checks if another transaction has a conflicting explicit lock request
/// in the queue that would force us to wait.
/// \param [in] mode LOCK_S or LOCK_X, possibly ORed with LOCK_GAP or
/// LOCK_REC_NOT_GAP, LOCK_INSERT_INTENTION
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of the record
/// \param [in] trx our transaction
/// \return conflicting lock or NULL
static ib_lock_t* lock_rec_other_has_conflicting(enum lock_mode mode, const buf_bib_lock_t* block, ulint heap_no, trx_t* trx);

/// \brief Checks if some transaction has an implicit X-lock on a secondary
/// index record.
/// \param [in] rec secondary index record
/// \param [in] index secondary index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \return transaction that owns the implicit X-lock, or NULL
static trx_t* lock_sec_rec_some_has_impl_off_kernel(const rec_t* rec, dict_index_t* index, const ulint* offsets);

/// \brief Creates a new record lock and inserts it into the page queue.
/// \details Does NOT check for deadlocks or lock compatibility.
/// \param [in] type_mode lock mode and wait flag; type is replaced by LOCK_REC
/// \param [in] space space id
/// \param [in] page_no page number
/// \param [in] heap_no heap number of the record
/// \param [in] n_bits number of bits in lock bitmap
/// \param [in] index index of record
/// \param [in] trx transaction
/// \return created lock
static ib_lock_t* lock_rec_create_low(ulint type_mode, ulint space, ulint page_no, ulint heap_no, ulint n_bits, dict_index_t* index, trx_t* trx);

/// \brief Creates a new record lock and inserts it to the lock queue.
/// \param [in] type_mode lock mode and wait flag; type is replaced by LOCK_REC
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of the record
/// \param [in] index index of record
/// \param [in] trx transaction
/// \return created lock
static ib_lock_t* lock_rec_create(ulint type_mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, trx_t* trx);

/// \brief Enqueues a waiting request for a record lock that cannot be granted
/// immediately.
/// \param [in] type_mode LOCK_S or LOCK_X, possibly ORed with LOCK_GAP or
/// LOCK_REC_NOT_GAP, ORed with LOCK_INSERT_INTENTION for insert
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of the record
/// \param [in] index index of record
/// \param [in] thr query thread
/// \return DB_LOCK_WAIT, DB_DEADLOCK, DB_QUE_THR_SUSPENDED, or DB_SUCCESS
static ulint lock_rec_enqueue_waiting(ulint type_mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, que_thr_t* thr);

/// \brief Adds a record lock request to the record queue.
/// \details Does NOT check for deadlocks or compatibility.
/// \param [in] type_mode lock mode, wait, gap etc.; type replaced by LOCK_REC
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of the record
/// \param [in] index index of record
/// \param [in] trx transaction
/// \return lock where the bit was set
static ib_lock_t* lock_rec_add_to_queue(ulint type_mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, trx_t* trx);

/// \brief General (slower) routine for locking a record.
/// \details Does NOT consider implicit locks. Checks explicit lock compatibility.
/// Sets next-key lock, or gap lock for page supremum.
/// \param [in] impl if TRUE, do not set lock when no wait is necessary
/// \param [in] mode LOCK_X or LOCK_S, possibly ORed with LOCK_GAP or
/// LOCK_REC_NOT_GAP
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of record
/// \param [in] index index of record
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, or error code
static ulint lock_rec_lock_slow(ibool impl, ulint mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, que_thr_t* thr);

/// \brief Tries to lock the specified record in the requested mode.
/// \details Falls back to slow path if fast path fails. Does NOT consider
/// implicit locks. Uses next-key or gap lock for page supremum.
/// \param [in] impl if TRUE, do not set lock when no wait is necessary
/// \param [in] mode LOCK_X or LOCK_S, possibly ORed with LOCK_GAP or
/// LOCK_REC_NOT_GAP
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of record
/// \param [in] index index of record
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, or error code
static ulint lock_rec_lock(ibool impl, ulint mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, que_thr_t* thr);

/// \brief Checks if a waiting record lock request still has to wait in a queue.
/// \param [in] wait_lock waiting record lock
/// \return TRUE if still has to wait
static ibool lock_rec_has_to_wait_in_queue(ib_lock_t* wait_lock);

/// \brief Grants a lock to a waiting lock request and releases the waiting transaction.
/// \param [in] lock lock
static void lock_grant(ib_lock_t* lock);

/// \brief Cancels a waiting record lock request and releases the waiting transaction that requested it.
/// \param [in] lock record lock
static void lock_rec_cancel(ib_lock_t* lock);

/// \brief Removes a record lock request, waiting or granted, from the queue and grants locks to other transactions in the queue if they now are entitled to a lock.
/// \param [in] in_lock record lock object; transactions waiting behind will get their lock requests granted, if they are now qualified to it
static void lock_rec_dequeue_from_page(ib_lock_t* in_lock);

/// \brief Removes a record lock request, waiting or granted, from the queue.
/// \param [in] in_lock record lock
static void lock_rec_discard(ib_lock_t* in_lock);

/// \brief Removes record lock objects set on an index page which is discarded.
/// \param [in] block buffer block
static void lock_rec_free_all_from_discard_page(const buf_bib_lock_t* block);

/// \brief Resets the lock bits for a single record.
/// \details Releases transactions waiting for lock requests here.
/// \param [in] block buffer block containing the record
/// \param [in] heap_no heap number of record
static void lock_rec_reset_and_release_wait(const buf_bib_lock_t* block, ulint heap_no);

/// \brief Makes a record to inherit the locks (except LOCK_INSERT_INTENTION type) of another record as gap type locks, but does not reset the lock bits of the other record.
/// \param [in] heir_block block containing the record which inherits
/// \param [in] block block containing the record from which inherited; does NOT reset the locks on this record
/// \param [in] heir_heap_no heap_no of the inheriting record
/// \param [in] heap_no heap_no of the donating record
static void lock_rec_inherit_to_gap(const buf_bib_lock_t* heir_block, const buf_bib_lock_t* block, ulint heir_heap_no, ulint heap_no);

/// \brief Makes a record to inherit the gap locks (except LOCK_INSERT_INTENTION type) of another record as gap type locks, but does not reset the lock bits of the other record.
/// \param [in] block block containing the record
/// \param [in] heir_heap_no heap_no of the inheriting record
/// \param [in] heap_no heap_no of the donating record
static void lock_rec_inherit_to_gap_if_gap_lock(const buf_bib_lock_t* block, ulint heir_heap_no, ulint heap_no);

/// \brief Moves the locks of a record to another record and resets the lock bits of the donating record.
/// \param [in] receiver buffer block of receiver
/// \param [in] donator buffer block of donator
/// \param [in] receiver_heap_no heap number of the record which receives the locks
/// \param [in] donator_heap_no heap number of the record which donates the locks
static void lock_rec_move(const buf_bib_lock_t* receiver, const buf_bib_lock_t* donator, ulint receiver_heap_no, ulint donator_heap_no);

static ibool lock_deadlock_occurs(ib_lock_t* lock, trx_t* trx);

static ulint lock_deadlock_recursive(trx_t* start, trx_t* trx, ib_lock_t* wait_lock, ulint* cost, ulint depth);

/// \brief Enqueues a waiting request for a table lock which cannot be granted immediately.
/// \details Checks for deadlocks.
/// \param [in] mode lock mode this transaction is requesting
/// \param [in] table table
/// \param [in] thr query thread
/// \return DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED, or DB_SUCCESS
static ulint ib_lock_table_enqueue_waiting(ulint mode, dict_table_t* table, que_thr_t* thr);

/// \brief Checks if a waiting table lock request still has to wait in a queue.
/// \param [in] wait_lock waiting table lock
/// \return TRUE if still has to wait
static ibool ib_lock_table_has_to_wait_in_queue(ib_lock_t* wait_lock);

/// \brief Removes a table lock request, waiting or granted, from the queue and grants locks to other transactions in the queue, if they now are entitled to a lock.
/// \param [in] in_lock table lock object; transactions waiting behind will get their lock requests granted, if they are now qualified to it
static void ib_lock_table_dequeue(ib_lock_t* in_lock);

/// \brief Removes locks of a transaction on a table to be dropped.
/// \details If remove_also_table_sx_locks is TRUE then table-level S and X
/// locks are also removed in addition to other table-level and record-level
/// locks. No lock that is going to be removed is allowed to be a wait lock.
/// \param [in] table table to be dropped
/// \param [in] trx a transaction
/// \param [in] remove_also_table_sx_locks also removes table S and X locks
static void lock_remove_all_on_table_for_trx(dict_table_t* table, trx_t* trx, ibool remove_also_table_sx_locks);

/// \brief Converts an implicit X-lock on a record to an explicit X-lock.
/// \details If a transaction has an implicit X-lock on a record but no explicit
/// X-lock set on it, this sets one. For a secondary index, the kernel mutex may
/// be temporarily released.
/// \param [in] block buffer block of rec
/// \param [in] rec user record on page
/// \param [in] index index of record
/// \param [in] offsets rec_get_offsets(rec, index)
static void lock_rec_convert_impl_to_expl(const buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, const ulint* offsets);

/// \brief Checks whether requesting a lock will result in a deadlock.
/// \param [in] lock lock being requested
/// \param [in] trx transaction requesting the lock
/// \return TRUE if a deadlock was detected and trx is chosen as victim;
/// FALSE if no deadlock or another trx was chosen as victim
static ibool lock_deadlock_occurs(ib_lock_t* lock, trx_t* trx);

/// \brief Recursively searches the waits-for graph for a deadlock.
/// \param [in] start recursion starting transaction
/// \param [in] trx a transaction waiting for a lock
/// \param [in] wait_lock the lock that trx is waiting for
/// \param [in,out] cost number of steps so far; used to cap search effort
/// \param [in] depth recursion depth; used to cap recursion
/// \return 0=no deadlock; LOCK_VICTIM_IS_START or LOCK_VICTIM_IS_OTHER on
/// deadlock; LOCK_EXCEED_MAX_DEPTH if exceeding limits
static ulint lock_deadlock_recursive(trx_t* start, trx_t* trx, ib_lock_t* wait_lock, ulint* cost, ulint depth);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

#define LK(a,b) (1 << ((a) * LOCK_NUM + (b)))
#define LKS(a,b) LK(a,b) | LK(b,a)

#define LOCK_MODE_COMPATIBILITY 0					\
 | LK(LOCK_IS, LOCK_IS) | LK(LOCK_IX, LOCK_IX) | LK(LOCK_S, LOCK_S)	\
 | LKS(LOCK_IX, LOCK_IS) | LKS(LOCK_IS, LOCK_AUTO_INC)			\
 | LKS(LOCK_S, LOCK_IS)							\
 | LKS(LOCK_AUTO_INC, LOCK_IS) | LKS(LOCK_AUTO_INC, LOCK_IX)

/* Define the stronger-or-equal lock relation in a ulint.  This relation
contains all pairs LK(mode1, mode2) where mode1 is stronger than or
equal to mode2. */
#define LOCK_MODE_STRONGER_OR_EQ 0					\
 | LK(LOCK_IS, LOCK_IS)							\
 | LK(LOCK_IX, LOCK_IS) | LK(LOCK_IX, LOCK_IX)				\
 | LK(LOCK_S, LOCK_IS) | LK(LOCK_S, LOCK_S)				\
 | LK(LOCK_AUTO_INC, LOCK_AUTO_INC)					\
 | LK(LOCK_X, LOCK_IS) | LK(LOCK_X, LOCK_IX) | LK(LOCK_X, LOCK_S)	\
 | LK(LOCK_X, LOCK_AUTO_INC) | LK(LOCK_X, LOCK_X)

IB_INTERN void lock_var_init(void)
{
	if constexpr (IB_DEBUG) {
		lock_print_waits = FALSE;
	}
	lock_sys = NULL;
	lock_deadlock_found = FALSE;
	lock_latest_err_stream = NULL;
}

IB_INLINE ibool lock_rec_get_nth_bit(const ib_lock_t* lock, ulint i)
{
	ut_ad(lock);
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	if (i >= lock->un_member.rec_lock.n_bits) {
		return FALSE;
	}
	ulint byte_index = i / 8;
	ulint bit_index = i % 8;
	return 1 & ((const byte*) &lock[1])[byte_index] >> bit_index;
}

#define lock_mutex_enter_kernel()	mutex_enter(&kernel_mutex)
#define lock_mutex_exit_kernel()	mutex_exit(&kernel_mutex)

IB_INTERN ibool lock_check_trx_id_sanity(trx_id_t trx_id, const rec_t* rec, dict_index_t* index, const ulint* offsets, ibool has_kernel_mutex)
{
	ibool is_ok = TRUE;
	ut_ad(rec_offs_validate(rec, index, offsets));
	if (!has_kernel_mutex) {
		mutex_enter(&kernel_mutex);
	}
	// A sanity check: the trx_id in rec must be smaller than the global trx id counter
	if (ut_dulint_cmp(trx_id, trx_sys->max_trx_id) >= 0) {
		ut_print_timestamp(ib_stream);
		ib_log(state, "  InnoDB: Error: transaction id associated with record\n");
		rec_print_new(ib_stream, rec, offsets);
		ib_log(state, "InnoDB: in ");
		dict_index_name_print(ib_stream, NULL, index);
		ib_log(state, "\nInnoDB: is " TRX_ID_FMT " which is higher than the global trx id counter " TRX_ID_FMT "!\nInnoDB: The table is corrupt. You have to do dump + drop + reimport.\n", TRX_ID_PREP_PRINTF(trx_id), TRX_ID_PREP_PRINTF(trx_sys->max_trx_id));
		is_ok = FALSE;
	}
	if (!has_kernel_mutex) {
		mutex_exit(&kernel_mutex);
	}
	return is_ok;
}

IB_INTERN ibool lock_clust_rec_cons_read_sees(const rec_t* rec, dict_index_t* index, const ulint* offsets, read_view_t* view)
{
	ut_ad(dict_index_is_clust(index));
	ut_ad(page_rec_is_user_rec(rec));
	ut_ad(rec_offs_validate(rec, index, offsets));
	// NOTE that we call this function while holding the search system latch. To obey the latching order we must NOT reserve the kernel mutex here!
	trx_id_t trx_id = row_get_rec_trx_id(rec, index, offsets);
	return read_view_sees_trx_id(view, trx_id);
}

IB_INTERN ulint lock_sec_rec_cons_read_sees(const rec_t* rec, const read_view_t* view)
{
	ut_ad(page_rec_is_user_rec(rec));
	// NOTE that we might call this function while holding the search system latch. To obey the latching order we must NOT reserve the kernel mutex here!
	if (recv_recovery_is_on()) {
		return FALSE;
	}
	trx_id_t max_trx_id = page_get_max_trx_id(page_align(rec));
	ut_ad(!ut_dulint_is_zero(max_trx_id));
	return ut_dulint_cmp(max_trx_id, view->up_limit_id) < 0;
}

IB_INTERN void lock_sys_create(ulint n_cells)
{
	lock_sys = IB_MEM_ALLOC(sizeof(lock_sys_t));
	lock_sys->rec_hash = hash_create(n_cells);
	// hash_create_mutexes(lock_sys->rec_hash, 2, SYNC_REC_LOCK);
	lock_latest_err_stream = os_file_create_tmpfile();
	ut_a(lock_latest_err_stream);
}

IB_INTERN void lock_sys_close(void)
{
	/* This can happen if we decide to abort during the startup phase. */
	if (lock_sys == NULL) {
		return;
	}

	/* hash_free_mutexes(lock_sys->rec_hash); */
	hash_table_free(lock_sys->rec_hash);
	lock_sys->rec_hash = NULL;

	if (lock_latest_err_stream != NULL) {
		fclose(lock_latest_err_stream);
		lock_latest_err_stream = NULL;
	}

	IB_MEM_FREE(lock_sys);
	lock_sys = NULL;
}

IB_INTERN ulint lock_get_size(void)
{
	return (ulint)sizeof(ib_lock_t);
}

IB_INLINE enum lock_mode lock_get_mode(const ib_lock_t* lock)
{
	ut_ad(lock);
	return lock->type_mode & LOCK_MODE_MASK;
}

IB_INLINE ibool lock_get_wait(const ib_lock_t* lock)
{
	ut_ad(lock);
	if (IB_UNLIKELY(lock->type_mode & LOCK_WAIT)) {
		return TRUE;
	}
	return FALSE;
}

IB_INTERN dict_table_t* lock_get_src_table(trx_t* trx, dict_table_t* dest, enum ib_lock_mode* mode)
{
	dict_table_t* src = NULL;
	*mode = LOCK_NONE;
	for (ib_lock_t* lock = UT_LIST_GET_FIRST(trx->trx_locks); lock; lock = UT_LIST_GET_NEXT(trx_locks, lock)) {
		if (!(lock_get_type_low(lock) & LOCK_TABLE)) {
			// We are only interested in table locks.
			continue;
		}
		ib_lock_table_t* tab_lock = &lock->un_member.tab_lock;
		if (dest == tab_lock->table) {
			// We are not interested in the destination table.
			continue;
		} else if (!src) {
			// This presumably is the source table.
			src = tab_lock->table;
			if (UT_LIST_GET_LEN(src->locks) != 1 || UT_LIST_GET_FIRST(src->locks) != lock) {
				// We only support the case when there is only one lock on this table.
				return NULL;
			}
		} else if (src != tab_lock->table) {
			// The transaction is locking more than two tables (src and dest): abort
			return NULL;
		}
		// Check that the source table is locked by LOCK_IX or LOCK_IS.
		enum lock_mode lock_mode = lock_get_mode(lock);
		if (lock_mode == LOCK_IX || lock_mode == LOCK_IS) {
			if (*mode != LOCK_NONE && *mode != lock_mode) {
				// There are multiple locks on src.
				return NULL;
			}
			*mode = lock_mode;
		}
	}
	if (!src) {
		// No source table lock found: flag the situation to caller
		src = dest;
	}
	return src;
}

IB_INTERN ibool lock_is_table_exclusive(dict_table_t* table, trx_t* trx)
{
	const ib_lock_t* lock;
	ibool ok = FALSE;
	ut_ad(table);
	ut_ad(trx);
	lock_mutex_enter_kernel();
	for (lock = UT_LIST_GET_FIRST(table->locks); lock; lock = UT_LIST_GET_NEXT(locks, &lock->un_member.tab_lock)) {
		if (lock->trx != trx) {
			// A lock on the table is held by some other transaction.
			goto not_ok;
		}
		if (!(lock_get_type_low(lock) & LOCK_TABLE)) {
			// We are interested in table locks only.
			continue;
		}
		switch (lock_get_mode(lock)) {
		case LOCK_IX:
			ok = TRUE;
			break;
		case LOCK_AUTO_INC:
			// It is allowed for trx to hold an auto_increment lock.
			break;
		default:
not_ok:
			// Other table locks than LOCK_IX are not allowed.
			ok = FALSE;
			goto func_exit;
		}
	}
func_exit:
	lock_mutex_exit_kernel();
	return ok;
}

IB_INLINE void lock_set_lock_and_trx_wait(ib_lock_t* lock, trx_t* trx)
{
	ut_ad(lock);
	ut_ad(trx->wait_lock == NULL);
	trx->wait_lock = lock;
	lock->type_mode |= LOCK_WAIT;
}

IB_INLINE void lock_reset_lock_and_trx_wait(ib_lock_t* lock)
{
	ut_ad(lock->trx->wait_lock == lock);
	ut_ad(lock_get_wait(lock));
	// Reset the back pointer in trx to this waiting lock request
	lock->trx->wait_lock = NULL;
	lock->type_mode &= ~LOCK_WAIT;
}

IB_INLINE ibool lock_rec_get_gap(const ib_lock_t* lock)
{
	ut_ad(lock);
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	if (lock->type_mode & LOCK_GAP) {
		return TRUE;
	}
	return FALSE;
}

IB_INLINE ibool lock_rec_get_rec_not_gap(const ib_lock_t* lock)
{
	ut_ad(lock);
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	if (lock->type_mode & LOCK_REC_NOT_GAP) {
		return TRUE;
	}
	return FALSE;
}

IB_INLINE ibool lock_rec_get_insert_intention(const ib_lock_t* lock)
{
	ut_ad(lock);
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	if (lock->type_mode & LOCK_INSERT_INTENTION) {
		return TRUE;
	}
	return FALSE;
}

IB_INLINE ulint lock_mode_stronger_or_eq(enum lock_mode mode1, enum lock_mode mode2)
{
	ut_ad(mode1 == LOCK_X || mode1 == LOCK_S || mode1 == LOCK_IX || mode1 == LOCK_IS || mode1 == LOCK_AUTO_INC);
	ut_ad(mode2 == LOCK_X || mode2 == LOCK_S || mode2 == LOCK_IX || mode2 == LOCK_IS || mode2 == LOCK_AUTO_INC);
	return (LOCK_MODE_STRONGER_OR_EQ) & LK(mode1, mode2);
}

IB_INLINE ulint lock_mode_compatible(enum lock_mode mode1, enum lock_mode mode2)
{
	ut_ad(mode1 == LOCK_X || mode1 == LOCK_S || mode1 == LOCK_IX || mode1 == LOCK_IS || mode1 == LOCK_AUTO_INC);
	ut_ad(mode2 == LOCK_X || mode2 == LOCK_S || mode2 == LOCK_IX || mode2 == LOCK_IS || mode2 == LOCK_AUTO_INC);
	return (LOCK_MODE_COMPATIBILITY) & LK(mode1, mode2);
}

IB_INLINE ibool lock_rec_has_to_wait(const trx_t* trx, ulint type_mode, const ib_lock_t* lock2, ibool lock_is_on_supremum)
{
	ut_ad(trx && lock2);
	ut_ad(lock_get_type_low(lock2) == LOCK_REC);

	if (trx != lock2->trx && !lock_mode_compatible(LOCK_MODE_MASK & type_mode, lock_get_mode(lock2))) {
		// We have somewhat complex rules when gap type record locks cause waits
		if ((lock_is_on_supremum || (type_mode & LOCK_GAP)) && !(type_mode & LOCK_INSERT_INTENTION)) {
			// Gap type locks without LOCK_INSERT_INTENTION flag do not need to wait for anything. This is because different users can have conflicting lock types on gaps.
			return FALSE;
		}
		if (!(type_mode & LOCK_INSERT_INTENTION) && lock_rec_get_gap(lock2)) {
			// Record lock (LOCK_ORDINARY or LOCK_REC_NOT_GAP does not need to wait for a gap type lock
			return FALSE;
		}
		if ((type_mode & LOCK_GAP) && lock_rec_get_rec_not_gap(lock2)) {
			// Lock on gap does not need to wait for a LOCK_REC_NOT_GAP type lock
			return FALSE;
		}
		if (lock_rec_get_insert_intention(lock2)) {
			// No lock request needs to wait for an insert intention lock to be removed. This is ok since our rules allow conflicting locks on gaps. This eliminates a spurious deadlock caused by a next-key lock waiting for an insert intention lock; when the insert intention lock was granted, the insert deadlocked on the waiting next-key lock. Also, insert intention locks do not disturb each other.
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

IB_INTERN ibool lock_has_to_wait(const ib_lock_t* lock1, const ib_lock_t* lock2)
{
	ut_ad(lock1 && lock2);
	if (lock1->trx != lock2->trx && !lock_mode_compatible(lock_get_mode(lock1), lock_get_mode(lock2))) {
		if (lock_get_type_low(lock1) == LOCK_REC) {
			ut_ad(lock_get_type_low(lock2) == LOCK_REC);
			// If this lock request is for a supremum record then the second bit on the lock bitmap is set
			return lock_rec_has_to_wait(lock1->trx, lock1->type_mode, lock2, lock_rec_get_nth_bit(lock1, 1));
		}
		return TRUE;
	}
	return FALSE;
}

/*============== RECORD LOCK BASIC FUNCTIONS ============================*/

IB_INLINE ulint lock_rec_get_n_bits(const ib_lock_t* lock)
{
	return lock->un_member.rec_lock.n_bits;
}

IB_INLINE void lock_rec_set_nth_bit(ib_lock_t* lock, ulint i)
{
	ulint byte_index = i / 8;
	ulint bit_index = i % 8;
	ut_ad(lock);
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	ut_ad(i < lock->un_member.rec_lock.n_bits);
	((byte*) &lock[1])[byte_index] |= 1 << bit_index;
}

IB_INTERN ulint lock_rec_find_set_bit(const ib_lock_t* lock)
{
	ulint i;
	for (i = 0; i < lock_rec_get_n_bits(lock); i++) {
		if (lock_rec_get_nth_bit(lock, i)) {
			return i;
		}
	}
	return ULINT_UNDEFINED;
}

IB_INLINE void lock_rec_reset_nth_bit(ib_lock_t* lock, ulint i)
{
	ulint byte_index = i / 8;
	ulint bit_index = i % 8;
	ut_ad(lock);
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	ut_ad(i < lock->un_member.rec_lock.n_bits);
	((byte*) &lock[1])[byte_index] &= ~(1 << bit_index);
}

IB_INLINE ib_lock_t* lock_rec_get_next_on_page(ib_lock_t* lock)
{
	ulint space = lock->un_member.rec_lock.space;
	ulint page_no = lock->un_member.rec_lock.page_no;
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	for (;;) {
		lock = HASH_GET_NEXT(hash, lock);
		if (!lock) {
			break;
		}
		if ((lock->un_member.rec_lock.space == space) && (lock->un_member.rec_lock.page_no == page_no)) {
			break;
		}
	}
	return lock;
}

IB_INLINE ib_lock_t* lock_rec_get_first_on_page_addr(ulint space, ulint page_no)
{
	ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	lock = HASH_GET_FIRST(lock_sys->rec_hash, lock_rec_hash(space, page_no));
	while (lock) {
		if ((lock->un_member.rec_lock.space == space) && (lock->un_member.rec_lock.page_no == page_no)) {
			break;
		}
		lock = HASH_GET_NEXT(hash, lock);
	}
	return lock;
}

IB_INTERN ibool lock_rec_expl_exist_on_page(ulint space, ulint page_no)
{
	ibool ret;
	mutex_enter(&kernel_mutex);
	if (lock_rec_get_first_on_page_addr(space, page_no)) {
		ret = TRUE;
	} else {
		ret = FALSE;
	}
	mutex_exit(&kernel_mutex);
	return ret;
}

IB_INLINE ib_lock_t* lock_rec_get_first_on_page(const buf_bib_lock_t* block)
{
	ulint hash = buf_block_get_lock_hash_val(block);
	ib_lock_t* lock;
	ulint space = buf_block_get_space(block);
	ulint page_no = buf_block_get_page_no(block);
	ut_ad(mutex_own(&kernel_mutex));
	lock = HASH_GET_FIRST(lock_sys->rec_hash, hash);
	while (lock) {
		if ((lock->un_member.rec_lock.space == space) && (lock->un_member.rec_lock.page_no == page_no)) {
			break;
		}
		lock = HASH_GET_NEXT(hash, lock);
	}
	return lock;
}

IB_INLINE ib_lock_t* lock_rec_get_next(ulint heap_no, ib_lock_t* lock)
{
	ut_ad(mutex_own(&kernel_mutex));
	do {
		ut_ad(lock_get_type_low(lock) == LOCK_REC);
		lock = lock_rec_get_next_on_page(lock);
	} while (lock && !lock_rec_get_nth_bit(lock, heap_no));
	return lock;
}

IB_INLINE ib_lock_t* lock_rec_get_first(const buf_bib_lock_t* block, ulint heap_no)
{
	ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	for (lock = lock_rec_get_first_on_page(block); lock; lock = lock_rec_get_next_on_page(lock)) {
		if (lock_rec_get_nth_bit(lock, heap_no)) {
			break;
		}
	}
	return lock;
}

static void lock_rec_bitmap_reset(ib_lock_t* lock)
{
	ulint n_bytes = lock_rec_get_n_bits(lock) / 8;
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	// Reset to zero the bitmap which resides immediately after the lock struct
	ut_ad((lock_rec_get_n_bits(lock) % 8) == 0);
	memset(&lock[1], 0, n_bytes);
}

static ib_lock_t* lock_rec_copy(const ib_lock_t* lock, mem_heap_t* heap)
{
	ulint size = sizeof(ib_lock_t) + lock_rec_get_n_bits(lock) / 8;
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	return mem_heap_dup(heap, lock, size);
}

IB_INTERN const ib_lock_t* lock_rec_get_prev(const ib_lock_t* in_lock, ulint heap_no)
{
	ib_lock_t* lock;
	ulint space = in_lock->un_member.rec_lock.space;
	ulint page_no = in_lock->un_member.rec_lock.page_no;
	ib_lock_t* found_lock = NULL;
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(lock_get_type_low(in_lock) == LOCK_REC);
	lock = lock_rec_get_first_on_page_addr(space, page_no);
	for (;;) {
		ut_ad(lock);
		if (lock == in_lock) {
			return found_lock;
		}
		if (lock_rec_get_nth_bit(lock, heap_no)) {
			found_lock = lock;
		}
		lock = lock_rec_get_next_on_page(lock);
	}
}

/*============= FUNCTIONS FOR ANALYZING TABLE LOCK QUEUE ================*/

IB_INLINE ib_lock_t* ib_lock_table_has(trx_t* trx, dict_table_t* table, enum lock_mode mode)
{
	ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	// Look for stronger locks the same trx already has on the table
	lock = UT_LIST_GET_LAST(table->locks);
	while (lock != NULL) {
		if (lock->trx == trx && lock_mode_stronger_or_eq(lock_get_mode(lock), mode)) {
			// The same trx already has locked the table in a mode stronger or equal to the mode given
			ut_ad(!lock_get_wait(lock));
			return lock;
		}
		lock = UT_LIST_GET_PREV(un_member.tab_lock.locks, lock);
	}
	return NULL;
}

/*============= FUNCTIONS FOR ANALYZING RECORD LOCK QUEUE ================*/

IB_INLINE ib_lock_t* lock_rec_has_expl(ulint precise_mode, const buf_bib_lock_t* block, ulint heap_no, trx_t* trx)
{
	ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad((precise_mode & LOCK_MODE_MASK) == LOCK_S || (precise_mode & LOCK_MODE_MASK) == LOCK_X);
	ut_ad(!(precise_mode & LOCK_INSERT_INTENTION));
	lock = lock_rec_get_first(block, heap_no);
	while (lock) {
		if (lock->trx == trx && lock_mode_stronger_or_eq(lock_get_mode(lock), precise_mode & LOCK_MODE_MASK) && !lock_get_wait(lock) && (!lock_rec_get_rec_not_gap(lock) || (precise_mode & LOCK_REC_NOT_GAP) || heap_no == PAGE_HEAP_NO_SUPREMUM) && (!lock_rec_get_gap(lock) || (precise_mode & LOCK_GAP) || heap_no == PAGE_HEAP_NO_SUPREMUM) && (!lock_rec_get_insert_intention(lock))) {
			return lock;
		}
		lock = lock_rec_get_next(heap_no, lock);
	}
	return NULL;
}

#ifdef IB_DEBUG
static ib_lock_t* lock_rec_other_has_expl_req(enum lock_mode mode, ulint gap, ulint wait, const buf_bib_lock_t* block, ulint heap_no, const trx_t* trx)
{
	ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(mode == LOCK_X || mode == LOCK_S);
	ut_ad(gap == 0 || gap == LOCK_GAP);
	ut_ad(wait == 0 || wait == LOCK_WAIT);
	lock = lock_rec_get_first(block, heap_no);
	while (lock) {
		if (lock->trx != trx && (gap || !(lock_rec_get_gap(lock) || heap_no == PAGE_HEAP_NO_SUPREMUM)) && (wait || !lock_get_wait(lock)) && lock_mode_stronger_or_eq(lock_get_mode(lock), mode)) {
			return lock;
		}
		lock = lock_rec_get_next(heap_no, lock);
	}
	return NULL;
}
#endif // IB_DEBUG

static ib_lock_t* lock_rec_other_has_conflicting(enum lock_mode mode, const buf_bib_lock_t* block, ulint heap_no, trx_t* trx)
{
	ut_ad(mutex_own(&kernel_mutex));
	ib_lock_t* lock = lock_rec_get_first(block, heap_no);
	if (IB_LIKELY_NULL(lock)) {
		if (IB_UNLIKELY(heap_no == PAGE_HEAP_NO_SUPREMUM)) {
			do {
				if (lock_rec_has_to_wait(trx, mode, lock, TRUE)) {
					return lock;
				}
				lock = lock_rec_get_next(heap_no, lock);
			} while (lock);
		} else {
			do {
				if (lock_rec_has_to_wait(trx, mode, lock, FALSE)) {
					return lock;
				}
				lock = lock_rec_get_next(heap_no, lock);
			} while (lock);
		}
	}
	return NULL;
}

IB_INLINE ib_lock_t* lock_rec_find_similar_on_page(ulint type_mode, ulint heap_no, ib_lock_t* lock, const trx_t* trx)
{
	ut_ad(mutex_own(&kernel_mutex));
	while (lock != NULL) {
		if (lock->trx == trx && lock->type_mode == type_mode && lock_rec_get_n_bits(lock) > heap_no) {
			return lock;
		}
		lock = lock_rec_get_next_on_page(lock);
	}
	return NULL;
}

static trx_t* lock_sec_rec_some_has_impl_off_kernel(const rec_t* rec, dict_index_t* index, const ulint* offsets)
{
	const page_t* page = page_align(rec);
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(!dict_index_is_clust(index));
	ut_ad(page_rec_is_user_rec(rec));
	ut_ad(rec_offs_validate(rec, index, offsets));
	// Some transaction may have an implicit x-lock on the record only if the max trx id for the page >= min trx id for the trx list, or database recovery is running. We do not write the changes of a page max trx id to the log, and therefore during recovery, this value for a page may be incorrect.
	if (!(ut_dulint_cmp(page_get_max_trx_id(page), trx_list_get_min_trx_id()) >= 0) && !recv_recovery_is_on()) {
		return NULL;
	}
	// Ok, in this case it is possible that some transaction has an implicit x-lock. We have to look in the clustered index.
	if (!lock_check_trx_id_sanity(page_get_max_trx_id(page), rec, index, offsets, TRUE)) {
		buf_page_print(page, 0);
		// The page is corrupt: try to avoid a crash by returning NULL
		return NULL;
	}
	return row_vers_impl_x_locked_off_kernel(rec, index, offsets);
}

IB_INTERN ulint lock_number_of_rows_locked(trx_t* trx)
{
	ib_lock_t* lock;
	ulint n_records = 0;
	ulint n_bits;
	ulint n_bit;
	lock = UT_LIST_GET_FIRST(trx->trx_locks);
	while (lock) {
		if (lock_get_type_low(lock) == LOCK_REC) {
			n_bits = lock_rec_get_n_bits(lock);
			for (n_bit = 0; n_bit < n_bits; n_bit++) {
				if (lock_rec_get_nth_bit(lock, n_bit)) {
					n_records++;
				}
			}
		}
		lock = UT_LIST_GET_NEXT(trx_locks, lock);
	}
	return n_records;
}

/*============== RECORD LOCK CREATION AND QUEUE MANAGEMENT =============*/

static ib_lock_t* lock_rec_create_low(ulint type_mode, ulint space, ulint page_no, ulint heap_no, ulint n_bits, dict_index_t* index, trx_t* trx)
{
	ut_ad(mutex_own(&kernel_mutex));
	if (IB_UNLIKELY(heap_no == PAGE_HEAP_NO_SUPREMUM)) {
		ut_ad(!(type_mode & LOCK_REC_NOT_GAP));
		type_mode = type_mode & ~(LOCK_GAP | LOCK_REC_NOT_GAP);
	}
    ulint n_bytes = 1 + (n_bits + LOCK_PAGE_BITMAP_MARGIN) / 8;
    ib_lock_t* lock = mem_heap_alloc(trx->lock_heap, sizeof(ib_lock_t) + n_bytes);
	UT_LIST_ADD_LAST(trx_locks, trx->trx_locks, lock);
	lock->trx = trx;
	lock->type_mode = (type_mode & ~LOCK_TYPE_MASK) | LOCK_REC;
	lock->index = index;
	lock->un_member.rec_lock.space = space;
	lock->un_member.rec_lock.page_no = page_no;
	lock->un_member.rec_lock.n_bits = n_bytes * 8;
	lock_rec_bitmap_reset(lock);
	lock_rec_set_nth_bit(lock, heap_no);
    HASH_INSERT(ib_lock_t, hash, lock_sys->rec_hash, lock_rec_fold(space, page_no), lock);
	if (IB_UNLIKELY(type_mode & LOCK_WAIT)) {
		lock_set_lock_and_trx_wait(lock, trx);
	}
    return lock;
}

static ib_lock_t* lock_rec_create(ulint type_mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, trx_t* trx) {
    const page_t* page;
    ulint space;
    ulint n_bits;
    ulint page_no;
	ut_ad(mutex_own(&kernel_mutex));
	space = buf_block_get_space(block);
    page_no = buf_block_get_page_no(block);
	page = block->frame;
	ut_ad(!!page_is_comp(page) == dict_table_is_comp(index->table));
	n_bits = page_dir_get_n_heap(page);
    return lock_rec_create_low(type_mode, space, page_no, heap_no, n_bits, index, trx);
}

static ulint lock_rec_enqueue_waiting(ulint type_mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, que_thr_t* thr) {
    ib_lock_t* lock;
    trx_t* trx;
	ut_ad(mutex_own(&kernel_mutex));
	if (IB_UNLIKELY(que_thr_stop(thr))) {
		UT_ERROR;
        return DB_QUE_THR_SUSPENDED;
	}
	trx = thr_get_trx(thr);
	switch (trx_get_dict_operation(trx)) {
	case TRX_DICT_OP_NONE:
		break;
	case TRX_DICT_OP_TABLE:
	case TRX_DICT_OP_INDEX:
		ut_print_timestamp(ib_stream);
        ib_log(state, "  InnoDB: Error: a record lock wait happens in a dictionary operation!\nInnoDB: ");
		dict_index_name_print(ib_stream, trx, index);
        ib_log(state, ".\nInnoDB: Submit a detailed bug report check the InnoDB website for details");
    }
    lock = lock_rec_create(type_mode | LOCK_WAIT, block, heap_no, index, trx);
	if (IB_UNLIKELY(lock_deadlock_occurs(lock, trx))) {
		lock_reset_lock_and_trx_wait(lock);
		lock_rec_reset_nth_bit(lock, heap_no);
        return DB_DEADLOCK;
	}
	if (trx->wait_lock == NULL) {
        return DB_SUCCESS;
	}
	trx->que_state = TRX_QUE_LOCK_WAIT;
	trx->was_chosen_as_deadlock_victim = FALSE;
	trx->wait_started = time(NULL);
	ut_a(que_thr_stop(thr));
	if constexpr (IB_DEBUG) {
		if (lock_print_waits) {
			ib_log(state, "Lock wait for trx %lu in index ", (ulong) ut_dulint_get_low(trx->id));
			ut_print_name(ib_stream, trx, FALSE, index->name);
		}
	}
    return DB_LOCK_WAIT;
}

static ib_lock_t* lock_rec_add_to_queue(ulint type_mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, trx_t* trx) {
    ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
#ifdef IB_DEBUG
	switch (type_mode & LOCK_MODE_MASK) {
	case LOCK_X:
	case LOCK_S:
		break;
	default:
		UT_ERROR;
	}
	if (!(type_mode & (LOCK_WAIT | LOCK_GAP))) {
        enum lock_mode mode = (type_mode & LOCK_MODE_MASK) == LOCK_S ? LOCK_X : LOCK_S;
        ib_lock_t* other_lock = lock_rec_other_has_expl_req(mode, 0, LOCK_WAIT, block, heap_no, trx);
		ut_a(!other_lock);
	}
#endif /* IB_DEBUG */
	type_mode |= LOCK_REC;
	if (IB_UNLIKELY(heap_no == PAGE_HEAP_NO_SUPREMUM)) {
		ut_ad(!(type_mode & LOCK_REC_NOT_GAP));
		type_mode = type_mode & ~(LOCK_GAP | LOCK_REC_NOT_GAP);
	}
	lock = lock_rec_get_first_on_page(block);
	while (lock != NULL) {
        if (lock_get_wait(lock) && (lock_rec_get_nth_bit(lock, heap_no))) {
			goto somebody_waits;
		}
		lock = lock_rec_get_next_on_page(lock);
	}
	if (IB_LIKELY(!(type_mode & LOCK_WAIT))) {
        lock = lock_rec_find_similar_on_page(type_mode, heap_no, lock_rec_get_first_on_page(block), trx);
		if (lock) {
			lock_rec_set_nth_bit(lock, heap_no);
            return lock;
		}
	}
somebody_waits:
    return lock_rec_create(type_mode, block, heap_no, index, trx);
}

IB_INLINE ibool lock_rec_lock_fast(ibool impl, ulint mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, que_thr_t* thr) {
    ib_lock_t* lock;
    trx_t* trx;
	ut_ad(mutex_own(&kernel_mutex));
    ut_ad((LOCK_MODE_MASK & mode) != LOCK_S || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IS));
    ut_ad((LOCK_MODE_MASK & mode) != LOCK_X || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IX));
    ut_ad((LOCK_MODE_MASK & mode) == LOCK_S || (LOCK_MODE_MASK & mode) == LOCK_X);
    ut_ad(mode - (LOCK_MODE_MASK & mode) == LOCK_GAP || mode - (LOCK_MODE_MASK & mode) == 0 || mode - (LOCK_MODE_MASK & mode) == LOCK_REC_NOT_GAP);
	lock = lock_rec_get_first_on_page(block);
	trx = thr_get_trx(thr);
	if (lock == NULL) {
		if (!impl) {
			lock_rec_create(mode, block, heap_no, index, trx);
		}
        return TRUE;
	}
	if (lock_rec_get_next_on_page(lock)) {
        return FALSE;
    }
    if (lock->trx != trx || lock->type_mode != (mode | LOCK_REC) || lock_rec_get_n_bits(lock) <= heap_no) {
        return FALSE;
    }
	if (!impl) {
		if (!lock_rec_get_nth_bit(lock, heap_no)) {
			lock_rec_set_nth_bit(lock, heap_no);
		}
	}
    return TRUE;
}

static ulint lock_rec_lock_slow(ibool impl, ulint mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, que_thr_t* thr) {
    trx_t* trx;
    ulint err;
	ut_ad(mutex_own(&kernel_mutex));
    ut_ad((LOCK_MODE_MASK & mode) != LOCK_S || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IS));
    ut_ad((LOCK_MODE_MASK & mode) != LOCK_X || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IX));
    ut_ad((LOCK_MODE_MASK & mode) == LOCK_S || (LOCK_MODE_MASK & mode) == LOCK_X);
    ut_ad(mode - (LOCK_MODE_MASK & mode) == LOCK_GAP || mode - (LOCK_MODE_MASK & mode) == 0 || mode - (LOCK_MODE_MASK & mode) == LOCK_REC_NOT_GAP);
	trx = thr_get_trx(thr);
	if (lock_rec_has_expl(mode, block, heap_no, trx)) {
		err = DB_SUCCESS;
	} else if (lock_rec_other_has_conflicting(mode, block, heap_no, trx)) {
        err = lock_rec_enqueue_waiting(mode, block, heap_no, index, thr);
	} else {
		if (!impl) {
            lock_rec_add_to_queue(LOCK_REC | mode, block, heap_no, index, trx);
		}
		err = DB_SUCCESS;
	}
    return err;
}

static ulint lock_rec_lock(ibool impl, ulint mode, const buf_bib_lock_t* block, ulint heap_no, dict_index_t* index, que_thr_t* thr) {
    ulint err;
	ut_ad(mutex_own(&kernel_mutex));
    ut_ad((LOCK_MODE_MASK & mode) != LOCK_S || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IS));
    ut_ad((LOCK_MODE_MASK & mode) != LOCK_X || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IX));
    ut_ad((LOCK_MODE_MASK & mode) == LOCK_S || (LOCK_MODE_MASK & mode) == LOCK_X);
    ut_ad(mode - (LOCK_MODE_MASK & mode) == LOCK_GAP || mode - (LOCK_MODE_MASK & mode) == LOCK_REC_NOT_GAP || mode - (LOCK_MODE_MASK & mode) == 0);
	if (lock_rec_lock_fast(impl, mode, block, heap_no, index, thr)) {
		err = DB_SUCCESS;
	} else {
        err = lock_rec_lock_slow(impl, mode, block, heap_no, index, thr);
    }
    return err;
}

static ibool lock_rec_has_to_wait_in_queue(ib_lock_t* wait_lock) {
    ib_lock_t* lock;
    ulint space;
    ulint page_no;
    ulint heap_no;
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(lock_get_wait(wait_lock));
	ut_ad(lock_get_type_low(wait_lock) == LOCK_REC);
	space = wait_lock->un_member.rec_lock.space;
	page_no = wait_lock->un_member.rec_lock.page_no;
	heap_no = lock_rec_find_set_bit(wait_lock);
	lock = lock_rec_get_first_on_page_addr(space, page_no);
	while (lock != wait_lock) {
        if (lock_rec_get_nth_bit(lock, heap_no) && lock_has_to_wait(wait_lock, lock)) {
            return TRUE;
        }
		lock = lock_rec_get_next_on_page(lock);
	}
    return FALSE;
}

static void lock_grant(ib_lock_t* lock) {
	ut_ad(mutex_own(&kernel_mutex));
	lock_reset_lock_and_trx_wait(lock);
	if constexpr (IB_DEBUG) {
		if (lock_print_waits) {
			ib_log(state, "Lock wait for trx %lu ends\n", (ulong) ut_dulint_get_low(lock->trx->id));
		}
	}
	if (lock->trx->que_state == TRX_QUE_LOCK_WAIT) {
		trx_end_lock_wait(lock->trx);
	}
}

static void lock_rec_cancel(ib_lock_t* lock) {
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(lock_get_type_low(lock) == LOCK_REC);
	lock_rec_reset_nth_bit(lock, lock_rec_find_set_bit(lock));
	lock_reset_lock_and_trx_wait(lock);
	trx_end_lock_wait(lock->trx);
}

static void lock_rec_dequeue_from_page(ib_lock_t* in_lock) {
    ulint space;
    ulint page_no;
    ib_lock_t* lock;
    trx_t* trx;
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(lock_get_type_low(in_lock) == LOCK_REC);
	trx = in_lock->trx;
	space = in_lock->un_member.rec_lock.space;
	page_no = in_lock->un_member.rec_lock.page_no;
    HASH_DELETE(ib_lock_t, hash, lock_sys->rec_hash, lock_rec_fold(space, page_no), in_lock);
	UT_LIST_REMOVE(trx_locks, trx->trx_locks, in_lock);
	lock = lock_rec_get_first_on_page_addr(space, page_no);
	while (lock != NULL) {
        if (lock_get_wait(lock) && !lock_rec_has_to_wait_in_queue(lock)) {
			lock_grant(lock);
		}
		lock = lock_rec_get_next_on_page(lock);
	}
}

static void lock_rec_discard(ib_lock_t* in_lock) {
    ulint space;
    ulint page_no;
    trx_t* trx;
	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(lock_get_type_low(in_lock) == LOCK_REC);
	trx = in_lock->trx;
	space = in_lock->un_member.rec_lock.space;
	page_no = in_lock->un_member.rec_lock.page_no;
    HASH_DELETE(ib_lock_t, hash, lock_sys->rec_hash, lock_rec_fold(space, page_no), in_lock);
	UT_LIST_REMOVE(trx_locks, trx->trx_locks, in_lock);
}

static void lock_rec_free_all_from_discard_page(const buf_bib_lock_t* block) {
    ulint space;
    ulint page_no;
    ib_lock_t* lock;
    ib_lock_t* next_lock;
	ut_ad(mutex_own(&kernel_mutex));
	space = buf_block_get_space(block);
	page_no = buf_block_get_page_no(block);
	lock = lock_rec_get_first_on_page_addr(space, page_no);
	while (lock != NULL) {
		ut_ad(lock_rec_find_set_bit(lock) == ULINT_UNDEFINED);
		ut_ad(!lock_get_wait(lock));
		next_lock = lock_rec_get_next_on_page(lock);
		lock_rec_discard(lock);
		lock = next_lock;
	}
}

/*============= RECORD LOCK MOVING AND INHERITING ===================*/

static void lock_rec_reset_and_release_wait(const buf_bib_lock_t* block, ulint heap_no) {
    ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	lock = lock_rec_get_first(block, heap_no);
	while (lock != NULL) {
		if (lock_get_wait(lock)) {
			lock_rec_cancel(lock);
		} else {
			lock_rec_reset_nth_bit(lock, heap_no);
		}
		lock = lock_rec_get_next(heap_no, lock);
	}
}

static void lock_rec_inherit_to_gap(const buf_bib_lock_t* heir_block, const buf_bib_lock_t* block, ulint heir_heap_no, ulint heap_no) {
    ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	lock = lock_rec_get_first(block, heap_no);
	while (lock != NULL) {
        if (!lock_rec_get_insert_intention(lock) && lock->trx->isolation_level != TRX_ISO_READ_COMMITTED && lock_get_mode(lock) == LOCK_X) {
            lock_rec_add_to_queue(LOCK_REC | LOCK_GAP | lock_get_mode(lock), heir_block, heir_heap_no, lock->index, lock->trx);
        }
		lock = lock_rec_get_next(heap_no, lock);
	}
}

static void lock_rec_inherit_to_gap_if_gap_lock(const buf_bib_lock_t* block, ulint heir_heap_no, ulint heap_no) {
    ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	lock = lock_rec_get_first(block, heap_no);
	while (lock != NULL) {
        if (!lock_rec_get_insert_intention(lock) && (heap_no == PAGE_HEAP_NO_SUPREMUM || !lock_rec_get_rec_not_gap(lock))) {
            lock_rec_add_to_queue(LOCK_REC | LOCK_GAP | lock_get_mode(lock), block, heir_heap_no, lock->index, lock->trx);
        }
		lock = lock_rec_get_next(heap_no, lock);
	}
}

static void lock_rec_move(const buf_bib_lock_t* receiver, const buf_bib_lock_t* donator, ulint receiver_heap_no, ulint donator_heap_no) {
    ib_lock_t* lock;
	ut_ad(mutex_own(&kernel_mutex));
	lock = lock_rec_get_first(donator, donator_heap_no);
	ut_ad(lock_rec_get_first(receiver, receiver_heap_no) == NULL);
	while (lock != NULL) {
        const ulint type_mode = lock->type_mode;
		lock_rec_reset_nth_bit(lock, donator_heap_no);
		if (IB_UNLIKELY(type_mode & LOCK_WAIT)) {
			lock_reset_lock_and_trx_wait(lock);
		}
        lock_rec_add_to_queue(type_mode, receiver, receiver_heap_no, lock->index, lock->trx);
		lock = lock_rec_get_next(donator_heap_no, lock);
	}
	ut_ad(lock_rec_get_first(donator, donator_heap_no) == NULL);
}

// -----------------------------------------------------------------------------------------
// MOVE group (alphabetical)
// -----------------------------------------------------------------------------------------
IB_INTERN void lock_move_rec_list_end(const buf_bib_lock_t* new_block, const buf_bib_lock_t* block, const rec_t* rec)
{
    const ulint comp = page_rec_is_comp(rec);
    lock_mutex_enter_kernel();
    // Note: when we move locks from record to record, waiting locks and possible granted gap type locks behind them are enqueued in the original order, because new elements are inserted to a hash table to the end of the hash chain, and lock_rec_add_to_queue does not reuse locks if there are waiters in the queue.
    for (ib_lock_t* lock = lock_rec_get_first_on_page(block); lock; lock = lock_rec_get_next_on_page(lock)) {
        const ulint type_mode = lock->type_mode;
        page_cur_t cur1;
        page_cur_position(rec, block, &cur1);
        if (page_cur_is_before_first(&cur1)) {
            page_cur_move_to_next(&cur1);
        }
        page_cur_t cur2;
        page_cur_set_before_first(new_block, &cur2);
        page_cur_move_to_next(&cur2);
        // Copy lock requests on user records to new page and reset the lock bits on the old
        while (!page_cur_is_after_last(&cur1)) {
            ulint heap_no;
            if (comp) {
                heap_no = rec_get_heap_no_new(page_cur_get_rec(&cur1));
            } else {
                heap_no = rec_get_heap_no_old(page_cur_get_rec(&cur1));
                ut_ad(!memcmp(page_cur_get_rec(&cur1), page_cur_get_rec(&cur2), rec_get_data_size_old(page_cur_get_rec(&cur2))));
            }
            if (lock_rec_get_nth_bit(lock, heap_no)) {
                lock_rec_reset_nth_bit(lock, heap_no);
                if (IB_UNLIKELY(type_mode & LOCK_WAIT)) {
                    lock_reset_lock_and_trx_wait(lock);
                }
                if (comp) {
                    heap_no = rec_get_heap_no_new(page_cur_get_rec(&cur2));
                } else {
                    heap_no = rec_get_heap_no_old(page_cur_get_rec(&cur2));
                }
                lock_rec_add_to_queue(type_mode, new_block, heap_no, lock->index, lock->trx);
            }
            page_cur_move_to_next(&cur1);
            page_cur_move_to_next(&cur2);
        }
    }
    lock_mutex_exit_kernel();
#ifdef IB_DEBUG_LOCK_VALIDATE
    ut_ad(lock_rec_validate_page(buf_block_get_space(block), buf_block_get_zip_size(block), buf_block_get_page_no(block)));
    ut_ad(lock_rec_validate_page(buf_block_get_space(new_block), buf_block_get_zip_size(block), buf_block_get_page_no(new_block)));
#endif
}

IB_INTERN void lock_move_rec_list_start(const buf_bib_lock_t* new_block, const buf_bib_lock_t* block, const rec_t* rec, const rec_t* old_end)
{
    const ulint comp = page_rec_is_comp(rec);
    ut_ad(block->frame == page_align(rec));
    ut_ad(new_block->frame == page_align(old_end));
    lock_mutex_enter_kernel();
    for (ib_lock_t* lock = lock_rec_get_first_on_page(block); lock != nullptr; lock = lock_rec_get_next_on_page(lock)) {
        page_cur_t cur1;
        page_cur_t cur2;
        const ulint type_mode = lock->type_mode;
        page_cur_set_before_first(block, &cur1);
        page_cur_move_to_next(&cur1);
        page_cur_position(old_end, new_block, &cur2);
        page_cur_move_to_next(&cur2);
        /* Copy lock requests on user records to new page and reset the lock bits on the old */
        while (page_cur_get_rec(&cur1) != rec) {
            ulint heap_no;
            if (comp) {
                heap_no = rec_get_heap_no_new(page_cur_get_rec(&cur1));
            } else {
                heap_no = rec_get_heap_no_old(page_cur_get_rec(&cur1));
                ut_ad(!memcmp(page_cur_get_rec(&cur1), page_cur_get_rec(&cur2), rec_get_data_size_old(page_cur_get_rec(&cur2))));
            }
            if (lock_rec_get_nth_bit(lock, heap_no)) {
                lock_rec_reset_nth_bit(lock, heap_no);
                if (IB_UNLIKELY(type_mode & LOCK_WAIT)) {
                    lock_reset_lock_and_trx_wait(lock);
                }
                if (comp) {
                    heap_no = rec_get_heap_no_new(page_cur_get_rec(&cur2));
                } else {
                    heap_no = rec_get_heap_no_old(page_cur_get_rec(&cur2));
                }
                lock_rec_add_to_queue(type_mode, new_block, heap_no, lock->index, lock->trx);
            }
            page_cur_move_to_next(&cur1);
            page_cur_move_to_next(&cur2);
        }

    if constexpr (IB_DEBUG) {
        if (page_rec_is_supremum(rec)) {
            for (ulint i = PAGE_HEAP_NO_USER_LOW; i < lock_rec_get_n_bits(lock); i++) {
                if (IB_UNLIKELY(lock_rec_get_nth_bit(lock, i))) {
                    ib_log(state, "lock_move_rec_list_start(): %lu not moved in %p\n", (ulong) i, (void*) lock);
                    UT_ERROR;
                }
            }
        }
    }
    }
    lock_mutex_exit_kernel();
#ifdef IB_DEBUG_LOCK_VALIDATE
    ut_ad(lock_rec_validate_page(buf_block_get_space(block), buf_block_get_zip_size(block), buf_block_get_page_no(block)));
#endif
}

IB_INTERN void lock_move_reorganize_page(const buf_bib_lock_t* block, const buf_bib_lock_t* oblock)
{
    lock_mutex_enter_kernel();
    ib_lock_t* lock = lock_rec_get_first_on_page(block);
    if (lock == NULL) {
        lock_mutex_exit_kernel();
        return;
    }
    mem_heap_t* heap = IB_MEM_HEAP_CREATE(256);
    // Copy first all the locks on the page to heap and reset the bitmaps in the original locks; chain the copies of the locks using the trx_locks field in them.
    UT_LIST_BASE_NODE_T(ib_lock_t) old_locks;
    UT_LIST_INIT(old_locks);
    do {
        // Make a copy of the lock
        ib_lock_t* old_lock = lock_rec_copy(lock, heap);
        UT_LIST_ADD_LAST(trx_locks, old_locks, old_lock);
        // Reset bitmap of lock
        lock_rec_bitmap_reset(lock);
        if (lock_get_wait(lock)) {
            lock_reset_lock_and_trx_wait(lock);
        }
        lock = lock_rec_get_next_on_page(lock);
    } while (lock != NULL);
    ulint comp = page_is_comp(block->frame);
    ut_ad(comp == page_is_comp(oblock->frame));
    for (lock = UT_LIST_GET_FIRST(old_locks); lock; lock = UT_LIST_GET_NEXT(trx_locks, lock)) {
        // NOTE: we copy also the locks set on the infimum and supremum of the page; the infimum may carry locks if an update of a record is occurring on the page, and its locks were temporarily stored on the infimum
        page_cur_t cur1;
        page_cur_t cur2;
        page_cur_set_before_first(block, &cur1);
        page_cur_set_before_first(oblock, &cur2);
        // Set locks according to old locks
        for (;;) {
            ut_ad(comp || !memcmp(page_cur_get_rec(&cur1), page_cur_get_rec(&cur2), rec_get_data_size_old(page_cur_get_rec(&cur2))));
            ulint old_heap_no;
            ulint new_heap_no;
            if (IB_LIKELY(comp)) {
                old_heap_no = rec_get_heap_no_new(page_cur_get_rec(&cur2));
                new_heap_no = rec_get_heap_no_new(page_cur_get_rec(&cur1));
            } else {
                old_heap_no = rec_get_heap_no_old(page_cur_get_rec(&cur2));
                new_heap_no = rec_get_heap_no_old(page_cur_get_rec(&cur1));
            }
            if (lock_rec_get_nth_bit(lock, old_heap_no)) {
                // Clear the bit in old_lock.
                ut_d(lock_rec_reset_nth_bit(lock, old_heap_no));
                // NOTE that the old lock bitmap could be too small for the new heap number!
                lock_rec_add_to_queue(lock->type_mode, block, new_heap_no, lock->index, lock->trx);
            }
            if (IB_UNLIKELY(new_heap_no == PAGE_HEAP_NO_SUPREMUM)) {
                ut_ad(old_heap_no == PAGE_HEAP_NO_SUPREMUM);
                break;
            }
            page_cur_move_to_next(&cur1);
            page_cur_move_to_next(&cur2);
        }
    if constexpr (IB_DEBUG) {
        ulint i = lock_rec_find_set_bit(lock);
        // Check that all locks were moved.
        if (IB_UNLIKELY(i != ULINT_UNDEFINED)) {
            ib_log(state, "lock_move_reorganize_page(): %lu not moved in %p\n", (ulong) i, (void*) lock);
            UT_ERROR;
        }
    }
    }
    lock_mutex_exit_kernel();
    IB_MEM_HEAP_FREE(heap);
#ifdef IB_DEBUG_LOCK_VALIDATE
    ut_ad(lock_rec_validate_page(buf_block_get_space(block), buf_block_get_zip_size(block), buf_block_get_page_no(block)));
#endif
}

// -----------------------------------------------------------------------------------------
// UPDATE group (alphabetical)
// -----------------------------------------------------------------------------------------
IB_INTERN void lock_update_copy_and_discard(const buf_bib_lock_t* new_block, const buf_bib_lock_t* block)
{
    lock_mutex_enter_kernel();
    // Move the locks on the supremum of the old page to the supremum of new_page
    lock_rec_move(new_block, block, PAGE_HEAP_NO_SUPREMUM, PAGE_HEAP_NO_SUPREMUM);
    lock_rec_free_all_from_discard_page(block);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_delete(const buf_bib_lock_t* block, const rec_t* rec)
{
    const page_t* page = block->frame;
    ut_ad(page == page_align(rec));
    ulint heap_no;
    ulint next_heap_no;
    if (page_is_comp(page)) {
        heap_no = rec_get_heap_no_new(rec);
        next_heap_no = rec_get_heap_no_new(page + rec_get_next_offs(rec, TRUE));
    } else {
        heap_no = rec_get_heap_no_old(rec);
        next_heap_no = rec_get_heap_no_old(page + rec_get_next_offs(rec, FALSE));
    }
    lock_mutex_enter_kernel();
    // Let the next record inherit the locks from rec, in gap mode
    lock_rec_inherit_to_gap(block, block, next_heap_no, heap_no);
    // Reset the lock bits on rec and release waiting transactions
    lock_rec_reset_and_release_wait(block, heap_no);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_discard(const buf_bib_lock_t* heir_block, ulint heir_heap_no, const buf_bib_lock_t* block)
{
    const page_t* page = block->frame;
    lock_mutex_enter_kernel();
    if (!lock_rec_get_first_on_page(block)) {
        // No locks exist on page, nothing to do
        lock_mutex_exit_kernel();
        return;
    }
    /* Inherit all the locks on the page to the record and reset all the locks on the page */
    if (page_is_comp(page)) {
        const rec_t* rec = page + PAGE_NEW_INFIMUM;
        ulint heap_no;
        do {
            heap_no = rec_get_heap_no_new(rec);
            lock_rec_inherit_to_gap(heir_block, block, heir_heap_no, heap_no);
            lock_rec_reset_and_release_wait(block, heap_no);
            rec = page + rec_get_next_offs(rec, TRUE);
        } while (heap_no != PAGE_HEAP_NO_SUPREMUM);
    } else {
        const rec_t* rec = page + PAGE_OLD_INFIMUM;
        ulint heap_no;
        do {
            heap_no = rec_get_heap_no_old(rec);
            lock_rec_inherit_to_gap(heir_block, block, heir_heap_no, heap_no);
            lock_rec_reset_and_release_wait(block, heap_no);
            rec = page + rec_get_next_offs(rec, FALSE);
        } while (heap_no != PAGE_HEAP_NO_SUPREMUM);
    }
    lock_rec_free_all_from_discard_page(block);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_insert(const buf_bib_lock_t* block, const rec_t* rec)
{
    ut_ad(block->frame == page_align(rec));
    // Inherit the gap-locking locks for rec, in gap mode, from the next record
    ulint receiver_heap_no;
    ulint donator_heap_no;
    if (page_rec_is_comp(rec)) {
        receiver_heap_no = rec_get_heap_no_new(rec);
        donator_heap_no = rec_get_heap_no_new(page_rec_get_next_low(rec, TRUE));
    } else {
        receiver_heap_no = rec_get_heap_no_old(rec);
        donator_heap_no = rec_get_heap_no_old(page_rec_get_next_low(rec, FALSE));
    }
    lock_mutex_enter_kernel();
    lock_rec_inherit_to_gap_if_gap_lock(block, receiver_heap_no, donator_heap_no);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_merge_left(const buf_bib_lock_t* left_block, const rec_t* orig_pred, const buf_bib_lock_t* right_block)
{
    ut_ad(left_block->frame == page_align(orig_pred));
    lock_mutex_enter_kernel();
    const rec_t* left_next_rec = page_rec_get_next_const(orig_pred);
    if (!page_rec_is_supremum(left_next_rec)) {
        // Inherit the locks on the supremum of the left page to the first record which was moved from the right page
        lock_rec_inherit_to_gap(left_block, left_block, page_rec_get_heap_no(left_next_rec), PAGE_HEAP_NO_SUPREMUM);
        // Reset the locks on the supremum of the left page, releasing waiting transactions
        lock_rec_reset_and_release_wait(left_block, PAGE_HEAP_NO_SUPREMUM);
    }
    // Move the locks from the supremum of right page to the supremum of the left page
    lock_rec_move(left_block, right_block, PAGE_HEAP_NO_SUPREMUM, PAGE_HEAP_NO_SUPREMUM);
    lock_rec_free_all_from_discard_page(right_block);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_merge_right(const buf_bib_lock_t* right_block, const rec_t* orig_succ, const buf_bib_lock_t* left_block)
{
    lock_mutex_enter_kernel();
    /* Inherit the locks from the supremum of the left page to the original successor of infimum on the right page, to which the left page was merged */
    lock_rec_inherit_to_gap(right_block, left_block, page_rec_get_heap_no(orig_succ), PAGE_HEAP_NO_SUPREMUM);
    /* Reset the locks on the supremum of the left page, releasing waiting transactions */
    lock_rec_reset_and_release_wait(left_block, PAGE_HEAP_NO_SUPREMUM);
    lock_rec_free_all_from_discard_page(left_block);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_root_raise(const buf_bib_lock_t* block, const buf_bib_lock_t* root)
{
    lock_mutex_enter_kernel();
    // Move the locks on the supremum of the root to the supremum of block 
    lock_rec_move(block, root, PAGE_HEAP_NO_SUPREMUM, PAGE_HEAP_NO_SUPREMUM);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_split_left(const buf_bib_lock_t* right_block, const buf_bib_lock_t* left_block)
{
    ulint heap_no = lock_get_min_heap_no(right_block);
    lock_mutex_enter_kernel();
    // Inherit the locks to the supremum of the left page from the successor of the infimum on the right page 
    lock_rec_inherit_to_gap(left_block, right_block, PAGE_HEAP_NO_SUPREMUM, heap_no);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_update_split_right(const buf_bib_lock_t* right_block, const buf_bib_lock_t* left_block)
{
    ulint heap_no = lock_get_min_heap_no(right_block);
    lock_mutex_enter_kernel();
    /* Move the locks on the supremum of the left page to the supremum of the right page */
    lock_rec_move(right_block, left_block, PAGE_HEAP_NO_SUPREMUM, PAGE_HEAP_NO_SUPREMUM);
    /* Inherit the locks to the supremum of left page from the successor of the infimum on right page */
    lock_rec_inherit_to_gap(left_block, right_block, PAGE_HEAP_NO_SUPREMUM, heap_no);
    lock_mutex_exit_kernel();
}

// -----------------------------------------------------------------------------------------
// REC group (alphabetical)
// -----------------------------------------------------------------------------------------
IB_INTERN void lock_rec_reset_and_inherit_gap_locks(const buf_bib_lock_t* heir_block, const buf_bib_lock_t* block, ulint heir_heap_no, ulint heap_no)
{
    mutex_enter(&kernel_mutex);
    lock_rec_reset_and_release_wait(heir_block, heir_heap_no);
    lock_rec_inherit_to_gap(heir_block, block, heir_heap_no, heap_no);
    mutex_exit(&kernel_mutex);
}

IB_INTERN void lock_rec_restore_from_page_infimum(const buf_bib_lock_t* block, const rec_t* rec, const buf_bib_lock_t* donator)
{
    ulint heap_no = page_rec_get_heap_no(rec);
    lock_mutex_enter_kernel();
    lock_rec_move(block, donator, heap_no, PAGE_HEAP_NO_INFIMUM);
    lock_mutex_exit_kernel();
}

IB_INTERN void lock_rec_store_on_page_infimum(const buf_bib_lock_t* block, const rec_t* rec)
{
    ulint heap_no = page_rec_get_heap_no(rec);
    ut_ad(block->frame == page_align(rec));
    lock_mutex_enter_kernel();
    lock_rec_move(block, block, PAGE_HEAP_NO_INFIMUM, heap_no);
    lock_mutex_exit_kernel();
}

/*=========== DEADLOCK CHECKING ======================================*/

static ibool lock_deadlock_occurs(ib_lock_t* lock, trx_t* trx)
{
	ut_ad(trx);
	ut_ad(lock);
	ut_ad(mutex_own(&kernel_mutex));
retry:
    // We check that adding this trx to the waits-for graph does not produce a cycle. First mark all active transactions with 0:
    trx_t* mark_trx = UT_LIST_GET_FIRST(trx_sys->trx_list);
	while (mark_trx) {
		mark_trx->deadlock_mark = 0;
		mark_trx = UT_LIST_GET_NEXT(trx_list, mark_trx);
	}
    ulint cost = 0;
    ulint ret = lock_deadlock_recursive(trx, trx, lock, &cost, 0);
	switch (ret) {
	case LOCK_VICTIM_IS_OTHER:
        // We chose some other trx as a victim: retry if there still is a deadlock
		goto retry;
	case LOCK_EXCEED_MAX_DEPTH:
        // If the lock search exceeds the max step or the max depth, the current trx will be the victim. Print its information.
		ut_print_timestamp(ib_stream);
        ib_log(state, "TOO DEEP OR LONG SEARCH IN THE LOCK TABLE WAITS-FOR GRAPH, WE WILL ROLL BACK FOLLOWING TRANSACTION \n");
		ib_log(state, "\n*** TRANSACTION:\n");
		trx_print(ib_stream, trx, 3000);
        ib_log(state, "*** WAITING FOR THIS LOCK TO BE GRANTED:\n");
		if (lock_get_type(lock) == LOCK_REC) {
			lock_rec_print(ib_stream, lock);
		} else {
			ib_lock_table_print(ib_stream, lock);
		}
		break;
	case LOCK_VICTIM_IS_START:
        ib_log(state, "*** WE ROLL BACK TRANSACTION (2)\n");
		break;
	default:
        // No deadlock detected
        return FALSE;
	}
	lock_deadlock_found = TRUE;
    return TRUE;
}

static ulint lock_deadlock_recursive(trx_t* start, trx_t* trx, ib_lock_t* wait_lock, ulint* cost, ulint depth)
{
	ut_a(trx);
	ut_a(start);
	ut_a(wait_lock);
	ut_ad(mutex_own(&kernel_mutex));
	if (trx->deadlock_mark == 1) {
        // We have already exhaustively searched the subtree starting from this trx
        return 0;
    }
	*cost = *cost + 1;
    ulint heap_no = ULINT_UNDEFINED;
    ib_lock_t* lock;
	if (lock_get_type_low(wait_lock) == LOCK_REC) {
		heap_no = lock_rec_find_set_bit(wait_lock);
		ut_a(heap_no != ULINT_UNDEFINED);
        ulint space = wait_lock->un_member.rec_lock.space;
        ulint page_no = wait_lock->un_member.rec_lock.page_no;
		lock = lock_rec_get_first_on_page_addr(space, page_no);
        // Position the iterator on the first matching record lock.
        while (lock != NULL && lock != wait_lock && !lock_rec_get_nth_bit(lock, heap_no)) {
			lock = lock_rec_get_next_on_page(lock);
		}
		if (lock == wait_lock) {
			lock = NULL;
		}
		ut_ad(lock == NULL || lock_rec_get_nth_bit(lock, heap_no));
	} else {
		lock = wait_lock;
	}
    // Look at the locks ahead of wait_lock in the lock queue
	for (;;) {
        // Get previous table lock.
		if (heap_no == ULINT_UNDEFINED) {
            lock = UT_LIST_GET_PREV(un_member.tab_lock.locks, lock);
		}
		if (lock == NULL) {
            // We can mark this subtree as searched
			trx->deadlock_mark = 1;
            return FALSE;
		}
		if (lock_has_to_wait(wait_lock, lock)) {
            ibool too_far = depth > LOCK_MAX_DEPTH_IN_DEADLOCK_CHECK || *cost > LOCK_MAX_N_STEPS_IN_DEADLOCK_CHECK;
            trx_t* ib_lock_trx = lock->trx;
			if (ib_lock_trx == start) {
                // We came back to the recursion starting point: a deadlock detected; or we have searched the waits-for graph too long
				innodb_state* state;
				ib_stream = lock_latest_err_stream;
				ut_print_timestamp(ib_stream);
                ib_log(state, "\n*** (1) TRANSACTION:\n");
				trx_print(ib_stream, wait_lock->trx, 3000);
                ib_log(state, "*** (1) WAITING FOR THIS LOCK TO BE GRANTED:\n");
				if (lock_get_type_low(wait_lock) == LOCK_REC) {
					lock_rec_print(ib_stream, wait_lock);
				} else {
					ib_lock_table_print(ib_stream, wait_lock);
				}
				ib_log(state, "*** (2) TRANSACTION:\n");
				trx_print(ib_stream, lock->trx, 3000);
                ib_log(state, "*** (2) HOLDS THE LOCK(S):\n");
				if (lock_get_type_low(lock) == LOCK_REC) {
					lock_rec_print(ib_stream, lock);
				} else {
					ib_lock_table_print(ib_stream, lock);
				}
                ib_log(state, "*** (2) WAITING FOR THIS LOCK TO BE GRANTED:\n");
                if (lock_get_type_low(start->wait_lock) == LOCK_REC) {
                    lock_rec_print(ib_stream, start->wait_lock);
				} else {
                    ib_lock_table_print(ib_stream, start->wait_lock);
				}
				if constexpr (IB_DEBUG) {
					if (lock_print_waits) {
						ib_log(state, "Deadlock detected\n");
					}
				}
                if (trx_weight_cmp(wait_lock->trx, start) >= 0) {
                    // Our recursion starting point transaction is 'smaller', let us choose 'start' as the victim and roll back it
                    return LOCK_VICTIM_IS_START;
                }
				lock_deadlock_found = TRUE;
                // Let us choose the transaction of wait_lock as a victim to try to avoid deadlocking our recursion starting point transaction
                ib_log(state, "*** WE ROLL BACK TRANSACTION (1)\n");
                wait_lock->trx->was_chosen_as_deadlock_victim = TRUE;
				lock_cancel_waiting_and_release(wait_lock);
                // Since trx and wait_lock are no longer in the waits-for graph, we can return FALSE; note that our selective algorithm can choose several transactions as victims, but still we may end up rolling back also the recursion starting point transaction!
                return LOCK_VICTIM_IS_OTHER;
            }
			if (too_far) {
				if constexpr (IB_DEBUG) {
					if (lock_print_waits) {
						ib_log(state, "Deadlock search exceeds max steps or depth.\n");
					}
				}
                // The information about transaction/lock to be rolled back is available in the top level. Do not print anything here.
                return LOCK_EXCEED_MAX_DEPTH;
            }
			if (ib_lock_trx->que_state == TRX_QUE_LOCK_WAIT) {
                // Another trx ahead has requested lock in an incompatible mode, and is itself waiting for a lock
                ulint ret = lock_deadlock_recursive(start, ib_lock_trx, ib_lock_trx->wait_lock, cost, depth + 1);
				if (ret != 0) {
                    return ret;
				}
			}
		}
        // Get the next record lock to check.
		if (heap_no != ULINT_UNDEFINED) {
			ut_a(lock != NULL);
			do {
				lock = lock_rec_get_next_on_page(lock);
            } while (lock != NULL && lock != wait_lock && !lock_rec_get_nth_bit(lock, heap_no));
			if (lock == wait_lock) {
				lock = NULL;
			}
		}
    } // end of the 'for (;;)'-loop
}

/*========================= TABLE LOCKS ==============================*/

IB_INLINE ib_lock_t* ib_lock_table_create(dict_table_t* table, ulint type_mode, trx_t* trx)
{
	ut_ad(table && trx);
	ut_ad(mutex_own(&kernel_mutex));
    ib_lock_t* lock = mem_heap_alloc(trx->lock_heap, sizeof(ib_lock_t));
	UT_LIST_ADD_LAST(trx_locks, trx->trx_locks, lock);
	lock->type_mode = type_mode | LOCK_TABLE;
	lock->trx = trx;
	lock->un_member.tab_lock.table = table;
	UT_LIST_ADD_LAST(un_member.tab_lock.locks, table->locks, lock);
	if (IB_UNLIKELY(type_mode & LOCK_WAIT)) {
		lock_set_lock_and_trx_wait(lock, trx);
	}
    return lock;
}

IB_INLINE void ib_lock_table_remove_low(ib_lock_t* lock)
{
	ut_ad(mutex_own(&kernel_mutex));
    trx_t* trx = lock->trx;
    dict_table_t* table = lock->un_member.tab_lock.table;
	UT_LIST_REMOVE(trx_locks, trx->trx_locks, lock);
	UT_LIST_REMOVE(un_member.tab_lock.locks, table->locks, lock);
}

static ulint ib_lock_table_enqueue_waiting(ulint mode, dict_table_t* table, que_thr_t* thr)
{
	ut_ad(mutex_own(&kernel_mutex));
    // Test if there already is some other reason to suspend thread: we do not enqueue a lock request if the query thread should be stopped anyway
	if (que_thr_stop(thr)) {
		UT_ERROR;
        return DB_QUE_THR_SUSPENDED;
	}
    trx_t* trx = thr_get_trx(thr);
	switch (trx_get_dict_operation(trx)) {
	case TRX_DICT_OP_NONE:
		break;
	case TRX_DICT_OP_TABLE:
	case TRX_DICT_OP_INDEX:
		ut_print_timestamp(ib_stream);
        ib_log(state, "  InnoDB: Error: a table lock wait happens in a dictionary operation!\nInnoDB: Table name ");
		ut_print_name(ib_stream, trx, TRUE, table->name);
        ib_log(state, ".\nInnoDB: Submit a detailed bug report, check the InnoDB website for details");
    }
    // Enqueue the lock request that will wait to be granted
    ib_lock_t* lock = ib_lock_table_create(table, mode | LOCK_WAIT, trx);
    // Check if a deadlock occurs: if yes, remove the lock request and return an error code
	if (lock_deadlock_occurs(lock, trx)) {
        // The order here is important, we don't want to lose the state of the lock before calling remove.
		ib_lock_table_remove_low(lock);
		lock_reset_lock_and_trx_wait(lock);
        return DB_DEADLOCK;
	}
	if (trx->wait_lock == NULL) {
        // Deadlock resolution chose another transaction as a victim, and we accidentally got our lock granted!
        return DB_SUCCESS;
    }
	trx->que_state = TRX_QUE_LOCK_WAIT;
	trx->was_chosen_as_deadlock_victim = FALSE;
	trx->wait_started = time(NULL);
	ut_a(que_thr_stop(thr));
    return DB_LOCK_WAIT;
}

IB_INLINE ib_lock_t* ib_lock_table_other_has_incompatible(trx_t* trx, ulint wait, dict_table_t* table, enum lock_mode mode)
{
	ut_ad(mutex_own(&kernel_mutex));
    ib_lock_t* lock = UT_LIST_GET_LAST(table->locks);
	while (lock != NULL) {
        if ((lock->trx != trx) && (!lock_mode_compatible(lock_get_mode(lock), mode)) && (wait || !(lock_get_wait(lock)))) {
            return lock;
        }
		lock = UT_LIST_GET_PREV(un_member.tab_lock.locks, lock);
	}
    return NULL;
}

IB_INTERN ulint ib_lock_table(ulint flags, dict_table_t* table, enum lock_mode mode, que_thr_t* thr)
{
    ut_ad(table && thr);
    if (flags & BTR_NO_LOCKING_FLAG) {
        return DB_SUCCESS;
    }
    ut_a(flags == 0);
    trx_t* trx = thr_get_trx(thr);
    lock_mutex_enter_kernel();
    // Look for stronger locks the same trx already has on the table
    if (ib_lock_table_has(trx, table, mode)) {
        lock_mutex_exit_kernel();
        return DB_SUCCESS;
    }
    // We have to check if the new lock is compatible with any locks other transactions have in the table lock queue.
    if (ib_lock_table_other_has_incompatible(trx, LOCK_WAIT, table, mode)) {
        // Another trx has a request on the table in an incompatible mode: this trx may have to wait
        ulint err = ib_lock_table_enqueue_waiting(mode | flags, table, thr);
        lock_mutex_exit_kernel();
        return err;
    }
    ib_lock_table_create(table, mode | flags, trx);
    ut_a(!flags || mode == LOCK_S || mode == LOCK_X);
    lock_mutex_exit_kernel();
    return DB_SUCCESS;
}

static ibool ib_lock_table_has_to_wait_in_queue(ib_lock_t* wait_lock)
{
    ut_ad(lock_get_wait(wait_lock));
    dict_table_t* table = wait_lock->un_member.tab_lock.table;
    ib_lock_t* lock = UT_LIST_GET_FIRST(table->locks);
    while (lock != wait_lock) {
        if (lock_has_to_wait(wait_lock, lock)) {
            return TRUE;
        }
        lock = UT_LIST_GET_NEXT(un_member.tab_lock.locks, lock);
    }
    return FALSE;
}

static void ib_lock_table_dequeue(ib_lock_t* in_lock)
{
    ut_ad(mutex_own(&kernel_mutex));
    ut_a(lock_get_type_low(in_lock) == LOCK_TABLE);
    ib_lock_t* lock = UT_LIST_GET_NEXT(un_member.tab_lock.locks, in_lock);
    ib_lock_table_remove_low(in_lock);
    // Check if waiting locks in the queue can now be granted: grant locks if there are no conflicting locks ahead.
    while (lock != NULL) {
        if (lock_get_wait(lock) && !ib_lock_table_has_to_wait_in_queue(lock)) {
            // Grant the lock
            lock_grant(lock);
        }
        lock = UT_LIST_GET_NEXT(un_member.tab_lock.locks, lock);
    }
}

/*=========================== LOCK RELEASE ==============================*/

IB_INTERN void lock_rec_unlock(trx_t* trx, const buf_bib_lock_t* block, const rec_t* rec, enum lock_mode lock_mode)
{
    ut_ad(trx && rec);
    ut_ad(block->frame == page_align(rec));
    ulint heap_no = page_rec_get_heap_no(rec);
    mutex_enter(&kernel_mutex);
    ib_lock_t* lock = lock_rec_get_first(block, heap_no);
    // Find the last lock with the same lock_mode and transaction from the record.
    ib_lock_t* release_lock = NULL;
    while (lock != NULL) {
        if (lock->trx == trx && lock_get_mode(lock) == lock_mode) {
            release_lock = lock;
            ut_a(!lock_get_wait(lock));
        }
        lock = lock_rec_get_next(heap_no, lock);
    }
    // If a record lock is found, release the record lock
    if (IB_LIKELY(release_lock != NULL)) {
        lock_rec_reset_nth_bit(release_lock, heap_no);
    } else {
        mutex_exit(&kernel_mutex);
        ut_print_timestamp(ib_stream);
        ib_log(state, "  InnoDB: Error: unlock row could not find a %lu mode lock on the record\n", (ulong) lock_mode);
        return;
    }
    // Check if we can now grant waiting lock requests
    lock = lock_rec_get_first(block, heap_no);
    while (lock != NULL) {
        if (lock_get_wait(lock) && !lock_rec_has_to_wait_in_queue(lock)) {
            // Grant the lock
            lock_grant(lock);
        }
        lock = lock_rec_get_next(heap_no, lock);
    }
    mutex_exit(&kernel_mutex);
}

IB_INTERN void lock_release_off_kernel(trx_t* trx)
{
    ut_ad(mutex_own(&kernel_mutex));
    ib_lock_t* lock = UT_LIST_GET_LAST(trx->trx_locks);
    ulint count = 0;
    while (lock != NULL) {
        count++;
        if (lock_get_type_low(lock) == LOCK_REC) {
            lock_rec_dequeue_from_page(lock);
        } else {
            ut_ad(lock_get_type_low(lock) & LOCK_TABLE);
            ib_lock_table_dequeue(lock);
        }
        if (count == LOCK_RELEASE_KERNEL_INTERVAL) {
            // Release the kernel mutex for a while, so that we do not monopolize it
            lock_mutex_exit_kernel();
            lock_mutex_enter_kernel();
            count = 0;
        }
        lock = UT_LIST_GET_LAST(trx->trx_locks);
    }
    mem_heap_empty(trx->lock_heap);
}

IB_INTERN void lock_cancel_waiting_and_release(ib_lock_t* lock)
{
    ut_ad(mutex_own(&kernel_mutex));
    if (lock_get_type_low(lock) == LOCK_REC) {
        lock_rec_dequeue_from_page(lock);
    } else {
        ut_ad(lock_get_type_low(lock) & LOCK_TABLE);
        ib_lock_table_dequeue(lock);
    }
    // Reset the wait flag and the back pointer to lock in trx
    lock_reset_lock_and_trx_wait(lock);
    // The following function releases the trx from lock wait
    trx_end_lock_wait(lock->trx);
}

/* True if a lock mode is S or X */
#define IS_LOCK_S_OR_X(lock) \
	(lock_get_mode(lock) == LOCK_S \
	 || lock_get_mode(lock) == LOCK_X)


static void lock_remove_all_on_table_for_trx(dict_table_t* table, trx_t* trx, ibool remove_also_table_sx_locks)
{
	ut_ad(mutex_own(&kernel_mutex));
    ib_lock_t* lock = UT_LIST_GET_LAST(trx->trx_locks);
	while (lock != NULL) {
        ib_lock_t* prev_lock = UT_LIST_GET_PREV(trx_locks, lock);
        if (lock_get_type_low(lock) == LOCK_REC && lock->index->table == table) {
			ut_a(!lock_get_wait(lock));
			lock_rec_discard(lock);
        } else if (lock_get_type_low(lock) & LOCK_TABLE && lock->un_member.tab_lock.table == table && (remove_also_table_sx_locks || !IS_LOCK_S_OR_X(lock))) {
			ut_a(!lock_get_wait(lock));
			ib_lock_table_remove_low(lock);
		}
		lock = prev_lock;
	}
}

IB_INTERN void lock_remove_all_on_table(dict_table_t* table, ibool remove_also_table_sx_locks)
{
	mutex_enter(&kernel_mutex);
    ib_lock_t* lock = UT_LIST_GET_FIRST(table->locks);
	while (lock != NULL) {
        ib_lock_t* prev_lock = UT_LIST_GET_PREV(un_member.tab_lock.locks, lock);
        // If we should remove all locks (remove_also_table_sx_locks is TRUE), or if the lock is not table-level S or X lock, then check we are not going to remove a wait lock.
        if (remove_also_table_sx_locks || !(lock_get_type(lock) == LOCK_TABLE && IS_LOCK_S_OR_X(lock))) {
			// HACK: For testing
			if (lock_get_wait(lock)) {
			       if (remove_also_table_sx_locks) {
					UT_ERROR;
				} else {
					goto next;
				}
			}
		}
        lock_remove_all_on_table_for_trx(table, lock->trx, remove_also_table_sx_locks);
		if (prev_lock == NULL) {
			if (lock == UT_LIST_GET_FIRST(table->locks)) {
                // lock was not removed, pick its successor
                lock = UT_LIST_GET_NEXT(un_member.tab_lock.locks, lock);
			} else {
                // lock was removed, pick the first one
				lock = UT_LIST_GET_FIRST(table->locks);
			}
        } else if (UT_LIST_GET_NEXT(un_member.tab_lock.locks, prev_lock) != lock) {
            // If lock was removed by lock_remove_all_on_table_for_trx() then pick the successor of prev_lock ...
            lock = UT_LIST_GET_NEXT(un_member.tab_lock.locks, prev_lock);
		} else {
next:
            // ... otherwise pick the successor of lock.
            lock = UT_LIST_GET_NEXT(un_member.tab_lock.locks, lock);
		}
	}
	mutex_exit(&kernel_mutex);
}

/*===================== VALIDATION AND DEBUGGING  ====================*/

IB_INTERN void ib_lock_table_print(innodb_state* state, const ib_lock_t* lock)
{
	ut_ad(mutex_own(&kernel_mutex));
	ut_a(lock_get_type_low(lock) == LOCK_TABLE);

	ib_log(state, "TABLE LOCK table ");
	ut_print_name(ib_stream, lock->trx, TRUE,
		      lock->un_member.tab_lock.table->name);
	ib_log(state, " trx id " TRX_ID_FMT,
		TRX_ID_PREP_PRINTF(lock->trx->id));

	if (lock_get_mode(lock) == LOCK_S) {
		ib_log(state, " lock mode S");
	} else if (lock_get_mode(lock) == LOCK_X) {
		ib_log(state, " lock mode X");
	} else if (lock_get_mode(lock) == LOCK_IS) {
		ib_log(state, " lock mode IS");
	} else if (lock_get_mode(lock) == LOCK_IX) {
		ib_log(state, " lock mode IX");
	} else if (lock_get_mode(lock) == LOCK_AUTO_INC) {
		ib_log(state, " lock mode AUTO-INC");
	} else {
		ib_log(state, " unknown lock mode %lu",
			(ulong) lock_get_mode(lock));
	}

	if (lock_get_wait(lock)) {
		ib_log(state, " waiting");
	}

	ib_log(state, "\n");
}

IB_INTERN void lock_rec_print(innodb_state* state, const ib_lock_t* lock)
{
	ulint space = lock->un_member.rec_lock.space;
	ulint page_no = lock->un_member.rec_lock.page_no;
	mtr_t mtr;
	mem_heap_t* heap = NULL;
	ulint offsets_[REC_OFFS_NORMAL_SIZE];
	ulint* offsets = offsets_;
	rec_offs_init(offsets_);
	ut_ad(mutex_own(&kernel_mutex));
	ut_a(lock_get_type_low(lock) == LOCK_REC);
	ib_log(state, "RECORD LOCKS space id %lu page no %lu n bits %lu ", (ulong) space, (ulong) page_no, (ulong) lock_rec_get_n_bits(lock));
	dict_index_name_print(ib_stream, lock->trx, lock->index);
	ib_log(state, " trx id " TRX_ID_FMT, TRX_ID_PREP_PRINTF(lock->trx->id));
	if (lock_get_mode(lock) == LOCK_S) {
		ib_log(state, " lock mode S");
	} else if (lock_get_mode(lock) == LOCK_X) {
		ib_log(state, " lock_mode X");
	} else {
		UT_ERROR;
	}
	if (lock_rec_get_gap(lock)) {
		ib_log(state, " locks gap before rec");
	}
	if (lock_rec_get_rec_not_gap(lock)) {
		ib_log(state, " locks rec but not gap");
	}
	if (lock_rec_get_insert_intention(lock)) {
		ib_log(state, " insert intention");
	}
	if (lock_get_wait(lock)) {
		ib_log(state, " waiting");
	}
	mtr_start(&mtr);
	ib_log(state, "\n");
	const buf_bib_lock_t* block = buf_page_try_get(space, page_no, &mtr);
	if (block) {
		for (ulint i = 0; i < lock_rec_get_n_bits(lock); i++) {
			if (lock_rec_get_nth_bit(lock, i)) {
				const rec_t* rec = page_find_rec_with_heap_no(buf_block_get_frame(block), i);
				offsets = rec_get_offsets(rec, lock->index, offsets, ULINT_UNDEFINED, &heap);
				ib_log(state, "Record lock, heap no %lu ", (ulong) i);
				rec_print_new(ib_stream, rec, offsets);
				ib_log(state, "\n");
			}
		}
	} else {
		for (ulint i = 0; i < lock_rec_get_n_bits(lock); i++) {
			ib_log(state, "Record lock, heap no %lu\n", (ulong) i);
		}
	}
	mtr_commit(&mtr);
	if (IB_LIKELY_NULL(heap)) {
		IB_MEM_HEAP_FREE(heap);
	}
}

#ifdef IB_DEBUG
// Print the number of lock structs from lock_print_info_summary() only in non-production builds for performance reasons.
#define PRINT_NUM_OF_LOCK_STRUCTS
#endif // IB_DEBUG

#ifdef PRINT_NUM_OF_LOCK_STRUCTS
static ulint lock_get_n_rec_locks(void)
{
	ut_ad(mutex_own(&kernel_mutex));
    ulint n_locks = 0;
    for (ulint i = 0; i < hash_get_n_cells(lock_sys->rec_hash); i++) {
        ib_lock_t* lock = HASH_GET_FIRST(lock_sys->rec_hash, i);
		while (lock) {
			n_locks++;
			lock = HASH_GET_NEXT(hash, lock);
		}
	}
    return n_locks;
}
#endif /* PRINT_NUM_OF_LOCK_STRUCTS */

IB_INTERN ibool lock_print_info_summary(innodb_state* state, ibool nowait)
{
    // if nowait is FALSE, wait on the kernel mutex, otherwise return immediately if fail to obtain the mutex.
	if (!nowait) {
		lock_mutex_enter_kernel();
	} else if (mutex_enter_nowait(&kernel_mutex)) {
        ib_log(state, "FAIL TO OBTAIN KERNEL MUTEX, SKIP LOCK INFO PRINTING\n");
        return FALSE;
    }
	if (lock_deadlock_found) {
        ib_log(state, "------------------------\nLATEST DETECTED DEADLOCK\n------------------------\n");
    }
    ib_log(state, "------------\nTRANSACTIONS\n------------\n");
    ib_log(state, "Trx id counter " TRX_ID_FMT "\n", TRX_ID_PREP_PRINTF(trx_sys->max_trx_id));
    ib_log(state, "Purge done for trx's n:o < " TRX_ID_FMT " undo n:o < " TRX_ID_FMT "\n", TRX_ID_PREP_PRINTF(purge_sys->purge_trx_no), TRX_ID_PREP_PRINTF(purge_sys->purge_undo_no));
    ib_log(state, "History list length %lu\n", (ulong) trx_sys->rseg_history_len);
#ifdef PRINT_NUM_OF_LOCK_STRUCTS
    ib_log(state, "Total number of lock structs in row lock hash table %lu\n", (ulong) lock_get_n_rec_locks());
#endif /* PRINT_NUM_OF_LOCK_STRUCTS */
    return TRUE;
}

IB_INTERN void lock_print_info_all_transactions(innodb_state* state)
{
    ibool load_page_first = TRUE;
    ulint nth_trx = 0;
    ulint nth_lock = 0;
	ib_log(state, "LIST OF TRANSACTIONS FOR EACH SESSION:\n");
    // First print info on non-active transactions
    trx_t* trx = UT_LIST_GET_FIRST(trx_sys->client_trx_list);
	while (trx) {
		if (trx->conc_state == TRX_NOT_STARTED) {
			ib_log(state, "---");
			trx_print(ib_stream, trx, 600);
		}
		trx = UT_LIST_GET_NEXT(client_trx_list, trx);
	}
loop:
	trx = UT_LIST_GET_FIRST(trx_sys->trx_list);
    ulint i = 0;
    // Since we temporarily release the kernel mutex when reading a database page in below, variable trx may be obsolete now and we must loop through the trx list to get probably the same trx, or some other trx.
	while (trx && (i < nth_trx)) {
		trx = UT_LIST_GET_NEXT(trx_list, trx);
		i++;
	}
	if (trx == NULL) {
		lock_mutex_exit_kernel();
		ut_ad(lock_validate());
		return;
	}
	if (nth_lock == 0) {
		ib_log(state, "---");
		trx_print(ib_stream, trx, 600);
		if (trx->read_view) {
            ib_log(state, "Trx read view will not see trx with id >= " TRX_ID_FMT ", sees < " TRX_ID_FMT "\n", TRX_ID_PREP_PRINTF(trx->read_view->low_limit_id), TRX_ID_PREP_PRINTF(trx->read_view->up_limit_id));
        }
		if (trx->que_state == TRX_QUE_LOCK_WAIT) {
            ib_log(state, "------- TRX HAS BEEN WAITING %lu SEC FOR THIS LOCK TO BE GRANTED:\n", (ulong) difftime(time(NULL), trx->wait_started));
			if (lock_get_type_low(trx->wait_lock) == LOCK_REC) {
				lock_rec_print(ib_stream, trx->wait_lock);
			} else {
				ib_lock_table_print(ib_stream, trx->wait_lock);
			}
			ib_log(state, "------------------\n");
		}
	}
	if (!srv_print_innodb_lock_monitor) {
		nth_trx++;
		goto loop;
	}
    i = 0;
    // Look at the note about the trx loop above why we loop here: lock may be an obsolete pointer now.
    ib_lock_t* lock = UT_LIST_GET_FIRST(trx->trx_locks);
	while (lock && (i < nth_lock)) {
		lock = UT_LIST_GET_NEXT(trx_locks, lock);
		i++;
	}
	if (lock == NULL) {
		nth_trx++;
		nth_lock = 0;
		goto loop;
	}
	if (lock_get_type_low(lock) == LOCK_REC) {
		if (load_page_first) {
            ulint space = lock->un_member.rec_lock.space;
            ulint zip_size = fil_space_get_zip_size(space);
            ulint page_no = lock->un_member.rec_lock.page_no;
			if (IB_UNLIKELY(zip_size == ULINT_UNDEFINED)) {
                // It is a single table tablespace and the .ibd file is missing (TRUNCATE TABLE probably stole the locks): just print the lock without attempting to load the page in the buffer pool.
                ib_log(state, "RECORD LOCKS on non-existing space %lu\n", (ulong) space);
				goto print_rec;
			}
			lock_mutex_exit_kernel();
            mtr_t mtr;
			mtr_start(&mtr);
            buf_page_get_with_no_latch(space, zip_size, page_no, &mtr);
			mtr_commit(&mtr);
			load_page_first = FALSE;
			lock_mutex_enter_kernel();
			goto loop;
		}
print_rec:
		lock_rec_print(ib_stream, lock);
	} else {
		ut_ad(lock_get_type_low(lock) & LOCK_TABLE);
		ib_lock_table_print(ib_stream, lock);
	}
	load_page_first = TRUE;
	nth_lock++;
	if (nth_lock >= 10) {
        ib_log(state, "10 LOCKS PRINTED FOR THIS TRX: SUPPRESSING FURTHER PRINTS\n");
		nth_trx++;
		nth_lock = 0;
		goto loop;
	}
	goto loop;
}

#ifdef IB_DEBUG
static ibool ib_lock_table_queue_validate(dict_table_t* table)
{
    ut_ad(mutex_own(&kernel_mutex));
    ib_lock_t* lock = UT_LIST_GET_FIRST(table->locks);
    while (lock) {
        ut_a(((lock->trx)->conc_state == TRX_ACTIVE) || ((lock->trx)->conc_state == TRX_PREPARED) || ((lock->trx)->conc_state == TRX_COMMITTED_IN_MEMORY));
        if (!lock_get_wait(lock)) {
            ut_a(!ib_lock_table_other_has_incompatible(lock->trx, 0, table, lock_get_mode(lock)));
        } else {
            ut_a(ib_lock_table_has_to_wait_in_queue(lock));
        }
        lock = UT_LIST_GET_NEXT(un_member.tab_lock.locks, lock);
    }
    return TRUE;
}

static ibool lock_rec_queue_validate(const buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, const ulint* offsets)
{
	ut_a(rec);
	ut_a(block->frame == page_align(rec));
	ut_ad(rec_offs_validate(rec, index, offsets));
	ut_ad(!page_rec_is_comp(rec) == !rec_offs_comp(offsets));
	ulint heap_no = page_rec_get_heap_no(rec);
	lock_mutex_enter_kernel();
	if (!page_rec_is_user_rec(rec)) {
		ib_lock_t* lock = lock_rec_get_first(block, heap_no);
		while (lock) {
			switch(lock->trx->conc_state) {
			case TRX_ACTIVE:
			case TRX_PREPARED:
			case TRX_COMMITTED_IN_MEMORY:
				break;
			default:
				UT_ERROR;
			}
			ut_a(trx_in_trx_list(lock->trx));
			if (lock_get_wait(lock)) {
				ut_a(lock_rec_has_to_wait_in_queue(lock));
			}
			if (index) {
				ut_a(lock->index == index);
			}
			lock = lock_rec_get_next(heap_no, lock);
		}
		lock_mutex_exit_kernel();
		return TRUE;
	}
	if (!index);
	else if (dict_index_is_clust(index)) {
		trx_t* impl_trx = lock_clust_rec_some_has_impl(rec, index, offsets);
		if (impl_trx && lock_rec_other_has_expl_req(LOCK_S, 0, LOCK_WAIT, block, heap_no, impl_trx)) {
			ut_a(lock_rec_has_expl(LOCK_X | LOCK_REC_NOT_GAP, block, heap_no, impl_trx));
		}
#if 0
	} else {
		/* The kernel mutex may get released temporarily in the next function call: we have to release lock table mutex to obey the latching order */
		/* If this thread is holding the file space latch (fil_space_t::latch), the following check WILL break latching order and may cause a deadlock of threads. */
		/* NOTE: This is a bogus check that would fail in the following case: Our transaction is updating a row. After it has updated the clustered index record, it goes to a secondary index record and finds someone else holding an explicit S- or X-lock on that secondary index record, presumably from a locking read. Our transaction cannot update the secondary index immediately, but places a waiting X-lock request on the secondary index record. There is nothing illegal in this. The assertion is simply too strong. */
		/* From the locking point of view, each secondary index is a separate table. A lock that is held on secondary index rec does not give any rights to modify or read the clustered index rec. Therefore, we can think of the sec index as a separate 'table' from the clust index 'table'. Conversely, a transaction that has acquired a lock on and modified a clustered index record may need to wait for a lock on the corresponding record in a secondary index. */
		impl_trx = lock_sec_rec_some_has_impl_off_kernel(rec, index, offsets);
		if (impl_trx && lock_rec_other_has_expl_req(LOCK_S, 0, LOCK_WAIT, block, heap_no, impl_trx)) {
			ut_a(lock_rec_has_expl(LOCK_X | LOCK_REC_NOT_GAP, block, heap_no, impl_trx));
		}
#endif
	}
	ib_lock_t* lock = lock_rec_get_first(block, heap_no);
	while (lock) {
		ut_a(lock->trx->conc_state == TRX_ACTIVE || lock->trx->conc_state == TRX_PREPARED || lock->trx->conc_state == TRX_COMMITTED_IN_MEMORY);
		ut_a(trx_in_trx_list(lock->trx));
		if (index) {
			ut_a(lock->index == index);
		}
		if (!lock_rec_get_gap(lock) && !lock_get_wait(lock)) {
			enum lock_mode mode;
			if (lock_get_mode(lock) == LOCK_S) {
				mode = LOCK_X;
			} else {
				mode = LOCK_S;
			}
			ut_a(!lock_rec_other_has_expl_req(mode, 0, 0, block, heap_no, lock->trx));
		} else if (lock_get_wait(lock) && !lock_rec_get_gap(lock)) {
			ut_a(lock_rec_has_to_wait_in_queue(lock));
		}
		lock = lock_rec_get_next(heap_no, lock);
	}
	lock_mutex_exit_kernel();
	return TRUE;
}

static ibool lock_rec_validate_page(ulint space, ulint zip_size, ulint page_no)
{
    ulint nth_lock = 0;
    ulint nth_bit = 0;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    mem_heap_t* heap = NULL;
    mtr_t mtr;
    mtr_start(&mtr);
    ut_ad(zip_size != ULINT_UNDEFINED);
    buf_bib_lock_t* block = buf_page_get(space, zip_size, page_no, RW_X_LATCH, &mtr);
    buf_block_dbg_add_level(block, SYNC_NO_ORDER_CHECK);
    const page_t* page = block->frame;
    lock_mutex_enter_kernel();
loop:
    ib_lock_t* lock = lock_rec_get_first_on_page_addr(space, page_no);
    if (!lock) {
        goto function_exit;
    }
    for (ulint i = 0; i < nth_lock; i++) {
        lock = lock_rec_get_next_on_page(lock);
        if (!lock) {
            goto function_exit;
        }
    }
    ut_a(trx_in_trx_list(lock->trx));
    ut_a(lock->trx->conc_state == TRX_ACTIVE || lock->trx->conc_state == TRX_PREPARED || lock->trx->conc_state == TRX_COMMITTED_IN_MEMORY);
# ifdef IB_SYNC_DEBUG
    // Only validate the record queues when this thread is not holding a space->latch. Deadlocks are possible due to latching order violation when IB_DEBUG is defined while IB_SYNC_DEBUG is not.
    if (!sync_thread_levels_contains(SYNC_FSP))
# endif /* IB_SYNC_DEBUG */
    for (ulint i = nth_bit; i < lock_rec_get_n_bits(lock); i++) {
        if (i == 1 || lock_rec_get_nth_bit(lock, i)) {
            dict_index_t* index = lock->index;
            const rec_t* rec = page_find_rec_with_heap_no(page, i);
            ut_a(rec);
            offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);
            ib_log(state, "Validating %lu %lu\n", (ulong) space, (ulong) page_no);
            lock_mutex_exit_kernel();
            // If this thread is holding the file space latch (fil_space_t::latch), the following check WILL break the latching order and may cause a deadlock of threads.
            lock_rec_queue_validate(block, rec, index, offsets);
            lock_mutex_enter_kernel();
            nth_bit = i + 1;
            goto loop;
        }
    }
    nth_bit = 0;
    nth_lock++;
    goto loop;
function_exit:
    lock_mutex_exit_kernel();
    mtr_commit(&mtr);
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
    return TRUE;
}

static ibool lock_validate(void)
{
    lock_mutex_enter_kernel();
    
    trx_t* trx = UT_LIST_GET_FIRST(trx_sys->trx_list);
    while (trx) {
        ib_lock_t* lock = UT_LIST_GET_FIRST(trx->trx_locks);
        while (lock) {
            if (lock_get_type_low(lock) & LOCK_TABLE) {
                ib_lock_table_queue_validate(lock->un_member.tab_lock.table);
            }
            lock = UT_LIST_GET_NEXT(trx_locks, lock);
        }
        trx = UT_LIST_GET_NEXT(trx_list, trx);
    }
    
    for (ulint i = 0; i < hash_get_n_cells(lock_sys->rec_hash); i++) {
        dulint limit = ut_dulint_zero;
        for (;;) {
            ib_lock_t* lock = HASH_GET_FIRST(lock_sys->rec_hash, i);
            while (lock) {
                ut_a(trx_in_trx_list(lock->trx));
                ulint space = lock->un_member.rec_lock.space;
                ulint page_no = lock->un_member.rec_lock.page_no;
                if (ut_dulint_cmp(ut_dulint_create(space, page_no), limit) >= 0) {
                    break;
                }
                lock = HASH_GET_NEXT(hash, lock);
            }
            if (!lock) {
                break;
            }
            lock_mutex_exit_kernel();
            lock_rec_validate_page(space, fil_space_get_zip_size(space), page_no);
            lock_mutex_enter_kernel();
            limit = ut_dulint_create(space, page_no + 1);
        }
    }
    
    lock_mutex_exit_kernel();
    return TRUE;
}
#endif // IB_DEBUG

// ============ RECORD LOCK CHECKS FOR ROW OPERATIONS ====================

IB_INTERN ulint lock_rec_insert_check_and_lock(ulint flags, const rec_t* rec, buf_bib_lock_t* block, dict_index_t* index, que_thr_t* thr, mtr_t* mtr, ibool* inherit)
{
    ut_ad(block->frame == page_align(rec));
    if (flags & BTR_NO_LOCKING_FLAG) {
        return DB_SUCCESS;
    }
    trx_t* trx = thr_get_trx(thr);
    const rec_t* next_rec = page_rec_get_next_const(rec);
    ulint next_rec_heap_no = page_rec_get_heap_no(next_rec);
    lock_mutex_enter_kernel();
    // When inserting a record into an index, the table must be at least IX-locked or we must be building an index, in which case the table must be at least S-locked.
    ut_ad(ib_lock_table_has(trx, index->table, LOCK_IX) || (*index->name == TEMP_INDEX_PREFIX && ib_lock_table_has(trx, index->table, LOCK_S)));
    ib_lock_t* lock = lock_rec_get_first(block, next_rec_heap_no);
    if (IB_LIKELY(lock == NULL)) {
        // We optimize CPU time usage in the simplest case
        lock_mutex_exit_kernel();
        if (!dict_index_is_clust(index)) {
            // Update the page max trx id field
            page_update_max_trx_id(block, buf_block_get_page_zip(block), trx->id, mtr);
        }
        *inherit = FALSE;
        return DB_SUCCESS;
    }
    *inherit = TRUE;
    // If another transaction has an explicit lock request which locks the gap, waiting or granted, on the successor, the insert has to wait. An exception is the case where the lock by the another transaction is a gap type lock which it placed to wait for its turn to insert. We do not consider that kind of a lock conflicting with our insert. This eliminates an unnecessary deadlock which resulted when 2 transactions had to wait for their insert. Both had waiting gap type lock requests on the successor, which produced an unnecessary deadlock.
    ulint err;
    if (lock_rec_other_has_conflicting(LOCK_X | LOCK_GAP | LOCK_INSERT_INTENTION, block, next_rec_heap_no, trx)) {
        // Note that we may get DB_SUCCESS also here!
        err = lock_rec_enqueue_waiting(LOCK_X | LOCK_GAP | LOCK_INSERT_INTENTION, block, next_rec_heap_no, index, thr);
    } else {
        err = DB_SUCCESS;
    }
    lock_mutex_exit_kernel();
    if ((err == DB_SUCCESS) && !dict_index_is_clust(index)) {
        // Update the page max trx id field
        page_update_max_trx_id(block, buf_block_get_page_zip(block), trx->id, mtr);
    }
	if constexpr (IB_DEBUG) {
		mem_heap_t* heap = NULL;
		ulint offsets_[REC_OFFS_NORMAL_SIZE];
		rec_offs_init(offsets_);
		const ulint* offsets = rec_get_offsets(next_rec, index, offsets_, ULINT_UNDEFINED, &heap);
		ut_ad(lock_rec_queue_validate(block, next_rec, index, offsets));
		if (IB_LIKELY_NULL(heap)) {
			IB_MEM_HEAP_FREE(heap);
		}
	}
    return err;
}

static void lock_rec_convert_impl_to_expl(const buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, const ulint* offsets)
{
	trx_t* impl_trx;

	ut_ad(mutex_own(&kernel_mutex));
	ut_ad(page_rec_is_user_rec(rec));
	ut_ad(rec_offs_validate(rec, index, offsets));
	ut_ad(!page_rec_is_comp(rec) == !rec_offs_comp(offsets));

	if (dict_index_is_clust(index)) {
		impl_trx = lock_clust_rec_some_has_impl(rec, index, offsets);
	} else {
		impl_trx = lock_sec_rec_some_has_impl_off_kernel(
			rec, index, offsets);
	}

	if (impl_trx) {
		ulint	heap_no = page_rec_get_heap_no(rec);

		/* If the transaction has no explicit x-lock set on the
		record, set one for it */

		if (!lock_rec_has_expl(LOCK_X | LOCK_REC_NOT_GAP, block,
				       heap_no, impl_trx)) {

			lock_rec_add_to_queue(
				LOCK_REC | LOCK_X | LOCK_REC_NOT_GAP,
				block, heap_no, index, impl_trx);
		}
	}
}

IB_INTERN ulint lock_clust_rec_modify_check_and_lock(ulint flags, const buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, const ulint* offsets, que_thr_t* thr)
{
	ut_ad(rec_offs_validate(rec, index, offsets));
	ut_ad(dict_index_is_clust(index));
	ut_ad(block->frame == page_align(rec));
	if (flags & BTR_NO_LOCKING_FLAG) {
		return DB_SUCCESS;
	}
	ulint heap_no = rec_offs_comp(offsets) ? rec_get_heap_no_new(rec) : rec_get_heap_no_old(rec);
	lock_mutex_enter_kernel();
	ut_ad(ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IX));
	/* If a transaction has no explicit x-lock set on the record, set one for it */
	lock_rec_convert_impl_to_expl(block, rec, index, offsets);
	ulint err = lock_rec_lock(TRUE, LOCK_X | LOCK_REC_NOT_GAP, block, heap_no, index, thr);
	lock_mutex_exit_kernel();
	ut_ad(lock_rec_queue_validate(block, rec, index, offsets));
	return err;
}

IB_INTERN ulint lock_sec_rec_modify_check_and_lock(ulint flags, buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, que_thr_t* thr, mtr_t* mtr)
{
	ut_ad(!dict_index_is_clust(index));
	ut_ad(block->frame == page_align(rec));
	if (flags & BTR_NO_LOCKING_FLAG) {
		return DB_SUCCESS;
	}
	ulint heap_no = page_rec_get_heap_no(rec);
	// Another transaction cannot have an implicit lock on the record, because when we come here, we already have modified the clustered index record, and this 
	// would not have been possible if another active transaction had modified this secondary index record.
	lock_mutex_enter_kernel();
	ut_ad(ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IX));
	ulint err = lock_rec_lock(TRUE, LOCK_X | LOCK_REC_NOT_GAP, block, heap_no, index, thr);
	lock_mutex_exit_kernel();
	if constexpr (IB_DEBUG) {
		mem_heap_t* heap = NULL;
		ulint offsets_[REC_OFFS_NORMAL_SIZE];
		const ulint* offsets;
		rec_offs_init(offsets_);
		offsets = rec_get_offsets(rec, index, offsets_, ULINT_UNDEFINED, &heap);
		ut_ad(lock_rec_queue_validate(block, rec, index, offsets));
		if (IB_LIKELY_NULL(heap)) {
			IB_MEM_HEAP_FREE(heap);
		}
	}
	if (err == DB_SUCCESS) {
		/* Update the page max trx id field */
		page_update_max_trx_id(block, buf_block_get_page_zip(block), thr_get_trx(thr)->id, mtr);
	}
	return err;
}

IB_INTERN ulint lock_sec_rec_read_check_and_lock(ulint flags, const buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, const ulint* offsets, enum lock_mode mode, ulint gap_mode, que_thr_t* thr)
{
	ut_ad(!dict_index_is_clust(index));
	ut_ad(block->frame == page_align(rec));
	ut_ad(page_rec_is_user_rec(rec) || page_rec_is_supremum(rec));
	ut_ad(rec_offs_validate(rec, index, offsets));
	ut_ad(mode == LOCK_X || mode == LOCK_S);
	if (flags & BTR_NO_LOCKING_FLAG) {
		return DB_SUCCESS;
	}
	ulint heap_no = page_rec_get_heap_no(rec);
	lock_mutex_enter_kernel();
	ut_ad(mode != LOCK_X || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IX));
	ut_ad(mode != LOCK_S || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IS));
	/* Some transaction may have an implicit x-lock on the record only if the max trx id for the page >= min trx id for the trx list or a database recovery is running. */
	if (((ut_dulint_cmp(page_get_max_trx_id(block->frame), trx_list_get_min_trx_id()) >= 0) || recv_recovery_is_on()) && !page_rec_is_supremum(rec)) {
		lock_rec_convert_impl_to_expl(block, rec, index, offsets);
	}
	ulint err = lock_rec_lock(FALSE, mode | gap_mode, block, heap_no, index, thr);
	lock_mutex_exit_kernel();
	ut_ad(lock_rec_queue_validate(block, rec, index, offsets));
	return err;
}

IB_INTERN ulint lock_clust_rec_read_check_and_lock(ulint flags, const buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, const ulint* offsets, enum lock_mode mode, ulint gap_mode, que_thr_t* thr)
{
	ut_ad(dict_index_is_clust(index));
	ut_ad(block->frame == page_align(rec));
	ut_ad(page_rec_is_user_rec(rec) || page_rec_is_supremum(rec));
	ut_ad(gap_mode == LOCK_ORDINARY || gap_mode == LOCK_GAP || gap_mode == LOCK_REC_NOT_GAP);
	ut_ad(rec_offs_validate(rec, index, offsets));
	if (flags & BTR_NO_LOCKING_FLAG) {
		return DB_SUCCESS;
	}
	ulint heap_no = page_rec_get_heap_no(rec);
	lock_mutex_enter_kernel();
	ut_ad(mode != LOCK_X || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IX));
	ut_ad(mode != LOCK_S || ib_lock_table_has(thr_get_trx(thr), index->table, LOCK_IS));
	if (IB_LIKELY(heap_no != PAGE_HEAP_NO_SUPREMUM)) {
		lock_rec_convert_impl_to_expl(block, rec, index, offsets);
	}
	ulint err = lock_rec_lock(FALSE, mode | gap_mode, block, heap_no, index, thr);
	lock_mutex_exit_kernel();
	ut_ad(lock_rec_queue_validate(block, rec, index, offsets));
	return err;
}

IB_INTERN ulint lock_clust_rec_read_check_and_lock_alt(ulint flags, const buf_bib_lock_t* block, const rec_t* rec, dict_index_t* index, enum lock_mode mode, ulint gap_mode, que_thr_t* thr)
{
	mem_heap_t* tmp_heap = NULL;
	ulint offsets_[REC_OFFS_NORMAL_SIZE];
	ulint* offsets = offsets_;
	rec_offs_init(offsets_);
	offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &tmp_heap);
	ulint ret = lock_clust_rec_read_check_and_lock(flags, block, rec, index, offsets, mode, gap_mode, thr);
	if (tmp_heap) {
		IB_MEM_HEAP_FREE(tmp_heap);
	}
	return ret;
}

IB_INTERN ulint lock_get_type(const ib_lock_t* lock)
{
	return lock_get_type_low(lock);
}

IB_INTERN ib_uint64_t lock_get_trx_id(const ib_lock_t* lock)
{
	return trx_get_id(lock->trx);
}

IB_INTERN const char* lock_get_mode_str(const ib_lock_t* lock)
{
	ibool is_gap_lock = lock_get_type_low(lock) == LOCK_REC && lock_rec_get_gap(lock);
	switch (lock_get_mode(lock)) {
	case LOCK_S:
		if (is_gap_lock) {
			return "S,GAP";
		} else {
			return "S";
		}
	case LOCK_X:
		if (is_gap_lock) {
			return "X,GAP";
		} else {
			return "X";
		}
	case LOCK_IS:
		if (is_gap_lock) {
			return "IS,GAP";
		} else {
			return "IS";
		}
	case LOCK_IX:
		if (is_gap_lock) {
			return "IX,GAP";
		} else {
			return "IX";
		}
	case LOCK_AUTO_INC:
		return "AUTO_INC";
	default:
		return "UNKNOWN";
	}
}

IB_INTERN const char* lock_get_type_str(const ib_lock_t* lock)
{
	switch (lock_get_type_low(lock)) {
	case LOCK_REC:
		return "RECORD";
	case LOCK_TABLE:
		return "TABLE";
	default:
		return "UNKNOWN";
	}
}

IB_INLINE dict_table_t* lock_get_table(const ib_lock_t* lock)
{
	switch (lock_get_type_low(lock)) {
	case LOCK_REC:
		return lock->index->table;
	case LOCK_TABLE:
		return lock->un_member.tab_lock.table;
	default:
		UT_ERROR;
		return NULL;
	}
}

IB_INTERN ib_uint64_t lock_get_table_id(const ib_lock_t* lock)
{
	dict_table_t* table = lock_get_table(lock);
	return (ib_uint64_t)ut_conv_dulint_to_longlong(table->id);
}

IB_INTERN const char* lock_get_table_name(const ib_lock_t* lock)
{
	dict_table_t* table = lock_get_table(lock);
	return table->name;
}

IB_INTERN const dict_index_t* lock_rec_get_index(const ib_lock_t* lock)
{
	ut_a(lock_get_type_low(lock) == LOCK_REC);
	return lock->index;
}

IB_INTERN const char* lock_rec_get_index_name(const ib_lock_t* lock)
{
	ut_a(lock_get_type_low(lock) == LOCK_REC);
	return lock->index->name;
}

IB_INTERN ulint lock_rec_get_space_id(const ib_lock_t* lock)
{
	ut_a(lock_get_type_low(lock) == LOCK_REC);
	return lock->un_member.rec_lock.space;
}

IB_INTERN ulint lock_rec_get_page_no(const ib_lock_t* lock)
{
	ut_a(lock_get_type_low(lock) == LOCK_REC);
	return lock->un_member.rec_lock.page_no;
}