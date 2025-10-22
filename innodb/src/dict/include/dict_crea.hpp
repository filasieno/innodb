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

/// \file dict_crea.hpp
/// \brief Database object creation
/// \details Originally created by Heikki Tuuri in 1/8/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "dict_types.hpp"
#include "dict_dict.hpp"
#include "que_types.hpp"
#include "row_types.hpp"
#include "mtr_mtr.hpp"

// -----------------------------------------------------------------------------------------
// type definitions
// -----------------------------------------------------------------------------------------

/// \brief Table create node structure
/// \details Represents a node in the query execution graph for table creation operations.
///          Contains the table definition and child nodes for inserting table and column
///          definitions into the data dictionary.
/// \field common Node type: QUE_NODE_TABLE_CREATE
/// \field table Table to create, built as a memory data structure with dict_mem_... functions
/// \field tab_def Child node which does the insert of the table definition; the row to be inserted is built by the parent node
/// \field col_def Child node which does the inserts of the column definitions; the row to be inserted is built by the parent node
/// \field commit_node Child node which performs a commit after a successful table creation
/// \field state Node execution state
/// \field col_no Next column definition to insert
/// \field heap Memory heap used as auxiliary storage
struct tab_node_struct{
    que_common_t    common;
    dict_table_t*   table;
    ins_node_t*     tab_def;
    ins_node_t*     col_def;
    commit_node_t*  commit_node;
    // Local storage for this graph node 
    ulint           state;
    ulint           col_no;
    mem_heap_t*     heap;
};

/// \brief Index create node structure
/// \details Represents a node in the query execution graph for index creation operations.
///          Contains the index definition and child nodes for inserting index and field
///          definitions into the data dictionary.
/// \field common Node type: QUE_NODE_INDEX_CREATE
/// \field index Index to create, built as a memory data structure with dict_mem_... functions
/// \field ind_def Child node which does the insert of the index definition; the row to be inserted is built by the parent node
/// \field field_def Child node which does the inserts of the field definitions; the row to be inserted is built by the parent node
/// \field commit_node Child node which performs a commit after a successful index creation
/// \field state Node execution state
/// \field page_no Root page number of the index
/// \field table Table which owns the index
/// \field ind_row Index definition row built
/// \field field_no Next field definition to insert
/// \field heap Memory heap used as auxiliary storage
struct ind_node_struct{
    que_common_t    common;
    dict_index_t*   index;
    ins_node_t*     ind_def;
    ins_node_t*     field_def;
    commit_node_t*  commit_node;
    
    // Local storage for this graph node
    ulint           state;
    ulint           page_no;
    dict_table_t*   table;
    dtuple_t*       ind_row;
    ulint           field_no;
    mem_heap_t*     heap;
};

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

// Table create node states
constinit ulint TABLE_BUILD_TABLE_DEF = 1;
constinit ulint TABLE_BUILD_COL_DEF   = 2;
constinit ulint TABLE_COMMIT_WORK     = 3;
constinit ulint TABLE_ADD_TO_CACHE    = 4;
constinit ulint TABLE_COMPLETED       = 5;

// Index create node states
constinit ulint INDEX_BUILD_INDEX_DEF   = 1;
constinit ulint INDEX_BUILD_FIELD_DEF   = 2;
constinit ulint INDEX_CREATE_INDEX_TREE = 3;
constinit ulint INDEX_COMMIT_WORK       = 4;
constinit ulint INDEX_ADD_TO_CACHE      = 5;

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Creates a table create graph.
/// \return own: table create node
/// \param [in] table table to create, built as a memory data structure
/// \param [in] heap heap where created
/// \param [in] commit if TRUE commit transaction
IB_INTERN tab_node_t* tab_create_graph_create(dict_table_t* table, mem_heap_t* heap, ibool commit);

/// \brief Creates an index create graph.
/// \return own: index create node
/// \param [in] index index to create, built as a memory data structure
/// \param [in] heap heap where created
/// \param [in] commit TRUE if transaction should be commit
IB_INTERN ind_node_t* ind_create_graph_create(dict_index_t* index, mem_heap_t* heap, ibool commit);

/// \brief Creates a table.
/// \details This is a high-level function used in SQL execution graphs.
/// \return query thread to run next or NULL
/// \param [in] thr query thread
IB_INTERN que_thr_t* dict_create_table_step(que_thr_t* thr);

/// \brief Creates an index.
/// \details This is a high-level function used in SQL execution graphs.
/// \return query thread to run next or NULL
/// \param [in] thr query thread
IB_INTERN que_thr_t* dict_create_index_step(que_thr_t* thr);

/// \brief Truncates the index tree associated with a row in SYS_INDEXES table.
/// \return new root page number, or FIL_NULL on failure
/// \param [in] table the table the index belongs to
/// \param [in] space 0=truncate, nonzero=create the index tree in the given tablespace
/// \param [in,out] pcur persistent cursor pointing to record in the clustered index of SYS_INDEXES table. The cursor may be repositioned in this call.
/// \param [in] mtr mtr having the latch on the record page. The mtr may be committed and restarted in this call.
IB_INTERN ulint dict_truncate_index_tree(dict_table_t* table, ulint space, btr_pcur_t* pcur, mtr_t* mtr);

/// \brief Drops the index tree associated with a row in SYS_INDEXES table.
/// \param [in,out] rec record in the clustered index of SYS_INDEXES table
/// \param [in] mtr mtr having the latch on the record page
IB_INTERN void dict_drop_index_tree(rec_t* rec, mtr_t* mtr);

/// \brief Creates the foreign key constraints system tables inside InnoDB at database creation or database start if they are not found or are not of the right form.
/// \return DB_SUCCESS or error code
IB_INTERN ulint dict_create_or_check_foreign_constraint_tables(void);

/// \brief Adds foreign key definitions to data dictionary tables in the database.
/// \details We look at table->foreign_list, and also generate names to constraints that were not named by the user. A generated constraint has a name of the format databasename/tablename_ibfk_NUMBER, where the numbers start from 1, and are given locally for this table, that is, the number is not global, as in the old format constraints < 4.0.18 it used to be.
/// \return error code or DB_SUCCESS
/// \param [in] start_id if we are actually doing ALTER TABLE ADD CONSTRAINT, we want to generate constraint numbers which are bigger than in the table so far; we number the constraints from start_id + 1 up; start_id should be set to 0 if we are creating a new table, or if the table so far has no constraints for which the name was generated here
/// \param [in] table table
/// \param [in] trx transaction
IB_INTERN ulint dict_create_add_foreigns_to_dictionary(ulint start_id, dict_table_t* table, trx_t* trx);
