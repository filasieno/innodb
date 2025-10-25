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
/// \details Created 10/4/1994 Heikki Tuuri
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#include "page_page.hpp"
#include "buf_types.hpp"

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

#ifdef IB_DEBUG

IB_INLINE page_t* page_cur_get_page(page_cur_t* cur)
{
    ut_ad(cur);
    ut_ad(page_align(cur->rec) == cur->block->frame);

    return page_align(cur->rec);
}

IB_INLINE buf_block_t* page_cur_get_block(page_cur_t* cur)
{
    ut_ad(cur);
    ut_ad(page_align(cur->rec) == cur->block->frame);
    return cur->block;
}

IB_INLINE page_zip_des_t* page_cur_get_page_zip(page_cur_t* cur)
{
    return buf_block_get_page_zip(page_cur_get_block(cur));
}

IB_INLINE rec_t* page_cur_get_rec(page_cur_t* cur)
{
    ut_ad(cur);
    ut_ad(page_align(cur->rec) == cur->block->frame);
    return cur->rec;
}

#endif // IB_DEBUG

/// \brief Sets the cursor object to point before the first user record on the page.
IB_INLINE void page_cur_set_before_first( const buf_block_t* block, page_cur_t* cur)
{
    cur->block = (buf_block_t*) block;
    cur->rec = page_get_infimum_rec(buf_block_get_frame(cur->block));
}

/// \brief Sets the cursor object to point after the last user record on the page.
IB_INLINE void page_cur_set_after_last(const buf_block_t* block, page_cur_t* cur)
{
    cur->block = (buf_block_t*) block;
    cur->rec = page_get_supremum_rec(buf_block_get_frame(cur->block));
}

IB_INLINE ibool page_cur_is_before_first(const page_cur_t* cur)
{
    ut_ad(cur);
    ut_ad(page_align(cur->rec) == cur->block->frame);
    return page_rec_is_infimum(cur->rec);
}

IB_INLINE ibool page_cur_is_after_last(const page_cur_t* cur)
{
    ut_ad(cur);
    ut_ad(page_align(cur->rec) == cur->block->frame);
    return page_rec_is_supremum(cur->rec);
}

/// \brief Positions the cursor on the given record.
IB_INLINE void page_cur_position( const rec_t* rec, const buf_block_t* block, page_cur_t* cur)
{
    ut_ad(rec && block && cur);
    ut_ad(page_align(rec) == block->frame);
    cur->rec = (rec_t*) rec;
    cur->block = (buf_block_t*) block;
}

/// \brief Invalidates a page cursor by setting the record pointer NULL.
IB_INLINE void page_cur_invalidate(page_cur_t* cur)
{
    ut_ad(cur);
    cur->rec = NULL;
    cur->block = NULL;
}

/// \brief Moves the cursor to the next record on page.
IB_INLINE void page_cur_move_to_next(page_cur_t* cur)
{
    ut_ad(!page_cur_is_after_last(cur));
    cur->rec = page_rec_get_next(cur->rec);
}

/// \brief Moves the cursor to the previous record on page.
IB_INLINE void page_cur_move_to_prev(page_cur_t* cur)
{
    ut_ad(!page_cur_is_before_first(cur));
    cur->rec = page_rec_get_prev(cur->rec);
}

#ifndef IB_HOTBACKUP

IB_INLINE ulint page_cur_search( const buf_block_t* block, const dict_index_t* dict_index, const dtuple_t* tuple, ulint mode, page_cur_t* cursor)
{
    ulint low_matched_fields = 0;
    ulint low_matched_bytes = 0;
    ulint up_matched_fields = 0;
    ulint up_matched_bytes = 0;
    ut_ad(dtuple_check_typed(tuple));
    page_cur_search_with_match(block, dict_index, tuple, mode, &up_matched_fields, &up_matched_bytes, &low_matched_fields, &low_matched_bytes, cursor);
    return low_matched_fields;
}

IB_INLINE rec_t* page_cur_tuple_insert(page_cur_t* cursor, const dtuple_t* tuple, dict_index_t* dict_index, ulint n_ext, mtr_t* mtr)
{
    ulint size = rec_get_converted_size(dict_index, tuple, n_ext);
    mem_heap_t* heap = mem_heap_create(size + (4 + REC_OFFS_HEADER_SIZE + dtuple_get_n_fields(tuple)) * sizeof *offsets);
    rec_t* rec = rec_convert_dtuple_to_rec((byte*) mem_heap_alloc(heap, size), dict_index, tuple, n_ext);
    ulint* offsets = rec_get_offsets(rec, dict_index, NULL, ULINT_UNDEFINED, &heap);
#ifdef WITH_ZIP
    if (buf_block_get_page_zip(cursor->block)) {
        rec = page_cur_insert_rec_zip(&cursor->rec, cursor->block, dict_index, rec, offsets, mtr);
    } else
#endif // WITH_ZIP
    {
        rec = page_cur_insert_rec_low(cursor->rec, dict_index, rec, offsets, mtr);
    }
    mem_heap_free(heap);
    return rec;
}

#endif // !IB_HOTBACKUP

IB_INLINE rec_t* page_cur_rec_insert(
	page_cur_t* cursor, const rec_t* rec, dict_index_t* dict_index, ulint* offsets, mtr_t* mtr)
{
#ifdef WITH_ZIP
    if (buf_block_get_page_zip(cursor->block)) {
        return page_cur_insert_rec_zip(&cursor->rec, cursor->block, dict_index, rec, offsets, mtr);
    } else
#endif // WITH_ZIP
    {
        return page_cur_insert_rec_low(cursor->rec, dict_index, rec, offsets, mtr);
    }
}
