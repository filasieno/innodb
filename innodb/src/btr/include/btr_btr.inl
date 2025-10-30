// Copyright (c) 1994, 2010, Innobase Oy. All Rights Reserved.
//
// // This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA

/// \file btr_btr.inl
/// \brief The B-tree
/// \details Created 6/2/1994 Heikki Tuuri.

#include "mach_data.hpp"

#ifndef IB_HOTBACKUP
#include "mtr_mtr.hpp"
#include "mtr_log.hpp"

#ifdef WITH_ZIP
  #include "page_zip.hpp"
#endif // WITH_ZIP

// Maximum B-tree page level (not really a hard limit).
// Used in debug assertions in btr_page_set_level and btr_page_get_level_low
constinit ulint BTR_MAX_NODE_LEVEL = 50;

IB_INLINE buf_block_t* btr_block_get(ulint space, ulint zip_size, ulint page_no, ulint mode, mtr_t* mtr)
{
	buf_block_t* block = buf_page_get(space, zip_size, page_no, mode, mtr);
	if (mode != RW_NO_LATCH) {
		buf_block_dbg_add_level(block, SYNC_TREE_NODE);
	}
	return block;
}

IB_INLINE page_t* btr_page_get(ulint space, ulint zip_size, ulint page_no, ulint mode, mtr_t* mtr)
{
	return buf_block_get_frame(btr_block_get(space, zip_size, page_no,mode, mtr));
}

IB_INLINE void btr_page_set_index_id(page_t* page, page_zip_des_t* page_zip, dulint id, mtr_t* mtr)
{
	if constexpr (WITH_ZIP) {
		if (IB_LIKELY_NULL(page_zip)) {
			mach_write_to_8(page + (PAGE_HEADER + PAGE_INDEX_ID), id);
			page_zip_write_header(page_zip,
					      page + (PAGE_HEADER + PAGE_INDEX_ID),
					      8, mtr);
			return;
		}
	}
	mlog_write_dulint(page + (PAGE_HEADER + PAGE_INDEX_ID), id, mtr);
}
#endif /* !IB_HOTBACKUP */

IB_INLINE dulint btr_page_get_index_id(const page_t* page)
{
	return(mach_read_from_8(page + PAGE_HEADER + PAGE_INDEX_ID));
}

#ifndef IB_HOTBACKUP

IB_INLINE ulint btr_page_get_level_low(const page_t* page)
{
	ut_ad(page);
	ulint level = mach_read_from_2(page + PAGE_HEADER + PAGE_LEVEL);
	ut_ad(level <= BTR_MAX_NODE_LEVEL);
	return level;
}

IB_INLINE ulint btr_page_get_level(const page_t* page, mtr_t* mtr __attribute__((unused)))
{
	ut_ad(page != nullptr);
	ut_ad(mtr != nullptr);
	return btr_page_get_level_low(page);
}

IB_INLINE void btr_page_set_level(page_t* page, page_zip_des_t* page_zip, ulint level, mtr_t* mtr)
{
	ut_ad(page != nullptr);
	ut_ad(mtr != nullptr);
	ut_ad(level <= BTR_MAX_NODE_LEVEL);

	if constexpr (WITH_ZIP) {
		if (IB_LIKELY_NULL(page_zip)) {
			mach_write_to_2(page + (PAGE_HEADER + PAGE_LEVEL), level);
			page_zip_write_header(page_zip, page + (PAGE_HEADER + PAGE_LEVEL), 2, mtr);
			return;
		}
	}
	mlog_write_ulint(page + (PAGE_HEADER + PAGE_LEVEL), level, MLOG_2BYTES, mtr);
}

IB_INLINE ulint btr_page_get_next(const page_t* page, mtr_t* mtr __attribute__((unused)))
{
	ut_ad(page && mtr);
	ut_ad(mtr_memo_contains_page(mtr, page, MTR_MEMO_PAGE_X_FIX) || mtr_memo_contains_page(mtr, page, MTR_MEMO_PAGE_S_FIX));

	return(mach_read_from_4(page + FIL_PAGE_NEXT));
}

IB_INLINE void btr_page_set_next(page_t* page, page_zip_des_t* page_zip, ulint next, mtr_t* mtr)
{
	ut_ad(page && mtr);

	if constexpr (WITH_ZIP) {
		if (IB_LIKELY_NULL(page_zip)) {
			mach_write_to_4(page + FIL_PAGE_NEXT, next);
			page_zip_write_header(page_zip, page + FIL_PAGE_NEXT, 4, mtr);
			return;
		}
	}
	mlog_write_ulint(page + FIL_PAGE_NEXT, next, MLOG_4BYTES, mtr);
}

IB_INLINE ulint btr_page_get_prev(const page_t* page, mtr_t* mtr __attribute__((unused)))
{
	ut_ad(page && mtr);

	return(mach_read_from_4(page + FIL_PAGE_PREV));
}

IB_INLINE void btr_page_set_prev(page_t* page, page_zip_des_t* page_zip, ulint prev, mtr_t* mtr)
{
	ut_ad(page != nullptr);
	ut_ad(mtr != nullptr);

	if constexpr (WITH_ZIP) {
		if (IB_LIKELY_NULL(page_zip)) {
			mach_write_to_4(page + FIL_PAGE_PREV, prev);
			page_zip_write_header(page_zip, page + FIL_PAGE_PREV, 4, mtr);
			return;
		}
	}
	mlog_write_ulint(page + FIL_PAGE_PREV, prev, MLOG_4BYTES, mtr);
}

IB_INLINE ulint btr_node_ptr_get_child_page_no(const rec_t* rec, const ulint* offsets)
{
	ut_ad(!rec_offs_comp(offsets) || rec_get_node_ptr_flag(rec));
	
	ulint len;
	const byte* field = rec_get_nth_field(rec, offsets, rec_offs_n_fields(offsets) - 1, &len);
	ut_ad(len == 4);
	ulint page_no = mach_read_from_4(field);
	if (IB_UNLIKELY(page_no == 0)) {
		ib_log(state, "InnoDB: a nonsensical page number 0 in a node ptr record at offset %lu\n", (ulong) page_offset(rec));
		buf_page_print(page_align(rec), 0);
	}
	return page_no;
}

IB_INLINE void btr_leaf_page_release(buf_block_t* block, ulint latch_mode, mtr_t* mtr)
{
	ut_ad(latch_mode == BTR_SEARCH_LEAF || latch_mode == BTR_MODIFY_LEAF);
	ut_ad(!mtr_memo_contains(mtr, block, MTR_MEMO_MODIFY));
	mtr_memo_release(mtr, block, latch_mode == BTR_SEARCH_LEAF ? MTR_MEMO_PAGE_S_FIX : MTR_MEMO_PAGE_X_FIX);
}
#endif // ! IB_HOTBACKUP
