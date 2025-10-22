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

/// \file dict_boot.cpp
/// \brief Data dictionary creation and booting
/// \details Originally created by Heikki Tuuri in 4/18/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "dict_boot.hpp"

#ifdef IB_DO_NOT_INLINE
#include "dict0boot.inl"
#endif

#include "dict_crea.hpp"
#include "btr_btr.hpp"
#include "dict_load.hpp"
#include "dict_load.hpp"
#include "trx_trx.hpp"
#include "srv_srv.hpp"
#include "ibuf_ibuf.hpp"
#include "buf_flu.hpp"
#include "log_recv.hpp"
#include "os_file.hpp"

IB_INTERN dict_hdr_t* dict_hdr_get(mtr_t* mtr)
{
    buf_block_t* block = buf_page_get(DICT_HDR_SPACE, 0, DICT_HDR_PAGE_NO, RW_X_LATCH, mtr);
    dict_hdr_t* header = DICT_HDR + buf_block_get_frame(block);
    buf_block_dbg_add_level(block, SYNC_DICT_HEADER);
    return(header);
}

/// \brief Returns a new table, index, or tree id.
/// \param [in] type DICT_HDR_ROW_ID, ...
/// \return the new id
IB_INTERN dulint dict_hdr_get_new_id(ulint type)
{
	ut_ad((type == DICT_HDR_TABLE_ID) || (type == DICT_HDR_INDEX_ID));
	
	mtr_t mtr;
	mtr_start(&mtr);
	dict_hdr_t* dict_hdr = dict_hdr_get(&mtr);
	dulint id = mtr_read_dulint(dict_hdr + type, &mtr);
	id = ut_dulint_add(id, 1);
	mlog_write_dulint(dict_hdr + type, id, &mtr);
	mtr_commit(&mtr);
	return(id);
}

/// \brief Writes the current value of the row id counter to the dictionary header file page.
IB_INTERN void dict_hdr_flush_row_id(void)
{
	ut_ad(mutex_own(&(dict_sys->mutex)));

	dict_hdr_t* dict_hdr;
	dulint id = dict_sys->row_id;
	mtr_start(&mtr);
	mtr_t dict_hdr = dict_hdr_get(&mtr);
	mlog_write_dulint(dict_hdr + DICT_HDR_ROW_ID, id, &mtr);
	mtr_commit(&mtr);
}

/// \brief Creates the file page for the dictionary header. This function is called only at the database creation.
/// \param [in] mtr mtr
/// \return TRUE if succeed
static ibool dict_hdr_create(mtr_t* mtr)
{
	ut_ad(mtr);

	// reate the dictionary header file block in a new, allocated file segment in the system tablespace
	buf_block_t* block = fseg_create(DICT_HDR_SPACE, 0, DICT_HDR + DICT_HDR_FSEG_HEADER, mtr);
	ut_a(DICT_HDR_PAGE_NO == buf_block_get_page_no(block));
	dict_hdr_t* dict_header = dict_hdr_get(mtr);

	// Start counting row, table, index, and tree ids from DICT_HDR_FIRST_ID 
	mlog_write_dulint(dict_header + DICT_HDR_ROW_ID,   ut_dulint_create(0, DICT_HDR_FIRST_ID), mtr);
	mlog_write_dulint(dict_header + DICT_HDR_TABLE_ID, ut_dulint_create(0, DICT_HDR_FIRST_ID), mtr);
	mlog_write_dulint(dict_header + DICT_HDR_INDEX_ID, ut_dulint_create(0, DICT_HDR_FIRST_ID), mtr);
	
	// Obsolete, but we must initialize it to 0 anyway. 
	mlog_write_dulint(dict_header + DICT_HDR_MIX_ID,   ut_dulint_create(0, DICT_HDR_FIRST_ID), mtr);

	// Create the B-tree roots for the clustered indexes of the basic system tables
	ulint root_page_no;
	//--------------------------
	root_page_no = btr_create(DICT_CLUSTERED | DICT_UNIQUE, DICT_HDR_SPACE, 0, DICT_TABLES_ID, dict_ind_redundant, mtr);
	if (root_page_no == FIL_NULL) {
		return(FALSE);
	}
	mlog_write_ulint(dict_header + DICT_HDR_TABLES, root_page_no, MLOG_4BYTES, mtr);

	//--------------------------
	root_page_no = btr_create(DICT_UNIQUE, DICT_HDR_SPACE, 0, DICT_TABLE_IDS_ID, dict_ind_redundant, mtr);
	if (root_page_no == FIL_NULL) {
		return(FALSE);
	}
	mlog_write_ulint(dict_header + DICT_HDR_TABLE_IDS, root_page_no, MLOG_4BYTES, mtr);
	
	//--------------------------
	root_page_no = btr_create(DICT_CLUSTERED | DICT_UNIQUE, DICT_HDR_SPACE, 0, DICT_COLUMNS_ID, dict_ind_redundant, mtr);
	if (root_page_no == FIL_NULL) {
		return(FALSE);
	}
	mlog_write_ulint(dict_header + DICT_HDR_COLUMNS, root_page_no, MLOG_4BYTES, mtr);
	
	//--------------------------
	root_page_no = btr_create(DICT_CLUSTERED | DICT_UNIQUE, DICT_HDR_SPACE, 0, DICT_INDEXES_ID, dict_ind_redundant, mtr);
	if (root_page_no == FIL_NULL) {
		return(FALSE);
	}
	mlog_write_ulint(dict_header + DICT_HDR_INDEXES, root_page_no, MLOG_4BYTES, mtr);
	
	//--------------------------
	root_page_no = btr_create(DICT_CLUSTERED | DICT_UNIQUE, DICT_HDR_SPACE, 0, DICT_FIELDS_ID, dict_ind_redundant, mtr);
	if (root_page_no == FIL_NULL) {
		return(FALSE);
	}
	mlog_write_ulint(dict_header + DICT_HDR_FIELDS, root_page_no, MLOG_4BYTES, mtr);
	

	return TRUE;
}

/// \brief Initializes the data dictionary memory structures when the database is started.
/// \details This function is also called when the data dictionary is created.
IB_INTERN void dict_boot(void)
{
	 table;
	dict_index_t* index;
	 dict_hdr;
	mtr_t mtr;
	ulint error;

	mtr_start(&mtr);

	// Create the hash tables etc.
	dict_init();

	mem_heap_t* heap = mem_heap_create(450);

	mutex_enter(&(dict_sys->mutex));

	// Get the dictionary heade
	dict_hdr_t* dict_hdr = dict_hdr_get(&mtr);

	// Because we only write new row ids to disk-based data structure (dictionary header) when it is divisible by DICT_HDR_ROW_ID_WRITE_MARGIN, 
	// in recovery we will not recover the latest value of the row id counter. Therefore we advance the counter at the database startup to avoid overlapping values. 
	// Note that when a user after database startup first time asks for a new row id, then because the counter is now divisible by ...
	// _MARGIN, it will immediately be updated to the disk-based header. 
	dict_sys->row_id = ut_dulint_add(ut_dulint_align_up(mtr_read_dulint(dict_hdr + DICT_HDR_ROW_ID, &mtr), DICT_HDR_ROW_ID_WRITE_MARGIN), DICT_HDR_ROW_ID_WRITE_MARGIN);

	// Insert into the dictionary cache the descriptions of the basic system tables

	// -------------------------
	dict_table_t* table = dict_mem_table_create("SYS_TABLES", DICT_HDR_SPACE, 8, 0);

	dict_mem_table_add_col(table, heap, "NAME", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "ID", DATA_BINARY, 0, 0);
	// ROW_FORMAT = (N_COLS >> 31) ? COMPACT : REDUNDANT
	dict_mem_table_add_col(table, heap, "N_COLS", DATA_INT, 0, 4);
	// TYPE is either DICT_TABLE_ORDINARY, or (TYPE & DICT_TF_COMPACT) and (TYPE & DICT_TF_FORMAT_MASK) are nonzero and TYPE = table->flags */
	dict_mem_table_add_col(table, heap, "TYPE", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "MIX_ID", DATA_BINARY, 0, 0);
	// MIX_LEN may contain additional table flags when ROW_FORMAT!=REDUNDANT.  Currently, these flags include DICT_TF2_TEMPORARY.
	dict_mem_table_add_col(table, heap, "MIX_LEN", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "CLUSTER_NAME", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "SPACE", DATA_INT, 0, 4);

	table->id = DICT_TABLES_ID;

	dict_table_add_to_cache(table, heap);
	dict_sys->sys_tables = table;
	mem_heap_empty(heap);

	index = dict_mem_index_create("SYS_TABLES", "CLUST_IND", DICT_HDR_SPACE, DICT_UNIQUE | DICT_CLUSTERED, 1);

	dict_mem_index_add_field(index, "NAME", 0);

	index->id = DICT_TABLES_ID;

	error = dict_index_add_to_cache(table, index, mtr_read_ulint(dict_hdr + DICT_HDR_TABLES, MLOG_4BYTES, &mtr), FALSE);
	ut_a(error == DB_SUCCESS);

	// -------------------------
	index = dict_mem_index_create("SYS_TABLES", "ID_IND", DICT_HDR_SPACE, DICT_UNIQUE, 1);
	dict_mem_index_add_field(index, "ID", 0);
	index->id = DICT_TABLE_IDS_ID;
	error = dict_index_add_to_cache(table, index, mtr_read_ulint(dict_hdr + DICT_HDR_TABLE_IDS, MLOG_4BYTES, &mtr), FALSE);
	ut_a(error == DB_SUCCESS);

	// -------------------------
	table = dict_mem_table_create("SYS_COLUMNS", DICT_HDR_SPACE, 7, 0);

	dict_mem_table_add_col(table, heap, "TABLE_ID", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "POS", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "NAME", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "MTYPE", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "PRTYPE", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "LEN", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "PREC", DATA_INT, 0, 4);

	table->id = DICT_COLUMNS_ID;

	dict_table_add_to_cache(table, heap);
	dict_sys->sys_columns = table;
	mem_heap_empty(heap);

	index = dict_mem_index_create("SYS_COLUMNS", "CLUST_IND", DICT_HDR_SPACE, DICT_UNIQUE | DICT_CLUSTERED, 2);
	dict_mem_index_add_field(index, "TABLE_ID", 0);
	dict_mem_index_add_field(index, "POS", 0);
	index->id = DICT_COLUMNS_ID;
	error = dict_index_add_to_cache(table, index, mtr_read_ulint(dict_hdr + DICT_HDR_COLUMNS, MLOG_4BYTES, &mtr), FALSE);
	ut_a(error == DB_SUCCESS);

	// -------------------------
	table = dict_mem_table_create("SYS_INDEXES", DICT_HDR_SPACE, 7, 0);

	dict_mem_table_add_col(table, heap, "TABLE_ID", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "ID", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "NAME", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "N_FIELDS", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "TYPE", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "SPACE", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "PAGE_NO", DATA_INT, 0, 4);

	/* The '+ 2' below comes from the fields DB_TRX_ID, DB_ROLL_PTR */
#if DICT_SYS_INDEXES_PAGE_NO_FIELD != 6 + 2
#error "DICT_SYS_INDEXES_PAGE_NO_FIELD != 6 + 2"
#endif
#if DICT_SYS_INDEXES_SPACE_NO_FIELD != 5 + 2
#error "DICT_SYS_INDEXES_SPACE_NO_FIELD != 5 + 2"
#endif
#if DICT_SYS_INDEXES_TYPE_FIELD != 4 + 2
#error "DICT_SYS_INDEXES_TYPE_FIELD != 4 + 2"
#endif
#if DICT_SYS_INDEXES_NAME_FIELD != 2 + 2
#error "DICT_SYS_INDEXES_NAME_FIELD != 2 + 2"
#endif

	table->id = DICT_INDEXES_ID;
	dict_table_add_to_cache(table, heap);
	dict_sys->sys_indexes = table;
	mem_heap_empty(heap);

	index = dict_mem_index_create("SYS_INDEXES", "CLUST_IND", DICT_HDR_SPACE, DICT_UNIQUE | DICT_CLUSTERED, 2);
	dict_mem_index_add_field(index, "TABLE_ID", 0);
	dict_mem_index_add_field(index, "ID", 0);
	index->id = DICT_INDEXES_ID;
	error = dict_index_add_to_cache(table, index, mtr_read_ulint(dict_hdr + DICT_HDR_INDEXES, MLOG_4BYTES, &mtr), FALSE);
	ut_a(error == DB_SUCCESS);

	/*-------------------------*/
	table = dict_mem_table_create("SYS_FIELDS", DICT_HDR_SPACE, 3, 0);

	dict_mem_table_add_col(table, heap, "INDEX_ID", DATA_BINARY, 0, 0);
	dict_mem_table_add_col(table, heap, "POS", DATA_INT, 0, 4);
	dict_mem_table_add_col(table, heap, "COL_NAME", DATA_BINARY, 0, 0);

	table->id = DICT_FIELDS_ID;
	dict_table_add_to_cache(table, heap);
	dict_sys->sys_fields = table;
	mem_heap_free(heap);

	index = dict_mem_index_create("SYS_FIELDS", "CLUST_IND", DICT_HDR_SPACE, DICT_UNIQUE | DICT_CLUSTERED, 2);
	dict_mem_index_add_field(index, "INDEX_ID", 0);
	dict_mem_index_add_field(index, "POS", 0);
	index->id = DICT_FIELDS_ID;
	error = dict_index_add_to_cache(table, index, mtr_read_ulint(dict_hdr + DICT_HDR_FIELDS, MLOG_4BYTES, &mtr), FALSE);
	ut_a(error == DB_SUCCESS);

	mtr_commit(&mtr);
	// -------------------------

	// Initialize the insert buffer table and index for each tablespace */

	ibuf_init_at_db_start();

	// Load definitions of other indexes on system tables */
	dict_load_sys_table(dict_sys->sys_tables);
	dict_load_sys_table(dict_sys->sys_columns);
	dict_load_sys_table(dict_sys->sys_indexes);
	dict_load_sys_table(dict_sys->sys_fields);

	mutex_exit(&(dict_sys->mutex));
}

/// \brief Inserts the basic system table data into themselves in the database creation.
static void dict_insert_initial_data(void)
{
	// Does nothing yet
}

/// \brief Creates and initializes the data dictionary at the database creation.
IB_INTERN void dict_create(void)
{
	mtr_t mtr;
	mtr_start(&mtr);
	dict_hdr_create(&mtr);
	mtr_commit(&mtr);
	dict_boot();
	dict_insert_initial_data();
}
