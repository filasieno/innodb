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

/// \file dict_dict.hpp

/// \brief Data dictionary system
/// \details Originally created by Heikki Tuuri in 1/8/1996
/// \author Fabio N. Filasieno
/// \date 22/10/2025

#pragma once

#include "univ.i"
#include "dict_types.hpp"
#include "dict_mem.hpp"
#include "data_type.hpp"
#include "data_data.hpp"
#include "mem_mem.hpp"
#include "rem_types.hpp"
#include "ut_mem.hpp"
#include "ut_lst.hpp"
#include "hash_hash.hpp"
#include "ut_rnd.hpp"
#include "ut_byte.hpp"
#include "trx_types.hpp"
#include "srv_srv.hpp"

#ifndef IB_HOTBACKUP

#include "sync_sync.h"
#include "sync_rw.h"

/// \brief Makes all characters in a NUL-terminated UTF-8 string lower case.
/// \param [in,out] a string to put in lower case
IB_INTERN void dict_casedn_str(char* a);

/// \brief Get the database name length in a table name.
/// \param [in] name table name in the form dbname '/' tablename
/// \return database name length
IB_INTERN ulint dict_get_db_IB_NAME_LEN(const char* name);

/// \brief Return the end of table name where we have removed dbname and '/'.
/// \param [in] name table name in the form dbname '/' tablename
/// \return table name
const char* dict_remove_db_name(const char* name);

/// \brief Returns a table object based on table id.
/// \param [in] recovery recovery flag
/// \param [in] table_id table id
/// \param [in] trx transaction handle
/// \return table, NULL if does not exist
IB_INTERN dict_table_t* dict_table_get_on_id(ib_recovery_t recovery, dulint table_id, trx_t* trx);

/// \brief Decrements the count of open handles to a table.
/// \param [in,out] table table
/// \param [in] dict_locked TRUE=data dictionary locked
IB_INTERN void dict_table_decrement_handle_count(dict_table_t* table, ibool dict_locked);

/// \brief Increments the count of open client handles to a table.
/// \param [in,out] table table
/// \param [in] dict_locked TRUE=data dictionary locked
IB_INTERN void dict_table_increment_handle_count(dict_table_t* table, ibool dict_locked);

/// \brief Inits the data dictionary module.
IB_INTERN void dict_init(void);

/// \brief Closes the data dictionary module.
IB_INTERN void dict_close(void);

/// \brief Gets the column data type.
/// \param [in] col column
/// \param [out] type data type
IB_INLINE void dict_col_copy_type(const dict_col_t* col, dtype_t* type);

#endif // !IB_HOTBACKUP 

#ifdef IB_DEBUG

/// \brief Assert that a column and a data type match.
/// \param [in] col column
/// \param [in] type data type
/// \return TRUE
IB_INLINE ibool dict_col_type_assert_equal(const dict_col_t* col, const dtype_t* type);

#endif // IB_DEBUG

#ifndef IB_HOTBACKUP

/// \brief Returns the minimum size of the column.
/// \param [in] col column
/// \return minimum size
IB_INLINE ulint dict_col_get_min_size(const dict_col_t* col);

/// \brief Returns the maximum size of the column.
/// \param [in] col column
/// \return maximum size
IB_INLINE ulint dict_col_get_max_size(const dict_col_t* col);

/// \brief Returns the size of a fixed size column, 0 if not a fixed size column.
/// \param [in] col column
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT
/// \return fixed size, or 0
IB_INLINE ulint dict_col_get_fixed_size(const dict_col_t* col, ulint comp);

/// \brief Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a column.
/// \details For fixed length types it is the fixed length of the type, otherwise 0.
/// \param [in] col column
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT
/// \return SQL null storage size in ROW_FORMAT=REDUNDANT
IB_INLINE ulint dict_col_get_sql_null_size(const dict_col_t* col, ulint comp);

/// \brief Gets the column number.
/// \param [in] col column
/// \return col->ind, table column position (starting from 0)
IB_INLINE ulint dict_col_get_no(const dict_col_t* col);

/// \brief Gets the column position in the clustered index.
/// \param [in] col table column
/// \param [in] clust_index clustered index
IB_INLINE ulint dict_col_get_clust_pos(const dict_col_t* col, const dict_index_t* clust_index);

/// \brief If the given column name is reserved for InnoDB system columns, return TRUE.
/// \param [in] name column name
/// \return TRUE if name is reserved
IB_INTERN ibool dict_col_name_is_reserved(const char* name);
#endif /* !IB_HOTBACKUP */

/// \brief Adds system columns to a table object.
/// \param [in,out] table table
/// \param [in] heap temporary heap
IB_INTERN void dict_table_add_system_columns(dict_table_t* table, mem_heap_t* heap);

#ifndef IB_HOTBACKUP
/// \brief Adds a table object to the dictionary cache.
/// \param [in] table table
/// \param [in] heap temporary heap
IB_INTERN void dict_table_add_to_cache(dict_table_t* table, mem_heap_t* heap);

/// \brief Removes a table object from the dictionary cache.
/// \param [in,own] table table
IB_INTERN void dict_table_remove_from_cache(dict_table_t* table);

/// \brief Renames a table object.
/// \param [in,out] table table
/// \param [in] new_name new name
/// \param [in] rename_also_foreigns in ALTER TABLE we want to preserve the original table name in constraints which reference it
/// \return TRUE if success
IB_INTERN ibool dict_table_rename_in_cache(dict_table_t* table, const char* new_name, ibool rename_also_foreigns);

/// \brief Removes an index from the dictionary cache.
/// \param [in,out] table table
/// \param [in,own] index index
IB_INTERN void dict_index_remove_from_cache(dict_table_t* table, dict_index_t* index);

/// \brief Change the id of a table object in the dictionary cache.
/// \details This is used in DISCARD TABLESPACE.
/// \param [in,out] table table object already in cache
/// \param [in] new_id new id to set
IB_INTERN void dict_table_change_id_in_cache(dict_table_t* table, dulint new_id);
/// \brief Adds a foreign key constraint object to the dictionary cache.
/// \details May free the object if there already is an object with the same identifier in. At least one of foreign table or referenced table must already be in the dictionary cache!
/// \param [in,own] foreign foreign key constraint
/// \param [in] check_charsets TRUE=check charset compatibility
/// \return DB_SUCCESS or error code
IB_INTERN ulint dict_foreign_add_to_cache(dict_foreign_t* foreign, ibool check_charsets);
/// \brief Check if the index is referenced by a foreign key, if TRUE return the matching instance NULL otherwise.
/// \param [in] table InnoDB table
/// \param [in] index InnoDB index
/// \return pointer to foreign key struct if index is defined for foreign key, otherwise NULL
IB_INTERN dict_foreign_t* dict_table_get_referenced_constraint(dict_table_t* table, dict_index_t* index);

/// \brief Checks if a table is referenced by foreign keys.
/// \param [in] table InnoDB table
/// \return TRUE if table is referenced by a foreign key
IB_INTERN ibool dict_table_is_referenced_by_foreign_key(const dict_table_t* table);

/// \brief Replace the index in the foreign key list that matches this index's definition with an equivalent index.
/// \param [in,out] table table
/// \param [in] index index to be replaced
IB_INTERN void dict_table_replace_index_in_foreign_list(dict_table_t* table, dict_index_t* index);

/// \brief Checks if a index is defined for a foreign key constraint.
/// \details Index is a part of a foreign key constraint if the index is referenced by foreign key or index is a foreign key index
/// \param [in] table InnoDB table
/// \param [in] index InnoDB index
/// \return pointer to foreign key struct if index is defined for foreign key, otherwise NULL
IB_INTERN dict_foreign_t* dict_table_get_foreign_constraint(dict_table_t* table, dict_index_t* index);

/// \brief Scans a table create SQL string and adds to the data dictionary the foreign key constraints declared in the string.
/// \details This function should be called after the indexes for a table have been created. Each foreign key constraint must be accompanied with indexes in bot participating tables. The indexes are allowed to contain more fields than mentioned in the constraint.
/// \param [in] trx transaction
/// \param [in] sql_string table create statement where foreign keys are declared like: FOREIGN KEY (a, b) REFERENCES table2(c, d), table2 can be written also with the database name before it: test.table2; the default database id the database of parameter name
/// \param [in] name table full name in the normalized form database_name/table_name
/// \param [in] reject_fks if TRUE, fail with error code DB_CANNOT_ADD_CONSTRAINT if any foreign keys are found.
/// \return error code or DB_SUCCESS
IB_INTERN ulint dict_create_foreign_constraints(trx_t* trx, const char* sql_string, const char* name, ibool reject_fks);
/// \brief Parses the CONSTRAINT id's to be dropped in an ALTER TABLE statement.
/// \param [in] heap heap from which we can allocate memory
/// \param [in] trx transaction
/// \param [in] table table
/// \param [out] n number of constraints to drop
/// \param [out] constraints_to_drop id's of the constraints to drop
/// \return DB_SUCCESS or DB_CANNOT_DROP_CONSTRAINT if syntax error or the constraint id does not match
IB_INTERN ulint dict_foreign_parse_drop_constraints(mem_heap_t* heap, trx_t* trx, dict_table_t* table, ulint* n, const char*** constraints_to_drop);

/// \brief Returns a table object and optionally increment its open handle count.
/// \details NOTE! This is a high-level function to be used mainly from outside the 'dict' directory. Inside this directory dict_table_get_low is usually the appropriate function.
/// \param [in] table_name table name
/// \param [in] inc_count whether to increment the open handle count on the table
/// \return table, NULL if does not exist
IB_INTERN dict_table_t* dict_table_get(const char* table_name, ibool inc_count);

/// \brief Returns a table instance based on table id.
/// \param [in] recovery recovery flag
/// \param [in] table_id table id
/// \param [in] ref_count increment open handle count if TRUE
/// \return table, NULL if does not exist
IB_INTERN dict_table_t* dict_table_get_using_id(ib_recovery_t recovery, dulint table_id, ibool ref_count);

/// \brief Returns a index object, based on table and index id, and memoryfixes it.
/// \param [in] table table
/// \param [in] index_id index id
/// \return index, NULL if does not exist
IB_INTERN dict_index_t* dict_index_get_on_id_low(dict_table_t* table, dulint index_id);

/// \brief Checks if a table is in the dictionary cache.
/// \param [in] table_name table name
/// \return table, NULL if not found
IB_INLINE dict_table_t* dict_table_check_if_in_cache_low(const char* table_name);

/// \brief Gets a table; loads it to the dictionary cache if necessary.
/// \details A low-level function.
/// \param [in] table_name table name
/// \return table, NULL if not found
IB_INLINE dict_table_t* dict_table_get_low(const char* table_name);

/// \brief Returns a table object based on table id.
/// \param [in] recovery recovery flag
/// \param [in] table_id table id
/// \return table, NULL if does not exist
IB_INLINE dict_table_t* dict_table_get_on_id_low(ib_recovery_t recovery, dulint table_id);
/// \brief Find an index that is equivalent to the one passed in and is not marked for deletion.
/// \param [in] foreign foreign key
/// \return index equivalent to foreign->foreign_index, or NULL
IB_INTERN dict_index_t* dict_foreign_find_equiv_index(dict_foreign_t* foreign);

/// \brief Returns an index object by matching on the name and column names and if more than one index matches return the index with the max id
/// \param [in] table table
/// \param [in] name the index name to find
/// \param [in] columns array of column names
/// \param [in] n_cols number of columns
/// \return matching index, NULL if not found
IB_INTERN dict_index_t* dict_table_get_index_by_max_id(dict_table_t* table, const char* name, const char** columns, ulint n_cols);

/// \brief Returns a column's name.
/// \param [in] table table
/// \param [in] col_nr column number
/// \return column name. NOTE: not guaranteed to stay valid if table is modified in any way (columns added, etc.).
IB_INTERN const char* dict_table_get_col_name(const dict_table_t* table, ulint col_nr);

/// \brief Returns a column's ordinal value.
/// \param [in] table table
/// \param [in] name column name
/// \return column pos. -1 if not found. NOTE: not guaranteed to stay valid if table is modified in any way (columns added, etc.).
IB_INTERN int dict_table_get_col_no(const dict_table_t* table, const char* name);

/// \brief Prints a table definition.
/// \param [in] table table
IB_INTERN void dict_table_print(dict_table_t* table);

/// \brief Prints a table data.
/// \param [in] table table
IB_INTERN void dict_table_print_low(dict_table_t* table);

/// \brief Prints a table data when we know the table name.
/// \param [in] name table name
IB_INTERN void dict_table_print_by_name(const char* name);

/// \brief Outputs info on foreign keys of a table.
/// \param [in] create_table_format if TRUE then print in a format suitable to be inserted into a CREATE TABLE, otherwise in the format of SHOW TABLE STATUS
/// \param [in] stream stream where to print
/// \param [in] trx transaction
/// \param [in] table table
IB_INTERN void dict_print_info_on_foreign_keys(ibool create_table_format, ib_stream_t stream, trx_t* trx, dict_table_t* table);

/// \brief Outputs info on a foreign key of a table in a format suitable for CREATE TABLE.
/// \param [in] stream file where to print
/// \param [in] trx transaction
/// \param [in] foreign foreign key constraint
/// \param [in] add_newline whether to add a newline
IB_INTERN void dict_print_info_on_foreign_key_in_create_format(ib_stream_t stream, trx_t* trx, dict_foreign_t* foreign, ibool add_newline);

/// \brief Displays the names of the index and the table.
/// \param [in] stream output stream
/// \param [in] trx transaction
/// \param [in] index index to print
IB_INTERN void dict_index_name_print(ib_stream_t stream, trx_t* trx, const dict_index_t* index);

#ifdef IB_DEBUG

	/// \brief Gets the first index on the table (the clustered index).
	/// \param [in] table table
	/// \return index, NULL if none exists
	IB_INLINE dict_index_t* dict_table_get_first_index(const dict_table_t* table);

	/// \brief Gets the next index on the table.
	/// \param [in] index index
	/// \return index, NULL if none left
	IB_INLINE dict_index_t* dict_table_get_next_index(const dict_index_t* index);

#else /* IB_DEBUG */

	#define dict_table_get_first_index(table) UT_LIST_GET_FIRST((table)->indexes)
	#define dict_table_get_next_index(index) UT_LIST_GET_NEXT(indexes, index)

#endif /* IB_DEBUG */

#endif /* !IB_HOTBACKUP */

/// \brief Check whether the index is the clustered index.
/// \param [in] index index
/// \return nonzero for clustered index, zero for other indexes
IB_INLINE ulint dict_index_is_clust(const dict_index_t* index) __attribute__((pure));

/// \brief Check whether the index is unique.
/// \param [in] index index
/// \return nonzero for unique index, zero for other indexes
IB_INLINE ulint dict_index_is_unique(const dict_index_t* index) __attribute__((pure));

/// \brief Check whether the index is the insert buffer tree.
/// \param [in] index index
/// \return nonzero for insert buffer, zero for other indexes
IB_INLINE ulint dict_index_is_ibuf(const dict_index_t* index) __attribute__((pure));

/// \brief Check whether the index is a secondary index or the insert buffer tree.
/// \param [in] index index
/// \return nonzero for insert buffer, zero for other indexes
IB_INLINE ulint dict_index_is_sec_or_ibuf(const dict_index_t* index) __attribute__((pure));

/// \brief Gets the number of user-defined columns in a table in the dictionary cache.
/// \param [in] table table
/// \return number of user-defined (e.g., not ROW_ID) columns of a table
IB_INLINE ulint dict_table_get_n_user_cols(const dict_table_t* table);

/// \brief Gets the number of system columns in a table in the dictionary cache.
/// \param [in] table table
/// \return number of system (e.g., ROW_ID) columns of a table
IB_INLINE ulint dict_table_get_n_sys_cols(const dict_table_t* table);

/// \brief Gets the number of all columns (also system) in a table in the dictionary cache.
/// \param [in] table table
/// \return number of columns of a table
IB_INLINE ulint dict_table_get_n_cols(const dict_table_t* table);

#ifdef IB_DEBUG

	/// \brief Gets the nth column of a table.
	/// \param [in] table table
	/// \param [in] pos position of column
	/// \return pointer to column object
	IB_INLINE dict_col_t* dict_table_get_nth_col(const dict_table_t* table, ulint pos);

	/// \brief Gets the given system column of a table.
	/// \param [in] table table
	/// \param [in] sys DATA_ROW_ID, ...
	/// \return pointer to column object
	IB_INLINE dict_col_t* dict_table_get_sys_col(const dict_table_t* table, ulint sys);

#else // IB_DEBUG

	#define dict_table_get_nth_col(table, pos) ((table)->cols + (pos))
	#define dict_table_get_sys_col(table, sys) ((table)->cols + (table)->n_cols + (sys) - DATA_N_SYS_COLS)

#endif // IB_DEBUG

/// \brief Gets the given system column number of a table.
/// \param [in] table table
/// \param [in] sys DATA_ROW_ID, ...
/// \return column number
IB_INLINE ulint dict_table_get_sys_col_no(const dict_table_t* table, ulint sys);

#ifndef IB_HOTBACKUP
	/// \brief Returns the minimum data size of an index record.
	/// \param [in] index index
	/// \return minimum data size in bytes
	IB_INLINE ulint dict_index_get_min_size(const dict_index_t* index);
#endif // !IB_HOTBACKUP

/// \brief Check whether the table uses the compact page format.
/// \param [in] table table
/// \return TRUE if table uses the compact page format
IB_INLINE ibool dict_table_is_comp(const dict_table_t* table);

/// \brief Determine the file format of a table.
/// \param [in] table table
/// \return file format version
IB_INLINE ulint dict_table_get_format(const dict_table_t* table);

/// \brief Set the file format of a table.
/// \param [in,out] table table
/// \param [in] format file format version
IB_INLINE void dict_table_set_format(dict_table_t* table, ulint format);

/// \brief Extract the compressed page size from table flags.
/// \param [in] flags flags
/// \return compressed page size, or 0 if not compressed
IB_INLINE ulint dict_table_flags_to_zip_size(ulint flags) __attribute__((const));

/// \brief Check whether the table uses the compressed compact page format.
/// \param [in] table table
/// \return compressed page size, or 0 if not compressed
IB_INLINE ulint dict_table_zip_size(const dict_table_t* table);

/// \brief Checks if a column is in the ordering columns of the clustered index of a table.
/// \details Column prefixes are treated like whole columns.
/// \param [in] table table
/// \param [in] n column number
/// \return TRUE if the column, or its prefix, is in the clustered key
IB_INTERN ibool dict_table_col_in_clustered_key(const dict_table_t* table, ulint n);

#ifndef IB_HOTBACKUP

/// \brief Copies types of columns contained in table to tuple and sets all fields of the tuple to the SQL NULL value.
/// \param [in,out] tuple data tuple
/// \param [in] table table
/// \details This function should be called right after dtuple_create().
IB_INTERN void dict_table_copy_types(dtuple_t* tuple, const dict_table_t* table);

/// \brief Looks for an index with the given id.
/// \param [in] id index id
/// \return index or NULL if not found from cache
/// \details NOTE that we do not reserve the dictionary mutex: this function is for emergency purposes like printing info of a corrupt database page!
IB_INTERN dict_index_t* dict_index_find_on_id_low(dulint id);

/// \brief Adds an index to the dictionary cache.
/// \param [in] table table on which the index is
/// \param [in] index index; NOTE! The index memory object is freed in this function!
/// \param [in] page_no root page number of the index
/// \param [in] strict TRUE=refuse to create the index if records could be too big to fit in an B-tree page
/// \return DB_SUCCESS, DB_TOO_BIG_RECORD, or DB_CORRUPTION
IB_INTERN ulint dict_index_add_to_cache(dict_table_t* table, dict_index_t* index, ulint page_no, ibool strict);

/// \brief Removes an index from the dictionary cache.
/// \param [in,out] table table
/// \param [in] index index
IB_INTERN void dict_index_remove_from_cache(dict_table_t* table, dict_index_t* index);
#endif /* !IB_HOTBACKUP */

/// \brief Gets the number of fields in the internal representation of an index, including fields added by the dictionary system.
/// \param [in] index an internal representation of index (in the dictionary cache)
/// \return number of fields
IB_INLINE ulint dict_index_get_n_fields(const dict_index_t* index);

/// \brief Gets the number of fields in the internal representation of an index that uniquely determine the position of an index entry in the index, if we do not take multiversioning into account: in the B-tree use the value returned by dict_index_get_n_unique_in_tree.
/// \param [in] index an internal representation of index (in the dictionary cache)
/// \return number of fields
IB_INLINE ulint dict_index_get_n_unique(const dict_index_t* index);

/// \brief Gets the number of fields in the internal representation of an index which uniquely determine the position of an index entry in the index, if we also take multiversioning into account.
/// \param [in] index an internal representation of index (in the dictionary cache)
/// \return number of fields
IB_INLINE ulint dict_index_get_n_unique_in_tree(const dict_index_t* index);

/// \brief Gets the number of user-defined ordering fields in the index.
/// \param [in] index an internal representation of index (in the dictionary cache)
/// \return number of fields
/// \details In the internal representation we add the row id to the ordering fields to make all indexes unique, but this function returns the number of fields the user defined in the index as ordering fields.
IB_INLINE ulint dict_index_get_n_ordering_defined_by_user(const dict_index_t* index);

#ifdef IB_DEBUG

	/// \brief Gets the nth field of an index.
	/// \param [in] index index
	/// \param [in] pos position of field
	/// \return pointer to field object
	IB_INLINE dict_field_t* dict_index_get_nth_field(const dict_index_t* index, ulint pos);

#else // IB_DEBUG 

	#define dict_index_get_nth_field(index, pos) ((index)->fields + (pos))

#endif // IB_DEBUG

/// \brief Gets pointer to the nth column in an index.
/// \param [in] index index
/// \param [in] pos position of the field
/// \return column
IB_INLINE const dict_col_t* dict_index_get_nth_col(const dict_index_t* index, ulint pos);

/// \brief Gets the column number of the nth field in an index.
/// \param [in] index index
/// \param [in] pos position of the field
/// \return column number
IB_INLINE ulint dict_index_get_nth_col_no(const dict_index_t* index, ulint pos);

/// \brief Looks for column n in an index.
/// \param [in] index index
/// \param [in] n column number
/// \return position in internal representation of the index; ULINT_UNDEFINED if not contained
IB_INTERN ulint dict_index_get_nth_col_pos(const dict_index_t* index, ulint n);

/// \brief Returns TRUE if the index contains a column or a prefix of that column.
/// \param [in] index index
/// \param [in] n column number
/// \return TRUE if contains the column or its prefix
IB_INTERN ibool dict_index_contains_col_or_prefix(const dict_index_t* index, ulint n);

/// \brief Looks for a matching field in an index.
/// \param [in] index index from which to search
/// \param [in] index2 index
/// \param [in] n field number in index2
/// \return position in internal representation of the index; ULINT_UNDEFINED if not contained
/// \details The column has to be the same. The column in index must be complete, or must contain a prefix longer than the column in index2. That is, we must be able to construct the prefix in index2 from the prefix in index.
IB_INTERN ulint dict_index_get_nth_field_pos(const dict_index_t* index, const dict_index_t* index2, ulint n);

/// \brief Looks for column n position in the clustered index.
/// \param [in] table table
/// \param [in] n column number
/// \return position in internal representation of the clustered index
IB_INTERN ulint dict_table_get_nth_col_pos(const dict_table_t* table, ulint n);

/// \brief Returns the position of a system column in an index.
/// \param [in] index index
/// \param [in] type DATA_ROW_ID, ...
/// \return position, ULINT_UNDEFINED if not contained
IB_INLINE ulint dict_index_get_sys_col_pos(const dict_index_t* index, ulint type);

/// \brief Adds a column to index.
/// \param [in,out] index index
/// \param [in] table table
/// \param [in] col column
/// \param [in] prefix_len column prefix length
IB_INTERN void dict_index_add_col(dict_index_t* index, const dict_table_t* table, dict_col_t* col, ulint prefix_len);

#ifndef IB_HOTBACKUP
/// \brief Copies types of fields contained in index to tuple.
/// \param [in,out] tuple data tuple
/// \param [in] index index
/// \param [in] n_fields number of field types to copy
IB_INTERN void dict_index_copy_types(dtuple_t* tuple, const dict_index_t* index, ulint n_fields);
#endif // !IB_HOTBACKUP

/// \brief Gets the field column.
/// \param [in] field index field
/// \return field->col, pointer to the table column
IB_INLINE const dict_col_t* dict_field_get_col(const dict_field_t* field);

#ifndef IB_HOTBACKUP

/// \brief Returns an index object if it is found in the dictionary cache.
/// \param [in] index_id index id
/// \return index, NULL if not found
/// \details Assumes that dict_sys->mutex is already being held.
IB_INTERN dict_index_t* dict_index_get_if_in_cache_low(dulint index_id);

#if defined IB_DEBUG || defined IB_BUF_DEBUG

/// \brief Returns an index object if it is found in the dictionary cache.
/// \param [in] index_id index id
/// \return index, NULL if not found
IB_INTERN dict_index_t* dict_index_get_if_in_cache(dulint index_id);

#endif /* IB_DEBUG || IB_BUF_DEBUG */

#ifdef IB_DEBUG

/// \brief Checks that a tuple has n_fields_cmp value in a sensible range, so that no comparison can occur with the page number field in a node pointer.
/// \param [in] index index tree
/// \param [in] tuple tuple used in a search
/// \return TRUE if ok
IB_INTERN ibool dict_index_check_search_tuple(const dict_index_t* index, const dtuple_t* tuple);

#endif /* IB_DEBUG */

/// \brief Builds a node pointer out of a physical record and a page number.
/// \param [in] index index
/// \param [in] rec record for which to build node pointer
/// \param [in] page_no page number to put in node pointer
/// \param [in] heap memory heap where pointer created
/// \param [in] level level of rec in tree: 0 means leaf level
/// \return own: node pointer
IB_INTERN dtuple_t* dict_index_build_node_ptr(const dict_index_t* index, const rec_t* rec, ulint page_no, mem_heap_t* heap, ulint level);

/// \brief Copies an initial segment of a physical record, long enough to specify an index entry uniquely.
/// \param [in] index index
/// \param [in] rec record for which to copy prefix
/// \param [out] n_fields number of fields copied
/// \param [in,out] buf memory buffer for the copied prefix, or NULL
/// \param [in,out] buf_size buffer size
/// \return pointer to the prefix record
IB_INTERN rec_t* dict_index_copy_rec_order_prefix(const dict_index_t* index, const rec_t* rec, ulint* n_fields, byte** buf, ulint* buf_size);

/// \brief Builds a typed data tuple out of a physical record.
/// \param [in] index index
/// \param [in] rec record for which to build data tuple
/// \param [in] n_fields number of data fields
/// \param [in] heap memory heap where tuple created
/// \return own: data tuple
IB_INTERN dtuple_t* dict_index_build_data_tuple(dict_index_t* index, rec_t* rec, ulint n_fields, mem_heap_t* heap);

/// \brief Gets the space id of the root of the index tree.
/// \param [in] index index
/// \return space id
IB_INLINE ulint dict_index_get_space(const dict_index_t* index);

/// \brief Sets the space id of the root of the index tree.
/// \param [in,out] index index
/// \param [in] space space id
IB_INLINE void dict_index_set_space(dict_index_t* index, ulint space);

/// \brief Gets the page number of the root of the index tree.
/// \param [in] tree index
/// \return page number
IB_INLINE ulint dict_index_get_page(const dict_index_t* tree);

/// \brief Sets the page number of the root of index tree.
/// \param [in,out] index index
/// \param [in] page page number
IB_INLINE void dict_index_set_page(dict_index_t* index, ulint page);

/// \brief Gets the read-write lock of the index tree.
/// \param [in] index index
/// \return read-write lock
IB_INLINE rw_lock_t* dict_index_get_lock(dict_index_t* index);

/// \brief Returns free space reserved for future updates of records.
/// \return number of free bytes on page, reserved for updates
/// \details This is relevant only in the case of many consecutive inserts, as updates which make the records bigger might fragment the index.
IB_INLINE ulint dict_index_get_space_reserve(void);

/// \brief Calculates the minimum record length in an index.
/// \param [in] index index
IB_INTERN ulint dict_index_calc_min_rec_len(const dict_index_t* index);

/// \brief Calculates new estimates for table and index statistics.
/// \param [in,out] table table
/// \param [in] has_dict_mutex TRUE if the caller has the dictionary mutex
/// \details The statistics are used in query optimization.
IB_INTERN void dict_update_statistics_low(dict_table_t* table, ibool has_dict_mutex);

/// \brief Calculates new estimates for table and index statistics.
/// \param [in,out] table table
/// \details The statistics are used in query optimization.
IB_INTERN void dict_update_statistics(dict_table_t* table);

/// \brief Reserves the dictionary system mutex.
IB_INTERN void dict_mutex_enter(void);

/// \brief Releases the dictionary system mutex.
IB_INTERN void dict_mutex_exit(void);

/// \brief Lock the appropriate mutex to protect index->stat_n_diff_key_vals[].
/// \param [in] index index
/// \details index->id is used to pick the right mutex and it should not change before dict_index_stat_mutex_exit() is called on this index.
IB_INTERN void dict_index_stat_mutex_enter(const dict_index_t* index);

/// \brief Unlock the appropriate mutex that protects index->stat_n_diff_key_vals[].
/// \param [in] index index
IB_INTERN void dict_index_stat_mutex_exit(const dict_index_t* index);

/// \brief Checks if the database name in two table names is the same.
/// \param [in] name1 table name in the form dbname '/' tablename
/// \param [in] name2 table name in the form dbname '/' tablename
/// \return TRUE if same db name
IB_INTERN ibool dict_tables_IB_HAVE_same_db(const char* name1, const char* name2);

/// \brief Removes an index from the cache
/// \param [in,out] table table
/// \param [in] index index
IB_INTERN void dict_index_remove_from_cache(dict_table_t* table, dict_index_t* index);

/// \brief Get index by name
/// \param [in] table table
/// \param [in] name name of the index to find
/// \return index, NULL if does not exist
IB_INTERN dict_index_t* dict_table_get_index_on_name(dict_table_t* table, const char* name);

/// \brief In case there is more than one index with the same name return the index with the min(id).
/// \param [in] table table
/// \param [in] name name of the index to find
/// \return index, NULL if does not exist
IB_INTERN dict_index_t* dict_table_get_index_on_name_and_min_id(dict_table_t* table, const char* name);

/// \brief Locks the data dictionary exclusively for performing a table create or other data dictionary modification operation.
/// \param [in] trx transaction
IB_INTERN void dict_lock_data_dictionary(trx_t* trx);

/// \brief Unlocks the data dictionary exclusive lock.
/// \param [in] trx transaction
IB_INTERN void dict_unlock_data_dictionary(trx_t* trx);

/// \brief Locks the data dictionary in shared mode from modifications, for performing foreign key check, rollback, or other operation invisible to users.
/// \param [in] trx transaction
IB_INTERN void dict_freeze_data_dictionary(trx_t* trx);

/// \brief Unlocks the data dictionary shared lock.
/// \param [in] trx transaction
IB_INTERN void dict_unfreeze_data_dictionary(trx_t* trx);

/// \brief Reset dict variables.
IB_INTERN void dict_var_init(void);

/* Buffers for storing detailed information about the latest foreign key
and unique key errors */
extern ib_stream_t dict_foreign_err_file;
extern mutex_t	dict_foreign_err_mutex; /* mutex protecting the buffers */

/** the dictionary system */
extern dict_sys_t*	dict_sys;
/** the data dictionary rw-latch protecting dict_sys */
extern rw_lock_t	dict_operation_lock;

/* Dictionary system struct */
struct dict_sys_struct{
	mutex_t		mutex;		/*!< mutex protecting the data
					dictionary; protects also the
					disk-based dictionary system tables;
					this mutex serializes CREATE TABLE
					and DROP TABLE, as well as reading
					the dictionary data for a table from
					system tables */
	dulint		row_id;		/*!< the next row id to assign;
					NOTE that at a checkpoint this
					must be written to the dict system
					header and flushed to a file; in
					recovery this must be derived from
					the log records */
	hash_table_t*	table_hash;	/*!< hash table of the tables, based
					on name */
	hash_table_t*	table_id_hash;	/*!< hash table of the tables, based
					on id */
	UT_LIST_BASE_NODE_T(dict_table_t)
			table_LRU;	/*!< LRU list of tables */
	ulint		size;		/*!< varying space in bytes occupied
					by the data dictionary table and
					index objects */
	dict_table_t*	sys_tables;	/*!< SYS_TABLES table */
	dict_table_t*	sys_columns;	/*!< SYS_COLUMNS table */
	dict_table_t*	sys_indexes;	/*!< SYS_INDEXES table */
	dict_table_t*	sys_fields;	/*!< SYS_FIELDS table */
};
#endif /* !IB_HOTBACKUP */

/** dummy index for ROW_FORMAT=REDUNDANT supremum and infimum records */
extern dict_index_t*	dict_ind_redundant;

/** dummy index for ROW_FORMAT=COMPACT supremum and infimum records */
extern dict_index_t*	dict_ind_compact;

/**********************************************************************//**
Inits dict_ind_redundant and dict_ind_compact. */
IB_INTERN
void
dict_ind_init(void);
/*===============*/

/**********************************************************************//**
Closes the data dictionary module. */
IB_INTERN
void
dict_close(void);
/*============*/

#ifndef IB_DO_NOT_INLINE
	#include "dict_dict.inl"
#endif
