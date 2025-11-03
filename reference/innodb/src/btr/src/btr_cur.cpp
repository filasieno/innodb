// Copyright (c) 1994, 2010, Innobase Oy. All Rights Reserved.
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
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA

/// \file btr_cur.cpp

/// \brief The index tree cursor
/// \details All changes that row operations make to a B-tree or the records there must go through this module! Undo log records are written here of every modify or 
/// insert of a clustered index record. 
/// \note To make sure we do not run out of disk space during a pessimistic insert or update, we have to reserve 2 x the height of the index tree many pages in the tablespace
//  before we start the operation, because if leaf splitting has been started, it is difficult to undo, except by crashing the database and doing a roll-forward.
/// \details Originally created by Heikki Tuuri on 10/16/1994.
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#include "defs.i"
#include <zlib.h>
#include "btr_cur.hpp"

#ifdef IB_DO_NOT_INLINE
	#include "btr_cur.inl"
#endif

#include "row_upd.hpp"

#ifndef IB_HOTBACKUP

#include "mtr_log.hpp"
#include "page_page.hpp"
#include "page_zip.hpp"
#include "rem_rec.hpp"
#include "rem_cmp.hpp"
#include "buf_lru.hpp"
#include "btr_btr.hpp"
#include "btr_sea.hpp"
#include "trx_rec.hpp"
#include "trx_roll.hpp" // trx_is_recv()
#include "que_que.hpp"
#include "row_row.hpp"
#include "srv_srv.hpp"
#include "ibuf_ibuf.hpp"
#include "lock_lock.hpp"

#ifdef IB_DEBUG
/** If the following is set to TRUE, this module prints a lot of
trace information of individual record operations */
IB_INTERN ibool	btr_cur_print_record_ops = FALSE;
#endif // IB_DEBUG
/** Number of searches down the B-tree in btr_cur_search_to_nth_level(). */
IB_INTERN ulint	btr_cur_n_non_sea	= 0;
/** Number of successful adaptive hash index lookups in
btr_cur_search_to_nth_level(). */
IB_INTERN ulint	btr_cur_n_sea		= 0;
/** Old value of btr_cur_n_non_sea.  Copied by
srv_refresh_innodb_monitor_stats().  Referenced by
srv_printf_innodb_monitor(). */
IB_INTERN ulint	btr_cur_n_non_sea_old	= 0;
/** Old value of btr_cur_n_sea.  Copied by
srv_refresh_innodb_monitor_stats().  Referenced by
srv_printf_innodb_monitor(). */
IB_INTERN ulint	btr_cur_n_sea_old	= 0;
/** In the optimistic insert, if the insert does not fit, but this much space
can be released by page reorganize, then it is reorganized */
constinit ulint BTR_CUR_PAGE_REORGANIZE_LIMIT = (IB_PAGE_SIZE / 32);
/** The structure of a BLOB part header */
/* @{ */
/*--------------------------------------*/
constinit ulint BTR_BLOB_HDR_PART_LEN = 0;	/*!< BLOB part len on this
						page */
constinit ulint BTR_BLOB_HDR_NEXT_PAGE_NO = 4;	/*!< next BLOB part page no,
						FIL_NULL if none */
/*--------------------------------------*/
constinit ulint BTR_BLOB_HDR_SIZE = 8;	/*!< Size of a BLOB
						part header, in bytes */
/* @} */
#endif // !IB_HOTBACKUP
/** A BLOB field reference full of zero, for use in assertions and tests.
Initially, BLOB field references are set to zero, in
dtuple_convert_big_rec(). */
IB_INTERN const byte field_ref_zero[BTR_EXTERN_FIELD_REF_SIZE];
#ifndef IB_HOTBACKUP

/// \brief Marks all extern fields in a record as owned by the record.
/// \param [in/out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in/out] rec record in a clustered index
/// \param [in] index index of the page
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] mtr mtr, or NULL if not logged
/// \details This function should be called if the delete mark of a record is removed: a not delete marked record always owns all its extern fields.
static void btr_cur_unmark_extern_fields(page_zip_des_t* page_zip, rec_t* rec, dict_index_t* index, const ulint* offsets, mtr_t* mtr);

/// \brief Adds path information to the cursor for the current page, for which the binary search has been performed.
/// \param [in] cursor cursor positioned on a page
/// \param [in] height height of the page in tree; 0 means leaf node
/// \param [in] root_height root node height in tree
static void btr_cur_add_path_info(btr_cur_t* cursor, ulint height, ulint root_height);

/// \brief Frees the externally stored fields for a record, if the field is mentioned in the update vector.
/// \param [in] index index of rec; the index tree MUST be X-latched
/// \param [in] rec record
/// \param [in] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] update update vector
/// \param [in] rb_ctx rollback context
/// \param [in] mtr mini-transaction handle which contains an X-latch to record page and to the tree
static void btr_rec_free_updated_extern_fields(dict_index_t* index, rec_t* rec, page_zip_des_t* page_zip, const ulint* offsets, const upd_t* update, enum trx_rb_ctx rb_ctx, mtr_t* mtr);

/// \brief Frees the externally stored fields for a record.
/// \param [in] index index of the data, the index tree MUST be X-latched
/// \param [in] rec record
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] rb_ctx rollback context
/// \param [in] mtr mini-transaction handle which contains an X-latch to record page and to the index tree
static void btr_rec_free_externally_stored_fields(dict_index_t* index, rec_t* rec, const ulint* offsets, page_zip_des_t* page_zip, enum trx_rb_ctx rb_ctx, mtr_t* mtr);

/// \brief Gets the externally stored size of a record, in units of a database page.
/// \param [in] rec record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return externally stored part, in units of a database page
static ulint btr_rec_get_externally_stored_len(rec_t* rec, const ulint* offsets);

/// \brief Inserts a record if there is enough space, or if enough space can be freed by reorganizing.
/// \param [in] cursor cursor on page after which to insert; cursor stays valid
/// \param [in] tuple tuple to insert; the size info need not have been stored to tuple
/// \param [in] n_ext number of externally stored columns
/// \param [in] mtr mtr
/// \return pointer to inserted record if succeed, else NULL
static rec_t* btr_cur_insert_if_possible(btr_cur_t* cursor, const dtuple_t* tuple, ulint n_ext, mtr_t* mtr);
#ifdef IB_DEBUG

/// \brief Report information about a transaction.
/// \param [in] trx transaction
/// \param [in] index index
/// \param [in] op operation
static void btr_cur_trx_report(trx_t* trx, const dict_index_t* index, const char* op);

#endif // IB_DEBUG

/// \brief See if there is enough place in the page modification log to log an update-in-place.
/// \param [in/out] page_zip compressed page
/// \param [in/out] block buffer page
/// \param [in] dict_index the index corresponding to the block
/// \param [in] length size needed
/// \param [in] create TRUE=delete-and-insert, FALSE=update-in-place
/// \param [in] mtr mini-transaction
/// \return TRUE if enough place
static ibool btr_cur_update_alloc_zip(page_zip_des_t* page_zip, buf_block_t* block, dict_index_t* dict_index, ulint length, ibool create, mtr_t* mtr);
#endif // !IB_HOTBACKUP

/// \brief The following function is used to set the deleted bit of a record.
/// \param [in/out] rec physical record
/// \param [in/out] page_zip compressed page (or NULL)
/// \param [in] flag nonzero if delete marked
IB_INLINE void btr_rec_set_deleted_flag(rec_t* rec, page_zip_des_t* page_zip, ulint flag)
{
	if (page_rec_is_comp(rec)) {
		rec_set_deleted_flag_new(rec, page_zip, flag);
	} else {
		ut_ad(!page_zip);
		rec_set_deleted_flag_old(rec, flag);
	}
}

#ifndef IB_HOTBACKUP

//==================== B-TREE SEARCH =========================

/// \brief Reset global configuration variables.
IB_INTERN void btr_cur_var_init(void)
{
    btr_cur_n_non_sea = 0;
    btr_cur_n_sea = 0;
    btr_cur_n_non_sea_old = 0;
    btr_cur_n_sea_old = 0;
#ifdef IB_DEBUG
    btr_cur_print_record_ops = FALSE;
#endif // IB_DEBUG
}


/// \brief Latches the leaf page or pages requested.
/// \param [in] page leaf page where the search converged
/// \param [in] space space id
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] page_no page number of the leaf
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...
/// \param [in] cursor cursor
/// \param [in] mtr mtr
static void btr_cur_latch_leaves(page_t* page, ulint space, ulint zip_size, ulint page_no, ulint latch_mode, btr_cur_t* cursor, mtr_t* mtr)
{
	ut_ad(page && mtr);
	switch (latch_mode) {
	case BTR_SEARCH_LEAF:
	case BTR_MODIFY_LEAF:
		{
			ulint mode = latch_mode == BTR_SEARCH_LEAF ? RW_S_LATCH : RW_X_LATCH;
			buf_block_t* get_block = btr_block_get(space, zip_size, page_no, mode, mtr);
#ifdef IB_BTR_DEBUG
		ut_a(page_is_comp(get_block->frame) == page_is_comp(page));
#endif // IB_BTR_DEBUG
		get_block->check_index_page_at_flush = TRUE;
		return;
		}
	case BTR_MODIFY_TREE:
		// x-latch also brothers from left to right
		{
			ulint left_page_no = btr_page_get_prev(page, mtr);
			if (left_page_no != FIL_NULL) {
				buf_block_t* get_block = btr_block_get(space, zip_size, left_page_no, RW_X_LATCH, mtr);
#ifdef IB_BTR_DEBUG
				ut_a(page_is_comp(get_block->frame)
				     == page_is_comp(page));
				ut_a(btr_page_get_next(get_block->frame, mtr)
				     == page_get_page_no(page));
#endif // IB_BTR_DEBUG
				get_block->check_index_page_at_flush = TRUE;
			}
			buf_block_t* get_block = btr_block_get(space, zip_size, page_no, RW_X_LATCH, mtr);
#ifdef IB_BTR_DEBUG
			ut_a(page_is_comp(get_block->frame) == page_is_comp(page));
#endif // IB_BTR_DEBUG
			get_block->check_index_page_at_flush = TRUE;
			ulint right_page_no = btr_page_get_next(page, mtr);
			if (right_page_no != FIL_NULL) {
				buf_block_t* get_block = btr_block_get(space, zip_size, right_page_no, RW_X_LATCH, mtr);
#ifdef IB_BTR_DEBUG
				ut_a(page_is_comp(get_block->frame)
				     == page_is_comp(page));
				ut_a(btr_page_get_prev(get_block->frame, mtr)
				     == page_get_page_no(page));
#endif // IB_BTR_DEBUG
				get_block->check_index_page_at_flush = TRUE;
			}
		}
		return;
	case BTR_SEARCH_PREV:
	case BTR_MODIFY_PREV:
		{
			ulint mode = latch_mode == BTR_SEARCH_PREV ? RW_S_LATCH : RW_X_LATCH;
			// latch also left brother
			ulint left_page_no = btr_page_get_prev(page, mtr);
			if (left_page_no != FIL_NULL) {
				buf_block_t* get_block = btr_block_get(space, zip_size, left_page_no, mode, mtr);
				cursor->left_block = get_block;
#ifdef IB_BTR_DEBUG
				ut_a(page_is_comp(get_block->frame)
				     == page_is_comp(page));
				ut_a(btr_page_get_next(get_block->frame, mtr)
				     == page_get_page_no(page));
#endif // IB_BTR_DEBUG
				get_block->check_index_page_at_flush = TRUE;
			}
			buf_block_t* get_block = btr_block_get(space, zip_size, page_no, mode, mtr);
#ifdef IB_BTR_DEBUG
			ut_a(page_is_comp(get_block->frame) == page_is_comp(page));
#endif // IB_BTR_DEBUG
			get_block->check_index_page_at_flush = TRUE;
		}
		return;
	}
	UT_ERROR;
}

/// \brief Searches an index tree and positions a tree cursor on a given level.
/// \details Note that if mode is PAGE_CUR_LE, which is used in inserts, then cursor->up_match and cursor->low_match both will have sensible values. 
/// If mode is PAGE_CUR_GE, then up_match will have a sensible value.
/// If mode is PAGE_CUR_LE, cursor is left at the place where an insert of the search tuple should be performed in the B-tree. InnoDB does an insert
/// immediately after the cursor. Thus, the cursor may end up on a user record, or on a page infimum record.
/// \note n_fields_cmp in tuple must be set so that it cannot be compared to node pointer page number fields on the upper levels of the tree!
/// \param [in] dict_index index
/// \param [in] level the tree level of search
/// \param [in] tuple data tuple; NOTE: n_fields_cmp in tuple must be set so that it cannot get compared to the node ptr page number field!
/// \param [in] mode PAGE_CUR_L, ...; Inserts should always be made using PAGE_CUR_LE to search the position!
/// \param [in] latch_mode BTR_SEARCH_LEAF, ..., ORed with BTR_INSERT and BTR_ESTIMATE; cursor->left_block is used to store a pointer to the left neighbor page, in the cases BTR_SEARCH_PREV and BTR_MODIFY_PREV; NOTE that if has_search_latch is != 0, we maybe do not have a latch set on the cursor page, we assume the caller uses his search latch to protect the record!
/// \param [in/out] cursor tree cursor; the cursor page is s- or x-latched, but see also above!
/// \param [in] has_search_latch info on the latch mode the caller currently has on btr_search_latch: RW_S_LATCH, or 0
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_cur_search_to_nth_level(dict_index_t* dict_index, ulint level, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_cur_t* cursor, ulint has_search_latch, const char* file, ulint line, mtr_t* mtr)
{
    ulint root_height = 0; // remove warning
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    // Currently, PAGE_CUR_LE is the only search mode used for searches ending to upper levels
    ut_ad(level == 0 || mode == PAGE_CUR_LE);
    ut_ad(dict_index_check_search_tuple(dict_index, tuple));
    ut_ad(!dict_index_is_ibuf(dict_index) || ibuf_inside());
    ut_ad(dtuple_check_typed(tuple));
#ifdef IB_DEBUG
    cursor->up_match = ULINT_UNDEFINED;
    cursor->low_match = ULINT_UNDEFINED;
#endif
    ulint insert_planned = latch_mode & BTR_INSERT;
    ulint estimate = latch_mode & BTR_ESTIMATE;
    ulint ignore_sec_unique = latch_mode & BTR_IGNORE_SEC_UNIQUE;
    latch_mode = latch_mode & ~(BTR_INSERT | BTR_ESTIMATE | BTR_IGNORE_SEC_UNIQUE);
    ut_ad(!insert_planned || (mode == PAGE_CUR_LE));
    cursor->flag = BTR_CUR_BINARY;
    cursor->index = dict_index;
#ifndef BTR_CUR_ADAPT
    buf_block_t* guess = NULL;
#else
    btr_search_t* info = btr_search_get_info(dict_index);
    buf_block_t* guess = info->root_guess;
#ifdef BTR_CUR_HASH_ADAPT
#ifdef IB_SEARCH_PERF_STAT
    info->n_searches++;
#endif
    if (rw_lock_get_writer(&btr_search_latch) == RW_LOCK_NOT_LOCKED && latch_mode <= BTR_MODIFY_LEAF && info->last_hash_succ && !estimate
#ifdef PAGE_CUR_LE_OR_EXTENDS
        && mode != PAGE_CUR_LE_OR_EXTENDS
#endif // PAGE_CUR_LE_OR_EXTENDS
        // If !has_search_latch, we do a dirty read of btr_search_enabled below, and btr_search_guess_on_hash() will have to check it again.
        && IB_LIKELY(btr_search_enabled) && btr_search_guess_on_hash(dict_index, info, tuple, mode, latch_mode, cursor, has_search_latch, mtr)) {
        // Search using the hash index succeeded
        ut_ad(cursor->up_match != ULINT_UNDEFINED || mode != PAGE_CUR_GE);
        ut_ad(cursor->up_match != ULINT_UNDEFINED || mode != PAGE_CUR_LE);
        ut_ad(cursor->low_match != ULINT_UNDEFINED || mode != PAGE_CUR_LE);
        btr_cur_n_sea++;
        return;
    }
#endif // BTR_CUR_HASH_ADAPT
#endif // BTR_CUR_ADAPT
    btr_cur_n_non_sea++;
    // If the hash search did not succeed, do binary search down the tree
    if (has_search_latch) {
        // Release possible search latch to obey latching order
        rw_lock_s_unlock(&btr_search_latch);
    }
    // Store the position of the tree latch we push to mtr so that we know how to release it when we have latched leaf node(s)
    ulint savepoint = mtr_set_savepoint(mtr);
    if (latch_mode == BTR_MODIFY_TREE) {
        mtr_x_lock(dict_index_get_lock(dict_index), mtr);
    } else if (latch_mode == BTR_CONT_MODIFY_TREE) {
        // Do nothing
        ut_ad(mtr_memo_contains(mtr, dict_index_get_lock(dict_index), MTR_MEMO_X_LOCK));
    } else {
        mtr_s_lock(dict_index_get_lock(dict_index), mtr);
    }
    page_cur_t* page_cursor = btr_cur_get_page_cur(cursor);
    ulint space = dict_index_get_space(dict_index);
    ulint page_no = dict_index_get_page(dict_index);
    ulint up_match = 0;
    ulint up_bytes = 0;
    ulint low_match = 0;
    ulint low_bytes = 0;
    ulint height = ULINT_UNDEFINED;
    // We use these modified search modes on non-leaf levels of the B-tree. These let us end up in the right B-tree leaf. In that leaf we use the original search mode.
    ulint page_mode;
    switch (mode) {
    case PAGE_CUR_GE:
        page_mode = PAGE_CUR_L;
        break;
    case PAGE_CUR_G:
        page_mode = PAGE_CUR_LE;
        break;
    default:
#ifdef PAGE_CUR_LE_OR_EXTENDS
        ut_ad(mode == PAGE_CUR_L || mode == PAGE_CUR_LE || mode == PAGE_CUR_LE_OR_EXTENDS);
#else // PAGE_CUR_LE_OR_EXTENDS
        ut_ad(mode == PAGE_CUR_L || mode == PAGE_CUR_LE);
#endif // PAGE_CUR_LE_OR_EXTENDS
        page_mode = mode;
        break;
    }
    // Loop and search until we arrive at the desired level
    for (;;) {
		ulint zip_size = dict_table_zip_size(dict_index->table);
		ulint rw_latch = RW_NO_LATCH;
		ulint buf_mode = BUF_GET;
		if (height == 0 && latch_mode <= BTR_MODIFY_LEAF) {
			rw_latch = latch_mode;
			if (insert_planned
			    && ibuf_should_try(dict_index, ignore_sec_unique)) {
				// Try insert to the insert buffer if the page is not in the buffer pool
				buf_mode = BUF_GET_IF_IN_POOL;
			}
		}
retry_page_get:
		buf_block_t* block = buf_page_get_gen(space, zip_size, page_no,
					 rw_latch, guess, buf_mode,
					 file, line, mtr);
		if (block == NULL) {
			// This must be a search to perform an insert; try insert to the insert buffer
			ut_ad(buf_mode == BUF_GET_IF_IN_POOL);
			ut_ad(insert_planned);
			ut_ad(cursor->thr);
			if (ibuf_insert(tuple, dict_index, space, zip_size, page_no, cursor->thr)) {
				// Insertion to the insert buffer succeeded
				cursor->flag = BTR_CUR_INSERT_TO_IBUF;
				if (IB_LIKELY_NULL(heap)) {
					IB_MEM_HEAP_FREE(heap);
				}
				goto func_exit;
			}
			// Insert to the insert buffer did not succeed: retry page get
			buf_mode = BUF_GET;
			goto retry_page_get;
		}
		page_t* page = buf_block_get_frame(block);
		block->check_index_page_at_flush = TRUE;
		if (rw_latch != RW_NO_LATCH) {
#ifdef IB_ZIP_DEBUG
			const page_zip_des_t* page_zip = buf_block_get_page_zip(block);
			ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
			buf_block_dbg_add_level(block, SYNC_TREE_NODE);
		}
		ut_ad(0 == ut_dulint_cmp(dict_index->id, btr_page_get_index_id(page)));
		if (IB_UNLIKELY(height == ULINT_UNDEFINED)) {
			// We are in the root node
			height = btr_page_get_level(page, mtr);
			root_height = height;
			cursor->tree_height = root_height + 1;
#ifdef BTR_CUR_ADAPT
			if (block != guess) {
				info->root_guess = block;
			}
#endif
		}
		if (height == 0) {
			if (rw_latch == RW_NO_LATCH) {
				btr_cur_latch_leaves(page, space, zip_size, page_no, latch_mode, cursor, mtr);
			}
			if ((latch_mode != BTR_MODIFY_TREE)
			    && (latch_mode != BTR_CONT_MODIFY_TREE)) {
				// Release the tree s-latch
				mtr_release_s_latch_at_savepoint(mtr, savepoint, dict_index_get_lock(dict_index));
			}
			page_mode = mode;
		}
		page_cur_search_with_match(block, dict_index, tuple, page_mode, &up_match, &up_bytes, &low_match, &low_bytes, page_cursor);
		if (estimate) {
			btr_cur_add_path_info(cursor, height, root_height);
		}
		// If this is the desired level, leave the loop
		ut_ad(height == btr_page_get_level(page_cur_get_page(page_cursor), mtr));
		if (level == height) {
			if (level > 0) {
				// x-latch the page
				page = btr_page_get(space, zip_size, page_no, RW_X_LATCH, mtr);
				ut_a((ibool)!!page_is_comp(page) == dict_table_is_comp(dict_index->table));
			}
			break;
		}
		ut_ad(height > 0);
		height--;
		guess = NULL;
		rec_t* node_ptr = page_cur_get_rec(page_cursor);
		offsets = rec_get_offsets(node_ptr, cursor->index, offsets, ULINT_UNDEFINED, &heap);
		// Go to the child node
		page_no = btr_node_ptr_get_child_page_no(node_ptr, offsets);
	}
	if (IB_LIKELY_NULL(heap)) {
		IB_MEM_HEAP_FREE(heap);
	}
	if (level == 0) {
		cursor->low_match = low_match;
		cursor->low_bytes = low_bytes;
		cursor->up_match = up_match;
		cursor->up_bytes = up_bytes;
#ifdef BTR_CUR_ADAPT
		// We do a dirty read of btr_search_enabled here. We will properly check btr_search_enabled again in btr_search_build_page_hash_index() before building a page hash index, while holding btr_search_latch.
		if (IB_LIKELY(btr_search_enabled)) {
			btr_search_info_update(dict_index, cursor);
		}
#endif
		ut_ad(cursor->up_match != ULINT_UNDEFINED || mode != PAGE_CUR_GE);
		ut_ad(cursor->up_match != ULINT_UNDEFINED || mode != PAGE_CUR_LE);
		ut_ad(cursor->low_match != ULINT_UNDEFINED || mode != PAGE_CUR_LE);
	}
func_exit:
	if (has_search_latch) {
		rw_lock_s_lock(&btr_search_latch);
	}
}

/// \brief Opens a cursor at either end of an index.
/// \param [in] from_left TRUE if open to the low end, FALSE if to the high end
/// \param [in] dict_index index
/// \param [in] latch_mode latch mode
/// \param [in] cursor cursor
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_cur_open_at_index_side_func(ibool from_left, dict_index_t* dict_index, ulint latch_mode, btr_cur_t* cursor, const char* file, ulint line, mtr_t* mtr)
{
    ulint root_height = 0; // remove warning
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    ulint estimate = latch_mode & BTR_ESTIMATE;
    latch_mode = latch_mode & ~BTR_ESTIMATE;
    // Store the position of the tree latch we push to mtr so that we know how to release it when we have latched the leaf node
    ulint savepoint = mtr_set_savepoint(mtr);
    if (latch_mode == BTR_MODIFY_TREE) {
        mtr_x_lock(dict_index_get_lock(dict_index), mtr);
    } else {
        mtr_s_lock(dict_index_get_lock(dict_index), mtr);
    }
    page_cur_t* page_cursor = btr_cur_get_page_cur(cursor);
    cursor->index = dict_index;
    ulint space = dict_index_get_space(dict_index);
    ulint zip_size = dict_table_zip_size(dict_index->table);
    ulint page_no = dict_index_get_page(dict_index);
    ulint height = ULINT_UNDEFINED;
    for (;;) {
        buf_block_t* block = buf_page_get_gen(space, zip_size, page_no, RW_NO_LATCH, NULL, BUF_GET, file, line, mtr);
        page_t* page = buf_block_get_frame(block);
        ut_ad(0 == ut_dulint_cmp(dict_index->id, btr_page_get_index_id(page)));
        block->check_index_page_at_flush = TRUE;
        if (height == ULINT_UNDEFINED) {
            // We are in the root node
            height = btr_page_get_level(page, mtr);
            root_height = height;
        }
        if (height == 0) {
            btr_cur_latch_leaves(page, space, zip_size, page_no, latch_mode, cursor, mtr);
            // In versions <= 3.23.52 we had forgotten to release the tree latch here. If in an index scan we had to scan far to find a record visible to the current transaction, that could starve others waiting for the tree latch.
            if ((latch_mode != BTR_MODIFY_TREE) && (latch_mode != BTR_CONT_MODIFY_TREE)) {
                // Release the tree s-latch
                mtr_release_s_latch_at_savepoint(mtr, savepoint, dict_index_get_lock(dict_index));
            }
        }
        if (from_left) {
            page_cur_set_before_first(block, page_cursor);
        } else {
            page_cur_set_after_last(block, page_cursor);
        }
        if (height == 0) {
            if (estimate) {
                btr_cur_add_path_info(cursor, height, root_height);
            }
            break;
        }
        ut_ad(height > 0);
        if (from_left) {
            page_cur_move_to_next(page_cursor);
        } else {
            page_cur_move_to_prev(page_cursor);
        }
        if (estimate) {
            btr_cur_add_path_info(cursor, height, root_height);
        }
        height--;
        rec_t* node_ptr = page_cur_get_rec(page_cursor);
        offsets = rec_get_offsets(node_ptr, cursor->index, offsets, ULINT_UNDEFINED, &heap);
        // Go to the child node
        page_no = btr_node_ptr_get_child_page_no(node_ptr, offsets);
    }
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
}

/// \brief Positions a cursor at a randomly chosen position within a B-tree.
/// \param [in] dict_index index
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...
/// \param [in/out] cursor B-tree cursor
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_cur_open_at_rnd_pos_func(dict_index_t* dict_index, ulint latch_mode, btr_cur_t* cursor, const char* file, ulint line, mtr_t* mtr)
{
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    if (latch_mode == BTR_MODIFY_TREE) {
        mtr_x_lock(dict_index_get_lock(dict_index), mtr);
    } else {
        mtr_s_lock(dict_index_get_lock(dict_index), mtr);
    }
    page_cur_t* page_cursor = btr_cur_get_page_cur(cursor);
    cursor->index = dict_index;
    ulint space = dict_index_get_space(dict_index);
    ulint zip_size = dict_table_zip_size(dict_index->table);
    ulint page_no = dict_index_get_page(dict_index);
    ulint height = ULINT_UNDEFINED;
    for (;;) {
        buf_block_t* block = buf_page_get_gen(space, zip_size, page_no, RW_NO_LATCH, NULL, BUF_GET, file, line, mtr);
        page_t* page = buf_block_get_frame(block);
        ut_ad(0 == ut_dulint_cmp(dict_index->id, btr_page_get_index_id(page)));
        if (height == ULINT_UNDEFINED) {
            // We are in the root node
            height = btr_page_get_level(page, mtr);
        }
        if (height == 0) {
            btr_cur_latch_leaves(page, space, zip_size, page_no, latch_mode, cursor, mtr);
        }
        page_cur_open_on_rnd_user_rec(block, page_cursor);
        if (height == 0) {
            break;
        }
        ut_ad(height > 0);
        height--;
        rec_t* node_ptr = page_cur_get_rec(page_cursor);
        offsets = rec_get_offsets(node_ptr, cursor->index, offsets, ULINT_UNDEFINED, &heap);
        // Go to the child node
        page_no = btr_node_ptr_get_child_page_no(node_ptr, offsets);
    }
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
}

//==================== B-TREE INSERT =========================

/// \brief Inserts a record if there is enough space, or if enough space can be freed by reorganizing. 
/// \details Differs from btr_cur_optimistic_insert because no heuristics is applied to whether it pays to use CPU time for reorganizing the page or not.
/// \param [in] cursor cursor on page after which to insert; cursor stays valid
/// \param [in] tuple tuple to insert; the size info need not have been stored to tuple
/// \param [in] n_ext number of externally stored columns
/// \param [in] mtr mtr
/// \return pointer to inserted record if succeed, else NULL
static rec_t* btr_cur_insert_if_possible(btr_cur_t* cursor, const dtuple_t* tuple, ulint n_ext, mtr_t* mtr)
{
    ut_ad(dtuple_check_typed(tuple));
    buf_block_t* block = btr_cur_get_block(cursor);
    ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
    page_cur_t* page_cursor = btr_cur_get_page_cur(cursor);
    // Now, try the insert
    rec_t* rec = page_cur_tuple_insert(page_cursor, tuple, cursor->index, n_ext, mtr);
    if (IB_UNLIKELY(!rec)) {
        // If record did not fit, reorganize
        if (btr_page_reorganize(block, cursor->index, mtr)) {
            page_cur_search(block, cursor->index, tuple, PAGE_CUR_LE, page_cursor);
            rec = page_cur_tuple_insert(page_cursor, tuple, cursor->index, n_ext, mtr);
        }
    }
    return rec;
}

/// \brief For an insert, checks the locks and does the undo logging if desired.
/// \param [in] flags undo logging and locking flags: if not zero, the parameters index and thr should be specified
/// \param [in] cursor cursor on page after which to insert
/// \param [in] entry entry to insert
/// \param [in] thr query thread or NULL
/// \param [in/out] mtr mini-transaction
/// \param [out] inherit TRUE if the inserted new record maybe should inherit LOCK_GAP type locks from the successor record
/// \return DB_SUCCESS, DB_WAIT_LOCK, DB_FAIL, or error number
IB_INLINE ulint btr_cur_ins_lock_and_undo(ulint flags, btr_cur_t* cursor, const dtuple_t* entry, que_thr_t* thr, mtr_t* mtr, ibool* inherit)
{
    // Check if we have to wait for a lock: enqueue an explicit lock request if yes
    rec_t* rec = btr_cur_get_rec(cursor);
    dict_index_t* dict_index = cursor->index;
    ulint err = lock_rec_insert_check_and_lock(flags, rec, btr_cur_get_block(cursor), dict_index, thr, mtr, inherit);
    if (err != DB_SUCCESS) {
        return err;
    }
    if (dict_index_is_clust(dict_index) && !dict_index_is_ibuf(dict_index)) {
        roll_ptr_t roll_ptr;
        err = trx_undo_report_row_operation(flags, TRX_UNDO_INSERT_OP, thr, dict_index, entry, NULL, 0, NULL, &roll_ptr);
        if (err != DB_SUCCESS) {
            return err;
        }
        // Now we can fill in the roll ptr field in entry
        if (!(flags & BTR_KEEP_SYS_FLAG)) {
            row_upd_index_entry_sys_field(entry, dict_index, DATA_ROLL_PTR, roll_ptr);
        }
    }
    return DB_SUCCESS;
}

#ifdef IB_DEBUG

/// \brief Report information about a transaction.
/// \param [in] trx transaction
/// \param [in] index index
/// \param [in] op operation
static void btr_cur_trx_report(trx_t* trx, const dict_index_t* index, const char* op)
{
	ib_log(state, "Trx with id " TRX_ID_FMT " going to ",
		TRX_ID_PREP_PRINTF(trx->id));
	ib_log(state, op);
	dict_index_name_print(state->stream, trx, index);
	ib_log(state, "\n");
}

#endif // IB_DEBUG

/// \brief Tries to perform an insert to a page in an index tree, next to cursor. 
/// \details It is assumed that mtr holds an x-latch on the page. The operation does not succeed if there is too little space on the page.
//  If there is just one record on the page, the insert will always succeed; this is to prevent trying to split a page with just one record.
/// \param [in] flags undo logging and locking flags: if not zero, the parameters index and thr should be specified
/// \param [in] cursor cursor on page after which to insert; cursor stays valid
/// \param [in,out] entry entry to insert
/// \param [out] rec pointer to inserted record if succeed
/// \param [out] big_rec big rec vector whose fields have to be stored externally by the caller, or NULL
/// \param [in] n_ext number of externally stored columns
/// \param [in] thr query thread or NULL
/// \param [in] mtr mtr; if this function returns DB_SUCCESS on a leaf page of a secondary index in a compressed tablespace, the mtr must be committed before latching any further pages
/// \return DB_SUCCESS, DB_WAIT_LOCK, DB_FAIL, or error number
IB_INTERN ulint btr_cur_optimistic_insert(ulint flags, btr_cur_t* cursor, dtuple_t* entry, rec_t** rec, big_rec_t** big_rec, ulint n_ext, que_thr_t* thr, mtr_t* mtr)
{
    big_rec_t* big_rec_vec = NULL;
    *big_rec = NULL;
    buf_block_t* block = btr_cur_get_block(cursor);
    page_t* page = buf_block_get_frame(block);
    dict_index_t* dict_index = cursor->index;
    ulint zip_size = buf_block_get_zip_size(block);
#ifdef IB_DEBUG_VALGRIND
    if (zip_size) {
        IB_MEM_ASSERT_RW(page, IB_PAGE_SIZE);
        IB_MEM_ASSERT_RW(block->page.zip.data, zip_size);
    }
#endif // IB_DEBUG_VALGRIND
    if (!dtuple_check_typed_no_assert(entry)) {
        ib_log(state, "InnoDB: Error in a tuple to insert into ");
        dict_index_name_print(state->stream, thr_get_trx(thr), dict_index);
    }
#ifdef IB_DEBUG
    if (btr_cur_print_record_ops && thr) {
        btr_cur_trx_report(thr_get_trx(thr), dict_index, "insert into ");
        dtuple_print(state->stream, entry);
    }
#endif // IB_DEBUG
    ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
    ulint max_size = page_get_max_insert_size_after_reorganize(page, 1);
    ibool leaf = page_is_leaf(page);
    // Calculate the record size when entry is converted to a record
    ulint rec_size = rec_get_converted_size(dict_index, entry, n_ext);
    if (page_rec_needs_ext(rec_size, page_is_comp(page), dtuple_get_n_fields(entry), zip_size)) {
        // The record is so big that we have to store some fields externally on separate database pages
        big_rec_vec = dtuple_convert_big_rec(dict_index, entry, &n_ext);
        if (IB_UNLIKELY(big_rec_vec == NULL)) {
            return DB_TOO_BIG_RECORD;
        }
        rec_size = rec_get_converted_size(dict_index, entry, n_ext);
    }
    if (IB_UNLIKELY(zip_size)) {
        // Estimate the free space of an empty compressed page. Subtract one byte for the encoded heap_no in the modification log.
        ulint free_space_zip = page_zip_empty_size(cursor->index->n_fields, zip_size) - 1;
        ulint n_uniq = dict_index_get_n_unique_in_tree(dict_index);
        ut_ad(dict_table_is_comp(dict_index->table));
        // There should be enough room for two node pointer records on an empty non-leaf page. This prevents infinite page splits.
        if (IB_LIKELY(entry->n_fields >= n_uniq) && IB_UNLIKELY(REC_NODE_PTR_SIZE + rec_get_converted_size_comp_prefix(dict_index, entry->fields, n_uniq, NULL) // On a compressed page, there is a two-byte entry in the dense page directory for every record. But there is no record header. - (REC_N_NEW_EXTRA_BYTES - 2) > free_space_zip / 2)) {
            if (big_rec_vec) {
                dtuple_convert_back_big_rec(dict_index, entry, big_rec_vec);
            }
            return DB_TOO_BIG_RECORD;
        }
    }
    // If there have been many consecutive inserts, and we are on the leaf level, check if we have to split the page to reserve enough free space for future updates of records.
    rec_t* dummy_rec;
    if (dict_index_is_clust(dict_index) && (page_get_n_recs(page) >= 2) && IB_LIKELY(leaf) && (dict_index_get_space_reserve() + rec_size > max_size) && (btr_page_get_split_rec_to_right(cursor, &dummy_rec) || btr_page_get_split_rec_to_left(cursor, &dummy_rec))) {
fail:
        ulint err = DB_FAIL;
fail_err:
        if (big_rec_vec) {
            dtuple_convert_back_big_rec(dict_index, entry, big_rec_vec);
        }
        return err;
    }
    if (IB_UNLIKELY(max_size < BTR_CUR_PAGE_REORGANIZE_LIMIT || max_size < rec_size) && IB_LIKELY(page_get_n_recs(page) > 1) && page_get_max_insert_size(page, 1) < rec_size) {
        goto fail;
    }
    // Check locks and write to the undo log, if specified
    ibool inherit;
    ulint err = btr_cur_ins_lock_and_undo(flags, cursor, entry, thr, mtr, &inherit);
    if (IB_UNLIKELY(err != DB_SUCCESS)) {
        goto fail_err;
    }
    page_cur_t* page_cursor = btr_cur_get_page_cur(cursor);
    // Now, try the insert
    {
        const rec_t* page_cursor_rec = page_cur_get_rec(page_cursor);
        *rec = page_cur_tuple_insert(page_cursor, entry, dict_index, n_ext, mtr);
        ibool reorg = page_cursor_rec != page_cur_get_rec(page_cursor);
        if (IB_UNLIKELY(reorg)) {
            ut_a(zip_size);
            ut_a(*rec);
        }
    }
    if (IB_UNLIKELY(!*rec) && IB_LIKELY(!reorg)) {
        // If the record did not fit, reorganize
        if (IB_UNLIKELY(!btr_page_reorganize(block, dict_index, mtr))) {
            ut_a(zip_size);
            goto fail;
        }
        ut_ad(zip_size || page_get_max_insert_size(page, 1) == max_size);
        reorg = TRUE;
        page_cur_search(block, dict_index, entry, PAGE_CUR_LE, page_cursor);
        *rec = page_cur_tuple_insert(page_cursor, entry, dict_index, n_ext, mtr);
        if (IB_UNLIKELY(!*rec)) {
            if (IB_LIKELY(zip_size != 0)) {
                goto fail;
            }
            ib_log(state, "InnoDB: Error: cannot insert tuple ");
            dtuple_print(state->stream, entry);
            ib_log(state, " into ");
            dict_index_name_print(state->stream, thr_get_trx(thr), dict_index);
            ib_log(state, "\nInnoDB: max insert size %lu\n", (ulong) max_size);
            UT_ERROR;
        }
    }
#ifdef BTR_CUR_HASH_ADAPT
    if (!reorg && leaf && (cursor->flag == BTR_CUR_HASH)) {
        btr_search_update_hash_node_on_insert(cursor);
    } else {
        btr_search_update_hash_on_insert(cursor);
    }
#endif
    if (!(flags & BTR_NO_LOCKING_FLAG) && inherit) {
        lock_update_insert(block, *rec);
    }
#if 0
    ib_log(state, "Insert into page %lu, max ins size %lu, rec %lu ind type %lu\n", buf_block_get_page_no(block), max_size, rec_size + PAGE_DIR_SLOT_SIZE, dict_index->type);
#endif
    if (leaf && !dict_index_is_clust(dict_index)) {
        // Update the free bits of the B-tree page in the insert buffer bitmap.
        // The free bits in the insert buffer bitmap must never exceed the free space on a page. It is safe to decrement or reset the bits in the bitmap in a mini-transaction that is committed before the mini-transaction that affects the free space.
        // It is unsafe to increment the bits in a separately committed mini-transaction, because in crash recovery, the free bits could momentarily be set too high.
        if (zip_size) {
            // Update the bits in the same mini-transaction.
            ibuf_update_free_bits_zip(block, mtr);
        } else {
            // Decrement the bits in a separate mini-transaction.
            ibuf_update_free_bits_if_full(block, max_size, rec_size + PAGE_DIR_SLOT_SIZE);
        }
    }
    *big_rec = big_rec_vec;
    return DB_SUCCESS;
}

/// \brief Performs an insert on a page of an index tree.
/// \details It is assumed that mtr holds an x-latch on the tree and on the cursor page. If the insert is made on the leaf level, 
/// to avoid deadlocks, mtr must also own x-latches to brothers of page, if those brothers exist.
/// \param [in] flags undo logging and locking flags: if not zero, the parameter thr should be specified; if no undo logging is specified, then the caller must have reserved  enough free extents in the file space so that the insertion will certainly succeed
/// \param [in] cursor cursor after which to insert; cursor stays valid
/// \param [in,out] entry entry to insert
/// \param [out] rec pointer to inserted record if succeed
/// \param [out] big_rec big rec vector whose fields have to be stored externally by the caller, or NULL
/// \param [in] n_ext number of externally stored columns
/// \param [in] thr query thread or NULL
/// \param [in] mtr mtr
/// \return DB_SUCCESS or error number
IB_INTERN ulint btr_cur_pessimistic_insert(ulint flags, btr_cur_t* cursor, dtuple_t* entry, rec_t** rec, big_rec_t** big_rec, ulint n_ext, que_thr_t* thr, mtr_t* mtr)
{
    ut_ad(dtuple_check_typed(entry));
    *big_rec = NULL;
    dict_index_t* dict_index = cursor->index;
    ulint zip_size = dict_table_zip_size(dict_index->table);
    big_rec_t* big_rec_vec = NULL;
    mem_heap_t* heap = NULL;
    ulint n_extents = 0;
    ut_ad(mtr_memo_contains(mtr, dict_index_get_lock(btr_cur_get_index(cursor)), MTR_MEMO_X_LOCK));
    ut_ad(mtr_memo_contains(mtr, btr_cur_get_block(cursor), MTR_MEMO_PAGE_X_FIX));
    // Try first an optimistic insert; reset the cursor flag: we do not assume anything of how it was positioned
    cursor->flag = BTR_CUR_BINARY;
    ulint err = btr_cur_optimistic_insert(flags, cursor, entry, rec, big_rec, n_ext, thr, mtr);
    if (err != DB_FAIL) {
        return err;
    }
    // Retry with a pessimistic insert. Check locks and write to undo log, if specified
    ibool dummy_inh;
    err = btr_cur_ins_lock_and_undo(flags, cursor, entry, thr, mtr, &dummy_inh);
    if (err != DB_SUCCESS) {
        return err;
    }
    if (!(flags & BTR_NO_UNDO_LOG_FLAG)) {
        // First reserve enough free space for the file segments of the index tree, so that the insert will not fail because of lack of space
        n_extents = cursor->tree_height / 16 + 3;
        ulint n_reserved;
        ibool success = fsp_reserve_free_extents(&n_reserved, dict_index->space, n_extents, FSP_NORMAL, mtr);
        if (!success) {
            return DB_OUT_OF_FILE_SPACE;
        }
    }
    if (page_rec_needs_ext(rec_get_converted_size(dict_index, entry, n_ext), dict_table_is_comp(dict_index->table), dict_index_get_n_fields(dict_index), zip_size)) {
        // The record is so big that we have to store some fields externally on separate database pages
        if (IB_LIKELY_NULL(big_rec_vec)) {
            // This should never happen, but we handle the situation in a robust manner.
            ut_ad(0);
            dtuple_convert_back_big_rec(dict_index, entry, big_rec_vec);
        }
        big_rec_vec = dtuple_convert_big_rec(dict_index, entry, &n_ext);
        if (big_rec_vec == NULL) {
            if (n_extents > 0) {
                fil_space_release_free_extents(dict_index->space, n_reserved);
            }
            return DB_TOO_BIG_RECORD;
        }
    }
    if (dict_index_get_page(dict_index) == buf_block_get_page_no(btr_cur_get_block(cursor))) {
        // The page is the root page
        *rec = btr_root_raise_and_insert(cursor, entry, n_ext, mtr);
    } else {
        *rec = btr_page_split_and_insert(cursor, entry, n_ext, mtr);
    }
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
    ut_ad(page_rec_get_next(btr_cur_get_rec(cursor)) == *rec);
#ifdef BTR_CUR_ADAPT
    btr_search_update_hash_on_insert(cursor);
#endif
    if (!(flags & BTR_NO_LOCKING_FLAG)) {
        lock_update_insert(btr_cur_get_block(cursor), *rec);
    }
    if (n_extents > 0) {
        fil_space_release_free_extents(dict_index->space, n_reserved);
    }
    *big_rec = big_rec_vec;
    return DB_SUCCESS;
}

//==================== B-TREE UPDATE =========================

/// \brief For an update, checks the locks and does the undo logging.
/// \param [in] flags undo logging and locking flags
/// \param [in] cursor cursor on record to update
/// \param [in] update update vector
/// \param [in] cmpl_info compiler info on secondary index updates
/// \param [in] thr query thread
/// \param [in/out] mtr mini-transaction
/// \param [out] roll_ptr roll pointer
/// \return DB_SUCCESS, DB_WAIT_LOCK, or error number
IB_INLINE ulint btr_cur_upd_lock_and_undo(ulint flags, btr_cur_t* cursor, const upd_t* update, ulint cmpl_info, que_thr_t* thr, mtr_t* mtr, roll_ptr_t* roll_ptr)
{
    ut_ad(cursor && update && thr && roll_ptr);
    rec_t* rec = btr_cur_get_rec(cursor);
    dict_index_t* dict_index = cursor->index;
    if (!dict_index_is_clust(dict_index)) {
        // We do undo logging only when we update a clustered index record
        return lock_sec_rec_modify_check_and_lock(flags, btr_cur_get_block(cursor), rec, dict_index, thr, mtr);
    }
    // Check if we have to wait for a lock: enqueue an explicit lock request if yes
    ulint err = DB_SUCCESS;
    if (!(flags & BTR_NO_LOCKING_FLAG)) {
        mem_heap_t* heap = NULL;
        ulint offsets_[REC_OFFS_NORMAL_SIZE];
        rec_offs_init(offsets_);
        err = lock_clust_rec_modify_check_and_lock(flags, btr_cur_get_block(cursor), rec, dict_index, rec_get_offsets(rec, dict_index, offsets_, ULINT_UNDEFINED, &heap), thr);
        if (IB_LIKELY_NULL(heap)) {
            IB_MEM_HEAP_FREE(heap);
        }
        if (err != DB_SUCCESS) {
            return err;
        }
    }
    // Append the info about the update in the undo log
    err = trx_undo_report_row_operation(flags, TRX_UNDO_MODIFY_OP, thr, dict_index, NULL, update, cmpl_info, rec, roll_ptr);
    return err;
}

/// \brief Writes a redo log record of updating a record in-place.
/// \param [in] flags flags
/// \param [in] rec record
/// \param [in] dict_index index where cursor positioned
/// \param [in] update update vector
/// \param [in] trx transaction
/// \param [in] roll_ptr roll ptr
/// \param [in] mtr mtr
IB_INLINE void btr_cur_update_in_place_log(ulint flags, rec_t* rec, dict_index_t* dict_index, const upd_t* update, trx_t* trx, roll_ptr_t roll_ptr, mtr_t* mtr)
{
    ut_ad(flags < 256);
    page_t* page = page_align(rec);
    ut_ad(!!page_is_comp(page) == dict_table_is_comp(dict_index->table));
    byte* log_ptr = mlog_open_and_write_index(mtr, rec, dict_index, page_is_comp(page) ? MLOG_COMP_REC_UPDATE_IN_PLACE : MLOG_REC_UPDATE_IN_PLACE, 1 + DATA_ROLL_PTR_LEN + 14 + 2 + MLOG_BUF_MARGIN);
    if (!log_ptr) {
        // Logging in mtr is switched off during crash recovery
        return;
    }
    // The code below assumes index is a clustered index: change index to the clustered index if we are updating a secondary index record (or we could as well skip writing the 
    // sys col values to the log in this case because they are not needed for a secondary index record update)
    dict_index = dict_table_get_first_index(dict_index->table);
    mach_write_to_1(log_ptr, flags);
    log_ptr++;
    log_ptr = row_upd_write_sys_vals_to_log(dict_index, trx, roll_ptr, log_ptr, mtr);
    mach_write_to_2(log_ptr, page_offset(rec));
    log_ptr += 2;
    row_upd_index_write_log(update, log_ptr, mtr);
}
#endif // IB_HOTBACKUP

/// \brief Parses a redo log record of updating a record in-place.
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in/out] page page or NULL
/// \param [in/out] page_zip compressed page, or NULL
/// \param [in] dict_index index corresponding to page
/// \return end of log record or NULL
IB_INTERN byte* btr_cur_parse_update_in_place(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip, dict_index_t* dict_index)
{
    if (end_ptr < ptr + 1) {
        return NULL;
    }
    ulint flags = mach_read_from_1(ptr);
    ptr++;
    ulint pos;
    trx_id_t trx_id;
    roll_ptr_t roll_ptr;
    ptr = row_upd_parse_sys_vals(ptr, end_ptr, &pos, &trx_id, &roll_ptr);
    if (ptr == NULL) {
        return NULL;
    }
    if (end_ptr < ptr + 2) {
        return NULL;
    }
    ulint rec_offset = mach_read_from_2(ptr);
    ptr += 2;
    ut_a(rec_offset <= IB_PAGE_SIZE);
    mem_heap_t* heap = IB_MEM_HEAP_CREATE(256);
    upd_t* update;
    ptr = row_upd_index_parse(ptr, end_ptr, heap, &update);
    if (!ptr || !page) {
        goto func_exit;
    }
    ut_a((ibool)!!page_is_comp(page) == dict_table_is_comp(dict_index->table));
    rec_t* rec = page + rec_offset;
    // We do not need to reserve btr_search_latch, as the page is only being recovered, and there cannot be a hash index to it.
    ulint* offsets = rec_get_offsets(rec, dict_index, NULL, ULINT_UNDEFINED, &heap);
    if (!(flags & BTR_KEEP_SYS_FLAG)) {
        row_upd_rec_sys_fields_in_recovery(rec, page_zip, offsets, pos, trx_id, roll_ptr);
    }
    row_upd_rec_in_place(rec, dict_index, offsets, update, page_zip);
func_exit:
    IB_MEM_HEAP_FREE(heap);
    return ptr;
}

#ifndef IB_HOTBACKUP

/// \brief See if there is enough place in the page modification log to log an update-in-place.
/// \param [in/out] page_zip compressed page
/// \param [in/out] block buffer page
/// \param [in] dict_index the index corresponding to the block
/// \param [in] length size needed
/// \param [in] create TRUE=delete-and-insert, FALSE=update-in-place
/// \param [in] mtr mini-transaction
/// \return TRUE if enough place
static ibool btr_cur_update_alloc_zip(page_zip_des_t* page_zip, buf_block_t* block, dict_index_t* dict_index, ulint length, ibool create, mtr_t* mtr)
{
    ut_a(page_zip == buf_block_get_page_zip(block));
    ut_ad(page_zip);
    ut_ad(!dict_index_is_ibuf(dict_index));
    if (page_zip_available(page_zip, dict_index_is_clust(dict_index), length, create)) {
        return TRUE;
    }
    if (!page_zip->m_nonempty) {
        // The page has been freshly compressed, so recompressing it will not help.
        return FALSE;
    }
    if (!page_zip_compress(page_zip, buf_block_get_frame(block), dict_index, mtr)) {
        // Unable to compress the page
        return FALSE;
    }
    // After recompressing a page, we must make sure that the free bits in the insert buffer bitmap will not exceed the free space on the page.
    // Because this function will not attempt recompression unless page_zip_available() fails above, it is safe to reset the free bits if page_zip_available() fails again, below.
    // The free bits can safely be reset in a separate mini-transaction. If page_zip_available() succeeds below, we can be sure that the page_zip_compress() above did not reduce the free space available on the page.
    if (!page_zip_available(page_zip, dict_index_is_clust(dict_index), length, create)) {
        // Out of space: reset the free bits.
        if (!dict_index_is_clust(dict_index) && page_is_leaf(buf_block_get_frame(block))) {
            ibuf_reset_free_bits(block);
        }
        return FALSE;
    }
    return TRUE;
}

/// \brief Updates a record when the update causes no size changes in its fields. We assume here that the ordering fields of the record do not change.
/// \param [in] flags undo logging and locking flags
/// \param [in] cursor cursor on the record to update; cursor stays valid and positioned on the same record
/// \param [in] update update vector
/// \param [in] cmpl_info compiler info on secondary index updates
/// \param [in] thr query thread
/// \param [in] mtr mtr; must be committed before latching any further pages
/// \return DB_SUCCESS or error number
IB_INTERN ulint btr_cur_update_in_place(ulint flags, btr_cur_t* cursor, const upd_t* update, ulint cmpl_info, que_thr_t* thr, mtr_t* mtr)
{
    ulint err;

    rec_t* rec = btr_cur_get_rec(cursor);
    dict_index_t* dict_index = cursor->index;
    ut_ad(!!page_rec_is_comp(rec) == dict_table_is_comp(dict_index->table));
    // The insert buffer tree should never be updated in place.
    ut_ad(!dict_index_is_ibuf(dict_index));
    trx_t* trx = thr_get_trx(thr);
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    offsets = rec_get_offsets(rec, dict_index, offsets, ULINT_UNDEFINED, &heap);
    roll_ptr_t roll_ptr = ut_dulint_zero;
    
#ifdef IB_DEBUG
    if (btr_cur_print_record_ops && thr) {
        btr_cur_trx_report(trx, dict_index, "update ");
        rec_print_new(state->stream, rec, offsets);
    }
#endif // IB_DEBUG
    buf_block_t* block = btr_cur_get_block(cursor);
    page_zip_des_t* page_zip = buf_block_get_page_zip(block);
    // Check that enough space is available on the compressed page.
    if (IB_LIKELY_NULL(page_zip) && !btr_cur_update_alloc_zip(page_zip, block, dict_index, rec_offs_size(offsets), FALSE, mtr)) {
        return DB_ZIP_OVERFLOW;
    }
    // Do lock checking and undo logging
    err = btr_cur_upd_lock_and_undo(flags, cursor, update, cmpl_info, thr, mtr, &roll_ptr);
    if (IB_UNLIKELY(err != DB_SUCCESS)) {
        if (IB_LIKELY_NULL(heap)) {
            IB_MEM_HEAP_FREE(heap);
        }
        return err;
    }
    if (block->is_hashed) {
        // The function row_upd_changes_ord_field_binary works only if the update vector was built for a clustered index, we must NOT call it if index is secondary
        if (!dict_index_is_clust(dict_index) || row_upd_changes_ord_field_binary(NULL, dict_index, update)) {
            // Remove possible hash index pointer to this record
            btr_search_update_hash_on_delete(cursor);
        }
        rw_lock_x_lock(&btr_search_latch);
    }
    if (!(flags & BTR_KEEP_SYS_FLAG)) {
        row_upd_rec_sys_fields(rec, NULL, dict_index, offsets, trx, roll_ptr);
    }
    ulint was_delete_marked = rec_get_deleted_flag(rec, page_is_comp(buf_block_get_frame(block)));
    row_upd_rec_in_place(rec, dict_index, offsets, update, page_zip);
    if (block->is_hashed) {
        rw_lock_x_unlock(&btr_search_latch);
    }
    if (page_zip && !dict_index_is_clust(dict_index) && page_is_leaf(buf_block_get_frame(block))) {
        // Update the free bits in the insert buffer.
        ibuf_update_free_bits_zip(block, mtr);
    }
    btr_cur_update_in_place_log(flags, rec, dict_index, update, trx, roll_ptr, mtr);
    if (was_delete_marked && !rec_get_deleted_flag(rec, page_is_comp(buf_block_get_frame(block)))) {
        // The new updated record owns its possible externally stored fields
        btr_cur_unmark_extern_fields(page_zip, rec, dict_index, offsets, mtr);
    }
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
    return DB_SUCCESS;
}

/// \brief Tries to update a record on a page in an index tree. It is assumed that mtr holds an x-latch on the page. The operation does not succeed if there is too little space
//  on the page or if the update would result in too empty a page, so that tree compression is recommended. We assume here that the ordering fields of the record do not change.
/// \param [in] flags undo logging and locking flags
/// \param [in] cursor cursor on the record to update; cursor stays valid and positioned on the same record
/// \param [in] update update vector; this must also contain trx id and roll ptr fields
/// \param [in] cmpl_info compiler info on secondary index updates
/// \param [in] thr query thread
/// \param [in] mtr mtr; must be committed before latching any further pages
/// \return DB_SUCCESS, or DB_OVERFLOW if the updated record does not fit, DB_UNDERFLOW if the page would become too empty, or DB_ZIP_OVERFLOW if there is not enough space left on the compressed page
IB_INTERN ulint btr_cur_optimistic_update(ulint flags, btr_cur_t* cursor, const upd_t* update, ulint cmpl_info, que_thr_t* thr, mtr_t* mtr)
{
    buf_block_t* block = btr_cur_get_block(cursor);
    page_t* page = buf_block_get_frame(block);
    rec_t* rec = btr_cur_get_rec(cursor);
    rec_t* orig_rec = rec;
    dict_index_t* index = cursor->index;
    ut_ad(!!page_rec_is_comp(rec) == dict_table_is_comp(index->table));
    ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
    // The insert buffer tree should never be updated in place.
    ut_ad(!dict_index_is_ibuf(index));
    mem_heap_t* heap = IB_MEM_HEAP_CREATE(1024);
    ulint* offsets = rec_get_offsets(rec, index, NULL, ULINT_UNDEFINED, &heap);
#ifdef IB_DEBUG
    if (btr_cur_print_record_ops && thr) {
        btr_cur_trx_report(thr_get_trx(thr), index, "update ");
        rec_print_new(state->stream, rec, offsets);
    }
#endif // IB_DEBUG
    if (!row_upd_changes_field_size_or_external(index, offsets, update)) {
        // The simplest and the most common case: the update does not change the size of any field and none of the updated fields is externally stored in rec or update, and there is enough space on the compressed page to log the update.
        IB_MEM_HEAP_FREE(heap);
        return btr_cur_update_in_place(flags, cursor, update, cmpl_info, thr, mtr);
    }
    if (rec_offs_any_extern(offsets)) {
any_extern:
        // Externally stored fields are treated in pessimistic update
        IB_MEM_HEAP_FREE(heap);
        return DB_OVERFLOW;
    }
    for (ulint i = 0; i < upd_get_n_fields(update); i++) {
        if (dfield_is_ext(&upd_get_nth_field(update, i)->new_val)) {
            goto any_extern;
        }
    }
    page_cur_t* page_cursor = btr_cur_get_page_cur(cursor);
    ulint n_ext;
    dtuple_t* new_entry = row_rec_to_index_entry(ROW_COPY_DATA, rec, index, offsets, &n_ext, heap);
    // We checked above that there are no externally stored fields.
    ut_a(!n_ext);
    // The page containing the clustered index record corresponding to new_entry is latched in mtr. Thus the following call is safe.
    row_upd_index_replace_new_col_vals_index_pos(new_entry, index, update, FALSE, heap);
    ulint old_rec_size = rec_offs_size(offsets);
    ulint new_rec_size = rec_get_converted_size(index, new_entry, 0);
    page_zip_des_t* page_zip = buf_block_get_page_zip(block);
#ifdef IB_ZIP_DEBUG
    ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
    if (IB_LIKELY_NULL(page_zip) && !btr_cur_update_alloc_zip(page_zip, block, index, new_rec_size, TRUE, mtr)) {
        err = DB_ZIP_OVERFLOW;
        goto err_exit;
    }
    if (IB_UNLIKELY(new_rec_size >= (page_get_free_space_of_empty(page_is_comp(page)) / 2))) {
        err = DB_OVERFLOW;
        goto err_exit;
    }
    if (IB_UNLIKELY(page_get_data_size(page) - old_rec_size + new_rec_size < BTR_CUR_PAGE_COMPRESS_LIMIT)) {
        // The page would become too empty
        err = DB_UNDERFLOW;
        goto err_exit;
    }
    ulint max_size = old_rec_size + page_get_max_insert_size_after_reorganize(page, 1);
    if (!(((max_size >= BTR_CUR_PAGE_REORGANIZE_LIMIT) && (max_size >= new_rec_size)) || (page_get_n_recs(page) <= 1))) {
        // There was not enough space, or it did not pay to reorganize: for simplicity, we decide what to do assuming a reorganization is needed, though it might not be necessary
        err = DB_OVERFLOW;
        goto err_exit;
    }
    // Do lock checking and undo logging
    roll_ptr_t roll_ptr;
    ulint err = btr_cur_upd_lock_and_undo(flags, cursor, update, cmpl_info, thr, mtr, &roll_ptr);
    if (err != DB_SUCCESS) {
err_exit:
        IB_MEM_HEAP_FREE(heap);
        return err;
    }
    // Ok, we may do the replacement. Store on the page infimum the explicit locks on rec, before deleting rec (see the comment in btr_cur_pessimistic_update).
    lock_rec_store_on_page_infimum(block, rec);
    btr_search_update_hash_on_delete(cursor);
    // The call to row_rec_to_index_entry(ROW_COPY_DATA, ...) above invokes rec_offs_make_valid() to point to the copied record that the fields of new_entry point to. We have to undo it here.
    ut_ad(rec_offs_validate(NULL, index, offsets));
    rec_offs_make_valid(page_cur_get_rec(page_cursor), index, offsets);
    page_cur_delete_rec(page_cursor, index, offsets, mtr);
    page_cur_move_to_prev(page_cursor);
    trx_t* trx = thr_get_trx(thr);
    if (!(flags & BTR_KEEP_SYS_FLAG)) {
        row_upd_index_entry_sys_field(new_entry, index, DATA_ROLL_PTR, roll_ptr);
        row_upd_index_entry_sys_field(new_entry, index, DATA_TRX_ID, trx->id);
    }
    // There are no externally stored columns in new_entry
    rec = btr_cur_insert_if_possible(cursor, new_entry, 0/*n_ext*/, mtr);
    ut_a(rec); // <- We calculated above the insert would fit
    if (page_zip && !dict_index_is_clust(index) && page_is_leaf(page)) {
        // Update the free bits in the insert buffer.
        ibuf_update_free_bits_zip(block, mtr);
    }
    // Restore the old explicit lock state on the record
    lock_rec_restore_from_page_infimum(block, rec, block);
    page_cur_move_to_next(page_cursor);
    IB_MEM_HEAP_FREE(heap);
    return DB_SUCCESS;
}


/// \brief If, in a split, a new supremum record was created as the predecessor of the updated record, the supremum record must inherit exactly the locks on the updated record.
/// \details In the split it may have inherited locks from the successor of the updated record, which is not correct. This function restores the right locks for the new supremum.
/// \param [in] block buffer block of rec
/// \param [in] rec updated record
/// \param [in] mtr mtr
static void btr_cur_pess_upd_restore_supremum(buf_block_t* block, const rec_t* rec, mtr_t* mtr)
{
	page_t* page = buf_block_get_frame(block);
	if (page_rec_get_next(page_get_infimum_rec(page)) != rec) {
		// Updated record is not the first user record on its page
		return;
	}
	ulint space = buf_block_get_space(block);
	ulint zip_size = buf_block_get_zip_size(block);
	ulint prev_page_no = btr_page_get_prev(page, mtr);
	ut_ad(prev_page_no != FIL_NULL);
	buf_block_t* prev_block = buf_page_get_with_no_latch(space, zip_size, prev_page_no, mtr);
#ifdef IB_BTR_DEBUG
	ut_a(btr_page_get_next(prev_block->frame, mtr) == page_get_page_no(page));
#endif // IB_BTR_DEBUG
	// We must already have an x-latch on prev_block!
	ut_ad(mtr_memo_contains(mtr, prev_block, MTR_MEMO_PAGE_X_FIX));
	lock_rec_reset_and_inherit_gap_locks(prev_block, block, PAGE_HEAP_NO_SUPREMUM, page_rec_get_heap_no(rec));
}

/// \brief Performs an update of a record on a page of a tree.
/// \details It is assumed that mtr holds an x-latch on the tree and on the cursor page. If the update is made on the leaf level, to avoid deadlocks, mtr must also
//  own x-latches to brothers of page, if those brothers exist. We assume here that the ordering fields of the record do not change.
/// \return DB_SUCCESS or error code
/// \param [in] flags undo logging, locking, and rollback flags
/// \param [in] cursor cursor on the record to update
/// \param [in,out] heap pointer to memory heap, or NULL
/// \param [out] big_rec big rec vector whose fields have to be stored externally by the caller, or NULL
/// \param [in] update update vector; this is allowed also contain trx id and roll ptr fields, but the values in update vector have no effect
/// \param [in] cmpl_info compiler info on secondary index updates
/// \param [in] thr query thread
/// \param [in] mtr mtr; must be committed before latching any further pages
IB_INTERN ulint btr_cur_pessimistic_update(ulint flags, btr_cur_t* cursor, mem_heap_t** heap, big_rec_t** big_rec, const upd_t* update, ulint cmpl_info, que_thr_t* thr, mtr_t* mtr)
{
	big_rec_t*	big_rec_vec	= NULL;
	ulint n_extents = 0;
	ulint* offsets = NULL;
	*big_rec = NULL;
	buf_block_t* block = btr_cur_get_block(cursor);
	page_t* page = buf_block_get_frame(block);
	page_zip_des_t* page_zip = buf_block_get_page_zip(block);
	rec_t* rec = btr_cur_get_rec(cursor);
	dict_index_t* index = cursor->index;
	ut_ad(mtr_memo_contains(mtr, dict_index_get_lock(index),
				MTR_MEMO_X_LOCK));
	ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
#ifdef IB_ZIP_DEBUG
	ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
	// The insert buffer tree should never be updated in place.
	ut_ad(!dict_index_is_ibuf(index));
	ulint optim_err = btr_cur_optimistic_update(flags, cursor, update,
					      cmpl_info, thr, mtr);
	switch (optim_err) {
	case DB_UNDERFLOW:
	case DB_OVERFLOW:
	case DB_ZIP_OVERFLOW:
		break;
	default:
		return optim_err;
	}
	// Do lock checking and undo logging
	roll_ptr_t roll_ptr;
	ulint err = btr_cur_upd_lock_and_undo(flags, cursor, update, cmpl_info, thr, mtr, &roll_ptr);
	if (err != DB_SUCCESS) {
		return err;
	}
	if (optim_err == DB_OVERFLOW) {
		ulint reserve_flag;
		// First reserve enough free space for the file segments of the index tree, so that the update will not fail because of lack of space
		n_extents = cursor->tree_height / 16 + 3;
		if (flags & BTR_NO_UNDO_LOG_FLAG) {
			reserve_flag = FSP_CLEANING;
		} else {
			reserve_flag = FSP_NORMAL;
		}
		ulint n_reserved;
		if (!fsp_reserve_free_extents(&n_reserved, index->space, n_extents, reserve_flag, mtr)) {
			return DB_OUT_OF_FILE_SPACE;
		}
	}
	if (!*heap) {
		*heap = IB_MEM_HEAP_CREATE(1024);
	}
	offsets = rec_get_offsets(rec, index, NULL, ULINT_UNDEFINED, heap);
	trx_t* trx = thr_get_trx(thr);
	ulint n_ext;
	dtuple_t* new_entry = row_rec_to_index_entry(ROW_COPY_DATA, rec, index, offsets, &n_ext, *heap);
	// The call to row_rec_to_index_entry(ROW_COPY_DATA, ...) above invokes rec_offs_make_valid() to point to the copied record that the fields of new_entry point to. 
    // We have to undo it here.
	ut_ad(rec_offs_validate(NULL, index, offsets));
	rec_offs_make_valid(rec, index, offsets);
	// The page containing the clustered index record corresponding to new_entry is latched in mtr. If the clustered index record is delete-marked, then its externally
    //  stored fields cannot have been purged yet, because then the purge would also have removed the clustered index record itself. Thus the following call is safe.
	row_upd_index_replace_new_col_vals_index_pos(new_entry, index, update, FALSE, *heap);
	if (!(flags & BTR_KEEP_SYS_FLAG)) {
		row_upd_index_entry_sys_field(new_entry, index, DATA_ROLL_PTR, roll_ptr);
		row_upd_index_entry_sys_field(new_entry, index, DATA_TRX_ID, trx->id);
	}
	if ((flags & BTR_NO_UNDO_LOG_FLAG) && rec_offs_any_extern(offsets)) {
		// We are in a transaction rollback undoing a row update: we must free possible externally stored fields which got new values in the update, if they are not 
        // inherited values. They can be inherited if we have updated the primary key to another value, and then update it back again.
		ut_ad(big_rec_vec == NULL);
		btr_rec_free_updated_extern_fields(index, rec, page_zip, offsets, update, trx_is_recv(trx) ? RB_RECOVERY : RB_NORMAL, mtr);
	}
	// We have to set appropriate extern storage bits in the new record to be inserted: we have to remember which fields were such
	ut_ad(!page_is_comp(page) || !rec_get_node_ptr_flag(rec));
	offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, heap);
	n_ext += btr_push_update_extern_fields(new_entry, update, *heap);
	if (IB_LIKELY_NULL(page_zip)) {
		ut_ad(page_is_comp(page));
		if (page_zip_rec_needs_ext(rec_get_converted_size(index, new_entry, n_ext), TRUE, dict_index_get_n_fields(index), page_zip_get_size(page_zip))) {
			goto make_external;
		}
	} else if (page_rec_needs_ext(rec_get_converted_size(index, new_entry, n_ext), page_is_comp(page), 0, 0)) {
make_external:
		big_rec_vec = dtuple_convert_big_rec(index, new_entry, &n_ext);
		if (IB_UNLIKELY(big_rec_vec == NULL)) {
			err = DB_TOO_BIG_RECORD;
			goto return_after_reservations;
		}
	}
	// Store state of explicit locks on rec on the page infimum record, before deleting rec. The page infimum acts as a dummy carrier of the locks, taking care also of 
    // lock releases, before we can move the locks back on the actual record. There is a special case: if we are inserting on the root page and the insert causes a call of
    //  btr_root_raise_and_insert. Therefore we cannot in the lock system delete the lock structs set on the root page even if the root page carries just node pointers.
	lock_rec_store_on_page_infimum(block, rec);
	btr_search_update_hash_on_delete(cursor);
#ifdef IB_ZIP_DEBUG
	ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
	page_cur_t* page_cursor = btr_cur_get_page_cur(cursor);
	page_cur_delete_rec(page_cursor, index, offsets, mtr);
	page_cur_move_to_prev(page_cursor);
	rec = btr_cur_insert_if_possible(cursor, new_entry, n_ext, mtr);
	if (rec) {
		lock_rec_restore_from_page_infimum(btr_cur_get_block(cursor), rec, block);
		offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, heap);
		if (!rec_get_deleted_flag(rec, rec_offs_comp(offsets))) {
			// The new inserted record owns its possible externally stored fields
			btr_cur_unmark_extern_fields(page_zip, rec, index, offsets, mtr);
		}
		btr_cur_compress_if_useful(cursor, mtr);
		if (page_zip && !dict_index_is_clust(index) && page_is_leaf(page)) {
			// Update the free bits in the insert buffer.
			ibuf_update_free_bits_zip(block, mtr);
		}
		err = DB_SUCCESS;
		goto return_after_reservations;
	} else {
		ut_a(optim_err != DB_UNDERFLOW);
		// Out of space: reset the free bits.
		if (!dict_index_is_clust(index) && page_is_leaf(page)) {
			ibuf_reset_free_bits(block);
		}
	}
	// Was the record to be updated positioned as the first user record on its page?
	ibool was_first = page_cur_is_before_first(page_cursor);
	// The first parameter means that no lock checking and undo logging is made in the insert
	big_rec_t* dummy_big_rec;
	err = btr_cur_pessimistic_insert(BTR_NO_UNDO_LOG_FLAG | BTR_NO_LOCKING_FLAG | BTR_KEEP_SYS_FLAG, cursor, new_entry, &rec, &dummy_big_rec, n_ext, NULL, mtr);
	ut_a(rec);
	ut_a(err == DB_SUCCESS);
	ut_a(dummy_big_rec == NULL);
	if (dict_index_is_sec_or_ibuf(index)) {
		// Update PAGE_MAX_TRX_ID in the index page header. It was not updated by btr_cur_pessimistic_insert() because of BTR_NO_LOCKING_FLAG.
		buf_block_t* rec_block;
		rec_block = btr_cur_get_block(cursor);
		page_update_max_trx_id(rec_block, buf_block_get_page_zip(rec_block), trx->id, mtr);
	}
	if (!rec_get_deleted_flag(rec, rec_offs_comp(offsets))) {
		// The new inserted record owns its possible externally stored fields
		buf_block_t* rec_block = btr_cur_get_block(cursor);
#ifdef IB_ZIP_DEBUG
		ut_a(!page_zip || page_zip_validate(page_zip, page));
		page = buf_block_get_frame(rec_block);
#endif // IB_ZIP_DEBUG
		page_zip = buf_block_get_page_zip(rec_block);
		offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, heap);
		btr_cur_unmark_extern_fields(page_zip, rec, index, offsets, mtr);
	}
	lock_rec_restore_from_page_infimum(btr_cur_get_block(cursor), rec, block);
	// If necessary, restore also the correct lock state for a new, preceding supremum record created in a page split. While the old record was nonexistent,
    //  the supremum might have inherited its locks from a wrong record.
	if (!was_first) {
		btr_cur_pess_upd_restore_supremum(btr_cur_get_block(cursor), rec, mtr);
	}
return_after_reservations:
#ifdef IB_ZIP_DEBUG
	ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
	if (n_extents > 0) {
		fil_space_release_free_extents(index->space, n_reserved);
	}
	*big_rec = big_rec_vec;
	return err;
}

/*==================== B-TREE DELETE MARK AND UNMARK ===============*/

/// \brief Writes the redo log record for delete marking or unmarking of an index record.
/// \param [in] flags flags
/// \param [in] rec record
/// \param [in] index index of the record
/// \param [in] val value to set
/// \param [in] trx deleting transaction
/// \param [in] roll_ptr roll ptr to the undo log record
/// \param [in] mtr mtr
IB_INLINE void btr_cur_del_mark_set_clust_rec_log(ulint flags, rec_t* rec, dict_index_t* index, ibool val, trx_t* trx, roll_ptr_t roll_ptr, mtr_t* mtr)
{
	ut_ad(flags < 256);
	ut_ad(val <= 1);
	ut_ad(!!page_rec_is_comp(rec) == dict_table_is_comp(index->table));
	byte* log_ptr = mlog_open_and_write_index(mtr, rec, index, page_rec_is_comp(rec) ? MLOG_COMP_REC_CLUST_DELETE_MARK : MLOG_REC_CLUST_DELETE_MARK, 1 + 1 + DATA_ROLL_PTR_LEN + 14 + 2);
	if (!log_ptr) {
		// Logging in mtr is switched off during crash recovery
		return;
	}
	mach_write_to_1(log_ptr, flags);
	log_ptr++;
	mach_write_to_1(log_ptr, val);
	log_ptr++;
	log_ptr = row_upd_write_sys_vals_to_log(index, trx, roll_ptr, log_ptr, mtr);
	mach_write_to_2(log_ptr, page_offset(rec));
	log_ptr += 2;
	mlog_close(mtr, log_ptr);
}
#endif // !IB_HOTBACKUP

/// \brief Parses the redo log record for delete marking or unmarking of a clustered index record.
/// \return end of log record or NULL
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in/out] page page or NULL
/// \param [in/out] page_zip compressed page, or NULL
/// \param [in] index index corresponding to page
IB_INTERN byte* btr_cur_parse_del_mark_set_clust_rec(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip, dict_index_t* index)
{
	ut_ad(!page || !!page_is_comp(page) == dict_table_is_comp(index->table));
	if (end_ptr < ptr + 2) {
		return NULL;
	}
	ulint flags = mach_read_from_1(ptr);
	ptr++;
	ulint val = mach_read_from_1(ptr);
	ptr++;
	ulint pos;
	trx_id_t trx_id;
	roll_ptr_t roll_ptr;
	ptr = row_upd_parse_sys_vals(ptr, end_ptr, &pos, &trx_id, &roll_ptr);
	if (ptr == NULL) {
		return NULL;
	}
	if (end_ptr < ptr + 2) {
		return NULL;
	}
	ulint offset = mach_read_from_2(ptr);
	ptr += 2;
	ut_a(offset <= IB_PAGE_SIZE);
	if (page) {
		rec_t* rec = page + offset;
		// We do not need to reserve btr_search_latch, as the page is only being recovered, and there cannot be a hash index to it.
		btr_rec_set_deleted_flag(rec, page_zip, val);
		if (!(flags & BTR_KEEP_SYS_FLAG)) {
			mem_heap_t* heap = NULL;
			ulint offsets_[REC_OFFS_NORMAL_SIZE];
			rec_offs_init(offsets_);
			row_upd_rec_sys_fields_in_recovery(rec, page_zip, rec_get_offsets(rec, index, offsets_, ULINT_UNDEFINED, &heap), pos, trx_id, roll_ptr);
			if (IB_LIKELY_NULL(heap)) {
				IB_MEM_HEAP_FREE(heap);
			}
		}
	}
	return ptr;
}
#ifndef IB_HOTBACKUP

/// \brief Marks a clustered index record deleted. Writes an undo log record to undo log on this delete marking.
/// \details Writes in the trx id field the id of the deleting transaction, and in the roll ptr field pointer to the undo log record created.
/// \return DB_SUCCESS, DB_LOCK_WAIT, or error number
/// \param [in] flags undo logging and locking flags
/// \param [in] cursor cursor
/// \param [in] val value to set
/// \param [in] thr query thread
/// \param [in] mtr mtr
IB_INTERN ulint btr_cur_del_mark_set_clust_rec(ulint flags, btr_cur_t* cursor, ibool val, que_thr_t* thr, mtr_t* mtr)
{
	mem_heap_t* heap = NULL;
	ulint offsets_[REC_OFFS_NORMAL_SIZE];
	ulint* offsets = offsets_;
	rec_offs_init(offsets_);
	rec_t* rec = btr_cur_get_rec(cursor);
	dict_index_t* index = cursor->index;
	ut_ad(!!page_rec_is_comp(rec) == dict_table_is_comp(index->table));
	offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);
#ifdef IB_DEBUG
	if (btr_cur_print_record_ops && thr) {
		btr_cur_trx_report(thr_get_trx(thr), index, "del mark ");
		rec_print_new(state->stream, rec, offsets);
	}
#endif // IB_DEBUG
	ut_ad(dict_index_is_clust(index));
	ut_ad(!rec_get_deleted_flag(rec, rec_offs_comp(offsets)));
	ulint err = lock_clust_rec_modify_check_and_lock(flags, btr_cur_get_block(cursor), rec, index, offsets, thr);
	if (err != DB_SUCCESS) {
		goto func_exit;
	}
	roll_ptr_t roll_ptr;
	err = trx_undo_report_row_operation(flags, TRX_UNDO_MODIFY_OP, thr, index, NULL, NULL, 0, rec, &roll_ptr);
	if (err != DB_SUCCESS) {
		goto func_exit;
	}
	buf_block_t* block = btr_cur_get_block(cursor);
	if (block->is_hashed) {
		rw_lock_x_lock(&btr_search_latch);
	}
	page_zip_des_t* page_zip = buf_block_get_page_zip(block);
	btr_rec_set_deleted_flag(rec, page_zip, val);
	trx_t* trx = thr_get_trx(thr);
	if (!(flags & BTR_KEEP_SYS_FLAG)) {
		row_upd_rec_sys_fields(rec, page_zip, index, offsets, trx, roll_ptr);
	}
	if (block->is_hashed) {
		rw_lock_x_unlock(&btr_search_latch);
	}
	btr_cur_del_mark_set_clust_rec_log(flags, rec, index, val, trx, roll_ptr, mtr);
func_exit:
	if (IB_LIKELY_NULL(heap)) {
		IB_MEM_HEAP_FREE(heap);
	}
	return err;
}

/// \brief Writes the redo log record for a delete mark setting of a secondary index record.
/// \param [in] rec record
/// \param [in] val value to set
/// \param [in] mtr mtr
IB_INLINE void btr_cur_del_mark_set_sec_rec_log(rec_t* rec, ibool val, mtr_t* mtr)
{
	byte* log_ptr;
	ut_ad(val <= 1);
	log_ptr = mlog_open(mtr, 11 + 1 + 2);
	if (!log_ptr) {
		// Logging in mtr is switched off during crash recovery: in that case mlog_open returns NULL
		return;
	}
	log_ptr = mlog_write_initial_log_record_fast(rec, MLOG_REC_SEC_DELETE_MARK, log_ptr, mtr);
	mach_write_to_1(log_ptr, val);
	log_ptr++;
	mach_write_to_2(log_ptr, page_offset(rec));
	log_ptr += 2;
	mlog_close(mtr, log_ptr);
}
#endif // !IB_HOTBACKUP

/// \brief Parses the redo log record for delete marking or unmarking of a secondary index record.
/// \return end of log record or NULL
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in/out] page page or NULL
/// \param [in/out] page_zip compressed page, or NULL
IB_INTERN byte* btr_cur_parse_del_mark_set_sec_rec(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip)
{
	ulint val;
	ulint offset;
	rec_t* rec;
	if (end_ptr < ptr + 3) {
		return NULL;
	}
	val = mach_read_from_1(ptr);
	ptr++;
	offset = mach_read_from_2(ptr);
	ptr += 2;
	ut_a(offset <= IB_PAGE_SIZE);
	if (page) {
		rec = page + offset;
		// We do not need to reserve btr_search_latch, as the page is only being recovered, and there cannot be a hash index to it.
		btr_rec_set_deleted_flag(rec, page_zip, val);
	}
	return ptr;
}
#ifndef IB_HOTBACKUP

/// \brief Sets a secondary index record delete mark to TRUE or FALSE.
/// \return DB_SUCCESS, DB_LOCK_WAIT, or error number
/// \param [in] flags locking flag
/// \param [in] cursor cursor
/// \param [in] val value to set
/// \param [in] thr query thread
/// \param [in] mtr mtr
IB_INTERN ulint btr_cur_del_mark_set_sec_rec(ulint flags, btr_cur_t* cursor, ibool val, que_thr_t* thr, mtr_t* mtr)
{
	buf_block_t* block;
	rec_t* rec;
	ulint err;
	block = btr_cur_get_block(cursor);
	rec = btr_cur_get_rec(cursor);
#ifdef IB_DEBUG
	if (btr_cur_print_record_ops && thr) {
		btr_cur_trx_report(thr_get_trx(thr), cursor->index, "del mark ");
		rec_print(state->stream, rec, cursor->index);
	}
#endif // IB_DEBUG
	err = lock_sec_rec_modify_check_and_lock(flags, btr_cur_get_block(cursor), rec, cursor->index, thr, mtr);
	if (err != DB_SUCCESS) {
		return err;
	}
	ut_ad(!!page_rec_is_comp(rec) == dict_table_is_comp(cursor->index->table));
	if (block->is_hashed) {
		rw_lock_x_lock(&btr_search_latch);
	}
	btr_rec_set_deleted_flag(rec, buf_block_get_page_zip(block), val);
	if (block->is_hashed) {
		rw_lock_x_unlock(&btr_search_latch);
	}
	btr_cur_del_mark_set_sec_rec_log(rec, val, mtr);
	return DB_SUCCESS;
}

/// \brief Clear a secondary index record's delete mark. This function is only used by the insert buffer insert merge mechanism.
/// \param [in/out] rec record to delete unmark
/// \param [in/out] page_zip compressed page corresponding to rec, or NULL when the tablespace is uncompressed
/// \param [in] mtr mtr
IB_INTERN void btr_cur_del_unmark_for_ibuf(rec_t* rec, page_zip_des_t* page_zip, mtr_t* mtr)
{
    // We do not need to reserve btr_search_latch, as the page has just been read to the buffer pool and there cannot be a hash index to it.
    btr_rec_set_deleted_flag(rec, page_zip, FALSE);
    btr_cur_del_mark_set_sec_rec_log(rec, FALSE, mtr);
}
/*==================== B-TREE RECORD REMOVE =========================*/

/// \brief Tries to compress a page of the tree if it seems useful. It is assumed that mtr holds an x-latch on the tree and on the cursor page.
/// \details To avoid deadlocks, mtr must also own x-latches to brothers of page, if those brothers exist. NOTE: it is assumed that the caller 
// has reserved enough free extents so that the compression will always succeed if done!
/// \return TRUE if compression occurred
/// \param [in] cursor cursor on the page to compress; cursor does not stay valid if compression occurs
/// \param [in] mtr mtr
IB_INTERN ibool btr_cur_compress_if_useful(btr_cur_t* cursor, mtr_t* mtr)
{
	ut_ad(mtr_memo_contains(mtr, dict_index_get_lock(btr_cur_get_index(cursor)), MTR_MEMO_X_LOCK));
	ut_ad(mtr_memo_contains(mtr, btr_cur_get_block(cursor), MTR_MEMO_PAGE_X_FIX));
	return (btr_cur_compress_recommendation(cursor, mtr) && btr_compress(cursor, mtr));
}

/// \brief Removes the record on which the tree cursor is positioned on a leaf page.
/// \details It is assumed that the mtr has an x-latch on the page where the cursor is positioned, but no latch on the whole tree.
/// \return TRUE if success, i.e., the page did not become too empty
/// \param [in] cursor cursor on leaf page, on the record to delete; cursor stays valid: if deletion succeeds, on function exit it points to the successor of the deleted record
/// \param [in] mtr mtr; if this function returns TRUE on a leaf page of a secondary index, the mtr must be committed before latching any further pages
IB_INTERN ibool btr_cur_optimistic_delete(btr_cur_t* cursor, mtr_t* mtr)
{
	mem_heap_t* heap = NULL;
	ulint offsets_[REC_OFFS_NORMAL_SIZE];
	ulint* offsets = offsets_;
	rec_offs_init(offsets_);
	ut_ad(mtr_memo_contains(mtr, btr_cur_get_block(cursor), MTR_MEMO_PAGE_X_FIX));
	// This is intended only for leaf page deletions
	buf_block_t* block = btr_cur_get_block(cursor);
	ut_ad(page_is_leaf(buf_block_get_frame(block)));
	rec_t* rec = btr_cur_get_rec(cursor);
	offsets = rec_get_offsets(rec, cursor->index, offsets, ULINT_UNDEFINED, &heap);
	ibool no_compress_needed = !rec_offs_any_extern(offsets) && btr_cur_can_delete_without_compress(cursor, rec_offs_size(offsets), mtr);
	if (no_compress_needed) {
		page_t* page = buf_block_get_frame(block);
		page_zip_des_t* page_zip = buf_block_get_page_zip(block);
		ulint max_ins = 0;
		lock_update_delete(block, rec);
		btr_search_update_hash_on_delete(cursor);
		if (!page_zip) {
			max_ins = page_get_max_insert_size_after_reorganize(page, 1);
		}
#ifdef IB_ZIP_DEBUG
		ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
		page_cur_delete_rec(btr_cur_get_page_cur(cursor), cursor->index, offsets, mtr);
#ifdef IB_ZIP_DEBUG
		ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
		if (dict_index_is_clust(cursor->index) || dict_index_is_ibuf(cursor->index) || !page_is_leaf(page)) {
			// The insert buffer does not handle inserts to clustered indexes, to non-leaf pages of secondary index B-trees, or to the insert buffer.
		} else if (page_zip) {
			ibuf_update_free_bits_zip(block, mtr);
		} else {
			ibuf_update_free_bits_low(block, max_ins, mtr);
		}
	}
	if (IB_LIKELY_NULL(heap)) {
		IB_MEM_HEAP_FREE(heap);
	}
	return no_compress_needed;
}

/// \brief Removes the record on which the tree cursor is positioned. Tries to compress the page if its fillfactor drops below a threshold or if it is the only page on the level.
/// \details It is assumed that mtr holds an x-latch on the tree and on the cursor page. To avoid deadlocks, mtr must also own x-latches to brothers of page, if those brothers exist.
/// \return TRUE if compression occurred
/// \param [out] err DB_SUCCESS or DB_OUT_OF_FILE_SPACE; the latter may occur because we may have to update node pointers on upper levels, and in the case of variable length keys these may actually grow in size
/// \param [in] has_reserved_extents TRUE if the caller has already reserved enough free extents so that he knows that the operation will succeed
/// \param [in] cursor cursor on the record to delete; if compression does not occur, the cursor stays valid: it points to successor of deleted record on function exit
/// \param [in] rb_ctx rollback context
/// \param [in] mtr mtr
IB_INTERN ibool btr_cur_pessimistic_delete(ulint* err, ibool has_reserved_extents, btr_cur_t* cursor, enum trx_rb_ctx rb_ctx, mtr_t* mtr)
{
	ibool ret = FALSE;
	buf_block_t* block = btr_cur_get_block(cursor);
	page_t* page = buf_block_get_frame(block);
	dict_index_t* index = btr_cur_get_index(cursor);
	ut_ad(mtr_memo_contains(mtr, dict_index_get_lock(index), MTR_MEMO_X_LOCK));
	ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
	ulint n_extents = 0;
	ulint n_reserved;
	if (!has_reserved_extents) {
		// First reserve enough free space for the file segments of the index tree, so that the node pointer updates will not fail because of lack of space
		n_extents = cursor->tree_height / 32 + 1;
		ibool success = fsp_reserve_free_extents(&n_reserved, index->space, n_extents, FSP_CLEANING, mtr);
		if (!success) {
			*err = DB_OUT_OF_FILE_SPACE;
			return FALSE;
		}
	}
	mem_heap_t* heap = IB_MEM_HEAP_CREATE(1024);
	rec_t* rec = btr_cur_get_rec(cursor);
	page_zip_des_t* page_zip = buf_block_get_page_zip(block);
#ifdef IB_ZIP_DEBUG
	ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
	ulint* offsets = rec_get_offsets(rec, index, NULL, ULINT_UNDEFINED, &heap);
	if (rec_offs_any_extern(offsets)) {
		btr_rec_free_externally_stored_fields(index,
						      rec, offsets, page_zip,
						      rb_ctx, mtr);
#ifdef IB_ZIP_DEBUG
		ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
	}
	if (IB_UNLIKELY(page_get_n_recs(page) < 2)
	    && IB_UNLIKELY(dict_index_get_page(index)
			     != buf_block_get_page_no(block))) {
		// If there is only one record, drop the whole page in btr_discard_page, if this is not the root page
		btr_discard_page(cursor, mtr);
		*err = DB_SUCCESS;
		ret = TRUE;
		goto return_after_reservations;
	}
	lock_update_delete(block, rec);
	ulint level = btr_page_get_level(page, mtr);
	if (level > 0
	    && IB_UNLIKELY(rec == page_rec_get_next(
				     page_get_infimum_rec(page)))) {
		rec_t* next_rec = page_rec_get_next(rec);
		if (btr_page_get_prev(page, mtr) == FIL_NULL) {
			// If we delete the leftmost node pointer on a non-leaf level, we must mark the new leftmost node pointer as the predefined minimum record. This will make page_zip_validate() 
            // fail until page_cur_delete_rec() completes. This is harmless, because everything will take place within a single mini-transaction and because writing to the redo log is an atomic operation (performed by mtr_commit()).
			btr_set_min_rec_mark(next_rec, mtr);
		} else {
			// Otherwise, if we delete the leftmost node pointer on a page, we have to change the father node pointer so that it is equal to the new leftmost node pointer on the page
			btr_node_ptr_delete(index, block, mtr);
			dtuple_t* node_ptr = dict_index_build_node_ptr(
				index, next_rec, buf_block_get_page_no(block),
				heap, level);
			btr_insert_on_non_leaf_level(index,
						     level + 1, node_ptr, mtr);
		}
	}
	btr_search_update_hash_on_delete(cursor);
	page_cur_delete_rec(btr_cur_get_page_cur(cursor), index, offsets, mtr);
#ifdef IB_ZIP_DEBUG
	ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
	ut_ad(btr_check_node_ptr(index, block, mtr));
	*err = DB_SUCCESS;
return_after_reservations:
	IB_MEM_HEAP_FREE(heap);
	if (ret == FALSE) {
		ret = btr_cur_compress_if_useful(cursor, mtr);
	}
	if (n_extents > 0) {
		fil_space_release_free_extents(index->space, n_reserved);
	}
	return ret;
}

/// \brief Adds path information to the cursor for the current page, for which the binary search has been performed.
/// \param [in] cursor cursor positioned on a page
/// \param [in] height height of the page in tree; 0 means leaf node
/// \param [in] root_height root node height in tree
static void btr_cur_add_path_info(btr_cur_t* cursor, ulint height, ulint root_height)
{
    ut_a(cursor->path_arr);
    if (root_height >= BTR_PATH_ARRAY_N_SLOTS - 1) {
        // Do nothing; return empty path
        btr_path_t* slot = cursor->path_arr;
        slot->nth_rec = ULINT_UNDEFINED;
        return;
    }
    if (height == 0) {
        // Mark end of slots for path
        btr_path_t* slot = cursor->path_arr + root_height + 1;
        slot->nth_rec = ULINT_UNDEFINED;
    }
    rec_t* rec = btr_cur_get_rec(cursor);
    btr_path_t* slot = cursor->path_arr + (root_height - height);
    slot->nth_rec = page_rec_get_n_recs_before(rec);
    slot->n_recs = page_get_n_recs(page_align(rec));
}

/// \brief Estimates the number of rows in a given index range.
/// \param [in] index index
/// \param [in] tuple1 range start, may also be empty tuple
/// \param [in] mode1 search mode for range start
/// \param [in] tuple2 range end, may also be empty tuple
/// \param [in] mode2 search mode for range end
/// \return estimated number of rows
IB_INTERN ib_int64_t btr_estimate_n_rows_in_range(dict_index_t* index, const dtuple_t* tuple1, ulint mode1, const dtuple_t* tuple2, ulint mode2);
{
	btr_path_t	path1[BTR_PATH_ARRAY_N_SLOTS];
	btr_path_t	path2[BTR_PATH_ARRAY_N_SLOTS];
	mtr_t mtr;
	mtr_start(&mtr);
	btr_cur_t cursor;
	cursor.path_arr = path1;
	if (dtuple_get_n_fields(tuple1) > 0) {
		btr_cur_search_to_nth_level(index, 0, tuple1, mode1, BTR_SEARCH_LEAF | BTR_ESTIMATE, &cursor, 0, __FILE__, __LINE__, &mtr);
	} else {
		btr_cur_open_at_index_side(TRUE, index, BTR_SEARCH_LEAF | BTR_ESTIMATE, &cursor, &mtr);
	}
	mtr_commit(&mtr);
	mtr_start(&mtr);
	cursor.path_arr = path2;
	if (dtuple_get_n_fields(tuple2) > 0) {
		btr_cur_search_to_nth_level(index, 0, tuple2, mode2, BTR_SEARCH_LEAF | BTR_ESTIMATE, &cursor, 0, __FILE__, __LINE__, &mtr);
	} else {
		btr_cur_open_at_index_side(FALSE, index, BTR_SEARCH_LEAF | BTR_ESTIMATE, &cursor, &mtr);
	}
	mtr_commit(&mtr);
	// We have the path information for the range in path1 and path2
	ib_int64_t n_rows = 1;
	ibool diverged = FALSE;	    /* This becomes true when the path is not
				    the same any more */
	ibool diverged_lot = FALSE;	    /* This becomes true when the paths are
				    not the same or adjacent any more */
	ulint divergence_level = 1000000; /* This is the level where paths diverged
				    a lot */
	for (ulint i = 0; ; i++) {
		ut_ad(i < BTR_PATH_ARRAY_N_SLOTS);
		btr_path_t* slot1 = path1 + i;
		btr_path_t* slot2 = path2 + i;
		if (slot1->nth_rec == ULINT_UNDEFINED
		    || slot2->nth_rec == ULINT_UNDEFINED) {
			if (i > divergence_level + 1) {
				// In trees whose height is > 1 our algorithm tends to underestimate: multiply the estimate by 2: This compensation accounts for the hierarchical nature of B-tree indexes where leaf-level estimates may not capture the full range distribution
				n_rows = n_rows * 2;
			}
			// Do not estimate the number of rows in the range to over 1 / 2 of the estimated rows in the whole table. This prevents overestimation that could lead to inefficient query plans when the range actually contains fewer rows
			if (n_rows > index->table->stat_n_rows / 2) {
				n_rows = index->table->stat_n_rows / 2;
				// If there are just 0 or 1 rows in the table, then we estimate all rows are in the range. For very small tables, the estimation becomes trivial and we assume the entire table matches the range
				if (n_rows == 0) {
					n_rows = index->table->stat_n_rows;
				}
			}
			return n_rows;
		}
		if (!diverged && slot1->nth_rec != slot2->nth_rec) {
			diverged = TRUE;
			if (slot1->nth_rec < slot2->nth_rec) {
				n_rows = slot2->nth_rec - slot1->nth_rec;
				if (n_rows > 1) {
					diverged_lot = TRUE;
					divergence_level = i;
				}
			} else {
				// Maybe the tree has changed between searches
				return 10;
			}
		} else if (diverged && !diverged_lot) {
			if (slot1->nth_rec < slot1->n_recs
			    || slot2->nth_rec > 1) {
				diverged_lot = TRUE;
				divergence_level = i;
				n_rows = 0;
				if (slot1->nth_rec < slot1->n_recs) {
					n_rows += slot1->n_recs
						- slot1->nth_rec;
				}
				if (slot2->nth_rec > 1) {
					n_rows += slot2->nth_rec - 1;
				}
			}
		} else if (diverged_lot) {
			n_rows = (n_rows * (slot1->n_recs + slot2->n_recs))
				/ 2;
		}
	}
}
/// \brief Estimates the number of different key values in a given index, for each n-column prefix of the index where n <= dict_index_get_n_unique(index).
/// \details The estimates are stored in the array index->stat_n_diff_key_vals.
/// \param [in] index index
IB_INTERN void btr_estimate_number_of_different_key_vals(dict_index_t* index)
{
    ulint n_cols = dict_index_get_n_unique(index);
    ib_int64_t* n_diff = IB_MEM_ZALLOC((n_cols + 1) * sizeof(ib_int64_t));
    ulint not_empty_flag = 0;
    ulint total_external_size = 0;
    mem_heap_t* heap = NULL;
    ulint offsets_rec_[REC_OFFS_NORMAL_SIZE];
    ulint offsets_next_rec_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets_rec = offsets_rec_;
    ulint* offsets_next_rec = offsets_next_rec_;
    rec_offs_init(offsets_rec_);
    rec_offs_init(offsets_next_rec_);
    // It makes no sense to test more pages than are contained in the index, thus we lower the number if it is too high
    ib_uint64_t n_sample_pages;
    if (srv_stats_sample_pages > index->stat_index_size) {
        if (index->stat_index_size > 0) {
            n_sample_pages = index->stat_index_size;
        } else {
            n_sample_pages = 1;
        }
    } else {
        n_sample_pages = srv_stats_sample_pages;
    }
    // We sample some pages in the index to get an estimate
    for (ulint i = 0; i < n_sample_pages; i++) {
        btr_cur_t cursor;
        page_t* page;
        rec_t* rec;
        rec_t* supremum;
        ulint matched_fields;
        ulint matched_bytes;
        mtr_t mtr;
        mtr_start(&mtr);
        btr_cur_open_at_rnd_pos(index, BTR_SEARCH_LEAF, &cursor, &mtr);
        // Count the number of different key values for each prefix of the key on this index page. If the prefix does not determine the index record uniquely in the B-tree, then we subtract one because otherwise our algorithm would give a wrong estimate for an index where there is just one key value.
        page = btr_cur_get_page(&cursor);
        supremum = page_get_supremum_rec(page);
        rec = page_rec_get_next(page_get_infimum_rec(page));
        if (rec != supremum) {
            not_empty_flag = 1;
            offsets_rec = rec_get_offsets(rec, index, offsets_rec, ULINT_UNDEFINED, &heap);
        }
        while (rec != supremum) {
            rec_t* next_rec = page_rec_get_next(rec);
            if (next_rec == supremum) {
                break;
            }
            matched_fields = 0;
            matched_bytes = 0;
            offsets_next_rec = rec_get_offsets(next_rec, index, offsets_next_rec, n_cols, &heap);
            cmp_rec_rec_with_match(rec, next_rec, offsets_rec, offsets_next_rec, index, &matched_fields, &matched_bytes);
            for (ulint j = matched_fields + 1; j <= n_cols; j++) {
                // We add one if this index record has a different prefix from the previous
                n_diff[j]++;
            }
            total_external_size += btr_rec_get_externally_stored_len(rec, offsets_rec);
            rec = next_rec;
            // Initialize offsets_rec for the next round and assign the old offsets_rec buffer to offsets_next_rec.
            {
                ulint* offsets_tmp = offsets_rec;
                offsets_rec = offsets_next_rec;
                offsets_next_rec = offsets_tmp;
            }
        }
        if (n_cols == dict_index_get_n_unique_in_tree(index)) {
            // If there is more than one leaf page in the tree, we add one because we know that the first record on the page certainly had a different prefix than the last record on the previous index page in the alphabetical order. Before this fix, if there was just one big record on each clustered index page, the algorithm grossly underestimated the number of rows in the table.
            if (btr_page_get_prev(page, &mtr) != FIL_NULL || btr_page_get_next(page, &mtr) != FIL_NULL) {
                n_diff[n_cols]++;
            }
        }
        offsets_rec = rec_get_offsets(rec, index, offsets_rec, ULINT_UNDEFINED, &heap);
        total_external_size += btr_rec_get_externally_stored_len(rec, offsets_rec);
        mtr_commit(&mtr);
    }
    // If we saw k borders between different key values on n_sample_pages leaf pages, we can estimate how many there will be in index->stat_n_leaf_pages
    // We must take into account that our sample actually represents also the pages used for external storage of fields (those pages are included in index->stat_n_leaf_pages)
    dict_index_stat_mutex_enter(index);
    for (ulint j = 0; j <= n_cols; j++) {
        index->stat_n_diff_key_vals[j] = ((n_diff[j] * (ib_int64_t)index->stat_n_leaf_pages + n_sample_pages - 1 + total_external_size + not_empty_flag) / (n_sample_pages + total_external_size));
        // If the tree is small, smaller than 10 * n_sample_pages + total_external_size, then the above estimate is ok. For bigger trees it is common that we do not see any borders between key values in the few pages we pick. But still there may be n_sample_pages different key values, or even more. Let us try to approximate that:
        ib_uint64_t add_on = index->stat_n_leaf_pages / (10 * (n_sample_pages + total_external_size));
        if (add_on > n_sample_pages) {
            add_on = n_sample_pages;
        }
        index->stat_n_diff_key_vals[j] += add_on;
    }
    dict_index_stat_mutex_exit(index);
    IB_MEM_FREE(n_diff);
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
}
/*================== EXTERNAL STORAGE OF BIG FIELDS ===================*/
/// \brief Gets the externally stored size of a record, in units of a database page.
/// \param [in] rec record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return externally stored part, in units of a database page
static ulint btr_rec_get_externally_stored_len(rec_t* rec, const ulint* offsets)
{
    ulint total_extern_len = 0;
    ut_ad(!rec_offs_comp(offsets) || !rec_get_node_ptr_flag(rec));
    ulint n_fields = rec_offs_n_fields(offsets);
    for (ulint i = 0; i < n_fields; i++) {
        if (rec_offs_nth_extern(offsets, i)) {
            ulint local_len;
            byte* data = rec_get_nth_field(rec, offsets, i, &local_len);
            local_len -= BTR_EXTERN_FIELD_REF_SIZE;
            ulint extern_len = mach_read_from_4(data + local_len + BTR_EXTERN_LEN + 4);
            total_extern_len += ut_calc_align(extern_len, IB_PAGE_SIZE);
        }
    }
    return total_extern_len / IB_PAGE_SIZE;
}
/// \brief Sets the ownership bit of an externally stored field in a record.
/// \param [in/out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in/out] rec clustered index record
/// \param [in] index index of the page
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] i field number
/// \param [in] val value to set
/// \param [in] mtr mtr, or NULL if not logged
static void btr_cur_set_ownership_of_extern_field(page_zip_des_t* page_zip, rec_t* rec, dict_index_t* index, const ulint* offsets, ulint i, ibool val, mtr_t* mtr)
{
    ulint local_len;
    byte* data = rec_get_nth_field(rec, offsets, i, &local_len);
    ut_a(local_len >= BTR_EXTERN_FIELD_REF_SIZE);
    local_len -= BTR_EXTERN_FIELD_REF_SIZE;
    ulint byte_val = mach_read_from_1(data + local_len + BTR_EXTERN_LEN);
    if (val) {
        byte_val = byte_val & (~BTR_EXTERN_OWNER_FLAG);
    } else {
        byte_val = byte_val | BTR_EXTERN_OWNER_FLAG;
    }
    if (IB_LIKELY_NULL(page_zip)) {
        mach_write_to_1(data + local_len + BTR_EXTERN_LEN, byte_val);
        page_zip_write_blob_ptr(page_zip, rec, index, offsets, i, mtr);
    } else if (IB_LIKELY(mtr != NULL)) {
        mlog_write_ulint(data + local_len + BTR_EXTERN_LEN, byte_val, MLOG_1BYTE, mtr);
    } else {
        mach_write_to_1(data + local_len + BTR_EXTERN_LEN, byte_val);
    }
}
/// \brief Marks not updated extern fields as not-owned by this record. The ownership is transferred to the updated record which is inserted elsewhere in the index tree. In purge only the owner of externally stored field is allowed to free the field.
/// \param [in/out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in/out] rec record in a clustered index
/// \param [in] index index of the page
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] update update vector
/// \param [in] mtr mtr, or NULL if not logged
IB_INTERN void btr_cur_mark_extern_inherited_fields(page_zip_des_t* page_zip, rec_t* rec, dict_index_t* index, const ulint* offsets, const upd_t* update, mtr_t* mtr)
{
    ut_ad(rec_offs_validate(rec, NULL, offsets));
    ut_ad(!rec_offs_comp(offsets) || !rec_get_node_ptr_flag(rec));
    if (!rec_offs_any_extern(offsets)) {
        return;
    }
    ulint n = rec_offs_n_fields(offsets);
    for (ulint i = 0; i < n; i++) {
        if (rec_offs_nth_extern(offsets, i)) {
            // Check it is not in updated fields
            if (update) {
                for (ulint j = 0; j < upd_get_n_fields(update); j++) {
                    if (upd_get_nth_field(update, j)->field_no == i) {
                        goto updated;
                    }
                }
            }
            btr_cur_set_ownership_of_extern_field(page_zip, rec, index, offsets, i, FALSE, mtr);
        updated:
            ;
        }
    }
}
/// \brief The complement of the previous function: in an update entry may inherit some externally stored fields from a record. We must mark them as inherited in entry, so that they are not freed in a rollback.
/// \param [in/out] entry updated entry to be inserted to clustered index
/// \param [in] update update vector
IB_INTERN void btr_cur_mark_dtuple_inherited_extern(dtuple_t* entry, const upd_t* update)
{
    for (ulint i = 0; i < dtuple_get_n_fields(entry); i++) {
        dfield_t* dfield = dtuple_get_nth_field(entry, i);
        byte* data;
        ulint len;
        ulint j;
        if (!dfield_is_ext(dfield)) {
            continue;
        }
        // Check if it is in updated fields
        for (j = 0; j < upd_get_n_fields(update); j++) {
            if (upd_get_nth_field(update, j)->field_no == i) {
                goto is_updated;
            }
        }
        data = dfield_get_data(dfield);
        len = dfield_get_len(dfield);
        data[len - BTR_EXTERN_FIELD_REF_SIZE + BTR_EXTERN_LEN] |= BTR_EXTERN_INHERITED_FLAG;
    is_updated:
        ;
    }
}
/// \brief Marks all extern fields in a record as owned by the record.
/// \details This function should be called if the delete mark of a record is removed: a not delete marked record always owns all its extern fields.
/// \param [in/out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in/out] rec record in a clustered index
/// \param [in] index index of the page
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] mtr mtr, or NULL if not logged
static void btr_cur_unmark_extern_fields(page_zip_des_t* page_zip, rec_t* rec, dict_index_t* index, const ulint* offsets, mtr_t* mtr)
{
    ulint n = rec_offs_n_fields(offsets);
    ut_ad(!rec_offs_comp(offsets) || !rec_get_node_ptr_flag(rec));
    if (!rec_offs_any_extern(offsets)) {
        return;
    }
    for (ulint i = 0; i < n; i++) {
        if (rec_offs_nth_extern(offsets, i)) {
            btr_cur_set_ownership_of_extern_field(page_zip, rec, index, offsets, i, TRUE, mtr);
        }
    }
}
/// \brief Marks all extern fields in a dtuple as owned by the record.
/// \param [in/out] entry clustered index entry
IB_INTERN void btr_cur_unmark_dtuple_extern_fields(dtuple_t* entry)
{
    for (ulint i = 0; i < dtuple_get_n_fields(entry); i++) {
        dfield_t* dfield = dtuple_get_nth_field(entry, i);
        if (dfield_is_ext(dfield)) {
            byte* data = dfield_get_data(dfield);
            ulint len = dfield_get_len(dfield);
            data[len - BTR_EXTERN_FIELD_REF_SIZE + BTR_EXTERN_LEN] &= ~BTR_EXTERN_OWNER_FLAG;
        }
    }
}
/// \brief Flags the data tuple fields that are marked as extern storage in the update vector. We use this function to remember which fields we must mark as extern storage in a record inserted for an update.
/// \param [in/out] tuple data tuple
/// \param [in] update update vector
/// \param [in] heap memory heap
/// \return number of flagged external columns
IB_INTERN ulint btr_push_update_extern_fields(dtuple_t* tuple, const upd_t* update, mem_heap_t* heap)
{
    ulint n_pushed = 0;
    ut_ad(tuple);
    ut_ad(update);
    const upd_field_t* uf = update->fields;
    ulint n = upd_get_n_fields(update);
    for (; n--; uf++) {
        if (dfield_is_ext(&uf->new_val)) {
            dfield_t* field = dtuple_get_nth_field(tuple, uf->field_no);
            if (!dfield_is_ext(field)) {
                dfield_set_ext(field);
                n_pushed++;
            }
            switch (uf->orig_len) {
                byte* data;
                ulint len;
                byte* buf;
            case 0:
                break;
            case BTR_EXTERN_FIELD_REF_SIZE:
                // Restore the original locally stored part of the column. In the undo log, InnoDB writes a longer prefix of externally stored columns, so that column prefixes in secondary indexes can be reconstructed.
                dfield_set_data(field, (byte*) dfield_get_data(field) + dfield_get_len(field) - BTR_EXTERN_FIELD_REF_SIZE, BTR_EXTERN_FIELD_REF_SIZE);
                dfield_set_ext(field);
                break;
            default:
                // Reconstruct the original locally stored part of the column. The data will have to be copied.
                ut_a(uf->orig_len > BTR_EXTERN_FIELD_REF_SIZE);
                data = dfield_get_data(field);
                len = dfield_get_len(field);
                buf = mem_heap_alloc(heap, uf->orig_len);
                // Copy the locally stored prefix.
                memcpy(buf, data, uf->orig_len - BTR_EXTERN_FIELD_REF_SIZE);
                // Copy the BLOB pointer.
                memcpy(buf + uf->orig_len - BTR_EXTERN_FIELD_REF_SIZE, data + len - BTR_EXTERN_FIELD_REF_SIZE, BTR_EXTERN_FIELD_REF_SIZE);
                dfield_set_data(field, buf, uf->orig_len);
                dfield_set_ext(field);
            }
        }
    }
    return n_pushed;
}
/// \brief Returns the length of a BLOB part stored on the header page.
/// \param [in] blob_header blob header
/// \return part length
static ulint btr_blob_get_part_len(const byte* blob_header)
{
    return mach_read_from_4(blob_header + BTR_BLOB_HDR_PART_LEN);
}
/// \brief Returns the page number where the next BLOB part is stored.
/// \param [in] blob_header blob header
/// \return page number or FIL_NULL if no more pages
static ulint btr_blob_get_next_page_no(const byte* blob_header)
{
    return mach_read_from_4(blob_header + BTR_BLOB_HDR_NEXT_PAGE_NO);
}
/// \brief Deallocate a buffer block that was reserved for a BLOB part.
/// \param [in] block buffer block
/// \param [in] all TRUE=remove also the compressed page if there is one
/// \param [in] mtr mini-transaction to commit
static void btr_blob_free(buf_block_t* block, ibool all, mtr_t* mtr)
{
    ulint space = buf_block_get_space(block);
    ulint page_no = buf_block_get_page_no(block);
    ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
    mtr_commit(mtr);
    buf_pool_mutex_enter();
    mutex_enter(&block->mutex);
    // Only free the block if it is still allocated to the same file page.
    if (buf_block_get_state(block) == BUF_BLOCK_FILE_PAGE && buf_block_get_space(block) == space && buf_block_get_page_no(block) == page_no) {
        if (buf_LRU_free_block(&block->page, all, NULL) != BUF_LRU_FREED && all && block->page.zip.data) {
            // Attempt to deallocate the uncompressed page if the whole block cannot be deallocted.
            buf_LRU_free_block(&block->page, FALSE, NULL);
        }
    }
    buf_pool_mutex_exit();
    mutex_exit(&block->mutex);
}

/// \brief Stores the fields in big_rec_vec to the tablespace and puts pointers to them in rec. The extern flags in rec will have to be set beforehand. The fields are stored on pages allocated from leaf node file segment of the index tree.
/// \param [in] index index of rec; the index tree MUST be X-latched
/// \param [in/out] rec_block block containing rec
/// \param [in/out] rec record
/// \param [in] offsets rec_get_offsets(rec, index); the "external storage" flags in offsets will not correspond to rec when this function returns
/// \param [in] big_rec_vec vector containing fields to be stored externally
/// \param [in] local_mtr mtr containing the latch to rec and to the tree
/// \return DB_SUCCESS or error
IB_INTERN ulint btr_store_big_rec_extern_fields(dict_index_t* index, buf_block_t* rec_block, rec_t* rec, const ulint* offsets, big_rec_t* big_rec_vec, mtr_t* local_mtr)
{
    ulint i;
    mtr_t mtr;
    mem_heap_t* heap = NULL;
    z_stream c_stream;
    ut_ad(rec_offs_validate(rec, index, offsets));
    ut_ad(mtr_memo_contains(local_mtr, dict_index_get_lock(index), MTR_MEMO_X_LOCK));
    ut_ad(mtr_memo_contains(local_mtr, rec_block, MTR_MEMO_PAGE_X_FIX));
    ut_ad(buf_block_get_frame(rec_block) == page_align(rec));
    ut_a(dict_index_is_clust(index));
    page_zip_des_t* page_zip = buf_block_get_page_zip(rec_block);
    ut_a(dict_table_zip_size(index->table) == buf_block_get_zip_size(rec_block));
    ulint space_id = buf_block_get_space(rec_block);
    ulint zip_size = buf_block_get_zip_size(rec_block);
    ulint rec_page_no = buf_block_get_page_no(rec_block);
    ut_a(fil_page_get_type(page_align(rec)) == FIL_PAGE_INDEX);
    if (IB_LIKELY_NULL(page_zip)) {
        int err;
        // Zlib deflate needs 128 kilobytes for the default window size, plus 512 << memLevel, plus a few kilobytes for small objects. We use reduced memLevel to limit the memory consumption, and preallocate the heap, hoping to avoid memory fragmentation.
        heap = IB_MEM_HEAP_CREATE(250000);
        page_zip_set_alloc(&c_stream, heap);
        err = deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15, 7, Z_DEFAULT_STRATEGY);
        ut_a(err == Z_OK);
    }
    // We have to create a file segment to the tablespace for each field and put the pointer to the field in rec
    for (i = 0; i < big_rec_vec->n_fields; i++) {
        ut_ad(rec_offs_nth_extern(offsets, big_rec_vec->fields[i].field_no));
        {
            ulint local_len;
            byte* field_ref = rec_get_nth_field(rec, offsets, big_rec_vec->fields[i].field_no, &local_len);
            ut_a(local_len >= BTR_EXTERN_FIELD_REF_SIZE);
            local_len -= BTR_EXTERN_FIELD_REF_SIZE;
            field_ref += local_len;
        }
        extern_len = big_rec_vec->fields[i].len;
        ut_a(extern_len > 0);
        ulint prev_page_no = FIL_NULL;
        if (IB_LIKELY_NULL(page_zip)) {
            int err = deflateReset(&c_stream);
            ut_a(err == Z_OK);
            c_stream.next_in = (void*) big_rec_vec->fields[i].data;
            c_stream.avail_in = extern_len;
        }
        for (;;) {
            buf_block_t* block;
            page_t* page;
            mtr_start(&mtr);
            ulint hint_page_no;
            if (prev_page_no == FIL_NULL) {
                hint_page_no = 1 + rec_page_no;
            } else {
                hint_page_no = prev_page_no + 1;
            }
            block = btr_page_alloc(index, hint_page_no, FSP_NO_DIR, 0, &mtr);
            if (IB_UNLIKELY(block == NULL)) {
                mtr_commit(&mtr);
                if (IB_LIKELY_NULL(page_zip)) {
                    deflateEnd(&c_stream);
                    IB_MEM_HEAP_FREE(heap);
                }
                return DB_OUT_OF_FILE_SPACE;
            }
            ulint page_no = buf_block_get_page_no(block);
            page = buf_block_get_frame(block);
            if (prev_page_no != FIL_NULL) {
                buf_block_t* prev_block;
                page_t* prev_page;
                prev_block = buf_page_get(space_id, zip_size, prev_page_no, RW_X_LATCH, &mtr);
                buf_block_dbg_add_level(prev_block, SYNC_EXTERN_STORAGE);
                prev_page = buf_block_get_frame(prev_block);
                if (IB_LIKELY_NULL(page_zip)) {
                    mlog_write_ulint(prev_page + FIL_PAGE_NEXT, page_no, MLOG_4BYTES, &mtr);
                    memcpy(buf_block_get_page_zip(prev_block)->data + FIL_PAGE_NEXT, prev_page + FIL_PAGE_NEXT, 4);
                } else {
                    mlog_write_ulint(prev_page + FIL_PAGE_DATA + BTR_BLOB_HDR_NEXT_PAGE_NO, page_no, MLOG_4BYTES, &mtr);
                }
            }
            if (IB_LIKELY_NULL(page_zip)) {
                int err;
                page_zip_des_t* blob_page_zip;
                // Write FIL_PAGE_TYPE to the redo log separately, before logging any other changes to the page, so that the debug assertions in recv_parse_or_apply_log_rec_body() can be made simpler. 
                // Before InnoDB Plugin 1.0.4, the initialization of FIL_PAGE_TYPE was logged as part of the mlog_log_string() below.
                mlog_write_ulint(page + FIL_PAGE_TYPE, prev_page_no == FIL_NULL ? FIL_PAGE_TYPE_ZBLOB : FIL_PAGE_TYPE_ZBLOB2, MLOG_2BYTES, &mtr);
                c_stream.next_out = page + FIL_PAGE_DATA;
                c_stream.avail_out = page_zip_get_size(page_zip) - FIL_PAGE_DATA;
                err = deflate(&c_stream, Z_FINISH);
                ut_a(err == Z_OK || err == Z_STREAM_END);
                ut_a(err == Z_STREAM_END || c_stream.avail_out == 0);
                // Write the "next BLOB page" pointer
                mlog_write_ulint(page + FIL_PAGE_NEXT, FIL_NULL, MLOG_4BYTES, &mtr);
                // Initialize the unused "prev page" pointer
                mlog_write_ulint(page + FIL_PAGE_PREV, FIL_NULL, MLOG_4BYTES, &mtr);
                // Write a back pointer to the record into the otherwise unused area. This information could be useful in debugging.
                // Later, we might want to implement the possibility to relocate BLOB pages. Then, we would need to be able to adjust the BLOB pointer in the record.
                // We do not store the heap number of the record, because it can change in page_zip_reorganize() or btr_page_reorganize().
                // However, also the page number of the record may change when B-tree nodes are split or merged.
                mlog_write_ulint(page + FIL_PAGE_FILE_FLUSH_LSN, space_id, MLOG_4BYTES, &mtr);
                mlog_write_ulint(page + FIL_PAGE_FILE_FLUSH_LSN + 4, rec_page_no, MLOG_4BYTES, &mtr);
                // Zero out the unused part of the page.
                memset(page + page_zip_get_size(page_zip) - c_stream.avail_out, 0, c_stream.avail_out);
                mlog_log_string(page + FIL_PAGE_FILE_FLUSH_LSN, page_zip_get_size(page_zip) - FIL_PAGE_FILE_FLUSH_LSN, &mtr);
                // Copy the page to compressed storage, because it will be flushed to disk from there.
                blob_page_zip = buf_block_get_page_zip(block);
                ut_ad(blob_page_zip);
                ut_ad(page_zip_get_size(blob_page_zip) == page_zip_get_size(page_zip));
                memcpy(blob_page_zip->data, page, page_zip_get_size(page_zip));
                if (err == Z_OK && prev_page_no != FIL_NULL) {
                    goto next_zip_page;
                }
                rec_block = buf_page_get(space_id, zip_size, rec_page_no, RW_X_LATCH, &mtr);
                buf_block_dbg_add_level(rec_block, SYNC_NO_ORDER_CHECK);
                if (err == Z_STREAM_END) {
                    mach_write_to_4(field_ref + BTR_EXTERN_LEN, 0);
                    mach_write_to_4(field_ref + BTR_EXTERN_LEN + 4, c_stream.total_in);
                } else {
                    memset(field_ref + BTR_EXTERN_LEN, 0, 8);
                }
                if (prev_page_no == FIL_NULL) {
                    mach_write_to_4(field_ref + BTR_EXTERN_SPACE_ID, space_id);
                    mach_write_to_4(field_ref + BTR_EXTERN_PAGE_NO, page_no);
                    mach_write_to_4(field_ref + BTR_EXTERN_OFFSET, FIL_PAGE_NEXT);
                }
                page_zip_write_blob_ptr(page_zip, rec, index, offsets, big_rec_vec->fields[i].field_no, &mtr);
next_zip_page:
                prev_page_no = page_no;
                // Commit mtr and release the uncompressed page frame to save memory.
                btr_blob_free(block, FALSE, &mtr);
                if (err == Z_STREAM_END) {
                    break;
                }
            } else {
                mlog_write_ulint(page + FIL_PAGE_TYPE, FIL_PAGE_TYPE_BLOB, MLOG_2BYTES, &mtr);
                ulint store_len;
                if (extern_len > (IB_PAGE_SIZE - FIL_PAGE_DATA - BTR_BLOB_HDR_SIZE - FIL_PAGE_DATA_END)) {
                    store_len = IB_PAGE_SIZE - FIL_PAGE_DATA - BTR_BLOB_HDR_SIZE - FIL_PAGE_DATA_END;
                } else {
                    store_len = extern_len;
                }
                mlog_write_string(page + FIL_PAGE_DATA + BTR_BLOB_HDR_SIZE, (const byte*) big_rec_vec->fields[i].data + big_rec_vec->fields[i].len - extern_len, store_len, &mtr);
                mlog_write_ulint(page + FIL_PAGE_DATA + BTR_BLOB_HDR_PART_LEN, store_len, MLOG_4BYTES, &mtr);
                mlog_write_ulint(page + FIL_PAGE_DATA + BTR_BLOB_HDR_NEXT_PAGE_NO, FIL_NULL, MLOG_4BYTES, &mtr);
                extern_len -= store_len;
                rec_block = buf_page_get(space_id, zip_size, rec_page_no, RW_X_LATCH, &mtr);
                buf_block_dbg_add_level(rec_block, SYNC_NO_ORDER_CHECK);
                mlog_write_ulint(field_ref + BTR_EXTERN_LEN, 0, MLOG_4BYTES, &mtr);
                mlog_write_ulint(field_ref + BTR_EXTERN_LEN + 4, big_rec_vec->fields[i].len - extern_len, MLOG_4BYTES, &mtr);
                if (prev_page_no == FIL_NULL) {
                    mlog_write_ulint(field_ref + BTR_EXTERN_SPACE_ID, space_id, MLOG_4BYTES, &mtr);
                    mlog_write_ulint(field_ref + BTR_EXTERN_PAGE_NO, page_no, MLOG_4BYTES, &mtr);
                    mlog_write_ulint(field_ref + BTR_EXTERN_OFFSET, FIL_PAGE_DATA, MLOG_4BYTES, &mtr);
                }
                prev_page_no = page_no;
                mtr_commit(&mtr);
                if (extern_len == 0) {
                    break;
                }
            }
        }
    }
    if (IB_LIKELY_NULL(page_zip)) {
        deflateEnd(&c_stream);
        IB_MEM_HEAP_FREE(heap);
    }
    return DB_SUCCESS;
}

/// \brief Check the FIL_PAGE_TYPE on an uncompressed BLOB page.
/// \param [in] space_id space id
/// \param [in] page_no page number
/// \param [in] page page
/// \param [in] read TRUE=read, FALSE=purge
static void btr_check_blob_fil_page_type(ulint space_id, ulint page_no, const page_t* page, ibool read)
{
    ulint type = fil_page_get_type(page);
    ut_a(space_id == page_get_space_id(page));
    ut_a(page_no == page_get_page_no(page));
    if (IB_UNLIKELY(type != FIL_PAGE_TYPE_BLOB)) {
        ulint flags = fil_space_get_flags(space_id);
        if (IB_LIKELY((flags & DICT_TF_FORMAT_MASK) == DICT_TF_FORMAT_51)) {
            /* Old versions of InnoDB did not initialize FIL_PAGE_TYPE on BLOB pages. Do not print anything about the type mismatch when reading a BLOB page that is in Antelope format.*/
            return;
        }
        ut_print_timestamp(state->stream);
        ib_log(state, "  InnoDB: FIL_PAGE_TYPE=%lu on BLOB %s space %lu page %lu flags %lx\n", (ulong) type, read ? "read" : "purge", (ulong) space_id, (ulong) page_no, (ulong) flags);
        UT_ERROR;
    }
}


/// \brief Frees the externally stored field for a record.
/// \details Frees the space in an externally stored field to the file space management if the field in data is owned by the externally stored field, in a rollback we may have the additional condition that the field must not be inherited.
/// \param [in] index index of the data, the index tree MUST be X-latched; if the tree height is 1, then also the root page must be X-latched! (this is relevant in the case this function is called from purge where 'data' is located on an undo log page, not an index page)
/// \param [in/out] field_ref field reference
/// \param [in] rec record containing field_ref, for page_zip_write_blob_ptr(), or NULL
/// \param [in] offsets rec_get_offsets(rec, index), or NULL
/// \param [in] page_zip compressed page corresponding to rec, or NULL if rec == NULL
/// \param [in] i field number of field_ref; ignored if rec == NULL
/// \param [in] rb_ctx rollback context
/// \param [in] local_mtr mtr containing the latch to data an an X-latch to the index tree
IB_INTERN void btr_free_externally_stored_field(dict_index_t* index, byte* field_ref, const rec_t* rec, const ulint* offsets, page_zip_des_t* page_zip, ulint i, enum trx_rb_ctx rb_ctx, mtr_t* local_mtr)
{
    page_t* page;
    ulint space_id;
    ulint rec_zip_size = dict_table_zip_size(index->table);
    ulint ext_zip_size;
    ulint page_no;
    ulint next_page_no;
    mtr_t mtr;
#ifdef IB_DEBUG
    ut_ad(mtr_memo_contains(local_mtr, dict_index_get_lock(index), MTR_MEMO_X_LOCK));
    ut_ad(mtr_memo_contains_page(local_mtr, field_ref, MTR_MEMO_PAGE_X_FIX));
    ut_ad(!rec || rec_offs_validate(rec, index, offsets));
    if (rec) {
        ulint local_len;
        const byte* f = rec_get_nth_field(rec, offsets, i, &local_len);
        ut_a(local_len >= BTR_EXTERN_FIELD_REF_SIZE);
        local_len -= BTR_EXTERN_FIELD_REF_SIZE;
        f += local_len;
        ut_ad(f == field_ref);
    }
#endif // IB_DEBUG
    if (IB_UNLIKELY(!memcmp(field_ref, field_ref_zero, BTR_EXTERN_FIELD_REF_SIZE))) {
        // In the rollback of uncommitted transactions, we may encounter a clustered index record whose BLOBs have not been written. There is nothing to free then.
        ut_a(rb_ctx == RB_RECOVERY || rb_ctx == RB_RECOVERY_PURGE_REC);
        return;
    }
    space_id = mach_read_from_4(field_ref + BTR_EXTERN_SPACE_ID);
    if (IB_UNLIKELY(space_id != dict_index_get_space(index))) {
        ext_zip_size = fil_space_get_zip_size(space_id);
        // This must be an undo log record in the system tablespace, that is, in row_purge_upd_exist_or_extern(). Currently, externally stored records are stored in the same tablespace as the referring records.
        ut_ad(!page_get_space_id(page_align(field_ref)));
        ut_ad(!rec);
        ut_ad(!page_zip);
    } else {
        ext_zip_size = rec_zip_size;
    }
    if (!rec) {
        // This is a call from row_purge_upd_exist_or_extern().
        ut_ad(!page_zip);
        rec_zip_size = 0;
    }
    for (;;) {
        buf_block_t* rec_block;
        buf_block_t* ext_block;
        mtr_start(&mtr);
        rec_block = buf_page_get(page_get_space_id(page_align(field_ref)), rec_zip_size, page_get_page_no(page_align(field_ref)), RW_X_LATCH, &mtr);
        buf_block_dbg_add_level(rec_block, SYNC_NO_ORDER_CHECK);
        page_no = mach_read_from_4(field_ref + BTR_EXTERN_PAGE_NO);
        if (// There is no external storage data
            page_no == FIL_NULL
            // This field does not own the externally stored field
            || (mach_read_from_1(field_ref + BTR_EXTERN_LEN) & BTR_EXTERN_OWNER_FLAG)
            // Rollback and inherited field
            || ((rb_ctx == RB_NORMAL || rb_ctx == RB_RECOVERY) && (mach_read_from_1(field_ref + BTR_EXTERN_LEN) & BTR_EXTERN_INHERITED_FLAG))) {
            // Do not free
            mtr_commit(&mtr);
            return;
        }
        ext_block = buf_page_get(space_id, ext_zip_size, page_no, RW_X_LATCH, &mtr);
        buf_block_dbg_add_level(ext_block, SYNC_EXTERN_STORAGE);
        page = buf_block_get_frame(ext_block);
        if (ext_zip_size) {
            // Note that page_zip will be NULL in row_purge_upd_exist_or_extern().
            switch (fil_page_get_type(page)) {
            case FIL_PAGE_TYPE_ZBLOB:
            case FIL_PAGE_TYPE_ZBLOB2:
                break;
            default:
                UT_ERROR;
            }
            next_page_no = mach_read_from_4(page + FIL_PAGE_NEXT);
            btr_page_free_low(index, ext_block, 0, &mtr);
            if (IB_LIKELY(page_zip != NULL)) {
                mach_write_to_4(field_ref + BTR_EXTERN_PAGE_NO, next_page_no);
                mach_write_to_4(field_ref + BTR_EXTERN_LEN + 4, 0);
                page_zip_write_blob_ptr(page_zip, rec, index, offsets, i, &mtr);
            } else {
                mlog_write_ulint(field_ref + BTR_EXTERN_PAGE_NO, next_page_no, MLOG_4BYTES, &mtr);
                mlog_write_ulint(field_ref + BTR_EXTERN_LEN + 4, 0, MLOG_4BYTES, &mtr);
            }
        } else {
            ut_a(!page_zip);
            btr_check_blob_fil_page_type(space_id, page_no, page, FALSE);
            next_page_no = mach_read_from_4(page + FIL_PAGE_DATA + BTR_BLOB_HDR_NEXT_PAGE_NO);
            // We must supply the page level (= 0) as an argument because we did not store it on the page (we save the space overhead from an index page header.
            btr_page_free_low(index, ext_block, 0, &mtr);
            mlog_write_ulint(field_ref + BTR_EXTERN_PAGE_NO, next_page_no, MLOG_4BYTES, &mtr);
            // Zero out the BLOB length. If the server crashes during the execution of this function, trx_rollback_or_clean_all_recovered() could dereference the half-deleted BLOB, fetching a wrong prefix for the BLOB.
            mlog_write_ulint(field_ref + BTR_EXTERN_LEN + 4, 0, MLOG_4BYTES, &mtr);
        }
        // Commit mtr and release the BLOB block to save memory.
        btr_blob_free(ext_block, TRUE, &mtr);
    }
}

/// \brief Frees the externally stored fields for a record.
/// \param [in] index index of the data, the index tree MUST be X-latched
/// \param [in/out] rec record
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] rb_ctx rollback context
/// \param [in] mtr mini-transaction handle which contains an X-latch to record page and to the index tree
static void btr_rec_free_externally_stored_fields(dict_index_t* index, rec_t* rec, const ulint* offsets, page_zip_des_t* page_zip, enum trx_rb_ctx rb_ctx, mtr_t* mtr)
{
    ut_ad(rec_offs_validate(rec, index, offsets));
    ut_ad(mtr_memo_contains_page(mtr, rec, MTR_MEMO_PAGE_X_FIX));
    // Free possible externally stored fields in the record
    ut_ad(dict_table_is_comp(index->table) == !!rec_offs_comp(offsets));
    ulint n_fields = rec_offs_n_fields(offsets);
    for (ulint i = 0; i < n_fields; i++) {
        if (rec_offs_nth_extern(offsets, i)) {
            ulint len;
            byte* data = rec_get_nth_field(rec, offsets, i, &len);
            ut_a(len >= BTR_EXTERN_FIELD_REF_SIZE);
            btr_free_externally_stored_field(index, data + len - BTR_EXTERN_FIELD_REF_SIZE, rec, offsets, page_zip, i, rb_ctx, mtr);
        }
    }
}

/// \brief Frees the externally stored fields for a record, if the field is mentioned in the update vector.
/// \param [in] index index of rec; the index tree MUST be X-latched
/// \param [in/out] rec record
/// \param [in] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] update update vector
/// \param [in] rb_ctx rollback context
/// \param [in] mtr mini-transaction handle which contains an X-latch to record page and to the tree
static void btr_rec_free_updated_extern_fields(dict_index_t* index, rec_t* rec, page_zip_des_t* page_zip, const ulint* offsets, const upd_t* update, enum trx_rb_ctx rb_ctx, mtr_t* mtr)
{
    ut_ad(rec_offs_validate(rec, index, offsets));
    ut_ad(mtr_memo_contains_page(mtr, rec, MTR_MEMO_PAGE_X_FIX));
	// Free possible externally stored fields in the record
    ulint n_fields = upd_get_n_fields(update);
    for (ulint i = 0; i < n_fields; i++) {
        const upd_field_t* ufield = upd_get_nth_field(update, i);
        if (rec_offs_nth_extern(offsets, ufield->field_no)) {
            ulint len;
            byte* data = rec_get_nth_field(rec, offsets, ufield->field_no, &len);
            ut_a(len >= BTR_EXTERN_FIELD_REF_SIZE);
            btr_free_externally_stored_field(index, data + len - BTR_EXTERN_FIELD_REF_SIZE, rec, offsets, page_zip, ufield->field_no, rb_ctx, mtr);
        }
    }
}


/// \brief Copies the prefix of an uncompressed BLOB. The clustered index record that points to this BLOB must be protected by a lock or a page latch.
/// \param [out] buf the externally stored part of the field, or a prefix of it
/// \param [in] len length of buf, in bytes
/// \param [in] space_id space id of the BLOB pages
/// \param [in] page_no page number of the first BLOB page
/// \param [in] offset offset on the first BLOB page
/// \return number of bytes written to buf
static ulint btr_copy_blob_prefix(byte* buf, ulint len, ulint space_id, ulint page_no, ulint offset)
{
    ulint copied_len = 0;
    for (;;) {
        mtr_t mtr;
        mtr_start(&mtr);
        buf_block_t* block = buf_page_get(space_id, 0, page_no, RW_S_LATCH, &mtr);
        buf_block_dbg_add_level(block, SYNC_EXTERN_STORAGE);
        const page_t* page = buf_block_get_frame(block);
        btr_check_blob_fil_page_type(space_id, page_no, page, TRUE);
        const byte* blob_header = page + offset;
        ulint part_len = btr_blob_get_part_len(blob_header);
        ulint copy_len = ut_min(part_len, len - copied_len);
        memcpy(buf + copied_len, blob_header + BTR_BLOB_HDR_SIZE, copy_len);
        copied_len += copy_len;
        page_no = btr_blob_get_next_page_no(blob_header);
        mtr_commit(&mtr);
        if (page_no == FIL_NULL || copy_len != part_len) {
            return copied_len;
        }
        // On other BLOB pages except the first the BLOB header always is at the page data start:
        offset = FIL_PAGE_DATA;
        ut_ad(copied_len <= len);
    }
}

/// \brief Copies the prefix of a compressed BLOB. The clustered index record that points to this BLOB must be protected by a lock or a page latch.
/// \param [in/out] d_stream the decompressing stream
/// \param [in] zip_size compressed BLOB page size
/// \param [in] space_id space id of the BLOB pages
/// \param [in] page_no page number of the first BLOB page
/// \param [in] offset offset on the first BLOB page
static void btr_copy_zblob_prefix(z_stream* d_stream, ulint zip_size, ulint space_id, ulint page_no, ulint offset)
{
    ulint page_type = FIL_PAGE_TYPE_ZBLOB;
    ut_ad(ut_is_2pow(zip_size));
    ut_ad(zip_size >= PAGE_ZIP_MIN_SIZE);
    ut_ad(zip_size <= IB_PAGE_SIZE);
    ut_ad(space_id);
    for (;;) {
        // There is no latch on bpage directly. Instead, bpage is protected by the B-tree page latch that is being held on the clustered index record, or, in row_merge_copy_blobs(), by an exclusive table lock.
        buf_page_t* bpage = buf_page_get_zip(space_id, zip_size, page_no);
        if (IB_UNLIKELY(!bpage)) {
            ut_print_timestamp(state->stream);
            ib_log(state, "  InnoDB: Cannot load compressed BLOB page %lu space %lu\n", (ulong) page_no, (ulong) space_id);
            return;
        }
        if (IB_UNLIKELY(fil_page_get_type(bpage->zip.data) != page_type)) {
            ut_print_timestamp(state->stream);
            ib_log(state, "  InnoDB: Unexpected type %lu of compressed BLOB page %lu space %lu\n", (ulong) fil_page_get_type(bpage->zip.data), (ulong) page_no, (ulong) space_id);
            goto end_of_blob;
        }
        ulint next_page_no = mach_read_from_4(bpage->zip.data + offset);
        if (IB_LIKELY(offset == FIL_PAGE_NEXT)) {
            // When the BLOB begins at page header, the compressed data payload does not immediately follow the next page pointer.
            offset = FIL_PAGE_DATA;
        } else {
            offset += 4;
        }
        d_stream->next_in = bpage->zip.data + offset;
        d_stream->avail_in = zip_size - offset;
        int err = inflate(d_stream, Z_NO_FLUSH);
        switch (err) {
        case Z_OK:
            if (!d_stream->avail_out) {
                goto end_of_blob;
            }
            break;
        case Z_STREAM_END:
            if (next_page_no == FIL_NULL) {
                goto end_of_blob;
            }
            // fall through
        default:
inflate_error:
            ut_print_timestamp(state->stream);
            ib_log(state, "  InnoDB: inflate() of compressed BLOB page %lu space %lu returned %d (%s)\n", (ulong) page_no, (ulong) space_id, err, d_stream->msg);
        case Z_BUF_ERROR:
            goto end_of_blob;
        }
        if (next_page_no == FIL_NULL) {
            if (!d_stream->avail_in) {
                ut_print_timestamp(state->stream);
                ib_log(state, "  InnoDB: unexpected end of compressed BLOB page %lu space %lu\n", (ulong) page_no, (ulong) space_id);
            } else {
                err = inflate(d_stream, Z_FINISH);
                switch (err) {
                case Z_STREAM_END:
                case Z_BUF_ERROR:
                    break;
                default:
                    goto inflate_error;
                }
            }
end_of_blob:
            buf_page_release_zip(bpage);
            return;
        }
        buf_page_release_zip(bpage);
        // On other BLOB pages except the first the BLOB header always is at the page header:
        page_no = next_page_no;
        offset = FIL_PAGE_NEXT;
        page_type = FIL_PAGE_TYPE_ZBLOB2;
    }
}

/// \brief Copies the prefix of an externally stored field of a record. The clustered index record that points to this BLOB must be protected by a lock or a page latch.
/// \param [out] buf the externally stored part of the field, or a prefix of it
/// \param [in] len length of buf, in bytes
/// \param [in] zip_size nonzero=compressed BLOB page size, zero for uncompressed BLOBs
/// \param [in] space_id space id of the first BLOB page
/// \param [in] page_no page number of the first BLOB page
/// \param [in] offset offset on the first BLOB page
/// \return number of bytes written to buf
static ulint btr_copy_externally_stored_field_prefix_low(byte* buf, ulint len, ulint zip_size, ulint space_id, ulint page_no, ulint offset)
{
    if (IB_UNLIKELY(len == 0)) {
        return 0;
    }
    if (IB_UNLIKELY(zip_size)) {
        // Zlib inflate needs 32 kilobytes for the default window size, plus a few kilobytes for small objects.
        mem_heap_t* heap = IB_MEM_HEAP_CREATE(40000);
        z_stream d_stream;
        page_zip_set_alloc(&d_stream, heap);
        int err = inflateInit(&d_stream);
        ut_a(err == Z_OK);
        d_stream.next_out = buf;
        d_stream.avail_out = len;
        d_stream.avail_in = 0;
        btr_copy_zblob_prefix(&d_stream, zip_size, space_id, page_no, offset);
        inflateEnd(&d_stream);
        IB_MEM_HEAP_FREE(heap);
        return d_stream.total_out;
    } else {
        return btr_copy_blob_prefix(buf, len, space_id, page_no, offset);
    }
}

/// \brief Copies the prefix of an externally stored field of a record. The clustered index record must be protected by a lock or a page latch.
/// \param [out] buf the field, or a prefix of it
/// \param [in] len length of buf, in bytes
/// \param [in] zip_size nonzero=compressed BLOB page size, zero for uncompressed BLOBs
/// \param [in] data 'internally' stored part of the field containing also the reference to the external part; must be protected by a lock or a page latch
/// \param [in] local_len length of data, in bytes
/// \return the length of the copied field, or 0 if the column was being or has been deleted
IB_INTERN ulint btr_copy_externally_stored_field_prefix(byte* buf, ulint len, ulint zip_size, const byte* data, ulint local_len)
{
    ut_a(local_len >= BTR_EXTERN_FIELD_REF_SIZE);
    local_len -= BTR_EXTERN_FIELD_REF_SIZE;
    if (IB_UNLIKELY(local_len >= len)) {
        memcpy(buf, data, len);
        return len;
    }
    memcpy(buf, data, local_len);
    data += local_len;
    ut_a(memcmp(data, field_ref_zero, BTR_EXTERN_FIELD_REF_SIZE));
    if (!mach_read_from_4(data + BTR_EXTERN_LEN + 4)) {
        // The externally stored part of the column has been (partially) deleted. Signal the half-deleted BLOB to the caller.
        return 0;
    }
    ulint space_id = mach_read_from_4(data + BTR_EXTERN_SPACE_ID);
    ulint page_no = mach_read_from_4(data + BTR_EXTERN_PAGE_NO);
    ulint offset = mach_read_from_4(data + BTR_EXTERN_OFFSET);
    return local_len + btr_copy_externally_stored_field_prefix_low(buf + local_len, len - local_len, zip_size, space_id, page_no, offset);
}

/// \brief Copies an externally stored field of a record to mem heap. The clustered index record must be protected by a lock or a page latch.
/// \param [out] len length of the whole field
/// \param [in] data 'internally' stored part of the field containing also the reference to the external part; must be protected by a lock or a page latch
/// \param [in] zip_size nonzero=compressed BLOB page size, zero for uncompressed BLOBs
/// \param [in] local_len length of data
/// \param [in] heap mem heap
/// \return the whole field copied to heap
static byte* btr_copy_externally_stored_field(ulint* len, const byte* data, ulint zip_size, ulint local_len, mem_heap_t* heap)
{
    ut_a(local_len >= BTR_EXTERN_FIELD_REF_SIZE);
    local_len -= BTR_EXTERN_FIELD_REF_SIZE;
    ulint space_id = mach_read_from_4(data + local_len + BTR_EXTERN_SPACE_ID);
    ulint page_no = mach_read_from_4(data + local_len + BTR_EXTERN_PAGE_NO);
    ulint offset = mach_read_from_4(data + local_len + BTR_EXTERN_OFFSET);
    // Currently a BLOB cannot be bigger than 4 GB; we leave the 4 upper bytes in the length field unused
    ulint extern_len = mach_read_from_4(data + local_len + BTR_EXTERN_LEN + 4);
    byte* buf = mem_heap_alloc(heap, local_len + extern_len);
    memcpy(buf, data, local_len);
    *len = local_len + btr_copy_externally_stored_field_prefix_low(buf + local_len, extern_len, zip_size, space_id, page_no, offset);
    return buf;
}

/// \brief Copies an externally stored field of a record to mem heap.
/// \param [in] rec record in a clustered index; must be protected by a lock or a page latch
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] zip_size nonzero=compressed BLOB page size, zero for uncompressed BLOBs
/// \param [in] no field number
/// \param [out] len length of the field
/// \param [in] heap mem heap
/// \return the field copied to heap
IB_INTERN byte* btr_rec_copy_externally_stored_field(const rec_t* rec, const ulint* offsets, ulint zip_size, ulint no, ulint* len, mem_heap_t* heap)
{
    ut_a(rec_offs_nth_extern(offsets, no));
    // An externally stored field can contain some initial data from the field, and in the last 20 bytes it has the space id, page number, and offset where the rest of the field data is stored, and the data length in addition to the data stored locally. We may need to store some data locally to get the local record length above the 128 byte limit so that field offsets are stored in two bytes, and the extern bit is available in those two bytes.
    ulint local_len;
    const byte* data = rec_get_nth_field(rec, offsets, no, &local_len);
    return btr_copy_externally_stored_field(len, data, zip_size, local_len, heap);
}
#endif // !IB_HOTBACKUP
