// Copyright (c) 1996, 2025, Innobase Oy. All Rights Reserved.
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

/// \file dict_crea.cpp
/// \brief Database object creation
/// \details Originally created by Heikki Tuuri in 1/8/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "dict_crea.hpp"

#include "btr_pcur.hpp"
#include "btr_btr.hpp"
#include "page_page.hpp"
#include "mach_data.hpp"
#include "dict_boot.hpp"
#include "dict_dict.hpp"
#include "que_que.hpp"
#include "row_ins.hpp"
#include "pars_pars.hpp"
#include "trx_roll.hpp"
#include "usr_sess.hpp"
#include "ut_vec.hpp"
#include "log_log.hpp"
#include "lock_lock.hpp"
#include "ddl_ddl.hpp"

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

static dtuple_t* dict_create_add_foreign_field_to_dictionary(ulint field_nr, dict_table_t* table, dict_foreign_t* foreign, trx_t* trx);
static dtuple_t* dict_create_add_foreign_to_dictionary(ulint* id_nr, dict_table_t* table, dict_foreign_t* foreign, trx_t* trx);
static dtuple_t* dict_create_search_tuple(const dtuple_t* tuple, mem_heap_t* heap);
static dtuple_t* dict_create_sys_columns_tuple(const dict_table_t* table, ulint i, mem_heap_t* heap);
static dtuple_t* dict_create_sys_fields_tuple(const dict_index_t* index, ulint i, mem_heap_t* heap);
static dtuple_t* dict_create_sys_indexes_tuple(const dict_index_t* index, mem_heap_t* heap);
static dtuple_t* dict_create_sys_tables_tuple(const dict_table_t* table, mem_heap_t* heap);
static ulint     dict_build_col_def_step(tab_node_t* node);
static ulint     dict_build_field_def_step(ind_node_t* node);
static ulint     dict_build_index_def_step(que_thr_t* thr, ind_node_t* node);
static ulint     dict_build_table_def_step(que_thr_t* thr, tab_node_t* node);
static ulint     dict_create_index_tree_step(ind_node_t* node);
static ulint     dict_foreign_eval_sql(pars_info_t* info, const char* sql, dict_table_t* table, dict_foreign_t* foreign, trx_t* trx);

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Based on a table object, this function builds the entry to be inserted in the SYS_TABLES system table.
/// \return the tuple which should be inserted
/// \param [in] table table
/// \param [in] heap memory heap from which the memory for the built tuple is allocated
static dtuple_t* dict_create_sys_tables_tuple(const dict_table_t* table, mem_heap_t* heap)
{
	static_assert(DICT_TF_COMPACT == 1, "DICT_TF_COMPACT != 1");
	ut_ad(table);
	ut_ad(heap);
	dict_table_t* sys_tables = dict_sys->sys_tables;
	dtuple_t* entry = dtuple_create(heap, 8 + DATA_N_SYS_COLS);
	dict_table_copy_types(entry, sys_tables);
	// 0: NAME -----------------------------
	dfield_t* dfield = dtuple_get_nth_field(entry, 0); // NAME
	dfield_set_data(dfield, table->name, ut_strlen(table->name));
	// 3: ID -------------------------------
	dfield = dtuple_get_nth_field(entry, 1); // ID
	byte* ptr = mem_heap_alloc(heap, 8);
	mach_write_to_8(ptr, table->id);
	dfield_set_data(dfield, ptr, 8);
	// 4: N_COLS ---------------------------
	dfield = dtuple_get_nth_field(entry, 2); // N_COLS
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, table->n_def | ((table->flags & DICT_TF_COMPACT) << 31));
	dfield_set_data(dfield, ptr, 4);
	// 5: TYPE -----------------------------
	dfield = dtuple_get_nth_field(entry, 3); // TYPE
	ptr = mem_heap_alloc(heap, 4);
	if (table->flags & (~DICT_TF_COMPACT & ~(~0 << DICT_TF_BITS))) {
		ut_a(table->flags & DICT_TF_COMPACT);
		ut_a(dict_table_get_format(table) >= DICT_TF_FORMAT_ZIP);
		ut_a((table->flags & DICT_TF_ZSSIZE_MASK)
		     <= (DICT_TF_ZSSIZE_MAX << DICT_TF_ZSSIZE_SHIFT));
		ut_a(!(table->flags & (~0 << DICT_TF2_BITS)));
		mach_write_to_4(ptr, table->flags & ~(~0 << DICT_TF_BITS));
	} else {
		mach_write_to_4(ptr, DICT_TABLE_ORDINARY);
	}
	dfield_set_data(dfield, ptr, 4);
	// 6: MIX_ID (obsolete) ---------------------------
	dfield = dtuple_get_nth_field(entry, 4); // MIX_ID
	ptr = mem_heap_zalloc(heap, 8);
	dfield_set_data(dfield, ptr, 8);
	// 7: MIX_LEN (additional flags) --------------------------
	dfield = dtuple_get_nth_field(entry, 5); // MIX_LEN
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, table->flags >> DICT_TF2_SHIFT);
	dfield_set_data(dfield, ptr, 4);
	// 8: CLUSTER_NAME ---------------------
	dfield = dtuple_get_nth_field(entry, 6); // CLUSTER_NAME
	dfield_set_null(dfield); // not supported
	// 9: SPACE ----------------------------
	dfield = dtuple_get_nth_field(entry, 7); // SPACE
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, table->space);
	dfield_set_data(dfield, ptr, 4);
	return entry;
}

/// \brief Based on a table object, this function builds the entry to be inserted in the SYS_COLUMNS system table.
/// \return the tuple which should be inserted
/// \param [in] table table
/// \param [in] i column number
/// \param [in] heap memory heap from which the memory for the built tuple is allocated
static dtuple_t* dict_create_sys_columns_tuple(const dict_table_t* table, ulint i, mem_heap_t* heap)
{
	dict_table_t*		sys_columns;
	dtuple_t*		entry;
	const dict_col_t* column;
	dfield_t* dfield;
	byte* ptr;
	const char* col_name;
	ut_ad(table);
	ut_ad(heap);
	column = dict_table_get_nth_col(table, i);
	sys_columns = dict_sys->sys_columns;
	entry = dtuple_create(heap, 7 + DATA_N_SYS_COLS);
	dict_table_copy_types(entry, sys_columns);

	// 0: TABLE_ID -----------------------
	dfield = dtuple_get_nth_field(entry, 0); // 0 -> TABLE_ID
	ptr = mem_heap_alloc(heap, 8);
	mach_write_to_8(ptr, table->id);
	dfield_set_data(dfield, ptr, 8);
	// 1: POS ----------------------------
	dfield = dtuple_get_nth_field(entry, 1); // 1 -> POS
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, i);
	dfield_set_data(dfield, ptr, 4);
	// 4: NAME ---------------------------
	dfield = dtuple_get_nth_field(entry, 2); // 2-> NAME
	col_name = dict_table_get_col_name(table, i);
	dfield_set_data(dfield, col_name, ut_strlen(col_name));
	// 3: MTYPE -------------------------- 
	dfield = dtuple_get_nth_field(entry, 3); //3 -> MTYPE
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, column->mtype);
	dfield_set_data(dfield, ptr, 4);
	// 6: PRTYPE -------------------------
	dfield = dtuple_get_nth_field(entry, 4); // 4 -> PRTYPE
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, column->prtype);
	dfield_set_data(dfield, ptr, 4);
	// 7: LEN ----------------------------
	dfield = dtuple_get_nth_field(entry, 5); // 5 -> LEN
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, column->len);
	dfield_set_data(dfield, ptr, 4);
	// 8: PREC ---------------------------
	dfield = dtuple_get_nth_field(entry, 6); 6 -> PREC
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, 0); // 0 unused
	dfield_set_data(dfield, ptr, 4);
	return entry;
}

/// \brief Builds a table definition to insert.
/// \return DB_SUCCESS or error code
/// \param [in] thr query thread
/// \param [in] node table create node
static ulint dict_build_table_def_step(que_thr_t* thr, tab_node_t* node)
{
	ut_ad(mutex_own(&(dict_sys->mutex)));
	dict_table_t* table = node->table;
	table->id = dict_hdr_get_new_id(DICT_HDR_TABLE_ID);
	thr_get_trx(thr)->table_id = table->id;
	if (srv_file_per_table) {
		// We create a new single-table tablespace for the table. We initially let it be 4 pages:
		//  - page 0 is the fsp header and an extent descriptor page,
		//  - page 1 is an ibuf bitmap page,
		//  - page 2 is the first inode page,
		//  - page 3 will contain the root of the clustered index of the table we create here.
		ulint space = 0;	/* reset to zero for the call below */
		const char* path_or_name;
		ibool is_path;
		if (table->dir_path_of_temp_table) {
			// We place tables created with CREATE TEMPORARY TABLE in the configured tmp dir.
			path_or_name = table->dir_path_of_temp_table;
			is_path = TRUE;
		} else {
			path_or_name = table->name;
			is_path = FALSE;
		}
		ut_ad(dict_table_get_format(table) <= DICT_TF_FORMAT_MAX);
		ut_ad(!dict_table_zip_size(table)
		      || dict_table_get_format(table) >= DICT_TF_FORMAT_ZIP);
		ulint flags = table->flags & ~(~0 << DICT_TF_BITS);
		ulint error = fil_create_new_single_table_tablespace(
			&space, path_or_name, is_path,
			flags == DICT_TF_COMPACT ? 0 : flags,
			FIL_IBD_FILE_INITIAL_SIZE);
		table->space = (unsigned int) space;
		if (error != DB_SUCCESS) {
			return error;
		}
		mtr_t mtr;
		mtr_start(&mtr);
		fsp_header_init(table->space, FIL_IBD_FILE_INITIAL_SIZE, &mtr);
		mtr_commit(&mtr);
	} else {
		// Create in the system tablespace: disallow new features
		table->flags &= (~0 << DICT_TF_BITS) | DICT_TF_COMPACT;
	}
	dtuple_t* row = dict_create_sys_tables_tuple(table, node->heap);
	row_ins_node_set_new_row(node->tab_def, row);
	return DB_SUCCESS;
}

/// \brief Builds a column definition to insert.
/// \return DB_SUCCESS
/// \param [in] node table create node
static ulint dict_build_col_def_step(tab_node_t* node)
{
	dtuple_t* row = dict_create_sys_columns_tuple(node->table, node->col_no, node->heap);
	row_ins_node_set_new_row(node->col_def, row);

	return DB_SUCCESS;
}

/// \brief Based on an index object, this function builds the entry to be inserted in the SYS_INDEXES system table.
/// \return the tuple which should be inserted
/// \param [in] index index
/// \param [in] heap memory heap from which the memory for the built tuple is allocated
static dtuple_t* dict_create_sys_indexes_tuple(const dict_index_t* index, mem_heap_t* heap)
{
	static_assert(DICT_SYS_INDEXES_SPACE_NO_FIELD == 7, "DICT_SYS_INDEXES_SPACE_NO_FIELD != 7");
	static_assert(DICT_SYS_INDEXES_PAGE_NO_FIELD == 8, "DICT_SYS_INDEXES_PAGE_NO_FIELD != 8");

	dict_table_t*	sys_indexes;
	dict_table_t*	table;
	dtuple_t*	entry;
	dfield_t*	dfield;
	byte*		ptr;
	ut_ad(mutex_own(&(dict_sys->mutex)));
	ut_ad(index);
	ut_ad(heap);
	sys_indexes = dict_sys->sys_indexes;
	table = dict_table_get_low(index->table_name);
	entry = dtuple_create(heap, 7 + DATA_N_SYS_COLS);
	dict_table_copy_types(entry, sys_indexes);	
	// 0: TABLE_ID -----------------------
	dfield = dtuple_get_nth_field(entry, 0); // TABLE_ID
	ptr = mem_heap_alloc(heap, 8);
	mach_write_to_8(ptr, table->id);
	dfield_set_data(dfield, ptr, 8);
	// 1: ID ----------------------------
	dfield = dtuple_get_nth_field(entry, 1); // ID
	ptr = mem_heap_alloc(heap, 8);
	mach_write_to_8(ptr, index->id);
	dfield_set_data(dfield, ptr, 8);
	// 4: NAME --------------------------
	dfield = dtuple_get_nth_field(entry, 2); // NAME	
	dfield_set_data(dfield, index->name, ut_strlen(index->name));
	// 5: N_FIELDS ----------------------
	dfield = dtuple_get_nth_field(entry, 3); // N_FIELDS
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, index->n_fields);
	dfield_set_data(dfield, ptr, 4);
	// 6: TYPE --------------------------
	dfield = dtuple_get_nth_field(entry, 4); // TYPE
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, index->type);
	dfield_set_data(dfield, ptr, 4);
	// 7: SPACE --------------------------
	dfield = dtuple_get_nth_field(entry, 5); // SPACE
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, index->space);
	dfield_set_data(dfield, ptr, 4);
	// 8: PAGE_NO --------------------------
	dfield = dtuple_get_nth_field(entry, 6); // PAGE_NO
	ptr = mem_heap_alloc(heap, 4);
	mach_write_to_4(ptr, FIL_NULL);
	dfield_set_data(dfield, ptr, 4);

	return entry;
}

/// \brief Based on an index object, this function builds the entry to be inserted in the SYS_FIELDS system table.
/// \return the tuple which should be inserted
/// \param [in] index index
/// \param [in] i field number
/// \param [in] heap memory heap from which the memory for the built tuple is allocated
static dtuple_t* dict_create_sys_fields_tuple(const dict_index_t* index, ulint i, mem_heap_t* heap)
{
	dict_table_t* sys_fields;
	dtuple_t* entry;
	dict_field_t* field;
	dfield_t* dfield;
	byte* ptr;
	ibool index_contains_column_prefix_field	= FALSE;
	ulint j;
	ut_ad(index);
	ut_ad(heap);
	for (j = 0; j < index->n_fields; j++) {
		if (dict_index_get_nth_field(index, j)->prefix_len > 0) {
			index_contains_column_prefix_field = TRUE;
			break;
		}
	}
	field = dict_index_get_nth_field(index, i);
	sys_fields = dict_sys->sys_fields;
	entry = dtuple_create(heap, 3 + DATA_N_SYS_COLS);
	dict_table_copy_types(entry, sys_fields);
	// 0: INDEX_ID -----------------------
	dfield = dtuple_get_nth_field(entry, 0); // 0 -> INDEX_ID
	ptr = mem_heap_alloc(heap, 8);
	mach_write_to_8(ptr, index->id);
	dfield_set_data(dfield, ptr, 8);
	// 1: POS + PREFIX LENGTH ----------------------------
	dfield = dtuple_get_nth_field(entry, 1); //1 -> POS
	ptr = mem_heap_alloc(heap, 4);
	if (index_contains_column_prefix_field) {
		// If there are column prefix fields in the index, then we store the number of the field to the 2 HIGH bytes and the prefix length to the 2 low bytes, 
		mach_write_to_4(ptr, (i << 16) + field->prefix_len);
	} else {
		// Else we store the number of the field to the 2 LOW bytes. This is to keep the storage format compatible with InnoDB versions < 4.0.14. 
		mach_write_to_4(ptr, i);
	}
	dfield_set_data(dfield, ptr, 4);
	// 4: COL_NAME -------------------------
	dfield = dtuple_get_nth_field(entry, 2/*COL_NAME*/);
	dfield_set_data(dfield, field->name, ut_strlen(field->name));
	// ---------------------------------
	return entry;
}

/// \brief Creates the tuple with which the index entry is searched for writing the index tree root page number, if such a tree is created.
/// \return the tuple for search
/// \param [in] tuple the tuple inserted in the SYS_INDEXES table
/// \param [in] heap memory heap from which the memory for the built tuple is allocated
static dtuple_t* dict_create_search_tuple(const dtuple_t* tuple, mem_heap_t* heap)
{
	dtuple_t*	search_tuple;
	const dfield_t*	field1;
	dfield_t*	field2;
	ut_ad(tuple && heap);
	search_tuple = dtuple_create(heap, 2);
	field1 = dtuple_get_nth_field(tuple, 0);
	field2 = dtuple_get_nth_field(search_tuple, 0);
	dfield_copy(field2, field1);
	field1 = dtuple_get_nth_field(tuple, 1);
	field2 = dtuple_get_nth_field(search_tuple, 1);
	dfield_copy(field2, field1);
	ut_ad(dtuple_validate(search_tuple));
	return search_tuple;
}

/// \brief Builds an index definition row to insert.
/// \return DB_SUCCESS or error code
/// \param [in] thr query thread
/// \param [in] node index create node
static ulint dict_build_index_def_step(que_thr_t* thr, ind_node_t* node)
{
	ut_ad(mutex_own(&(dict_sys->mutex)));
	trx_t* trx = thr_get_trx(thr);
	dict_index_t* index = node->index;
	dict_table_t* table = dict_table_get_low(index->table_name);
	if (table == NULL) {
		return DB_TABLE_NOT_FOUND;
	}
	trx->table_id = table->id;
	node->table = table;
	ut_ad((UT_LIST_GET_LEN(table->indexes) > 0) || dict_index_is_clust(index));
	index->id = dict_hdr_get_new_id(DICT_HDR_INDEX_ID);
	// Inherit the space id from the table; we store all indexes of a table in the same tablespace */
	index->space = table->space;
	node->page_no = FIL_NULL;
	dtuple_t* row = dict_create_sys_indexes_tuple(index, node->heap);
	node->ind_row = row;
	row_ins_node_set_new_row(node->ind_def, row);
	// Note that the index was created by this transaction. 
	index->trx_id = (ib_uint64_t) ut_conv_dulint_to_longlong(trx->id);
	return DB_SUCCESS;
}

/// \brief Builds a field definition row to insert.
/// \return DB_SUCCESS
/// \param [in] node index create node
static ulint dict_build_field_def_step(ind_node_t* node)
{
	dict_index_t* index = node->index;
	dtuple_t* row = dict_create_sys_fields_tuple(index, node->field_no, node->heap);
	row_ins_node_set_new_row(node->field_def, row);
	return DB_SUCCESS;
}

/// \brief Creates an index tree for the index if it is not a member of a cluster.
/// \return DB_SUCCESS or DB_OUT_OF_FILE_SPACE
/// \param [in] node index create node
static ulint dict_create_index_tree_step(ind_node_t* node)
{
	ut_ad(mutex_own(&(dict_sys->mutex)));
	dict_index_t* index = node->index;
	dict_table_t* table = node->table;
	dict_table_t* sys_indexes = dict_sys->sys_indexes;
	// Run a mini-transaction in which the index tree is allocated for the index and its root address is written to the index entry in sys_indexes
	mtr_t mtr;
	mtr_start(&mtr);
	dtuple_t* search_tuple = dict_create_search_tuple(node->ind_row, node->heap);
	btr_pcur_t pcur;
	btr_pcur_open(UT_LIST_GET_FIRST(sys_indexes->indexes), search_tuple, PAGE_CUR_L, BTR_MODIFY_LEAF,&pcur, &mtr);
	btr_pcur_move_to_next_user_rec(&pcur, &mtr);
	ulint zip_size = dict_table_zip_size(index->table);
	node->page_no = btr_create(index->type, index->space, zip_size, index->id, index, &mtr);
	// printf("Created a new index tree in space %lu root page %lu\n", index->space, index->page_no);
	page_rec_write_index_page_no(btr_pcur_get_rec(&pcur), DICT_SYS_INDEXES_PAGE_NO_FIELD, node->page_no, &mtr);
	btr_pcur_close(&pcur);
	mtr_commit(&mtr);
	if (node->page_no == FIL_NULL) {
		return DB_OUT_OF_FILE_SPACE;
	}
	return DB_SUCCESS;
}

/// \brief Drops the index tree associated with a row in SYS_INDEXES table.
/// \param [in,out] rec record in the clustered index of SYS_INDEXES table
/// \param [in] mtr mtr having the latch on the record page
IB_INTERN void dict_drop_index_tree(rec_t* rec, mtr_t* mtr)
{
	ut_ad(mutex_own(&(dict_sys->mutex)));
	ut_a(!dict_table_is_comp(dict_sys->sys_indexes));
	ulint len;
	const byte* ptr = rec_get_nth_field_old(rec, DICT_SYS_INDEXES_PAGE_NO_FIELD, &len);
	ut_ad(len == 4);
	ulint root_page_no = mtr_read_ulint(ptr, MLOG_4BYTES, mtr);
	if (root_page_no == FIL_NULL) {
		// The tree has already been freed
		return;
	}
	ptr = rec_get_nth_field_old(rec, DICT_SYS_INDEXES_SPACE_NO_FIELD, &len);
	ut_ad(len == 4);
	ulint space = mtr_read_ulint(ptr, MLOG_4BYTES, mtr);
	ulint zip_size = fil_space_get_zip_size(space);
	if (IB_UNLIKELY(zip_size == ULINT_UNDEFINED)) {
		// It is a single table tablespace and the .ibd file is missing: do nothing
		return;
	}
	// We free all the pages but the root page first; this operation may span several mini-transactions
	btr_free_but_not_root(space, zip_size, root_page_no);
	// Then we free the root page in the same mini-transaction where we write FIL_NULL to the appropriate field in the SYS_INDEXES record: this mini-transaction marks the B-tree totally freed */
	// printf("Dropping index tree in space %lu root page %lu\n", space, root_page_no); 
	btr_free_root(space, zip_size, root_page_no, mtr);
	page_rec_write_index_page_no(rec, DICT_SYS_INDEXES_PAGE_NO_FIELD, FIL_NULL, mtr);
}

/// \brief Truncates the index tree associated with a row in SYS_INDEXES table.
/// \return new root page number, or FIL_NULL on failure
/// \param [in] table the table the index belongs to
/// \param [in] space 0=truncate, nonzero=create the index tree in the given tablespace
/// \param [in,out] pcur persistent cursor pointing to record in the clustered index of SYS_INDEXES table. The cursor may be repositioned in this call.
/// \param [in] mtr mtr having the latch on the record page. The mtr may be committed and restarted in this call.
IB_INTERN ulint dict_truncate_index_tree(dict_table_t* table, ulint space, btr_pcur_t* pcur, mtr_t* mtr)
{
	ibool drop = !space;

	ut_ad(mutex_own(&(dict_sys->mutex)));
	ut_a(!dict_table_is_comp(dict_sys->sys_indexes));
	rec_t* rec = btr_pcur_get_rec(pcur);
	ulint len;
	const byte* ptr = rec_get_nth_field_old(rec, DICT_SYS_INDEXES_PAGE_NO_FIELD, &len);

	ut_ad(len == 4);
	ulint root_page_no = mtr_read_ulint(ptr, MLOG_4BYTES, mtr);
	if (drop && root_page_no == FIL_NULL) {
		// The tree has been freed
		ut_print_timestamp(state->stream);
		ib_log(state, "  InnoDB: Trying to TRUNCATE a missing index of table %s!\n", table->name);
		drop = FALSE;
	}

	ptr = rec_get_nth_field_old(rec, DICT_SYS_INDEXES_SPACE_NO_FIELD, &len);
	ut_ad(len == 4);
	if (drop) {
		space = mtr_read_ulint(ptr, MLOG_4BYTES, mtr);
	}

	ulint zip_size = fil_space_get_zip_size(space);
	if (IB_UNLIKELY(zip_size == ULINT_UNDEFINED)) {
		// It is a single table tablespace and the .ibd file is missing: do nothing
		ut_print_timestamp(state->stream);
		ib_log(state, "  InnoDB: Trying to TRUNCATE a missing .ibd file of table %s!\n", table->name);
		return FIL_NULL;
	}

	ptr = rec_get_nth_field_old(rec, DICT_SYS_INDEXES_TYPE_FIELD, &len);
	ut_ad(len == 4);
	ulint type = mach_read_from_4(ptr);

	ptr = rec_get_nth_field_old(rec, 1, &len);
	ut_ad(len == 8);
	dulint index_id = mach_read_from_8(ptr);

	if (!drop) {

		goto create;
	}

	// We free all the pages but the root page first; this operation may span several mini-transactions
	btr_free_but_not_root(space, zip_size, root_page_no);

	// Then we free the root page in the same mini-transaction where  write its new root page number to the appropriate field in the SYS_INDEXES record: this mini-transaction marks the B-tree totally truncated
	btr_page_get(space, zip_size, root_page_no, RW_X_LATCH, mtr);
	btr_free_root(space, zip_size, root_page_no, mtr);
create:
	// We will temporarily write FIL_NULL to the PAGE_NO field in SYS_INDEXES, so that the database will not get into an inconsistent state in case it crashes between the mtr_commit() below and the following mtr_commit() call. 
	page_rec_write_index_page_no(rec, DICT_SYS_INDEXES_PAGE_NO_FIELD, FIL_NULL, mtr);

	// We will need to commit the mini-transaction in order to avoid deadlocks in the btr_create() call, because otherwise we would be freeing and allocating pages in the same mini-transaction. 
	btr_pcur_store_position(pcur, mtr);
	mtr_commit(mtr);

	mtr_start(mtr);
	btr_pcur_restore_position(BTR_MODIFY_LEAF, pcur, mtr);

	/* Find the index corresponding to this SYS_INDEXES record. */
	for (dict_index_t* index = UT_LIST_GET_FIRST(table->indexes);
	     index;
	     index = UT_LIST_GET_NEXT(indexes, index)) {
		if (!ut_dulint_cmp(index->id, index_id)) {
			root_page_no = btr_create(type, space, zip_size,
						  index_id, index, mtr);
			index->page = (unsigned int) root_page_no;
			return root_page_no;
		}
	}

	ut_print_timestamp(state->stream);
	ib_log(state,"  InnoDB: Index %lu %lu of table %s is missing\n InnoDB: from the data dictionary during TRUNCATE!\n", ut_dulint_get_high(index_id), ut_dulint_get_low(index_id), table->name);
	return FIL_NULL;
}

/// \brief Creates a table create graph.
/// \return own: table create node
/// \param [in] table table to create, built as a memory data structure
/// \param [in] heap heap where created
/// \param [in] commit if TRUE commit transaction
IB_INTERN tab_node_t* tab_create_graph_create(dict_table_t* table, mem_heap_t* heap, ibool commit)
{
	tab_node_t* node = mem_heap_alloc(heap, sizeof(tab_node_t));
	node->common.type = QUE_NODE_CREATE_TABLE;
	node->table = table;
	node->state = TABLE_BUILD_TABLE_DEF;
	node->heap = mem_heap_create(256);
	node->tab_def = row_ins_node_create(INS_DIRECT, dict_sys->sys_tables, heap);
	node->tab_def->common.parent = node;
	node->col_def = row_ins_node_create(INS_DIRECT, dict_sys->sys_columns, heap);
	node->col_def->common.parent = node;
	if (commit) {
		node->commit_node = commit_node_create(heap);
		node->commit_node->common.parent = node;
	} else {
		node->commit_node = NULL;
	}
	return node;
}

/// \brief Creates an index create graph.
/// \return own: index create node
/// \param [in] index index to create, built as a memory data structure
/// \param [in] heap heap where created
/// \param [in] commit TRUE if transaction should be commit
IB_INTERN ind_node_t* ind_create_graph_create(dict_index_t* index, mem_heap_t* heap, ibool commit)
{
	ind_node_t* node = mem_heap_alloc(heap, sizeof(ind_node_t));
	node->common.type = QUE_NODE_CREATE_INDEX;
	node->index = index;
	node->state = INDEX_BUILD_INDEX_DEF;
	node->page_no = FIL_NULL;
	node->heap = mem_heap_create(256);
	node->ind_def = row_ins_node_create(INS_DIRECT, dict_sys->sys_indexes, heap);
	node->ind_def->common.parent = node;
	node->field_def = row_ins_node_create(INS_DIRECT, dict_sys->sys_fields, heap);
	node->field_def->common.parent = node;
	if (commit) {
		node->commit_node = commit_node_create(heap);
		node->commit_node->common.parent = node;
	} else {
		node->commit_node = NULL;
	}
	return node;
}

/// \brief Creates a table. This is a high-level function used in SQL execution graphs.
/// \return query thread to run next or NULL
/// \param [in] thr query thread
IB_INTERN que_thr_t* dict_create_table_step(que_thr_t* thr)
{
	ulint err = DB_ERROR;
	ut_ad(thr);
	ut_ad(mutex_own(&(dict_sys->mutex)));
	trx_t* trx = thr_get_trx(thr);
	tab_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_CREATE_TABLE);
	if (thr->prev_node == que_node_get_parent(node)) {
		node->state = TABLE_BUILD_TABLE_DEF;
	}
	if (node->state == TABLE_BUILD_TABLE_DEF) {
		// DO THE CHECKS OF THE CONSISTENCY CONSTRAINTS HERE
		err = dict_build_table_def_step(thr, node);
		if (err != DB_SUCCESS) {
			goto function_exit;
		}
		node->state = TABLE_BUILD_COL_DEF;
		node->col_no = 0;
		thr->run_node = node->tab_def;
		return thr;
	}

	if (node->state == TABLE_BUILD_COL_DEF) {
		if (node->col_no < (node->table)->n_def) {
			err = dict_build_col_def_step(node);
			if (err != DB_SUCCESS) {
				goto function_exit;
			}
			node->col_no++;
			thr->run_node = node->col_def;
			return thr;
		} else {
			node->state = TABLE_COMMIT_WORK;
		}
	}

	if (node->state == TABLE_COMMIT_WORK) {
		// Table was correctly defined: do NOT commit the transaction (CREATE TABLE does NOT do an implicit commit of the current transaction)
		node->state = TABLE_ADD_TO_CACHE;
		// thr->run_node = node->commit_node; return thr;
	}
	if (node->state == TABLE_ADD_TO_CACHE) {
		dict_table_add_to_cache(node->table, node->heap);
		err = DB_SUCCESS;
	}

function_exit:
	trx->error_state = err;
	if (err == DB_SUCCESS) {
		// Ok: do nothing
	} else if (err == DB_LOCK_WAIT) {
		return NULL;
	} else {
		// SQL error detected
		return NULL;
	}
	thr->run_node = que_node_get_parent(node);
	return thr;
}

/// \brief Creates an index. This is a high-level function used in SQL execution graphs.
/// \return query thread to run next or NULL
/// \param [in] thr query thread
IB_INTERN que_thr_t* dict_create_index_step(que_thr_t* thr)
{
	ulint err = DB_ERROR;
	ut_ad(thr);
	ut_ad(mutex_own(&(dict_sys->mutex)));
	trx_t* trx = thr_get_trx(thr);
	ind_node_t* node = thr->run_node;
	ut_ad(que_node_get_type(node) == QUE_NODE_CREATE_INDEX);
	if (thr->prev_node == que_node_get_parent(node)) {
		node->state = INDEX_BUILD_INDEX_DEF;
	}
	if (node->state == INDEX_BUILD_INDEX_DEF) {
		// DO THE CHECKS OF THE CONSISTENCY CONSTRAINTS HERE
		err = dict_build_index_def_step(thr, node);
		if (err != DB_SUCCESS) {
			goto function_exit;
		}
		node->state = INDEX_BUILD_FIELD_DEF;
		node->field_no = 0;
		thr->run_node = node->ind_def;
		return thr;
	}
	if (node->state == INDEX_BUILD_FIELD_DEF) {
		if (node->field_no < (node->index)->n_fields) {
			err = dict_build_field_def_step(node);
			if (err != DB_SUCCESS) {
				goto function_exit;
			}
			node->field_no++;
			thr->run_node = node->field_def;
			return thr;
		} else {
			node->state = INDEX_ADD_TO_CACHE;
		}
	}
	if (node->state == INDEX_ADD_TO_CACHE) {
		dulint index_id = node->index->id;
		err = dict_index_add_to_cache(node->table, node->index, FIL_NULL, TRUE);
		node->index = dict_index_get_if_in_cache_low(index_id);
		ut_a(!node->index == (err != DB_SUCCESS));
		if (err != DB_SUCCESS) {
			goto function_exit;
		}
		node->state = INDEX_CREATE_INDEX_TREE;
	}

	if (node->state == INDEX_CREATE_INDEX_TREE) {
		err = dict_create_index_tree_step(node);
		if (err != DB_SUCCESS) {
			dict_index_remove_from_cache(node->table, node->index);
			node->index = NULL;
			goto function_exit;
		}
		node->index->page = node->page_no;
		node->state = INDEX_COMMIT_WORK;
	}
	if (node->state == INDEX_COMMIT_WORK) {
		// Index was correctly defined: do NOT commit the transaction (CREATE INDEX does NOT currently do an implicit commit of the current transaction) */
		node->state = INDEX_CREATE_INDEX_TREE;
		// thr->run_node = node->commit_node; return thr; 
	}

function_exit:
	trx->error_state = err;
	if (err == DB_SUCCESS) {
		//  Ok: do nothing 
	} else if (err == DB_LOCK_WAIT) {
		return NULL;
	} else {
		// SQL error detected
		return NULL;
	}
	thr->run_node = que_node_get_parent(node);
	return thr;
}

/// \brief Creates the foreign key constraints system tables inside InnoDB at database creation or database start if they are not found or are not of the right form.
/// \return DB_SUCCESS or error code
IB_INTERN ulint dict_create_or_check_foreign_constraint_tables(void)
{
	mutex_enter(&(dict_sys->mutex));

	dict_table_t* table1 = dict_table_get_low("SYS_FOREIGN");
	dict_table_t* table2 = dict_table_get_low("SYS_FOREIGN_COLS");

	if (table1 && table2 && UT_LIST_GET_LEN(table1->indexes) == 3 && UT_LIST_GET_LEN(table2->indexes) == 1) {

		// Foreign constraint system tables have already been created, and they are ok
		if (!table1->cached) {
			dict_mem_table_free(table1);
		}

		if (!table2->cached) {
			dict_mem_table_free(table2);
		}
		mutex_exit(&(dict_sys->mutex));
		return DB_SUCCESS;
	}

	mutex_exit(&(dict_sys->mutex));
	trx_t* trx = trx_allocate_for_client(NULL);
	int started = trx_start(trx, ULINT_UNDEFINED);
	ut_a(started);
	trx->op_info = "creating foreign key sys tables";
	dict_lock_data_dictionary(trx);
	if (table1) {
		ib_log(state, "InnoDB: dropping incompletely created SYS_FOREIGN table\n");
		ddl_drop_table("SYS_FOREIGN", trx, TRUE);
		trx_commit(trx);
	}
	if (table2) {
		ib_log(state, "InnoDB: dropping incompletely created SYS_FOREIGN_COLS table\n");
		ddl_drop_table("SYS_FOREIGN_COLS", trx, TRUE);
		trx_commit(trx);
	}
	trx_start_if_not_started(trx);
	ib_log(state, "InnoDB: Creating foreign key constraint system tables\n");

	// NOTE: in dict_load_foreigns we use the fact that there are 2 secondary indexes on SYS_FOREIGN, and they are defined just like below 
	// NOTE: when designing InnoDB's foreign key support in 2001, we made an error and made the table names and the foreign key id of type 'CHAR' (internally, really a VARCHAR). We should have made the type VARBINARY, like in other InnoDB system tables, to get a clean design.

	ulint error = que_eval_sql(NULL,
			     "PROCEDURE CREATE_FOREIGN_SYS_TABLES_PROC () IS\n"
			     "BEGIN\n"
			     "CREATE TABLE\n"
			     "SYS_FOREIGN(ID CHAR, FOR_NAME CHAR,"
			     " REF_NAME CHAR, N_COLS INT);\n"
			     "CREATE UNIQUE CLUSTERED INDEX ID_IND"
			     " ON SYS_FOREIGN (ID);\n"
			     "CREATE INDEX FOR_IND"
			     " ON SYS_FOREIGN (FOR_NAME);\n"
			     "CREATE INDEX REF_IND"
			     " ON SYS_FOREIGN (REF_NAME);\n"
			     "CREATE TABLE\n"
			     "SYS_FOREIGN_COLS(ID CHAR, POS INT,"
			     " FOR_COL_NAME CHAR, REF_COL_NAME CHAR);\n"
			     "CREATE UNIQUE CLUSTERED INDEX ID_IND"
			     " ON SYS_FOREIGN_COLS (ID, POS);\n"
			     "END;\n"
			     , FALSE, trx);

	if (error != DB_SUCCESS) {
		ib_log(state, "InnoDB: error %lu in creation\n", (ulong) error);
		ut_a(error == DB_OUT_OF_FILE_SPACE || error == DB_TOO_MANY_CONCURRENT_TRXS);
		ib_log(state, "InnoDB: creation failed\nInnoDB: tablespace is full\nInnoDB: dropping incompletely created SYS_FOREIGN tables\n");
		ddl_drop_table("SYS_FOREIGN", trx, TRUE);
		ddl_drop_table("SYS_FOREIGN_COLS", trx, TRUE);
		trx_commit(trx);
		error = DB_MUST_GET_MORE_FILE_SPACE;
	}
	trx_commit(trx);
	dict_unlock_data_dictionary(trx);
	trx_free_for_client(trx);
	if (error == DB_SUCCESS) {
		ib_log(state, "InnoDB: Foreign key constraint system tables created\n");
	}
	return error;
}

/// \brief Evaluate the given foreign key SQL statement.
/// \return error code or DB_SUCCESS
/// \param [in] info info struct, or NULL
/// \param [in] sql SQL string to evaluate
/// \param [in] table table
/// \param [in] foreign foreign
/// \param [in] trx transaction
static ulint dict_foreign_eval_sql(pars_info_t* info, const char* sql, dict_table_t* table, dict_foreign_t* foreign, trx_t* trx)
{
	trx_start_if_not_started(trx);

	ulint error = que_eval_sql(info, sql, FALSE, trx);

	if (error == DB_DUPLICATE_KEY) {
		mutex_enter(&dict_foreign_err_mutex);
		ut_print_timestamp(state->stream);
		ib_log(state," Error in foreign key constraint creation for table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state, ".\nA foreign key constraint of name ");
		ut_print_name(state->stream, trx, TRUE, foreign->id);
		ib_log(state,"\nalready exists.(Note that internally InnoDB adds 'databasename'\n in front of the user-defined constraint name.)\nNote that InnoDB's FOREIGN KEY system tables store\nconstraint names as case-insensitive, with the\nstandard latin1_swedish_ci collation. If you\ncreate tables or databases whose names differ only in\nthe character case, then collisions in constraint\nnames can occur. Workaround: name your constraints\nexplicitly with unique names.\n");
		mutex_exit(&dict_foreign_err_mutex);
		return error;
	}

	if (error != DB_SUCCESS) {
		ib_log(state, "InnoDB: Foreign key constraint creation failed:\n InnoDB: internal error number %lu\n", (ulong) error);
		mutex_enter(&dict_foreign_err_mutex);
		ut_print_timestamp(state->stream);
		ib_log(state," Internal error in foreign key constraint creation for table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state, ".\nSee the .err log in the datadir for more information.\n");
		mutex_exit(&dict_foreign_err_mutex);
		return error;
	}

	return DB_SUCCESS;
}

/// \brief Add a single foreign key field definition to the data dictionary tables in the database.
/// \return error code or DB_SUCCESS
/// \param [in] field_nr foreign field number
/// \param [in] table table
/// \param [in] foreign foreign
/// \param [in] trx transaction
static ulint dict_create_add_foreign_field_to_dictionary(ulint field_nr, dict_table_t* table, dict_foreign_t* foreign, trx_t* trx)
{
	pars_info_t* info = pars_info_create();
	pars_info_add_str_literal(info, "id", foreign->id);
	pars_info_add_int4_literal(info, "pos", field_nr);
	pars_info_add_str_literal(info, "for_col_name", foreign->foreign_col_names[field_nr]);
	pars_info_add_str_literal(info, "ref_col_name", foreign->referenced_col_names[field_nr]);
	return(dict_foreign_eval_sql( info,
		"PROCEDURE P () IS\n"
		"BEGIN\n"
		"INSERT INTO SYS_FOREIGN_COLS VALUES"
		"(:id, :pos, :for_col_name, :ref_col_name);\n"
		"END;\n",
		table, foreign, trx));
}

/// \brief Add a single foreign key definition to the data dictionary tables in the database. We also generate names to constraints that were not named by the user.
/// \details A generated constraint has a name of the format databasename/tablename_ibfk_NUMBER, where the numbers start from 1, and are given locally for this table, that is, the number is not global, as in the old format constraints < 4.0.18 it used to be.
/// \return error code or DB_SUCCESS
/// \param [in,out] id_nr number to use in id generation; incremented if used
/// \param [in] table table
/// \param [in] foreign foreign
/// \param [in] trx transaction
static ulint dict_create_add_foreign_to_dictionary(ulint* id_nr, dict_table_t* table, dict_foreign_t* foreign, trx_t* trx)
{
	pars_info_t* info = pars_info_create();
	if (foreign->id == NULL) {
		// Generate a new constraint id
		ulint	namelen	= strlen(table->name);
		char*	id	= mem_heap_alloc(foreign->heap, namelen + 20);
		/* no overflow if number < 1e13 */
		sprintf(id, "%s_ibfk_%lu", table->name, (ulong) (*id_nr)++);
		foreign->id = id;
	}

	pars_info_add_str_literal(info, "id", foreign->id);
	pars_info_add_str_literal(info, "for_name", table->name);
	pars_info_add_str_literal(info, "ref_name", foreign->referenced_table_name);
	pars_info_add_int4_literal(info, "n_cols", foreign->n_fields + (foreign->type << 24));
	ulint error = dict_foreign_eval_sql(info,
		"PROCEDURE P () IS\n"
		"BEGIN\n"
		"INSERT INTO SYS_FOREIGN VALUES"
		"(:id, :for_name, :ref_name, :n_cols);\n"
		"END;\n"
		, table, foreign, trx);

	if (error != DB_SUCCESS) {
		return error;
	}

	for (ulint i = 0; i < foreign->n_fields; i++) {
		error = dict_create_add_foreign_field_to_dictionary(i, table, foreign, trx);
		if (error != DB_SUCCESS) {
			return error;
		}
	}

	error = dict_foreign_eval_sql(NULL,
		"PROCEDURE P () IS\n"
		"BEGIN\n"
		"COMMIT WORK;\n"
		"END;\n"
		, table, foreign, trx);
	return error;
}

/// \brief Adds foreign key definitions to data dictionary tables in the database.
/// \return error code or DB_SUCCESS
/// \param [in] start_id if we are actually doing ALTER TABLE ADD CONSTRAINT, we want to generate constraint numbers which are bigger than in the table so far; we number the constraints from start_id + 1 up; start_id should be set to 0 if we are creating a new table, or if the table so far has no constraints for which the name was generated here
/// \param [in] table table
/// \param [in] trx transaction
IB_INTERN ulint dict_create_add_foreigns_to_dictionary(ulint start_id, dict_table_t* table, trx_t* trx)
{
	ulint number = start_id + 1;
	ut_ad(mutex_own(&(dict_sys->mutex)));
	if (NULL == dict_table_get_low("SYS_FOREIGN")) {
		ib_log(state, "InnoDB: table SYS_FOREIGN not found  in internal data dictionary\n");
		return DB_ERROR;
	}
	for (dict_foreign_t* foreign = UT_LIST_GET_FIRST(table->foreign_list); foreign != nullptr; foreign = UT_LIST_GET_NEXT(foreign_list, foreign)) {
		ulint error = dict_create_add_foreign_to_dictionary(&number, table, foreign, trx);
		if (error != DB_SUCCESS) {
			return error;
		}
	}
	return DB_SUCCESS;
}
