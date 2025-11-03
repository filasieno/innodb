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

/// \file dict_mem.hpp
/// \brief Data dictionary memory object creation
/// \details Originally created by Heikki Tuuri in 1/8/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"
#include "dict_types.hpp"
#include "data_type.hpp"
#include "mem_mem.hpp"
#include "rem_types.hpp"
#include "btr_types.hpp"
#include "ut_mem.hpp"
#include "ut_lst.hpp"
#include "ut_rnd.hpp"
#include "ut_byte.hpp"
#include "hash_hash.hpp"
#include "trx_types.hpp"

#ifndef IB_HOTBACKUP
#include "lock_types.hpp"
#include "que_types.hpp"
#include "sync_rw.hpp"
#endif // !IB_HOTBACKUP



constinit ulint DICT_CLUSTERED = 1;
constinit ulint DICT_UNIQUE = 2;
constinit ulint DICT_UNIVERSAL = 4;
constinit ulint DICT_IBUF = 8;

constinit ulint DICT_TABLE_ORDINARY = 1;

#if 0 /* not implemented */
#define	DICT_TABLE_CLUSTER_MEMBER	2
#define	DICT_TABLE_CLUSTER		    3 /* this means that the table is really a cluster definition */
#endif

constinit ulint DICT_TF_COMPACT = 1;

constinit ulint DICT_TF_ZSSIZE_SHIFT = 1;
constinit ulint DICT_TF_ZSSIZE_MASK = (15 << DICT_TF_ZSSIZE_SHIFT);
constinit ulint DICT_TF_ZSSIZE_MAX = (IB_PAGE_SIZE_SHIFT - PAGE_ZIP_MIN_SIZE_SHIFT + 1);

constinit ulint DICT_TF_FORMAT_SHIFT = 5;
constinit ulint DICT_TF_FORMAT_MASK = ((~(~0 << (DICT_TF_BITS - DICT_TF_FORMAT_SHIFT))) << DICT_TF_FORMAT_SHIFT);
constinit ulint DICT_TF_FORMAT_51 = 0;
constinit ulint DICT_TF_FORMAT_ZIP = 1;
constinit ulint DICT_TF_FORMAT_MAX = DICT_TF_FORMAT_ZIP;
constinit ulint DICT_TF_BITS = 6;

#if (1 << (DICT_TF_BITS - DICT_TF_FORMAT_SHIFT)) <= DICT_TF_FORMAT_MAX
#error "DICT_TF_BITS is insufficient for DICT_TF_FORMAT_MAX"
#endif


/** @brief Additional table flags.

These flags will be stored in SYS_TABLES.MIX_LEN.  All unused flags
will be written as 0.  The column may contain garbage for tables
created with old versions of InnoDB that only implemented
ROW_FORMAT=REDUNDANT. */
constinit ulint DICT_TF2_SHIFT = DICT_TF_BITS;
constinit ulint DICT_TF2_TEMPORARY = 1;
constinit ulint DICT_TF2_BITS = (DICT_TF2_SHIFT + 1);

/// \brief Creates a table memory object.
/// \param [in] name table name
/// \param [in] space space where the clustered index of the table is placed; this parameter is ignored if the table is made a member of a cluster
/// \param [in] n_cols number of columns
/// \param [in] flags table flags
/// \return own: table object
IB_INTERN dict_table_t* dict_mem_table_create(const char* name, ulint space, ulint n_cols, ulint flags);

/// \brief Free a table memory object.
/// \param [in] table table
IB_INTERN void dict_mem_table_free(dict_table_t* table);

/// \brief Adds a column definition to a table.
/// \param [in] table table
/// \param [in] heap temporary memory heap, or NULL
/// \param [in] name column name, or NULL
/// \param [in] mtype main datatype
/// \param [in] prtype precise type
/// \param [in] len precision
IB_INTERN void dict_mem_table_add_col(dict_table_t* table, mem_heap_t* heap, const char* name, ulint mtype, ulint prtype, ulint len);

/// \brief Creates an index memory object.
/// \param [in] table_name table name
/// \param [in] index_name index name
/// \param [in] space space where the index tree is placed, ignored if the index is of the clustered type
/// \param [in] type DICT_UNIQUE, DICT_CLUSTERED, ... ORed
/// \param [in] n_fields number of fields
/// \return own: index object
IB_INTERN dict_index_t* dict_mem_index_create(const char* table_name, const char* index_name, ulint space, ulint type, ulint n_fields);

/// \brief Adds a field definition to an index.
/// \param [in] index index
/// \param [in] name column name
/// \param [in] prefix_len 0 or the column prefix length in a column prefix index like INDEX (textcol(25))
/// \note Does not take a copy of the column name if the field is a column. The memory occupied by the column name may be released only after publishing the index.
IB_INTERN void dict_mem_index_add_field(dict_index_t* index, const char* name, ulint prefix_len);

/// \brief Frees an index memory object.
/// \param [in] index index
IB_INTERN void dict_mem_index_free(dict_index_t* index);

/// \brief Creates and initializes a foreign constraint memory object.
/// \return own: foreign constraint struct
IB_INTERN dict_foreign_t* dict_mem_foreign_create(void);

/** Data structure for a column in a table */
struct dict_col_struct{
	/* @{ */
	DTYPE_FIELDS
	/* @} */

	unsigned	ind:10;		/*!< table column position
					(starting from 0) */
	unsigned	ord_part:1;	/*!< nonzero if this column
					appears in the ordering fields
					of an index */
};

/** @brief DICT_MAX_INDEX_COL_LEN is measured in bytes and is the maximum
indexed column length (or indexed prefix length).

It is set to 3*256, so that one can create a column prefix index on
256 characters of a TEXT or VARCHAR column also in the UTF-8
charset. In that charset, a character may take at most 3 bytes.  This
constant MUST NOT BE CHANGED, or the compatibility of InnoDB data
files would be at risk! */
constinit ulint DICT_MAX_INDEX_COL_LEN = REC_MAX_INDEX_COL_LEN;

/** Data structure for a field in an index */
struct dict_field_struct{
	dict_col_t*	col;		/*!< pointer to the table column */
	const char*	name;		/*!< name of the column */
	unsigned	prefix_len:10;	/*!< 0 or the length of the column
					prefix in bytes e.g., for
					INDEX (textcol(25));
					must be smaller than
					DICT_MAX_INDEX_COL_LEN; NOTE that
					in the UTF-8 charset, MySQL sets this
					to 3 * the prefix len in UTF-8 chars */
	unsigned	fixed_len:10;	/*!< 0 or the fixed length of the
					column if smaller than
					DICT_MAX_INDEX_COL_LEN */
};

/** Data structure for an index.  Most fields will be
initialized to 0, NULL or FALSE in dict_mem_index_create(). */
struct dict_index_struct{
	dulint		id;	/*!< id of the index */
	mem_heap_t*	heap;	/*!< memory heap */
	const char*	name;	/*!< index name */
	const char*	table_name;/*!< table name */
	dict_table_t*	table;	/*!< back pointer to table */
#ifndef IB_HOTBACKUP
	unsigned	space:32;
				/*!< space where the index tree is placed */
	unsigned	page:32;/*!< index tree root page number */
#endif /* !IB_HOTBACKUP */
	unsigned	type:4;	/*!< index type (DICT_CLUSTERED, DICT_UNIQUE,
				DICT_UNIVERSAL, DICT_IBUF) */
	unsigned	trx_id_offset:10;/*!< position of the trx id column
				in a clustered index record, if the fields
				before it are known to be of a fixed size,
				0 otherwise */
	unsigned	n_user_defined_cols:10;
				/*!< number of columns the user defined to
				be in the index: in the internal
				representation we add more columns */
	unsigned	n_uniq:10;/*!< number of fields from the beginning
				which are enough to determine an index
				entry uniquely */
	unsigned	n_def:10;/*!< number of fields defined so far */
	unsigned	n_fields:10;/*!< number of fields in the index */
	unsigned	n_nullable:10;/*!< number of nullable fields */
	unsigned	cached:1;/*!< TRUE if the index object is in the
				dictionary cache */
	unsigned	to_be_dropped:1;
				/*!< TRUE if this index is marked to be
				dropped in ha_innobase::prepare_drop_index(),
				otherwise FALSE */
	dict_field_t*	fields;	/*!< array of field descriptions */
#ifndef IB_HOTBACKUP
	UT_LIST_NODE_T(dict_index_t)
			indexes;/*!< list of indexes of the table */
	btr_search_t*	search_info; /*!< info used in optimistic searches */
	/*----------------------*/
	/** Statistics for query optimization */
	/* @{ */
	ib_int64_t*	stat_n_diff_key_vals;
				/*!< approximate number of different
				key values for this index, for each
				n-column prefix where n <=
				dict_get_n_unique(index); we
				periodically calculate new
				estimates */
	ulint		stat_index_size;
				/*!< approximate index size in
				database pages */
	ulint		stat_n_leaf_pages;
				/*!< approximate number of leaf pages in the
				index tree */
	rw_lock_t	lock;	/* read-write lock protecting the upper levels
				of the index tree */
	void*		cmp_ctx;/* Client compare context. For use defined
				column types and BLOBs the client is
				responsible for comparing the column values.
				This field is the argument for the callback
				compare function. */
	ib_uint64_t	trx_id; /* id of the transaction that created this
				index, or 0 if the index existed
				when InnoDB was started up */
	/* @} */
#endif /* !IB_HOTBACKUP */
#ifdef IB_DEBUG
	ulint		magic_n;/*!< magic number */
constinit ulint DICT_INDEX_MAGIC_N = 76789786;
#endif
};

/** Data structure for a foreign key constraint; an example:
FOREIGN KEY (A, B) REFERENCES TABLE2 (C, D).  Most fields will be
initialized to 0, NULL or FALSE in dict_mem_foreign_create(). */
struct dict_foreign_struct{
	mem_heap_t*	heap;		/*!< this object is allocated from
					this memory heap */
	char*		id;		/*!< id of the constraint as a
					null-terminated string */
	unsigned	n_fields:10;	/*!< number of indexes' first fields
					for which the foreign key
					constraint is defined: we allow the
					indexes to contain more fields than
					mentioned in the constraint, as long
					as the first fields are as mentioned */
	unsigned	type:6;		/*!< 0 or DICT_FOREIGN_ON_DELETE_CASCADE
					or DICT_FOREIGN_ON_DELETE_SET_NULL */
	char*		foreign_table_name;/*!< foreign table name */
	dict_table_t*	foreign_table;	/*!< table where the foreign key is */
	const char**	foreign_col_names;/*!< names of the columns in the
					foreign key */
	char*		referenced_table_name;/*!< referenced table name */
	dict_table_t*	referenced_table;/*!< table where the referenced key
					is */
	const char**	referenced_col_names;/*!< names of the referenced
					columns in the referenced table */
	dict_index_t*	foreign_index;	/*!< foreign index; we require that
					both tables contain explicitly defined
					indexes for the constraint: InnoDB
					does not generate new indexes
					implicitly */
	dict_index_t*	referenced_index;/*!< referenced index */
	UT_LIST_NODE_T(dict_foreign_t)
			foreign_list;	/*!< list node for foreign keys of the
					table */
	UT_LIST_NODE_T(dict_foreign_t)
			referenced_list;/*!< list node for referenced
					keys of the table */
};

/** The flags for ON_UPDATE and ON_DELETE can be ORed; the default is that
a foreign key constraint is enforced, therefore RESTRICT just means no flag */
constinit ulint DICT_FOREIGN_ON_DELETE_CASCADE = 1;
constinit ulint DICT_FOREIGN_ON_DELETE_SET_NULL = 2;
constinit ulint DICT_FOREIGN_ON_UPDATE_CASCADE = 4;
constinit ulint DICT_FOREIGN_ON_UPDATE_SET_NULL = 8;
constinit ulint DICT_FOREIGN_ON_DELETE_NO_ACTION = 16;
constinit ulint DICT_FOREIGN_ON_UPDATE_NO_ACTION = 32;

constinit ulint DICT_TABLE_MAGIC_N = 76333786;

/** Data structure for a database table.  Most fields will be
initialized to 0, NULL or FALSE in dict_mem_table_create(). */
struct dict_table_struct{
	dulint		id;	    /*!< id of the table */
	mem_heap_t*	heap;	/*!< memory heap */
	const char*	name;	/*!< table name */
	const char*	dir_path_of_temp_table;/*!< NULL or the directory path
				where a TEMPORARY table that was explicitly
				created by a user should be placed if
				innodb_file_per_table is defined;
				in Unix this is usually /tmp/..., in Windows
				temp\... */
	unsigned	space:32;
				/*!< space where the clustered index of the
				table is placed */
	unsigned	flags:DICT_TF2_BITS;/*!< DICT_TF_COMPACT, ... */
	unsigned	ibd_file_missing:1;
				/*!< TRUE if this is in a single-table
				tablespace and the .ibd file is missing; then
				we must return in ha_innodb.cc an error if the
				user tries to query such an orphaned table */
	unsigned	tablespace_discarded:1;
				/*!< this flag is set TRUE when the user
				calls DISCARD TABLESPACE on this
				table, and reset to FALSE in IMPORT
				TABLESPACE */
	unsigned	cached:1;/*!< TRUE if the table object has been added
				to the dictionary cache */
	unsigned	n_def:10;/*!< number of columns defined so far */
	unsigned	n_cols:10;/*!< number of columns */
	dict_col_t*	cols;	/*!< array of column descriptions */
	const char*	col_names;
				/*!< Column names packed in a character string
				"name1\0name2\0...nameN\0".  Until
				the string contains n_cols, it will be
				allocated from a temporary heap.  The final
				string will be allocated from table->heap. */
#ifndef IB_HOTBACKUP
	hash_node_t	name_hash; /*!< hash chain node */
	hash_node_t	id_hash; /*!< hash chain node */
	UT_LIST_BASE_NODE_T(dict_index_t)
			indexes; /*!< list of indexes of the table */
	UT_LIST_BASE_NODE_T(dict_foreign_t)
			foreign_list;/*!< list of foreign key constraints
				in the table; these refer to columns
				in other tables */
	UT_LIST_BASE_NODE_T(dict_foreign_t)
			referenced_list;/*!< list of foreign key constraints
				which refer to this table */
	UT_LIST_NODE_T(dict_table_t)
			table_LRU; /*!< node of the LRU list of tables */
	ulint		n_handles_opened;
				/*!< count of how many handles the user has
				opened to this table; dropping of the table is
				NOT allowed until this count gets to zero */
	ulint		n_foreign_key_checks_running;
				/*!< count of how many foreign key check
				operations are currently being performed
				on the table: we cannot drop the table while
				there are foreign key checks running on
				it! */
	UT_LIST_BASE_NODE_T(lock_t)
			locks; /*!< list of locks on the table */
#ifdef IB_DEBUG
	/*----------------------*/
	ibool		does_not_fit_in_memory;
				/*!< this field is used to specify in
				simulations tables which are so big
				that disk should be accessed: disk
				access is simulated by putting the
				thread to sleep for a while; NOTE that
				this flag is not stored to the data
				dictionary on disk, and the database
				will forget about value TRUE if it has
				to reload the table definition from
				disk */
#endif /* IB_DEBUG */
	/*----------------------*/
	unsigned	big_rows:1;
				/*!< flag: TRUE if the maximum length of
				a single row exceeds BIG_ROW_SIZE;
				initialized in dict_table_add_to_cache() */
				/** Statistics for query optimization */
				/* @{ */
	unsigned	stat_initialized:1; /*!< TRUE if statistics have
				been calculated the first time
				after database startup or table creation */
	ib_int64_t	stat_n_rows;
				/*!< approximate number of rows in the table;
				we periodically calculate new estimates */
	ulint		stat_clustered_index_size;
				/*!< approximate clustered index size in
				database pages */
	ulint		stat_sum_of_other_index_sizes;
				/*!< other indexes in database pages */
	ulint		stat_modified_counter;
				/*!< when a row is inserted, updated,
				or deleted, we add 1 to this number; we
			       	calculate new estimates for the stat_...
			       	values for the table and the indexes at an
			       	interval of 2 GB or when about 1 / 16 of table
			       	has been modified; also when an estimate
			       	operation is called for; the counter is reset
			       	to zero at statistics calculation; this
			       	counter is not protected by any latch, because
			       	this is only used for heuristics */
				/* @} */
	/*----------------------*/
#endif // !IB_HOTBACKUP 

#ifdef IB_DEBUG
	ulint		magic_n;/*!< magic number */

#endif // IB_DEBUG
};

