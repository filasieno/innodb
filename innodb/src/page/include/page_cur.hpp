// Copyright (c) 1994, 2009, Innobase Oy. All Rights Reserved.
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

/// \file page_cur.hpp
/// \brief The page cursor
/// \details Originally created on 10/4/1994 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"

#include "buf_types.hpp"
#include "page_page.hpp"
#include "rem_rec.hpp"
#include "data_data.hpp"
#include "mtr_mtr.hpp"

constinit ulint PAGE_CUR_ADAPT = 0;

/* Page cursor search modes; the values must be in this order! */

constinit ulint PAGE_CUR_UNSUPP = 0;
constinit ulint PAGE_CUR_G = 1;
constinit ulint PAGE_CUR_GE = 2;
constinit ulint PAGE_CUR_L = 3;
constinit ulint PAGE_CUR_LE = 4;
/*#define PAGE_CUR_LE_OR_EXTENDS 5*/ /* This is a search mode used in
				 "column LIKE 'abc%' ORDER BY column DESC";
				 we have to find strings which are <= 'abc' or
				 which extend it */
#ifdef IB_SEARCH_DEBUG
	#define PAGE_CUR_DBG	6	/* As PAGE_CUR_LE, but skips search shortcut */
#endif /* IB_SEARCH_DEBUG */

#ifdef IB_DEBUG


	/// \brief Gets pointer to the page frame where the cursor is positioned.
	/// \param [in] cur page cursor
	/// \return page
	IB_INLINE page_t* page_cur_get_page(page_cur_t* cur);

	/// \brief Gets pointer to the buffer block where the cursor is positioned.
	/// \param [in] cur page cursor
	/// \return page
	IB_INLINE buf_block_t* page_cur_get_block(page_cur_t* cur);

	/// \brief Gets pointer to the page zip descriptor where the cursor is positioned.
	/// \param [in] cur page cursor
	/// \return page zip descriptor
	IB_INLINE page_zip_des_t* page_cur_get_page_zip(page_cur_t* cur);

	/// \brief Gets the record where the cursor is positioned.
	/// \param [in] cur page cursor
	/// \return record
	IB_INLINE rec_t* page_cur_get_rec(page_cur_t* cur);
#else  // IB_DEBUG 
	#define page_cur_get_page(cur)		page_align((cur)->rec)
	#define page_cur_get_block(cur)	(cur)->block
	#define page_cur_get_page_zip(cur)	buf_block_get_page_zip((cur)->block)
	#define page_cur_get_rec(cur)		(cur)->rec
#endif // IB_DEBUG 

/// \brief Sets the cursor object to point before the first user record on the page.
/// \param [in] block buffer block
/// \param [in,out] cur page cursor
IB_INLINE void page_cur_set_before_first(const buf_block_t* block, page_cur_t* cur);

/// \brief Sets the cursor object to point after the last user record on the page.
/// \param [in] block buffer block
/// \param [in,out] cur page cursor
IB_INLINE void page_cur_set_after_last(const buf_block_t* block, page_cur_t* cur);

/// \brief Returns TRUE if the cursor is before first user record on page.
/// \param [in] cur cursor
/// \return TRUE if at start
IB_INLINE ibool page_cur_is_before_first(const page_cur_t* cur);

/// \brief Returns TRUE if the cursor is after last user record.
/// \param [in] cur cursor
/// \return TRUE if at end
IB_INLINE ibool page_cur_is_after_last(const page_cur_t* cur);

/// \brief Positions the cursor on the given record.
/// \param [in] rec record to position on
/// \param [in] block buffer block
/// \param [in,out] cur page cursor
IB_INLINE void page_cur_position(const rec_t* rec, const buf_block_t* block, page_cur_t* cur);

/// \brief Invalidates a page cursor by setting the record pointer NULL.
/// \param [in,out] cur page cursor
IB_INLINE void page_cur_invalidate(page_cur_t* cur);

/// \brief Moves the cursor to the next record on page.
/// \param [in,out] cur page cursor
IB_INLINE void page_cur_move_to_next(page_cur_t* cur);

/// \brief Moves the cursor to the previous record on page.
/// \param [in,out] cur page cursor
IB_INLINE void page_cur_move_to_prev(page_cur_t* cur);
#ifndef IB_HOTBACKUP

/// \brief Inserts a record next to page cursor.
/// \details Returns pointer to inserted record if succeed, i.e., enough space available, NULL otherwise. The cursor stays at the same logical position, but the physical position may change if it is pointing to a compressed page that was reorganized.
/// \param [in,out] cursor a page cursor
/// \param [in] tuple pointer to a data tuple
/// \param [in] index record descriptor
/// \param [in] n_ext number of externally stored columns
/// \param [in] mtr mini-transaction handle, or NULL
/// \return pointer to record if succeed, NULL otherwise
IB_INLINE rec_t* page_cur_tuple_insert(page_cur_t* cursor, const dtuple_t* tuple, dict_index_t* index, ulint n_ext, mtr_t* mtr);
#endif /* !IB_HOTBACKUP */

/// \brief Inserts a record next to page cursor.
/// \details Returns pointer to inserted record if succeed, i.e., enough space available, NULL otherwise. The cursor stays at the same logical position, but the physical position may change if it is pointing to a compressed page that was reorganized.
/// \param [in,out] cursor a page cursor
/// \param [in] rec record to insert
/// \param [in] index record descriptor
/// \param [in,out] offsets rec_get_offsets(rec, index)
/// \param [in] mtr mini-transaction handle, or NULL
/// \return pointer to record if succeed, NULL otherwise
IB_INLINE rec_t* page_cur_rec_insert(page_cur_t* cursor, const rec_t* rec, dict_index_t* index, ulint* offsets, mtr_t* mtr);

/// \brief Inserts a record next to page cursor on an uncompressed page.
/// \details Returns pointer to inserted record if succeed, i.e., enough space available, NULL otherwise. The cursor stays at the same position.
/// \return pointer to record if succeed, NULL otherwise
IB_INTERN rec_t* page_cur_insert_rec_low(rec_t* current_rec, dict_index_t* index, const rec_t* rec, ulint* offsets, mtr_t* mtr);

/// \brief Inserts a record next to page cursor on a compressed and uncompressed page.
/// \details Returns pointer to inserted record if succeed, i.e., enough space available, NULL otherwise. The cursor stays at the same position.
/// \return pointer to record if succeed, NULL otherwise
IB_INTERN rec_t* page_cur_insert_rec_zip(rec_t** current_rec, buf_block_t* block, dict_index_t* index, const rec_t* rec, ulint* offsets, mtr_t* mtr);

/// \brief Copies records from page to a newly created page, from a given record onward, including that record.
/// \details Infimum and supremum records are not copied.
IB_INTERN void page_copy_rec_list_end_to_created_page(page_t* new_page, rec_t* rec, dict_index_t* index, mtr_t* mtr);

/// \brief Deletes a record at the page cursor.
/// \details The cursor is moved to the next record after the deleted one.
IB_INTERN void page_cur_delete_rec(page_cur_t* cursor, dict_index_t* index, const ulint* offsets, mtr_t* mtr);
#ifndef IB_HOTBACKUP

/// \brief Searches the right position for a page cursor.
/// \param [in] block buffer block
/// \param [in] index record descriptor
/// \param [in] tuple data tuple
/// \param [in] mode PAGE_CUR_L, PAGE_CUR_LE, PAGE_CUR_G, or PAGE_CUR_GE
/// \param [out] cursor page cursor
/// \return number of matched fields on the left
IB_INLINE ulint page_cur_search(const buf_block_t* block, const dict_index_t* index, const dtuple_t* tuple, ulint mode, page_cur_t* cursor);

/// \brief Searches the right position for a page cursor.
IB_INTERN void page_cur_search_with_match(const buf_block_t* block, const dict_index_t* index, const dtuple_t* tuple, ulint mode, ulint* iup_matched_fields, ulint* iup_matched_bytes, ulint* ilow_matched_fields, ulint* ilow_matched_bytes, page_cur_t* cursor);

/// \brief Positions a page cursor on a randomly chosen user record on a page.
/// \details If there are no user records, sets the cursor on the infimum record.
IB_INTERN void page_cur_open_on_rnd_user_rec(buf_block_t* block, page_cur_t* cursor);
#endif /* !IB_HOTBACKUP */

/// \brief Parses a log record of a record insert on a page.
/// \return end of log record or NULL
IB_INTERN byte* page_cur_parse_insert_rec(ibool is_short, byte* ptr, byte* end_ptr, buf_block_t* block, dict_index_t* index, mtr_t* mtr);

/// \brief Parses a log record of copying a record list end to a new created page.
/// \return end of log record or NULL
IB_INTERN byte* page_parse_copy_rec_list_to_created_page(byte* ptr, byte* end_ptr, buf_block_t* block, dict_index_t* index, mtr_t* mtr);

/// \brief Parses log record of a record delete on a page.
/// \return pointer to record end or NULL
IB_INTERN byte* page_cur_parse_delete_rec(byte* ptr, byte* end_ptr, buf_block_t* block, dict_index_t* index, mtr_t* mtr);

/** Index page cursor */

struct page_cur_struct {
    byte* rec;       /*!< pointer to a record on page */
    buf_block_t* block; /*!< pointer to the block containing rec */
};

#ifndef IB_DO_NOT_INLINE
#include "page_cur.inl"
#endif

#endif
