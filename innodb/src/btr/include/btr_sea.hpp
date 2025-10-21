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

/// \file btr_sea.hpp
/// \brief The index tree adaptive search
/// \details Originally created by Heikki Tuuri in 2/17/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"

#include "rem_rec.hpp"
#include "dict_dict.hpp"
#include "btr_types.hpp"
#include "mtr_mtr.hpp"
#include "ha_ha.hpp"

/// \brief Creates and initializes the adaptive search system at a database start.
/// \param [in] hash_size hash index hash table size
IB_INTERN void btr_search_sys_create(ulint hash_size);
/// \brief Frees the adaptive search system at a database shutdown.
IB_INTERN void btr_search_sys_free(void);

/// \brief Disable the adaptive hash search system and empty the index.
IB_INTERN void btr_search_disable(void);
/// \brief Enable the adaptive hash search system.
IB_INTERN void btr_search_enable(void);

/// \brief Returns search info for an index.
/// \return search info; search mutex reserved
/// \param [in] index index
IB_INLINE btr_search_t* btr_search_get_info(dict_index_t* index);
/// \brief Creates and initializes a search info struct.
/// \return own: search info struct
/// \param [in] heap heap where created
IB_INTERN btr_search_t* btr_search_info_create(mem_heap_t* heap);
/// \brief Returns the value of ref_count.
/// \details The value is protected by btr_search_latch.
/// \return ref_count value.
/// \param [in] info search info.
IB_INTERN ulint btr_search_info_get_ref_count(btr_search_t* info);
/// \brief Updates the search info.
/// \param [in] index index of the cursor
/// \param [in] cursor cursor which was just positioned
IB_INLINE void btr_search_info_update(dict_index_t* index, btr_cur_t* cursor);
/// \brief Tries to guess the right search position based on the hash search info of the index.
/// \details Note that if mode is PAGE_CUR_LE, which is used in inserts, and the function returns TRUE, then cursor->up_match and cursor->low_match both have sensible values.
/// \return TRUE if succeeded
/// \param [in] index index
/// \param [in] info index search info
/// \param [in] tuple logical record
/// \param [in] mode PAGE_CUR_L, ...
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...
/// \param [out] cursor tree cursor
/// \param [in] has_search_latch latch mode the caller currently has on btr_search_latch: RW_S_LATCH, RW_X_LATCH, or 0
/// \param [in] mtr mtr
IB_INTERN ibool btr_search_guess_on_hash(dict_index_t* index, btr_search_t* info, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_cur_t* cursor, ulint has_search_latch, mtr_t* mtr);
/// \brief Moves or deletes hash entries for moved records.
/// \details If new_page is already hashed, then the hash index for page, if any, is dropped. If new_page is not hashed, and page is hashed, then a new hash index is built to new_page with the same parameters as page (this often happens when a page is split).
/// \param [in] new_block records are copied to this page
/// \param [in] block index page from which records were copied, and the copied records will be deleted from this page
/// \param [in] index record descriptor
IB_INTERN void btr_search_move_or_delete_hash_entries(buf_block_t* new_block, buf_block_t* block, dict_index_t* index);
/// \brief Drops a page hash index.
/// \param [in] block block containing index page, s- or x-latched, or an index page for which we know that block->buf_fix_count == 0
IB_INTERN void btr_search_drop_page_hash_index(buf_block_t* block);
/// \brief Drops a page hash index when a page is freed from a fseg to the file system.
/// \details Drops possible hash index if the page happens to be in the buffer pool.
/// \param [in] space space id
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] page_no page number
IB_INTERN void btr_search_drop_page_hash_when_freed(ulint space, ulint zip_size, ulint page_no);
/// \brief Updates the page hash index when a single record is inserted on a page.
/// \param [in] cursor cursor which was positioned to the place to insert using btr_cur_search_..., and the new record has been inserted next to the cursor
IB_INTERN void btr_search_update_hash_node_on_insert(btr_cur_t* cursor);
/// \brief Updates the page hash index when a single record is inserted on a page.
/// \param [in] cursor cursor which was positioned to the place to insert using btr_cur_search_..., and the new record has been inserted next to the cursor
IB_INTERN void btr_search_update_hash_on_insert(btr_cur_t* cursor);
/// \brief Updates the page hash index when a single record is deleted from a page.
/// \param [in] cursor cursor which was positioned on the record to delete using btr_cur_search_..., the record is not yet deleted
IB_INTERN void btr_search_update_hash_on_delete(btr_cur_t* cursor);
/// \brief Validates the search system.
/// \return TRUE if ok
IB_INTERN ibool btr_search_validate(void);
/// \brief Reset global configuration variables.
IB_INTERN void btr_search_var_init(void);
/// \brief Closes the adaptive search system at a database shutdown.
IB_INTERN void btr_search_sys_close(void);

// Flag: has the search system been enabled? Protected by btr_search_latch and btr_search_enabled_mutex.
extern char btr_search_enabled;

// The hash index system
typedef struct btr_search_sys_struct btr_search_sys_t;

// The hash index system
struct btr_search_sys_struct{
	hash_table_t* hash_index; // the adaptive hash index, mapping dtuple_fold values to rec_t pointers on index pages
};

// The adaptive hash index
extern btr_search_sys_t* btr_search_sys;

// The latch protecting the adaptive search system. This latch protects the (1) hash index; (2) columns of a record to which we have a pointer in the hash index; but does NOT protect: (3) next record offset field in a record; (4) next or previous records on the same page. Bear in mind (3) and (4) when using the hash index.
extern rw_lock_t* btr_search_latch_temp;

// The latch protecting the adaptive search system
#define btr_search_latch	(*btr_search_latch_temp)

#ifdef IB_SEARCH_PERF_STAT
// Number of successful adaptive hash index lookups
extern ulint btr_search_n_succ;
// Number of failed adaptive hash index lookups
extern ulint btr_search_n_hash_fail;
#endif /* IB_SEARCH_PERF_STAT */

// After change in n_fields or n_bytes in info, this many rounds are waited before starting the hash analysis again: this is to save CPU time when there is no hope in building a hash index.
constinit ulint BTR_SEARCH_HASH_ANALYSIS = 17;

// Limit of consecutive searches for trying a search shortcut on the search pattern
constinit ulint BTR_SEARCH_ON_PATTERN_LIMIT = 3;

// Limit of consecutive searches for trying a search shortcut using the hash index
constinit ulint BTR_SEARCH_ON_HASH_LIMIT = 3;

// We do this many searches before trying to keep the search latch over calls from MySQL. If we notice someone waiting for the latch, we again set this much timeout. This is to reduce contention.
constinit ulint BTR_SEA_TIMEOUT = 10000;

#ifndef IB_DO_NOT_INLINE
#include "btr0sea.inl"
#endif
