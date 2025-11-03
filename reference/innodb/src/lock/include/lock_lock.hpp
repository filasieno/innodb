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

/// \file lock_lock.hpp
/// \brief The transaction lock system.
/// \details Originally created by Heikki Tuuri on 5/7/1996.
/// \author Fabio N. Filasieno
/// \date 2025-10-20

#pragma once

#include "defs.hpp"
#include "buf_types.hpp"
#include "trx_types.hpp"
#include "mtr_types.hpp"
#include "rem_types.hpp"
#include "dict_types.hpp"
#include "que_types.hpp"
#include "lock_types.hpp"
#include "read_types.hpp"
#include "hash_hash.hpp"
#include "ut_vec.hpp"

#ifdef IB_DEBUG
	extern ibool lock_print_waits;
#endif // IB_DEBUG

// Buffer for storing information about the most recent deadlock error
extern ib_stream_t lock_latest_err_stream;

// Lock modes and types
// @{
constinit ulint LOCK_MODE_MASK = 0xFUL; // mask used to extract mode from the type_mode field in a lock
// Lock types
// @{
constinit ulint LOCK_TABLE = 16; // table lock
constinit ulint LOCK_REC = 32; // record lock
constinit ulint LOCK_TYPE_MASK = 0xF0UL; // mask used to extract lock type from the type_mode field in a lock
static_assert((LOCK_MODE_MASK & LOCK_TYPE_MASK) == 0, "LOCK_MODE_MASK & LOCK_TYPE_MASK");

constinit ulint LOCK_WAIT = 256; // Waiting lock flag; when set, it means that the lock has not yet been granted, it is just waiting for its turn in the wait queue
/* Precise modes */
constinit ulint LOCK_ORDINARY = 0; // ordinary next-key lock in contrast to LOCK_GAP or LOCK_REC_NOT_GAP
constinit ulint LOCK_GAP = 512; // lock holds only on the gap before the record
constinit ulint LOCK_REC_NOT_GAP = 1024; // lock only on the index record; does not block gap inserts
constinit ulint LOCK_INSERT_INTENTION = 2048; // waiting gap-type lock request to allow insert to wait for conflicting locks to clear
static_assert(((LOCK_WAIT|LOCK_GAP|LOCK_REC_NOT_GAP|LOCK_INSERT_INTENTION) & LOCK_MODE_MASK) == 0);
static_assert(((LOCK_WAIT|LOCK_GAP|LOCK_REC_NOT_GAP|LOCK_INSERT_INTENTION) & LOCK_TYPE_MASK) == 0);
/* @} */

/// \brief The lock system
extern lock_sys_t* lock_sys;

/// \defgroup lock_sys Lock system lifecycle
/// \brief Creation and teardown of the global lock system instance and related resources.
/// @{

/// \brief Creates the lock system at database start.
/// \param [in] n_cells number of slots in lock hash table
IB_INTERN void lock_sys_create(ulint n_cells);

/// \brief Closes the lock system at database shutdown.
IB_INTERN void lock_sys_close(void);

/// @}



/// \defgroup lock_var Lock module variables
/// \brief Helpers to initialize or reset module-level lock variables and state.
/// @{

/// \brief Reset the lock variables.
IB_INTERN void lock_var_init(void);

/// @}



/// \defgroup lock_table Table-level locking
/// \brief Acquire, query, and clean up table-level locks and table metadata related to locking.
/// @{

/// \brief Gets the source table of an ALTER TABLE transaction. The table must be covered by an IX or IS table lock.
/// \details Returns the source table of transaction, if it is covered by an IX or IS table lock; dest if there is no source table, and NULL if the transaction is locking more than two tables or an inconsistency is found.
/// \param [in] trx transaction
/// \param [in] dest destination of ALTER TABLE
/// \param [out] mode lock mode of the source table
/// \return source table, dest, or NULL as described
IB_INTERN dict_table_t* lock_get_src_table(trx_t* trx, dict_table_t* dest, enum ib_lock_mode* mode);

/// \brief Determine if the given table is exclusively "owned" by the given transaction.
/// \details Transaction holds LOCK_IX and possibly LOCK_AUTO_INC on the table.
/// \param [in] table table
/// \param [in] trx transaction
/// \return TRUE if table is only locked by trx, with LOCK_IX, and possibly LOCK_AUTO_INC
IB_INTERN ibool lock_is_table_exclusive(dict_table_t* table, trx_t* trx);

/// \brief Removes locks on a table to be dropped or truncated.
/// \details If remove_also_table_sx_locks is TRUE then table-level S and X locks are also removed in addition to other table-level and record-level locks. No lock, that is going to be removed, is allowed to be a wait lock.
/// \param [in] table table to be dropped or truncated
/// \param [in] remove_also_table_sx_locks also removes table S and X locks
IB_INTERN void lock_remove_all_on_table(dict_table_t* table, ibool remove_also_table_sx_locks);

/// \brief Locks the specified database table in the mode given.
/// \details If the lock cannot be granted immediately, the query thread is put to wait.
/// \param [in] flags if BTR_NO_LOCKING_FLAG bit is set, does nothing
/// \param [in] table database table in dictionary cache
/// \param [in] mode lock mode
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED
IB_INTERN ulint lock_table(ulint flags, dict_table_t* table, enum ib_lock_mode mode, que_thr_t* thr);

/// @}



/// \defgroup lock_clust Clustered-index record locking
/// \brief Operations that check or acquire locks on clustered index records.
/// @{

/// \brief Checks that a record is seen in a consistent read.
/// \param [in] rec user record which should be read or passed over by a read cursor
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] view consistent read view
/// \return TRUE if sees, or FALSE if an earlier version of the record should be retrieved
/// \note This function may be called while holding the search system latch. To obey the latching order, the kernel mutex is NOT reserved here.
IB_INTERN ibool lock_clust_rec_cons_read_sees(const rec_t* rec, ib_dict_index_t* index, const ulint* offsets, read_view_t* view);

/// \brief Checks if locks of other transactions prevent an immediate modify (update, delete mark, or delete unmark) of a clustered index record.
/// \details If they do, first tests if the query thread should anyway be suspended for some reason; if not, then puts the transaction and the query thread to the lock wait state and inserts a waiting request for a record x-lock to the lock queue.
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED
/// \param [in] flags if BTR_NO_LOCKING_FLAG bit is set, does nothing
/// \param [in] block buffer block of rec
/// \param [in] rec record which should be modified
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] thr query thread
IB_INTERN ulint lock_clust_rec_modify_check_and_lock(ulint flags, const buf_block_t* block, const rec_t* rec, ib_dict_index_t* index, const ulint* offsets, que_thr_t* thr);

/// \brief Checks if locks of other transactions prevent an immediate read, or passing over by a read cursor, of a clustered index record.
/// \details If they do, first tests if the query thread should anyway be suspended for some reason; if not, then puts the transaction and the query thread to the lock wait state and inserts a waiting request for a record lock to the lock queue. Sets the requested mode lock on the record.
/// \param [in] flags if BTR_NO_LOCKING_FLAG bit is set, does nothing
/// \param [in] block buffer block of rec
/// \param [in] rec user record or page supremum record which should be read or passed over by a read cursor
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] mode mode of the lock which the read cursor should set on records: LOCK_S or LOCK_X; the latter is possible in SELECT FOR UPDATE
/// \param [in] gap_mode LOCK_ORDINARY, LOCK_GAP, or LOCK_REC_NOT_GAP
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED
IB_INTERN ulint lock_clust_rec_read_check_and_lock(ulint flags, const buf_block_t* block, const rec_t* rec, ib_dict_index_t* index, const ulint* offsets, enum ib_lock_mode mode, ulint gap_mode, que_thr_t* thr);

/// \brief Checks if locks of other transactions prevent an immediate read, or passing over by a read cursor, of a clustered index record.
/// \details This is an alternative version of lock_clust_rec_read_check_and_lock() that does not require the parameter "offsets".
/// \param [in] flags if BTR_NO_LOCKING_FLAG bit is set, does nothing
/// \param [in] block buffer block of rec
/// \param [in] rec user record or page supremum record which should be read or passed over by a read cursor
/// \param [in] index clustered index
/// \param [in] mode mode of the lock which the read cursor should set on records: LOCK_S or LOCK_X; the latter is possible in SELECT FOR UPDATE
/// \param [in] gap_mode LOCK_ORDINARY, LOCK_GAP, or LOCK_REC_NOT_GAP
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED
IB_INTERN ulint lock_clust_rec_read_check_and_lock_alt(ulint flags, const buf_block_t* block, const rec_t* rec, ib_dict_index_t* index, enum ib_lock_mode mode, ulint gap_mode, que_thr_t* thr);

/// \brief Checks if some transaction has an implicit x-lock on a record in a clustered index.
/// \return transaction which has the x-lock, or NULL
/// \param [in] rec user record
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
IB_INLINE trx_t* lock_clust_rec_some_has_impl(const rec_t* rec, ib_dict_index_t* index, const ulint* offsets);

/// @}



/// \defgroup lock_sec Secondary-index record locking
/// \brief Operations that check or acquire locks on secondary index records.
/// @{

/// \brief Checks that a non-clustered index record is seen in a consistent read.
/// \details NOTE that a non-clustered index page contains so little information on its modifications that also in the case FALSE, the present version of rec may be the right, but we must check this from the clustered index record.
/// \param [in] rec user record which should be read or passed over by a read cursor
/// \param [in] view consistent read view
/// \return TRUE if certainly sees, or FALSE if an earlier version of the clustered index record might be needed
/// \note This function may be called while holding the search system latch. To obey the latching order, the kernel mutex is NOT reserved here.
IB_INTERN ulint lock_sec_rec_cons_read_sees(const rec_t* rec, const read_view_t* view);

/// \brief Checks if locks of other transactions prevent an immediate modify (delete mark or delete unmark) of a secondary index record.
/// \param [in] flags if BTR_NO_LOCKING_FLAG bit is set, does nothing
/// \param [in,out] block buffer block of rec
/// \param [in] rec record which should be modified; NOTE: as this is a secondary index, we always have to modify the clustered index record first: see the comment below
/// \param [in] index secondary index
/// \param [in] thr query thread
/// \param [in,out] mtr mini-transaction
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED
IB_INTERN ulint lock_sec_rec_modify_check_and_lock(ulint flags, buf_block_t* block, const rec_t* rec, ib_dict_index_t* index, que_thr_t* thr, mtr_t* mtr);

/// \brief Like the counterpart for a clustered index below, but now we read a secondary index record.
/// \param [in] flags if BTR_NO_LOCKING_FLAG bit is set, does nothing
/// \param [in] block buffer block of rec
/// \param [in] rec user record or page supremum record which should be read or passed over by a read cursor
/// \param [in] index secondary index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] mode mode of the lock which the read cursor should set on records: LOCK_S or LOCK_X; the latter is possible in SELECT FOR UPDATE
/// \param [in] gap_mode LOCK_ORDINARY, LOCK_GAP, or LOCK_REC_NOT_GAP
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED
IB_INTERN ulint lock_sec_rec_read_check_and_lock(ulint flags, const buf_block_t* block, const rec_t* rec, ib_dict_index_t* index, const ulint* offsets, enum ib_lock_mode mode, ulint gap_mode, que_thr_t* thr);
/// @}




/// \defgroup lock_rec Record-lock helpers and queries
/// \brief Utilities that inspect, mutate, or manage record-level locks and their metadata.
/// @{

/// \brief Returns TRUE if there are explicit record locks on a page.
/// \return TRUE if there are explicit record locks on the page
/// \param [in] space space id
/// \param [in] page_no page number
IB_INTERN ibool lock_rec_expl_exist_on_page(ulint space, ulint page_no);

/// \brief Looks for a set bit in a record lock bitmap. Returns ULINT_UNDEFINED, if none found.
/// \param [in] lock record lock with at least one bit set
/// \return bit index == heap number of the record, or ULINT_UNDEFINED if none found
IB_INTERN ulint lock_rec_find_set_bit(const ib_lock_t* lock);

/// \brief For a record lock, gets the page number on which the lock is.
/// \param [in] lock
/// \return page number
IB_INTERN ulint lock_rec_get_page_no(const ib_lock_t* lock);

/// \brief For a record lock, gets the tablespace number on which the lock is.
/// \return tablespace number
IB_INTERN ulint lock_rec_get_space_id(const ib_lock_t* lock);

/// \brief For a record lock, gets the index on which the lock is.
/// \param [in] lock the lock
/// \return index
IB_INTERN const ib_dict_index_t* lock_rec_get_index(const ib_lock_t* lock);

/// \brief For a record lock, gets the name of the index on which the lock is.
/// \param [in] lock the lock
/// \return name of the index
IB_INTERN const char* lock_rec_get_index_name(const ib_lock_t* lock);

/// \brief Restores the state of explicit lock requests on a single record, where the state was stored on the infimum of the page.
/// \param [in] block buffer block containing rec
/// \param [in] rec record whose lock state is restored
/// \param [in] donator page (rec is not necessarily on this page) whose infimum stored the lock state; lock bits are reset on the infimum
IB_INTERN void lock_rec_restore_from_page_infimum(const buf_block_t* block, const rec_t* rec, const buf_block_t* donator);

/// \brief Resets the original locks on heir and replaces them with gap type locks inherited from rec.
/// \param [in] heir_block block containing the record which inherits
/// \param [in] block block containing the record from which inherited; does NOT reset the locks on this record
/// \param [in] heir_heap_no heap_no of the inheriting record
/// \param [in] heap_no heap_no of the donating record
IB_INTERN void lock_rec_reset_and_inherit_gap_locks(const buf_block_t* heir_block, const buf_block_t* block, ulint heir_heap_no, ulint heap_no);

/// \brief Stores on the page infimum record the explicit locks of another record.
/// \details This function is used to store the lock state of a record when it is updated and the size of the record changes in the update. The record is in such an update moved, perhaps to another page. The infimum record acts as a dummy carrier record, taking care of lock releases while the actual record is being moved.
/// \param [in] block buffer block containing rec
/// \param [in] rec record whose lock state is stored on the infimum record of the same page; lock bits are reset on the record
IB_INTERN void lock_rec_store_on_page_infimum(const buf_block_t* block, const rec_t* rec);

/// \brief Removes a granted record lock of a transaction from the queue and grants locks to other transactions waiting in the queue if they now are entitled to a lock.
/// \param [in] trx transaction that has set a record lock
/// \param [in] block buffer block containing rec
/// \param [in] rec record
/// \param [in] ib_lock_mode LOCK_S or LOCK_X
IB_INTERN void lock_rec_unlock(trx_t* trx, const buf_block_t* block, const rec_t* rec, enum ib_lock_mode ib_lock_mode);

/// \brief Checks if locks of other transactions prevent an immediate insert of a record.
/// \details If they do, first tests if the query thread should anyway be suspended for some reason; if not, then puts the transaction and the query thread to the lock wait state and inserts a waiting request for a gap x-lock to the lock queue.
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DEADLOCK, or DB_QUE_THR_SUSPENDED
/// \param [in] flags if BTR_NO_LOCKING_FLAG bit is set, does nothing
/// \param [in] rec record after which to insert
/// \param [in,out] block buffer block of rec
/// \param [in] index index
/// \param [in] thr query thread
/// \param [in,out] mtr mini-transaction
/// \param [out] inherit set to TRUE if the new inserted record maybe should inherit LOCK_GAP type locks from the successor record
IB_INTERN ulint lock_rec_insert_check_and_lock(ulint flags, const rec_t* rec, buf_block_t* block, ib_dict_index_t* index, que_thr_t* thr, mtr_t* mtr, ibool* inherit);

/// @}



/// \defgroup lock_move Lock migration across pages
/// \brief Moving existing record lock state between pages/records during B-tree reorganization.
/// @{

/// \brief Moves the explicit locks on user records to another page if a record list end is moved to another page.
/// \param [in] new_block index page to move to
/// \param [in] block index page
/// \param [in] rec record on page: this is the first record moved
IB_INTERN void lock_move_rec_list_end(const buf_block_t* new_block, const buf_block_t* block, const rec_t* rec);

/// \brief Moves the explicit locks on user records to another page if a record list start is moved to another page.
/// \param [in] new_block index page to move to
/// \param [in] block index page
/// \param [in] rec record on page: this is the first record NOT copied
/// \param [in] old_end old previous-to-last record on new_page before the records were copied
IB_INTERN void lock_move_rec_list_start(const buf_block_t* new_block, const buf_block_t* block, const rec_t* rec, const rec_t* old_end);

/// \brief Updates the lock table when we have reorganized a page.
/// \details NOTE: we copy also the locks set on the infimum of the page; the infimum may carry locks if an update of a record is occurring on the page, and its locks were temporarily stored on the infimum.
/// \param [in] block old index page, now reorganized
/// \param [in] oblock copy of the old, not page
IB_INTERN void lock_move_reorganize_page(const buf_block_t* block, const buf_block_t* oblock);

/// @}



/// \defgroup lock_update Lock updates on page changes
/// \brief Update lock state in response to page-level structural changes (split, merge, discard, etc.).
/// @{

/// \brief Updates the lock table when a page is copied to another and the original page is removed from the chain of leaf pages, except if page is the root!
/// \param [in] new_block index page to which copied
/// \param [in] block index page; NOT the root!
IB_INTERN void lock_update_copy_and_discard(const buf_block_t* new_block, const buf_block_t* block);

/// \brief Updates the lock table when a page is discarded.
/// \param [in] heir_block index page which will inherit the locks
/// \param [in] heir_heap_no heap_no of the record which will inherit the locks
/// \param [in] block index page which will be discarded
IB_INTERN void lock_update_discard(const buf_block_t* heir_block, ulint heir_heap_no, const buf_block_t* block);

/// \brief Updates the lock table when a page is merged to the left.
/// \param [in] left_block left page to which merged
/// \param [in] orig_pred original predecessor of supremum on the left page before merge
/// \param [in] right_block merged index page which will be discarded
IB_INTERN void lock_update_merge_left(const buf_block_t* left_block, const rec_t* orig_pred, const buf_block_t* right_block);

/// \brief Updates the lock table when a page is merged to the right.
/// \param [in] right_block right page to which merged
/// \param [in] orig_succ original successor of infimum on the right page before merge
/// \param [in] left_block merged index page which will be discarded
IB_INTERN void lock_update_merge_right(const buf_block_t* right_block, const rec_t* orig_succ, const buf_block_t* left_block);

/// \brief Updates the lock table when the root page is copied to another in btr_root_raise_and_insert.
/// \details Note that we leave lock structs on the root page, even though they do not make sense on other than leaf pages: the reason is that in a pessimistic update the infimum record of the root page will act as a dummy carrier of the locks of the record to be updated.
/// \param [in] block index page to which copied
/// \param [in] root root page
IB_INTERN void lock_update_root_raise(const buf_block_t* block, const buf_block_t* root);

/// \brief Updates the lock table when a page is split to the left.
/// \param [in] right_block right page
/// \param [in] left_block left page
IB_INTERN void lock_update_split_left(const buf_block_t* right_block, const buf_block_t* left_block);

/// \brief Updates the lock table when a page is split to the right.
/// \param [in] right_block right page
/// \param [in] left_block left page
IB_INTERN void lock_update_split_right(const buf_block_t* right_block, const buf_block_t* left_block);

/// \brief Updates the lock table when a new user record is inserted.
/// \param [in] block buffer block containing rec
/// \param [in] rec the inserted record
IB_INTERN void lock_update_insert(const buf_block_t* block, const rec_t* rec);

/// \brief Updates the lock table when a record is removed.
/// \param [in] block buffer block containing rec
/// \param [in] rec the record to be removed
IB_INTERN void lock_update_delete(const buf_block_t* block, const rec_t* rec);

/// @}



/// \defgroup lock_print Lock state printing
/// \brief Introspection utilities to format and dump lock state for diagnostics and monitoring.
/// @{

/// \brief Prints info of locks for each transaction.
/// \param [in] stream stream where to print
IB_INTERN void lock_print_info_all_transactions(ib_stream_t stream);

/// \brief Prints info of locks for all transactions.
/// \details Returns FALSE if not able to obtain kernel mutex and exits without printing info.
/// \param [in] stream stream where to print
/// \param [in] nowait whether to wait for the kernel mutex
/// \return FALSE if not able to obtain kernel mutex
IB_INTERN ibool lock_print_info_summary(ib_stream_t stream, ibool nowait);

/// \brief Prints info of a record lock.
/// \param [in] stream stream where to print
/// \param [in] lock record type lock
IB_INTERN void lock_rec_print(ib_stream_t stream, const ib_lock_t* lock);

/// \brief Prints info of a table lock.
/// \param [in] stream stream where to print
/// \param [in] lock table type lock
IB_INTERN void lock_table_print(ib_stream_t stream, const ib_lock_t* lock);

/// @}



/// \defgroup lock_get Lock getters and metadata
/// \brief Read-only accessors for lock properties and related metadata.
/// @{

/// \brief Gets the mode of a lock in a human readable string.
/// \param [in] lock the lock
/// \return lock mode
IB_INTERN const char* lock_get_mode_str(const ib_lock_t* lock);

/// \brief Gets the size of a lock struct.
/// \return size in bytes
IB_INTERN ulint lock_get_size(void);

/// \brief Gets the name of the table on which the lock is.
/// \param [in] lock the lock
/// \return name of the table
IB_INTERN const char* lock_get_table_name(const ib_lock_t* lock);

/// \brief Gets the id of the table on which the lock is.
/// \param [in] lock the lock
/// \return id of the table
IB_INTERN ib_uint64_t lock_get_table_id(const ib_lock_t* lock);

/// \brief Gets the id of the transaction owning a lock.
/// \param [in] lock the lock
/// \return transaction id
IB_INTERN ib_uint64_t lock_get_trx_id(const ib_lock_t* lock);

/// \brief Gets the type of a lock in a human readable string.
/// \param [in] lock the lock
/// \return lock type
IB_INTERN const char* lock_get_type_str(const ib_lock_t* lock);

/// \brief Gets the type of a lock. Non-inline version for using outside of the lock module.
/// \param [in] lock lock
/// \return LOCK_TABLE or LOCK_REC
IB_INTERN ulint lock_get_type(const ib_lock_t* lock);

/// \brief Gets the heap_no of the smallest user record on a page.
/// \return heap_no of smallest user record, or PAGE_HEAP_NO_SUPREMUM
/// \param [in] block buffer block
IB_INLINE ulint lock_get_min_heap_no(const buf_block_t* block);

/// @}



/// \defgroup lock_flow Lock waits, flow, and checks
/// \brief Control flow around lock waits, cancellation, and sanity validations.
/// @{

/// \brief Checks if a lock request lock1 has to wait for request lock2.
/// \param [in] lock1 waiting lock
/// \param [in] lock2 another lock; NOTE that it is assumed that this has a lock bit set on the same record as in lock1 if the locks are record locks
/// \return TRUE if lock1 has to wait for lock2 to be removed
IB_INTERN ibool lock_has_to_wait(const ib_lock_t* lock1, const ib_lock_t* lock2);

/// \brief Releases transaction locks, and releases possible other transactions waiting because of these locks.
/// \param [in] trx transaction
IB_INTERN void lock_release_off_kernel(trx_t* trx);

/// \brief Checks that a transaction id is sensible, i.e., not in the future.
/// \param [in] trx_id trx id
/// \param [in] rec user record
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] has_kernel_mutex TRUE if the caller owns the kernel mutex
/// \return TRUE if ok
IB_INTERN ibool lock_check_trx_id_sanity(trx_id_t trx_id, const rec_t* rec, ib_dict_index_t* index, const ulint* offsets, ibool has_kernel_mutex);

/// \brief Cancels a waiting lock request and releases possible other transactions waiting behind it.
/// \param [in] lock waiting lock request
IB_INTERN void lock_cancel_waiting_and_release(ib_lock_t* lock);

/// \brief Return approximate number of record locks (bits set in the bitmap) for this transaction.
/// \details Since delete-marked records may be removed, the record count will not be precise.
/// \param [in] trx transaction
/// \return approximate number of rows locked
IB_INTERN ulint lock_number_of_rows_locked(trx_t* trx);
/// @}



/// \defgroup lock_inline_utils Inline hashing utilities
/// \brief Inline helpers used by the lock module to hash or fold page addresses.
/// @{

/// \brief Calculates the fold value of a page file address: used in inserting or searching for a lock in the hash table.
/// \param [in] space space
/// \param [in] page_no page number
/// \return folded value
IB_INLINE ulint lock_rec_fold(ulint space, ulint page_no) __attribute__((const));

/// \brief Calculates the hash value of a page file address: used in inserting or searching for a lock in the hash table.
/// \param [in] space space
/// \param [in] page_no page number
/// \return hashed value
IB_INLINE ulint lock_rec_hash(ulint space, ulint page_no);
/// @}




#ifndef IB_DO_NOT_INLINE
	#include "lock_lock.inl"
#endif

#endif
