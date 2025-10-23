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

/// \file dict_mem.cpp
/// \brief Data dictionary memory object creation
/// \details Originally created by Heikki Tuuri in 1/8/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "dict_mem.hpp"

#include "rem_rec.hpp"
#include "data_type.hpp"
#include "mach_data.hpp"
#include "dict_dict.hpp"

#ifndef IB_HOTBACKUP
	#include "lock_lock.h"
#endif // !IB_HOTBACKUP

// -----------------------------------------------------------------------------------------
// constants
// -----------------------------------------------------------------------------------------

constinit ulint DICT_HEAP_SIZE = 100; //!< initial memory heap size when creating a table or index object

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

/// \brief Append 'name' to 'col_names'.
/// \param [in] col_names existing column names, or NULL
/// \param [in] cols number of existing columns
/// \param [in] name new column name
/// \param [in] heap heap
/// \return new column names array
/// \see dict_table_t::col_names
static const char* dict_add_col_name(const char* col_names, ulint cols, const char* name, mem_heap_t* heap);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

IB_INTERN dict_table_t* dict_mem_table_create(const char* name, ulint space, ulint n_cols, ulint flags)
{
	ut_ad(name);
	ut_a(!(flags & (~0 << DICT_TF2_BITS)));
	mem_heap_t* heap = mem_heap_create(DICT_HEAP_SIZE);
	dict_table_t* table = mem_heap_zalloc(heap, sizeof(dict_table_t));
	table->heap = heap;
	table->flags = (unsigned int) flags;
	table->name = mem_heap_strdup(heap, name);
	table->space = (unsigned int) space;
	table->n_cols = (unsigned int) (n_cols + DATA_N_SYS_COLS);
	table->cols = mem_heap_alloc(heap, (n_cols + DATA_N_SYS_COLS) * sizeof(dict_col_t));
	ut_d(table->magic_n = DICT_TABLE_MAGIC_N);
	return table;
}

IB_INTERN void dict_mem_table_free(dict_table_t* table)
{
	ut_ad(table);
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
	ut_d(table->cached = FALSE);
	mem_heap_free(table->heap);
}


IB_INTERN void dict_mem_table_add_col(dict_table_t* table, mem_heap_t* heap, const char* name, ulint mtype, ulint prtype, ulint len)
{
	ut_ad(table);
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
	ut_ad(!heap == !name);
	ulint i = table->n_def++;
	if (name) {
		if (IB_UNLIKELY(table->n_def == table->n_cols)) {
			heap = table->heap;
		}
		if (IB_LIKELY(i) && IB_UNLIKELY(!table->col_names)) {
			// All preceding column names are empty.                                                                                                                  
			char* s = mem_heap_zalloc(heap, table->n_def);
			table->col_names = s;
		}
		table->col_names = dict_add_col_name(table->col_names, i, name, heap);
	}
	dict_col_t* col = dict_table_get_nth_col(table, i);
	col->ind = (unsigned int) i;
	col->ord_part = 0;
	col->mtype = (unsigned int) mtype;
	col->prtype = (unsigned int) prtype;
	col->len = (unsigned int) len;
#ifndef IB_HOTBACKUP
	ulint mbminlen;
	ulint mbmaxlen;
	dtype_get_mblen(mtype, prtype, &mbminlen, &mbmaxlen);
	col->mbminlen = (unsigned int) mbminlen;
	col->mbmaxlen = (unsigned int) mbmaxlen;
#endif /* !IB_HOTBACKUP */
}

IB_INTERN dict_index_t* dict_mem_index_create(const char* table_name, const char* index_name, ulint space, ulint type, ulint n_fields)
{
	ut_ad(table_name && index_name);
	mem_heap_t* heap = mem_heap_create(DICT_HEAP_SIZE);
	dict_index_t* index = mem_heap_zalloc(heap, sizeof(dict_index_t));
	index->heap = heap;
	index->type = type;
#ifndef IB_HOTBACKUP
	index->space = (unsigned int) space;
#endif /* !IB_HOTBACKUP */
	index->name = mem_heap_strdup(heap, index_name);
	index->table_name = table_name;
	index->n_fields = (unsigned int) n_fields;
	index->fields = mem_heap_alloc(heap, 1 + n_fields * sizeof(dict_field_t));
	/* The '1 +' above prevents allocation of an empty mem block.                                                                                              */
#ifdef IB_DEBUG
	index->magic_n = DICT_INDEX_MAGIC_N;
#endif /* IB_DEBUG */
	return index;
}

IB_INTERN dict_foreign_t* dict_mem_foreign_create(void)
{
	mem_heap_t* heap = mem_heap_create(100);
	dict_foreign_t* foreign = mem_heap_zalloc(heap, sizeof(dict_foreign_t));
	foreign->heap = heap;
	return foreign;
}

IB_INTERN void dict_mem_index_add_field(dict_index_t* index, const char* name, ulint prefix_len)
{
	ut_ad(index);
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	index->n_def++;
	dict_field_t* field = dict_index_get_nth_field(index, index->n_def - 1);
	field->name = name;
	field->prefix_len = (unsigned int) prefix_len;
}

IB_INTERN void dict_mem_index_free(dict_index_t* index)
{
	ut_ad(index);
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	mem_heap_free(index->heap);
}

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

static const char* dict_add_col_name(const char* col_names, ulint cols, const char* name, mem_heap_t* heap)
{
	ulint old_len;
	ut_ad(!cols == !col_names);
	// Find out length of existing array.                                                                                                                                       
	if (col_names) {
		const char* s = col_names;
		ulint i;
		for (i = 0; i < cols; i++) {
			s += strlen(s) + 1;
		}
		old_len = s - col_names;
	} else {
		old_len = 0;
	}
	ulint new_len = strlen(name) + 1;
	ulint total_len = old_len + new_len;
	char* res = mem_heap_alloc(heap, total_len);
	if (old_len > 0) {
		memcpy(res, col_names, old_len);
	}
	memcpy(res + old_len, name, new_len);
	return res;
}
