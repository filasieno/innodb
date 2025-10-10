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

/// @file include/btr_btr.inl
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
#define BTR_MAX_NODE_LEVEL	50

/// \brief Gets a buffer page and declares its latching order level.
/// \param space space id
/// \param zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param page_no page number
/// \param mode latch mode
/// \param mtr mtr
IB_INLINE buf_block_t* btr_block_get(ulint space, ulint zip_size, ulint page_no, ulint mode, mtr_t* mtr)
{
	buf_block_t* block = buf_page_get(space, zip_size, page_no, mode, mtr);
	if (mode != RW_NO_LATCH) {
		buf_block_dbg_add_level(block, SYNC_TREE_NODE);
	}
	return block;
}

/// \brief Gets a buffer page and declares its latching order level.
/// \param space space id
/// \param zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param page_no page number
/// \param mode latch mode
/// \param mtr mtr
IB_INLINE
page_t*
btr_page_get(ulint space, ulint zip_size, ulint page_no, ulint mode, mtr_t* mtr)
{
	return(buf_block_get_frame(btr_block_get(space, zip_size, page_no,mode, mtr)));
}

/// \brief Sets the index id field of a page.
/// \param page page to be created
/// \param page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param id index id
/// \param mtr mtr
IB_INLINE
void
btr_page_set_index_id(page_t* page, page_zip_des_t* page_zip, dulint id, mtr_t* mtr)
{
#ifdef WITH_ZIP
	if (IB_LIKELY_NULL(page_zip)) {
		mach_write_to_8(page + (PAGE_HEADER + PAGE_INDEX_ID), id);
		page_zip_write_header(page_zip,
				      page + (PAGE_HEADER + PAGE_INDEX_ID),
				      8, mtr);
	} else {
#endif /* WITH_ZIP */
		mlog_write_dulint(page + (PAGE_HEADER + PAGE_INDEX_ID),
				  id, mtr);
#ifdef WITH_ZIP
	}
#endif /* WITH_ZIP */
}
#endif /* !IB_HOTBACKUP */

/// \brief Gets the index id field of a page.
/// \return index id
/// \param page index page
IB_INLINE
dulint
btr_page_get_index_id(const page_t* page)
{
	return(mach_read_from_8(page + PAGE_HEADER + PAGE_INDEX_ID));
}

#ifndef IB_HOTBACKUP
/// \brief Gets the node level field in an index page.
/// \return level, leaf level == 0
/// \param page index page
IB_INLINE
ulint
btr_page_get_level_low(const page_t* page)
{
	ulint	level;

	ut_ad(page);

	level = mach_read_from_2(page + PAGE_HEADER + PAGE_LEVEL);

	ut_ad(level <= BTR_MAX_NODE_LEVEL);

	return(level);
}

/// \brief Gets the node level field in an index page.
/// \return level, leaf level == 0
/// \param page index page
/// \param mtr mini-transaction handle
IB_INLINE
ulint
btr_page_get_level(const page_t* page, mtr_t* mtr __attribute__((unused)))
{
	ut_ad(page && mtr);

	return(btr_page_get_level_low(page));
}

/// \brief Sets the node level field in an index page.
/// \param page index page
/// \param page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param level level, leaf level == 0
/// \param mtr mini-transaction handle
IB_INLINE
void
btr_page_set_level(page_t* page, page_zip_des_t* page_zip, ulint level, mtr_t* mtr)
{
	ut_ad(page && mtr);
	ut_ad(level <= BTR_MAX_NODE_LEVEL);

#ifdef WITH_ZIP
	if (IB_LIKELY_NULL(page_zip)) {
		mach_write_to_2(page + (PAGE_HEADER + PAGE_LEVEL), level);
		page_zip_write_header(page_zip, page + (PAGE_HEADER + PAGE_LEVEL), 2, mtr);
	} else {
#endif /* WITH_ZIP */
		mlog_write_ulint(page + (PAGE_HEADER + PAGE_LEVEL), level, MLOG_2BYTES, mtr);
#ifdef WITH_ZIP
	}
#endif /* WITH_ZIP */
}

/// \brief Gets the next index page number.
/// \return next page number
/// \param page index page
/// \param mtr mini-transaction handle
IB_INLINE
ulint
btr_page_get_next(const page_t* page, mtr_t* mtr __attribute__((unused)))
{
	ut_ad(page && mtr);
	ut_ad(mtr_memo_contains_page(mtr, page, MTR_MEMO_PAGE_X_FIX) || mtr_memo_contains_page(mtr, page, MTR_MEMO_PAGE_S_FIX));

	return(mach_read_from_4(page + FIL_PAGE_NEXT));
}

/// \brief Sets the next index page field.
/// \param page index page
/// \param page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param next next page number
/// \param mtr mini-transaction handle
IB_INLINE
void
btr_page_set_next(page_t* page, page_zip_des_t* page_zip, ulint next, mtr_t* mtr)
{
	ut_ad(page && mtr);

#ifdef WITH_ZIP
	if (IB_LIKELY_NULL(page_zip)) {
		mach_write_to_4(page + FIL_PAGE_NEXT, next);
		page_zip_write_header(page_zip, page + FIL_PAGE_NEXT, 4, mtr);
	} else {
#endif /* WITH_ZIP */
		mlog_write_ulint(page + FIL_PAGE_NEXT, next, MLOG_4BYTES, mtr);
#ifdef WITH_ZIP
	}
#endif /* WITH_ZIP */
}

/// \brief Gets the previous index page number.
/// \param page index page
/// \param mtr mini-transaction handle
/// \return prev page number
IB_INLINE ulint btr_page_get_prev(const page_t* page, mtr_t* mtr __attribute__((unused)))
{
	ut_ad(page && mtr);

	return(mach_read_from_4(page + FIL_PAGE_PREV));
}

/// \brief Sets the previous index page field.
/// \param page index page
/// \param page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param prev previous page number
/// \param mtr mini-transaction handle
IB_INLINE void btr_page_set_prev(page_t* page, page_zip_des_t* page_zip, ulint prev, mtr_t* mtr)
{
	ut_ad(page && mtr);

#ifdef WITH_ZIP
	if (IB_LIKELY_NULL(page_zip)) {
		mach_write_to_4(page + FIL_PAGE_PREV, prev);
		page_zip_write_header(page_zip, page + FIL_PAGE_PREV, 4, mtr);
	} else {
#endif /* WITH_ZIP */
		mlog_write_ulint(page + FIL_PAGE_PREV, prev, MLOG_4BYTES, mtr);
#ifdef WITH_ZIP
	}
#endif /* WITH_ZIP */
}

/// \brief Gets the child node file address in a node pointer.
/// \details The offsets array must contain all offsets for the record since
/// we read the last field according to offsets and assume that it contains
/// the child page number. In other words offsets must have been retrieved
/// with rec_get_offsets(n_fields=ULINT_UNDEFINED).
/// \param rec node pointer record
/// \param offsets array returned by rec_get_offsets()
/// \return child node address

IB_INLINE ulint btr_node_ptr_get_child_page_no(const rec_t* rec, const ulint* offsets)
{
	const byte*	field;
	ulint len;
	ulint page_no;

	ut_ad(!rec_offs_comp(offsets) || rec_get_node_ptr_flag(rec));
	field = rec_get_nth_field(rec, offsets, rec_offs_n_fields(offsets) - 1, &len);
	ut_ad(len == 4);
	page_no = mach_read_from_4(field);
	if (IB_UNLIKELY(page_no == 0)) {
		ib_logger(ib_stream, "InnoDB: a nonsensical page number 0 in a node ptr record at offset %lu\n", (ulong) page_offset(rec));
		buf_page_print(page_align(rec), 0);
	}
	return page_no;
}

/// \brief Releases the latches on a leaf page and bufferunfixes it.
/// \param block buffer block
/// \param latch_mode BTR_SEARCH_LEAF or BTR_MODIFY_LEAF
/// \param mtr mtr
IB_INLINE void btr_leaf_page_release(buf_block_t* block, ulint latch_mode, mtr_t* mtr)
{
	ut_ad(latch_mode == BTR_SEARCH_LEAF || latch_mode == BTR_MODIFY_LEAF);
	ut_ad(!mtr_memo_contains(mtr, block, MTR_MEMO_MODIFY));
	mtr_memo_release(mtr, block, latch_mode == BTR_SEARCH_LEAF ? MTR_MEMO_PAGE_S_FIX : MTR_MEMO_PAGE_X_FIX);
}
#endif // ! IB_HOTBACKUP
