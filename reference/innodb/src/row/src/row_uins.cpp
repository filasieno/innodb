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

/// \file row_uins.cpp
/// \brief Fresh insert undo
/// \details Originally created on 2/25/1997 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "row_uins.hpp"

#ifdef IB_DO_NOT_INLINE
#include "row_uins.inl"
#endif

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

/// \brief Removes a clustered index record.
/// \details The pcur in node was positioned on the record, now it is detached.
/// \param [in] node undo node
/// \return DB_SUCCESS or DB_OUT_OF_FILE_SPACE
static ulint row_undo_ins_remove_clust_rec(undo_node_t* node);

/// \brief Removes a secondary index entry if found.
/// \param [in] mode BTR_MODIFY_LEAF or BTR_MODIFY_TREE, depending on whether we wish optimistic or pessimistic descent down the index tree
/// \param [in] index index
/// \param [in] entry index entry to remove
/// \return DB_SUCCESS, DB_FAIL, or DB_OUT_OF_FILE_SPACE
static ulint row_undo_ins_remove_sec_low(ulint mode, dict_index_t* index, dtuple_t* entry);

/// \brief Removes a secondary index entry from the index if found.
/// \details Tries first optimistic, then pessimistic descent down the tree.
/// \param [in] index index
/// \param [in] entry index entry to insert
/// \return DB_SUCCESS or DB_OUT_OF_FILE_SPACE
static ulint row_undo_ins_remove_sec(dict_index_t* index, dtuple_t* entry);

/// \brief Parses the row reference and other info in a fresh insert undo record.
/// \param [in] recovery recovery flag
/// \param [in,out] node row undo node
static void row_undo_ins_parse_undo_rec(ib_recovery_t recovery, undo_node_t* node);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

#include "dict_dict.hpp"
#include "dict_boot.hpp"
#include "dict_crea.hpp"
#include "trx_undo.hpp"
#include "trx_roll.hpp"
#include "btr_btr.hpp"
#include "mach_data.hpp"
#include "row_undo.hpp"
#include "row_vers.hpp"
#include "trx_trx.hpp"
#include "trx_rec.hpp"
#include "row_row.hpp"
#include "row_upd.hpp"
#include "que_que.hpp"
#include "ibuf_ibuf.hpp"
#include "log_log.hpp"

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

static ulint row_undo_ins_remove_clust_rec(undo_node_t* node)
{
	ibool      success;
	ulint      n_tries = 0;
	mtr_t      mtr;
	mtr_start(&mtr);
	success = btr_pcur_restore_position(BTR_MODIFY_LEAF, &(node->pcur), &mtr);
	ut_a(success);
	if (ut_dulint_cmp(node->table->id, DICT_INDEXES_ID) == 0) {
		ut_ad(node->trx->dict_operation_lock_mode == RW_X_LATCH);
		dict_drop_index_tree(btr_pcur_get_rec(&(node->pcur)), &mtr);
		mtr_commit(&mtr);
		mtr_start(&mtr);
		success = btr_pcur_restore_position(BTR_MODIFY_LEAF, &(node->pcur), &mtr);
		ut_a(success);
	}
	btr_cur_t* btr_cur = btr_pcur_get_btr_cur(&(node->pcur));
	success = btr_cur_optimistic_delete(btr_cur, &mtr);

	btr_pcur_commit_specify_mtr(&(node->pcur), &mtr);
	if (success) {
		trx_undo_rec_release(node->trx, node->undo_no);
		return DB_SUCCESS;
	}
retry:
	mtr_start(&mtr);
	success = btr_pcur_restore_position(BTR_MODIFY_TREE, &(node->pcur), &mtr);
	ut_a(success);
	ulint err;
	btr_cur_pessimistic_delete(&err, FALSE, btr_cur, trx_is_recv(node->trx) ? RB_RECOVERY : RB_NORMAL, &mtr);
	if (err == DB_OUT_OF_FILE_SPACE && n_tries < BTR_CUR_RETRY_DELETE_N_TIMES) {
		btr_pcur_commit_specify_mtr(&(node->pcur), &mtr);
		n_tries++;
		os_thread_sleep(BTR_CUR_RETRY_SLEEP_TIME);
		goto retry;
	}
	btr_pcur_commit_specify_mtr(&(node->pcur), &mtr);
	trx_undo_rec_release(node->trx, node->undo_no);
	return(err);
}

static ulint row_undo_ins_remove_sec_low(ulint mode, dict_index_t* index, dtuple_t* entry)
{
	btr_pcur_t pcur;
	ibool      success;
	mtr_t      mtr;
	log_free_check();
	mtr_start(&mtr);
	ibool found = row_search_index_entry(index, entry, mode, &pcur, &mtr);
	btr_cur_t* btr_cur = btr_pcur_get_btr_cur(&pcur);
	if (!found) {
		btr_pcur_close(&pcur);
		mtr_commit(&mtr);
		return DB_SUCCESS;
	}
	ulint err;
	if (mode == BTR_MODIFY_LEAF) {
		success = btr_cur_optimistic_delete(btr_cur, &mtr);
		if (success) {
			err = DB_SUCCESS;
		} else {
			err = DB_FAIL;
		}
	} else {
		ut_ad(mode == BTR_MODIFY_TREE);
		ut_ad(!dict_index_is_clust(index));
		btr_cur_pessimistic_delete(&err, FALSE, btr_cur, RB_NORMAL, &mtr);
	}
	btr_pcur_close(&pcur);
	mtr_commit(&mtr);
	return(err);
}

static ulint row_undo_ins_remove_sec(dict_index_t* index, dtuple_t* entry)
{
	ulint n_tries = 0;
	ulint err = row_undo_ins_remove_sec_low(BTR_MODIFY_LEAF, index, entry);
	if (err == DB_SUCCESS) {
		return(err);
	}
retry:
	err = row_undo_ins_remove_sec_low(BTR_MODIFY_TREE, index, entry);
	if (err != DB_SUCCESS && n_tries < BTR_CUR_RETRY_DELETE_N_TIMES) {
		n_tries++;
		os_thread_sleep(BTR_CUR_RETRY_SLEEP_TIME);
		goto retry;
	}
	return(err);
}

static void row_undo_ins_parse_undo_rec(ib_recovery_t recovery, undo_node_t* node)
{
	dict_index_t* clust_index;
	byte*         ptr;
	undo_no_t     undo_no;
	dulint        table_id;
	ulint         type;
	ulint         dummy;
	ibool         dummy_extern;

	ut_ad(node);

	ptr = trx_undo_rec_get_pars(node->undo_rec, &type, &dummy, &dummy_extern, &undo_no, &table_id);
	ut_ad(type == TRX_UNDO_INSERT_REC);
	node->rec_type = type;

	node->update = NULL;
	node->table = dict_table_get_on_id(srv_force_recovery, table_id, node->trx);

	/* Skip the UNDO if we can't find the table or the .ibd file. */
	if (IB_UNLIKELY(node->table == NULL)) {
	} else if (IB_UNLIKELY(node->table->ibd_file_missing)) {
		node->table = NULL;
	} else {
		clust_index = dict_table_get_first_index(node->table);

		if (clust_index != NULL) {
			ptr = trx_undo_rec_get_row_ref(ptr, clust_index, &node->ref, node->heap);
		} else {
			ut_print_timestamp(state->stream);
			ib_log(state, "  InnoDB: table ");
			ut_print_name(state->stream, node->trx, TRUE, node->table->name);
			ib_log(state, " has no indexes, ignoring the table\n");

			node->table = NULL;
		}
	}
}

/// \brief Undoes a fresh insert of a row to a table.
/// \details A fresh insert means that the same clustered index unique key did not have any record, even delete marked, at the time of the insert. InnoDB is eager in a rollback: if it figures out that an index record will be removed in the purge anyway, it will remove it in the rollback.
/// \param [in] node undo node
/// \return DB_SUCCESS or DB_OUT_OF_FILE_SPACE
IB_INTERN ulint row_undo_ins(undo_node_t* node)
{
	ut_ad(node);
	ut_ad(node->state == UNDO_NODE_INSERT);

	row_undo_ins_parse_undo_rec(srv_force_recovery, node);

	if (!node->table || !row_undo_search_clust_to_pcur(node)) {
		trx_undo_rec_release(node->trx, node->undo_no);

		return DB_SUCCESS;
	}

	/* Iterate over all the indexes and undo the insert.*/

	/* Skip the clustered index (the first index) */
	node->index = dict_table_get_next_index(dict_table_get_first_index(node->table));

	while (node->index != NULL) {
		dtuple_t* entry = row_build_index_entry(node->row, node->ext, node->index, node->heap);
		ulint     err;
		if (IB_UNLIKELY(!entry)) {
			/* The database must have crashed after
			inserting a clustered index record but before
			writing all the externally stored columns of
			that record.  Because secondary index entries
			are inserted after the clustered index record,
			we may assume that the secondary index record
			does not exist.  However, this situation may
			only occur during the rollback of incomplete
			transactions. */
			ut_a(trx_is_recv(node->trx));
		} else {
			err = row_undo_ins_remove_sec(node->index, entry);

			if (err != DB_SUCCESS) {

				return(err);
			}
		}

		node->index = dict_table_get_next_index(node->index);
	}

	return row_undo_ins_remove_clust_rec(node);
}
