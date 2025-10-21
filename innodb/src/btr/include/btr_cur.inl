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

/// \file btr_cur.inl
/// \brief The index tree cursor
/// \details Originally created by Heikki Tuuri in 10/16/1994
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#ifndef IB_HOTBACKUP
#include "btr_btr.hpp"

#ifdef IB_DEBUG
/// \brief Returns the page cursor component of a tree cursor
/// \param [in] cursor tree cursor
/// \return pointer to page cursor component
IB_INLINE page_cur_t* btr_cur_get_page_cur(const btr_cur_t* cursor)
{
	return(&((btr_cur_t*) cursor)->page_cur);
}
#endif /* IB_DEBUG */
/// \brief Returns the buffer block on which the tree cursor is positioned
/// \param [in] cursor tree cursor
/// \return pointer to buffer block
IB_INLINE buf_block_t* btr_cur_get_block(btr_cur_t* cursor)
{
	return(page_cur_get_block(btr_cur_get_page_cur(cursor)));
}

/// \brief Returns the record pointer of a tree cursor
/// \param [in] cursor tree cursor
/// \return pointer to record
IB_INLINE rec_t* btr_cur_get_rec(btr_cur_t* cursor)
{
	return(page_cur_get_rec(&(cursor->page_cur)));
}

#ifdef WITH_ZIP
/// \brief Returns the compressed page on which the tree cursor is positioned
/// \param [in] cursor tree cursor
/// \return pointer to compressed page, or NULL if the page is not compressed
IB_INLINE page_zip_des_t* btr_cur_get_page_zip(btr_cur_t* cursor)
{
	return(buf_block_get_page_zip(btr_cur_get_block(cursor)));
}
#endif /* WITH_ZIP */

/// \brief Invalidates a tree cursor by setting record pointer to NULL
/// \param [in] cursor tree cursor
IB_INLINE void btr_cur_invalidate(btr_cur_t* cursor)
{
	page_cur_invalidate(&(cursor->page_cur));
}

/// \brief Returns the page of a tree cursor
/// \param [in] cursor tree cursor
/// \return pointer to page
IB_INLINE page_t* btr_cur_get_page(btr_cur_t* cursor)
{
	return(page_align(page_cur_get_rec(&(cursor->page_cur))));
}

/// \brief Returns the index of a cursor
/// \param [in] cursor B-tree cursor
/// \return index
IB_INLINE dict_index_t* btr_cur_get_index(btr_cur_t* cursor)
{
	return(cursor->index);
}

/// \brief Positions a tree cursor at a given record
/// \param [in] dict_index dict_index
/// \param [in] rec record in tree
/// \param [in] block buffer block of rec
/// \param [out] cursor cursor
IB_INLINE void btr_cur_position(dict_index_t* dict_index, rec_t* rec, buf_block_t* block, btr_cur_t* cursor)
{
	ut_ad(page_align(rec) == block->frame);
	page_cur_position(rec, block, btr_cur_get_page_cur(cursor));
	cursor->index = dict_index;
}

/// \brief Checks if compressing an index page where a btr cursor is placed makes sense
/// \param [in] cursor btr cursor
/// \param [in] mtr mtr
/// \return TRUE if compression is recommended
IB_INLINE ibool btr_cur_compress_recommendation(btr_cur_t* cursor, mtr_t* mtr)
{
	page_t* page;

	ut_ad(mtr_memo_contains(mtr, btr_cur_get_block(cursor), MTR_MEMO_PAGE_X_FIX));

	page = btr_cur_get_page(cursor);

	if ((page_get_data_size(page) < BTR_CUR_PAGE_COMPRESS_LIMIT) || ((btr_page_get_next(page, mtr) == FIL_NULL) && (btr_page_get_prev(page, mtr) == FIL_NULL))) {
		// The page fillfactor has dropped below a predefined minimum value OR the level in the B-tree contains just one page: we recommend compression if this is not the root page.

		return dict_index_get_page(cursor->index) != page_get_page_no(page);
	}

	return FALSE;
}

/// \brief Checks if the record on which the cursor is placed can be deleted without making tree compression necessary (or, recommended)
/// \param [in] cursor btr cursor
/// \param [in] rec_size rec_get_size(btr_cur_get_rec(cursor))
/// \param [in] mtr mtr
/// \return TRUE if can be deleted without recommended compression
IB_INLINE ibool btr_cur_can_delete_without_compress(btr_cur_t* cursor, ulint rec_size, mtr_t* mtr)
{
	page_t* page;

	ut_ad(mtr_memo_contains(mtr, btr_cur_get_block(cursor), MTR_MEMO_PAGE_X_FIX));

	page = btr_cur_get_page(cursor);

	if ((page_get_data_size(page) - rec_size < BTR_CUR_PAGE_COMPRESS_LIMIT) || ((btr_page_get_next(page, mtr) == FIL_NULL) && (btr_page_get_prev(page, mtr) == FIL_NULL)) || (page_get_n_recs(page) < 2)) {
		// The page fillfactor will drop below a predefined minimum value, OR the level in the B-tree contains just one page, OR the page will become empty: we recommend compression if this is not the root page.

		return dict_index_get_page(cursor->index) == page_get_page_no(page);
	}

	return TRUE;
}
#endif /* !IB_HOTBACKUP */
