// Copyright (c) 2005, 2010, Innobase Oy. All Rights Reserved.
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

/// \file row_merge.hpp
/// \brief Index build routines using a merge sort
/// \details Originally created on 13/06/2005 by Jan Lindstrom. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "data_data.hpp"
#include "dict_types.hpp"
#include "trx_types.hpp"
#include "que_types.hpp"
#include "mtr_mtr.hpp"
#include "rem_types.hpp"
#include "rem_rec.hpp"
#include "read_types.hpp"
#include "btr_types.hpp"
#include "lock_types.hpp"
#include "row_types.hpp"
#include "srv_srv.hpp"

/// Index field definition
struct merge_index_field_struct {
	ulint		prefix_len;	/*!< column prefix length, or 0
					if indexing the whole column */
	const char*	field_name;	/*!< field name */
};

/// Index field definition
typedef struct merge_index_field_struct merge_index_field_t;

/// Definition of an index being created
struct merge_index_def_struct {
	const char*		name;		/*!< index name */
	ulint			ind_type;	/*!< 0, DICT_UNIQUE,
						or DICT_CLUSTERED */
	ulint			n_fields;	/*!< number of fields
						in index */
	merge_index_field_t*	fields;		/*!< field definitions */
};

/// Definition of an index being created
typedef struct merge_index_def_struct merge_index_def_t;

/// \brief Sets an exclusive lock on a table, for the duration of creating indexes.
/// \param [in,out] trx transaction
/// \param [in] table table to lock
/// \param [in] mode LOCK_X or LOCK_S
/// \return error code or DB_SUCCESS
IB_INTERN ulint row_merge_lock_table(trx_t* trx, dict_table_t* table, enum lock_mode mode);
/// \brief Drop an index from the InnoDB system tables.
/// \details The data dictionary must have been locked exclusively by the caller, because the transaction will not be committed.
/// \param index index to be removed
/// \param table table
/// \param trx transaction handle
IB_INTERN void row_merge_drop_index(dict_index_t* index, dict_table_t* table, trx_t* trx);
/// \brief Drop those indexes which were created before an error occurred when building an index.
/// \details The data dictionary must have been locked exclusively by the caller, because the transaction will not be committed.
/// \param trx transaction
/// \param table table containing the indexes
/// \param index indexes to drop
/// \param num_created number of elements in index[]
IB_INTERN void row_merge_drop_indexes(trx_t* trx, dict_table_t* table, dict_index_t** index, ulint num_created);
/// \brief Drop all partially created indexes during crash recovery.
/// \param recovery recovery level setting
IB_INTERN void row_merge_drop_temp_indexes(ib_recovery_t recovery);
/// \brief Rename the tables in the data dictionary.
/// \details The data dictionary must have been locked exclusively by the caller, because the transaction will not be committed.
/// \param old_table old table, renamed to tmp_name
/// \param new_table new table, renamed to old_table->name
/// \param tmp_name new name for old_table
/// \param trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint row_merge_rename_tables(dict_table_t* old_table, dict_table_t* new_table, const char* tmp_name, trx_t* trx);

/// \brief Create a temporary table for creating a primary key, using the definition of an existing table.
/// \param table_name new table name
/// \param index_def the index definition of the primary key
/// \param table old table definition
/// \param trx transaction (sets error_state)
/// \return table, or NULL on error
IB_INTERN dict_table_t* row_merge_create_temporary_table(const char* table_name, const merge_index_def_t* index_def, const dict_table_t* table, trx_t* trx);
/// \brief Rename the temporary indexes in the dictionary to permanent ones.
/// \details The data dictionary must have been locked exclusively by the caller, because the transaction will not be committed.
/// \param trx transaction
/// \param table table with new indexes
/// \return DB_SUCCESS if all OK
IB_INTERN ulint row_merge_rename_indexes(trx_t* trx, dict_table_t* table);
/// \brief Create the index and load in to the dictionary.
/// \param trx trx (sets error_state)
/// \param table the index is on this table
/// \param index_def the index definition
/// \return index, or NULL on error
IB_INTERN dict_index_t* row_merge_create_index(trx_t* trx, dict_table_t* table, const merge_index_def_t* index_def);
/// \brief Check if a transaction can use an index.
/// \param trx transaction
/// \param index index to check
/// \return TRUE if index can be used by the transaction else FALSE
IB_INTERN ibool row_merge_is_index_usable(const trx_t* trx, const dict_index_t* index);
/// \brief If there are views that refer to the old table name then we "attach" to the new instance of the table else we drop it immediately.
/// \param trx transaction
/// \param table table instance to drop
/// \return DB_SUCCESS or error code
IB_INTERN ulint row_merge_drop_table(trx_t* trx, dict_table_t* table);

/*********************************************************************//**
Build indexes on a table by reading a clustered index,
creating a temporary file containing index entries, merge sorting
these index entries and inserting sorted index entries to indexes.
@return	DB_SUCCESS or error code */
IB_INTERN ulint row_merge_build_indexes(trx_t* trx, dict_table_t* old_table, dict_table_t* new_table, dict_index_t** indexes, ulint n_indexes, table_handle_t table);
#endif /* row0merge.h */
