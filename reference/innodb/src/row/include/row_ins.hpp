// Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.
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

/// \file row_ins.hpp
/// \brief Insert into a table
/// \details Originally created on 4/20/1996 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "data_data.hpp"
#include "que_types.hpp"
#include "dict_types.hpp"
#include "trx_types.hpp"
#include "row_types.hpp"

/// \brief Checks if foreign key constraint fails for an index entry.
/// \details Sets shared locks which lock either the success or the failure of the constraint. NOTE that the caller must have a shared latch on dict_foreign_key_check_lock.
/// \param [in] check_ref TRUE If we want to check that the referenced table is ok, FALSE if we want to check the foreign key table
/// \param [in] foreign foreign constraint; NOTE that the tables mentioned in it must be in the dictionary cache if they exist at all
/// \param [in] table if check_ref is TRUE, then the foreign table, else the referenced table
/// \param [in] entry index entry for index
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_NO_REFERENCED_ROW, or DB_ROW_IS_REFERENCED
IB_INTERN ulint row_ins_check_foreign_constraint(ibool check_ref, dict_foreign_t* foreign, dict_table_t* table, dtuple_t* entry, que_thr_t* thr);

/// \brief Creates an insert node struct.
/// \param [in] ins_type INS_VALUES, ...
/// \param [in] table table where to insert
/// \param [in] heap mem heap where created
/// \return own: insert node struct
IB_INTERN ins_node_t* row_ins_node_create(ib_ins_mode_t ins_type, dict_table_t* table, mem_heap_t* heap);

/// \brief Sets a new row to insert for an INS_DIRECT node.
/// \details This function is only used if we have constructed the row separately, which is a rare case; this function is quite slow.
/// \param [in] node insert node
/// \param [in] row new row (or first row) for the node
IB_INTERN void row_ins_node_set_new_row(ins_node_t* node, dtuple_t* row);

/// \brief Inserts an index entry to index.
/// \details Tries first optimistic, then pessimistic descent down the tree. If the entry matches enough to a delete marked record, performs the insert by updating or delete unmarking the delete marked record.
/// \param [in] index index
/// \param [in] entry index entry to insert
/// \param [in] n_ext number of externally stored columns
/// \param [in] foreign TRUE=check foreign key constraints
/// \param [in] thr query thread
/// \return DB_SUCCESS, DB_LOCK_WAIT, DB_DUPLICATE_KEY, or some other error code
IB_INTERN ulint row_ins_index_entry(dict_index_t* index, dtuple_t* entry, ulint n_ext, ibool foreign, que_thr_t* thr);

/// \brief Inserts a row to a table.
/// \details This is a high-level function used in SQL execution graphs.
/// \param [in] thr query thread
/// \return query thread to run next or NULL
IB_INTERN que_thr_t* row_ins_step(que_thr_t* thr);

/// \brief Creates an entry template for each index of a table.
/// \param [in] node row insert node
IB_INTERN void row_ins_node_create_entry_list(ins_node_t* node);

/// \struct ins_node_struct Insert node structure
/// \var que_common_t ins_node_struct::common
/// \brief Node type: QUE_NODE_INSERT
/// \var ib_ins_mode_t ins_node_struct::ins_type
/// \brief INS_VALUES, INS_SEARCHED, or INS_DIRECT
/// \var dtuple_t* ins_node_struct::row
/// \brief Row to insert
/// \var dict_table_t* ins_node_struct::table
/// \brief Table where to insert
/// \var sel_node_t* ins_node_struct::select
/// \brief Select in searched insert
/// \var que_node_t* ins_node_struct::values_list
/// \brief List of expressions to evaluate and insert in an INS_VALUES insert
/// \var ulint ins_node_struct::state
/// \brief Node execution state
/// \var dict_index_t* ins_node_struct::index
/// \brief NULL, or the next index where the index entry should be inserted


struct ins_node_struct{
	que_common_t	common;	/*!< node type: QUE_NODE_INSERT */
	ib_ins_mode_t	ins_type;/* INS_VALUES, INS_SEARCHED, or INS_DIRECT */
	dtuple_t*	row;	/*!< row to insert */
	dict_table_t*	table;	/*!< table where to insert */
	sel_node_t*	select;	/*!< select in searched insert */
	que_node_t*	values_list;/* list of expressions to evaluate and insert in an INS_VALUES insert */
	ulint		state;	/*!< node execution state */
	dict_index_t*	index;	/*!< NULL, or the next index where the index entry should be inserted */
	dtuple_t*	entry;	/*!< NULL, or entry to insert in the index; after a successful insert of the entry, this should be reset to NULL */
	UT_LIST_BASE_NODE_T(dtuple_t) entry_list;/* list of entries, one for each index */
	byte*		row_id_buf;/* buffer for the row id sys field in row */
	trx_id_t	trx_id;	/*!< trx id or the last trx which executed the node */
	byte*		trx_id_buf;/* buffer for the trx id sys field in row */
	mem_heap_t*	entry_sys_heap;
	/// \brief Memory heap used as auxiliary storage; entry_list and sys fields are stored here; if this is NULL, entry list should be created and buffers for sys fields in row allocated
	ulint		magic_n;
};

constinit ulint INS_NODE_MAGIC_N = 15849075;

#ifndef IB_DO_NOT_INLINE
#include "row_ins.inl"
#endif
