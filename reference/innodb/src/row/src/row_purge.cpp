// Copyright (c) 1997, 2009, Innobase Oy. All Rights Reserved.
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

/// \file row_purge.cpp
/// \brief Purge obsolete records
/// \details Originally created on 3/14/1997 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "row_purge.hpp"

#ifdef IB_DO_NOT_INLINE
#include "row_purge.inl"
#endif

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

#include "fsp_fsp.hpp"
#include "mach_data.hpp"
#include "trx_rseg.hpp"
#include "trx_trx.hpp"
#include "trx_roll.hpp"
#include "trx_undo.hpp"
#include "trx_purge.hpp"
#include "trx_rec.hpp"
#include "que_que.hpp"
#include "row_row.hpp"
#include "row_upd.hpp"
#include "row_vers.hpp"
#include "log_log.hpp"

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

/// \brief Repositions the pcur in the purge node on the clustered index record, if found.
/// \param [in] mode latching mode
/// \param [in] node row purge node
/// \param [in] mtr mtr
/// \return TRUE if the record was found
static ibool row_purge_reposition_pcur(ulint mode, purge_node_t* node, mtr_t* mtr);

/// \brief Removes a delete marked clustered index record if possible.
/// \param [in] node row purge node
/// \param [in] mode BTR_MODIFY_LEAF or BTR_MODIFY_TREE
/// \return TRUE if success, or if not found, or if modified after the delete marking
static ibool row_purge_remove_clust_if_poss_low(purge_node_t* node, ulint mode);

/// \brief Removes a clustered index record if it has not been modified after the delete marking.
/// \param [in] node row purge node
static void row_purge_remove_clust_if_poss(purge_node_t* node);

/// \brief Removes a secondary index entry if possible.
/// \param [in] node row purge node
/// \param [in] index index
/// \param [in] entry index entry
/// \param [in] mode latch mode BTR_MODIFY_LEAF or BTR_MODIFY_TREE
/// \return TRUE if success or if not found
static ibool row_purge_remove_sec_if_poss_low(purge_node_t* node, dict_index_t* index, const dtuple_t* entry, ulint mode);

/// \brief Removes a secondary index entry if possible.
/// \param [in] node row purge node
/// \param [in] index index
/// \param [in] entry index entry
static void row_purge_remove_sec_if_poss(purge_node_t* node, dict_index_t* index, dtuple_t* entry);

/// \brief Purges a delete marking of a record.
/// \param [in] node row purge node
static void row_purge_del_mark(purge_node_t* node);

/// \brief Purges an update of an existing record.
/// \details Also purges an update of a delete marked record if that record contained an externally stored field.
/// \param [in] node row purge node
static void row_purge_upd_exist_or_extern(purge_node_t* node);

/// \brief Parses the row reference and other info in a modify undo log record.
/// \param [in] node row undo node
/// \param [out] updated_extern TRUE if an externally stored field was updated
/// \param [in] thr query thread
/// \return TRUE if purge operation required: NOTE that then the CALLER must unfreeze data dictionary!
static ibool row_purge_parse_undo_rec(purge_node_t* node, ibool* updated_extern, que_thr_t* thr);

/// \brief Fetches an undo log record and does the purge for the recorded operation.
/// \details If none left, or the current purge completed, returns the control to the parent node, which is always a query thread node.
/// \param [in] node row purge node
/// \param [in] thr query thread
/// \return DB_SUCCESS if operation successfully completed, else error code
static ulint row_purge(purge_node_t* node, que_thr_t* thr);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Creates a purge node to a query graph.
/// \param [in] parent parent node, i.e., a thr node
/// \param [in] heap memory heap where created
/// \return own: purge node
IB_INTERN purge_node_t* row_purge_node_create(que_thr_t* parent, mem_heap_t* heap)
{
    purge_node_t* node;
    ut_ad(parent && heap);
    node = mem_heap_alloc(heap, sizeof(purge_node_t));
    node->common.type = QUE_NODE_PURGE;
    node->common.parent = parent;
    node->heap = IB_MEM_HEAP_CREATE(256);
    return node;
}

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

static ibool row_purge_reposition_pcur(ulint mode, purge_node_t* node, mtr_t* mtr)
{
    if (node->found_clust) {
        ibool found = btr_pcur_restore_position(mode, &(node->pcur), mtr);
        return found;
    }

    ibool found = row_search_on_row_ref(&(node->pcur), mode, node->table, node->ref, mtr);
    node->found_clust = found;

    if (found) {
        btr_pcur_store_position(&(node->pcur), mtr);
    }

    return found;
}

/// \return TRUE if success, or if not found, or if modified after the delete marking
static ibool row_purge_remove_clust_if_poss_low(purge_node_t* node, ulint mode)
{
    dict_index_t* index = dict_table_get_first_index(node->table);
    btr_pcur_t* pcur = &(node->pcur);
    btr_cur_t* btr_cur = btr_pcur_get_btr_cur(pcur);
    mtr_t mtr;
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    rec_offs_init(offsets_);
    mtr_start(&mtr);

    ibool success = row_purge_reposition_pcur(mode, node, &mtr);

    if (!success) {
        /* The record is already removed */
        btr_pcur_commit_specify_mtr(pcur, &mtr);
        return TRUE;
    }

    rec_t* rec = btr_pcur_get_rec(pcur);

    if (0 != ut_dulint_cmp(node->roll_ptr, row_get_rec_roll_ptr(rec, index, rec_get_offsets(rec, index, offsets_, ULINT_UNDEFINED, &heap)))) {
        if (IB_LIKELY_NULL(heap)) {
            IB_MEM_HEAP_FREE(heap);
        }
        /* Someone else has modified the record later: do not remove */
        btr_pcur_commit_specify_mtr(pcur, &mtr);
        return TRUE;
    }

    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }

    if (mode == BTR_MODIFY_LEAF) {
        success = btr_cur_optimistic_delete(btr_cur, &mtr);
    } else {
        ut_ad(mode == BTR_MODIFY_TREE);
        ulint err;
        btr_cur_pessimistic_delete(&err, FALSE, btr_cur, RB_NONE, &mtr);

        if (err == DB_SUCCESS) {
            success = TRUE;
        } else if (err == DB_OUT_OF_FILE_SPACE) {
            success = FALSE;
        } else {
            UT_ERROR;
        }
    }

    btr_pcur_commit_specify_mtr(pcur, &mtr);
    return success;
}

static void row_purge_remove_clust_if_poss(purge_node_t* node)
{
    /* ib_log(state, "Purge: Removing clustered record\n"); */
    ibool success = row_purge_remove_clust_if_poss_low(node, BTR_MODIFY_LEAF);
	if (success) {
		return;
	}

    ulint n_tries = 0;
retry:
	success = row_purge_remove_clust_if_poss_low(node, BTR_MODIFY_TREE);
    /* The delete operation may fail if we have little file space left: TODO: easiest to crash the database and restart with more file space */

	if (!success && n_tries < BTR_CUR_RETRY_DELETE_N_TIMES) {
		n_tries++;
		os_thread_sleep(BTR_CUR_RETRY_SLEEP_TIME);
		goto retry;
	}

	ut_a(success);
}

/// \return TRUE if success or if not found
static ibool row_purge_remove_sec_if_poss_low(purge_node_t* node, dict_index_t* index, const dtuple_t* entry, ulint mode)
{
    btr_pcur_t pcur;
    mtr_t mtr;
    mtr_t mtr_vers;

	log_free_check();
	mtr_start(&mtr);

    ibool found = row_search_index_entry(index, entry, mode, &pcur, &mtr);

	if (!found) {
        /* Not found. This is a legitimate condition. In a rollback, InnoDB will remove secondary recs that would be purged anyway. Then the actual purge will not find the secondary index record. Also, the purge itself is eager: if it comes to consider a secondary index record, and notices it does not need to exist in the index, it will remove it. Then if/when the purge comes to consider the secondary index record a second time, it will not exist any more in the index. */
        /* ib_log(state, "PURGE:........sec entry not found\n"); */
		/* dtuple_print(state->stream, entry); */
		btr_pcur_close(&pcur);
		mtr_commit(&mtr);
        return TRUE;
    }

    btr_cur_t* btr_cur = btr_pcur_get_btr_cur(&pcur);
    /* We should remove the index record if no later version of the row, which cannot be purged yet, requires its existence. If some requires, we should do nothing. */

	mtr_start(&mtr_vers);
    ibool success = row_purge_reposition_pcur(BTR_SEARCH_LEAF, node, &mtr_vers);

    ibool old_has = FALSE;
	if (success) {
        old_has = row_vers_old_has_index_entry(TRUE, btr_pcur_get_rec(&(node->pcur)), &mtr_vers, index, entry);
	}

	btr_pcur_commit_specify_mtr(&(node->pcur), &mtr_vers);

	if (!success || !old_has) {
		/* Remove the index record */
		if (mode == BTR_MODIFY_LEAF) {
			success = btr_cur_optimistic_delete(btr_cur, &mtr);
		} else {
			ut_ad(mode == BTR_MODIFY_TREE);
            ulint err;
            btr_cur_pessimistic_delete(&err, FALSE, btr_cur, RB_NONE, &mtr);
			success = err == DB_SUCCESS;
			ut_a(success || err == DB_OUT_OF_FILE_SPACE);
		}
	}

	btr_pcur_close(&pcur);
	mtr_commit(&mtr);
    return success;
}

static void row_purge_remove_sec_if_poss(purge_node_t* node, dict_index_t* index, dtuple_t* entry)
{
	/* ib_log(state, "Purge: Removing secondary record\n"); */
    ibool success = row_purge_remove_sec_if_poss_low(node, index, entry, BTR_MODIFY_LEAF);
	if (success) {
		return;
	}

    ulint n_tries = 0;
retry:
    success = row_purge_remove_sec_if_poss_low(node, index, entry, BTR_MODIFY_TREE);
    /* The delete operation may fail if we have little file space left: TODO: easiest to crash the database and restart with more file space */

	if (!success && n_tries < BTR_CUR_RETRY_DELETE_N_TIMES) {
		n_tries++;
		os_thread_sleep(BTR_CUR_RETRY_SLEEP_TIME);
		goto retry;
	}

	ut_a(success);
}

/// \brief Purges a delete marking of a record.
{
	ut_ad(node);
    mem_heap_t* heap = IB_MEM_HEAP_CREATE(1024);

	while (node->index != NULL) {
        dict_index_t* index = node->index;
		/* Build the index entry */
        dtuple_t* entry = row_build_index_entry(node->row, NULL, index, heap);
		ut_a(entry);
		row_purge_remove_sec_if_poss(node, index, entry);
		node->index = dict_table_get_next_index(node->index);
	}

	IB_MEM_HEAP_FREE(heap);

	row_purge_remove_clust_if_poss(node);
}

/// \brief Purges an update of an existing record.
{
	ut_ad(node);

	if (node->rec_type == TRX_UNDO_UPD_DEL_REC) {
		goto skip_secondaries;
	}

    mem_heap_t* heap = IB_MEM_HEAP_CREATE(1024);

	while (node->index != NULL) {
        dict_index_t* index = node->index;

        if (row_upd_changes_ord_field_binary(NULL, node->index, node->update)) {
			/* Build the older version of the index entry */
            dtuple_t* entry = row_build_index_entry(node->row, NULL, index, heap);
			ut_a(entry);
			row_purge_remove_sec_if_poss(node, index, entry);
		}

		node->index = dict_table_get_next_index(node->index);
	}

	IB_MEM_HEAP_FREE(heap);

skip_secondaries:
	/* Free possible externally stored fields */
    for (ulint i = 0; i < upd_get_n_fields(node->update); i++) {
        const upd_field_t* ufield = upd_get_nth_field(node->update, i);

		if (dfield_is_ext(&ufield->new_val)) {
            /* We use the fact that new_val points to node->undo_rec and get thus the offset of dfield data inside the undo record. Then we can calculate from node->roll_ptr the file address of the new_val data */
            ulint internal_offset = ((const byte*) dfield_get_data(&ufield->new_val)) - node->undo_rec;
			ut_a(internal_offset < IB_PAGE_SIZE);

            ibool is_insert;
            ulint rseg_id;
            ulint page_no;
            ulint offset;
            trx_undo_decode_roll_ptr(node->roll_ptr, &is_insert, &rseg_id, &page_no, &offset);

            mtr_t mtr;
			mtr_start(&mtr);

            /* We have to acquire an X-latch to the clustered index tree */
            dict_index_t* index = dict_table_get_first_index(node->table);
			mtr_x_lock(dict_index_get_lock(index), &mtr);

            /* NOTE: we must also acquire an X-latch to the root page of the tree. We will need it when we free pages from the tree. If the tree is of height 1, the tree X-latch does NOT protect the root page, because it is also a leaf page. Since we will have a latch on an undo log page, we would break the latching order if we would only later latch the root page of such a tree! */
			btr_root_get(index, &mtr);

            /* We assume in purge of externally stored fields that the space id of the undo log record is 0! */
            buf_block_t* block = buf_page_get(0, 0, page_no, RW_X_LATCH, &mtr);
			buf_block_dbg_add_level(block, SYNC_TRX_UNDO_PAGE);

            byte* data_field = buf_block_get_frame(block) + offset + internal_offset;

            ut_a(dfield_get_len(&ufield->new_val) >= BTR_EXTERN_FIELD_REF_SIZE);
            btr_free_externally_stored_field(index, data_field + dfield_get_len(&ufield->new_val) - BTR_EXTERN_FIELD_REF_SIZE, NULL, NULL, NULL, 0, RB_NONE, &mtr);
			mtr_commit(&mtr);
		}
	}
}

/// \brief Parses the row reference and other info in a modify undo log record.
/// \param [in] node row undo node
static ibool row_purge_parse_undo_rec(purge_node_t* node, ibool* updated_extern, que_thr_t* thr);
{
    ut_ad(node && thr);
    trx_t* trx = thr_get_trx(thr);

    ulint type;
    ulint cmpl_info;
    undo_no_t undo_no;
    dulint table_id;
    byte* ptr = trx_undo_rec_get_pars(node->undo_rec, &type, &cmpl_info, updated_extern, &undo_no, &table_id);
    node->rec_type = type;

    if (type == TRX_UNDO_UPD_DEL_REC && !(*updated_extern)) {
        return FALSE;
    }

    trx_id_t trx_id;
    roll_ptr_t roll_ptr;
    ulint info_bits;
    ptr = trx_undo_update_rec_get_sys_cols(ptr, &trx_id, &roll_ptr, &info_bits);
    node->table = NULL;

    if (type == TRX_UNDO_UPD_EXIST_REC && cmpl_info & UPD_NODE_NO_ORD_CHANGE && !(*updated_extern)) {
        /* Purge requires no changes to indexes: we may return */
        return FALSE;
    }

    /* Prevent DROP TABLE etc. from running when we are doing the purge for this row */
    dict_freeze_data_dictionary(trx);

    mutex_enter(&(dict_sys->mutex));

    // FIXME: srv_force_recovery should be passed in as an arg
    node->table = dict_table_get_on_id_low(srv_force_recovery, table_id);

    mutex_exit(&(dict_sys->mutex));

    if (node->table == NULL) {
        /* The table has been dropped: no need to do purge */
err_exit:
        dict_unfreeze_data_dictionary(trx);
        return FALSE;
    }

    if (node->table->ibd_file_missing) {
        /* We skip purge of missing .ibd files */
        node->table = NULL;
        goto err_exit;
    }

    dict_index_t* clust_index = dict_table_get_first_index(node->table);

    if (clust_index == NULL) {
        /* The table was corrupt in the data dictionary */
        goto err_exit;
    }

    ptr = trx_undo_rec_get_row_ref(ptr, clust_index, &(node->ref), node->heap);

    ptr = trx_undo_update_rec_get_update(ptr, clust_index, type, trx_id, roll_ptr, info_bits, trx, node->heap, &(node->update));

    /* Read to the partial row the fields that occur in indexes */
    if (!(cmpl_info & UPD_NODE_NO_ORD_CHANGE)) {
        ptr = trx_undo_rec_get_partial_row(ptr, clust_index, &node->row, type == TRX_UNDO_UPD_DEL_REC, node->heap);
    }

    return TRUE;
}

/// \brief Fetches an undo log record and does the purge for the recorded operation.
/// \details If none left, or the current purge completed, returns the control to the parent node, which is always a query thread node.
static ulint row_purge(purge_node_t* node, que_thr_t* thr);
{
    ut_ad(node && thr);
    trx_t* trx = thr_get_trx(thr);

    roll_ptr_t roll_ptr;
    node->undo_rec = trx_purge_fetch_next_rec(&roll_ptr, &(node->reservation), node->heap);
    if (!node->undo_rec) {
        /* Purge completed for this query thread */
        thr->run_node = que_node_get_parent(node);
        return DB_SUCCESS;
    }

    node->roll_ptr = roll_ptr;

    ibool purge_needed;
    ibool updated_extern = FALSE;
    if (node->undo_rec == &trx_purge_dummy_rec) {
        purge_needed = FALSE;
    } else {
        purge_needed = row_purge_parse_undo_rec(node, &updated_extern, thr);
        /* If purge_needed == TRUE, we must also remember to unfreeze data dictionary! */
    }

    if (purge_needed) {
        dict_index_t* clust_index = dict_table_get_first_index(node->table);
        node->found_clust = FALSE;
        node->index = dict_table_get_next_index(clust_index);

        if (node->rec_type == TRX_UNDO_DEL_MARK_REC) {
            row_purge_del_mark(node);
        } else if (updated_extern || node->rec_type == TRX_UNDO_UPD_EXIST_REC) {
            row_purge_upd_exist_or_extern(node);
        }

        if (node->found_clust) {
            btr_pcur_close(&(node->pcur));
        }

        dict_unfreeze_data_dictionary(trx);
    }

    /* Do some cleanup */
    trx_purge_rec_release(node->reservation);
    mem_heap_empty(node->heap);

    thr->run_node = node;
    return DB_SUCCESS;
}

/// \brief Does the purge operation for a single undo log record.
/// \details This is a high-level function used in an SQL execution graph.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* row_purge_step(que_thr_t* thr)
{
    ut_ad(thr);
    purge_node_t* node = thr->run_node;
    ut_ad(que_node_get_type(node) == QUE_NODE_PURGE);

    ulint err = row_purge(node, thr);
    ut_ad(err == DB_SUCCESS);

    return thr;
}
