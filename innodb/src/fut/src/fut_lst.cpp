// Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.
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

/// \file fut_lst.cpp
/// \brief File-based list utilities implementation
/// \details Originally created by Heikki Tuuri in 11/28/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "fut_lst.hpp"

#ifdef IB_DO_NOT_INLINE
	#include "fut_lst.inl"
#endif

#include "buf_buf.hpp"
#include "page_page.hpp"

// -----------------------------------------------------------------------------------------
// type definitions
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

#ifndef IB_HOTBACKUP
/// \brief Adds a node to an empty list.
/// \param [in] base pointer to base node of empty list
/// \param [in] node node to add
/// \param [in] mtr mini-transaction handle
static void flst_add_to_empty(flst_base_node_t* base, flst_node_t* node, mtr_t* mtr);
#endif

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Adds a node as the last node in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node node to add
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_add_last(flst_base_node_t* base, flst_node_t* node, mtr_t* mtr)
{
	ut_ad(mtr && base && node);
	ut_ad(base != node);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node, MTR_MEMO_PAGE_X_FIX));
	const ulint len = flst_get_len(base, mtr);
	fil_addr_t last_addr = flst_get_last(base, mtr);
	ulint space;
	fil_addr_t node_addr;
	buf_ptr_get_fsp_addr(node, &space, &node_addr);
	// If the list is not empty, call flst_insert_after
	if (len != 0) {
		flst_node_t* last_node;
		if (last_addr.page == node_addr.page) {
			last_node = page_align(node) + last_addr.boffset;
		} else {
			const ulint zip_size = fil_space_get_zip_size(space);
			last_node = fut_get_ptr(space, zip_size, last_addr, RW_X_LATCH, mtr);
		}
		flst_insert_after(base, last_node, node, mtr);
	} else {
		// else call flst_add_to_empty
		flst_add_to_empty(base, node, mtr);
	}
}

/// \brief Adds a node as the first node in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node node to add
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_add_first(flst_base_node_t* base, flst_node_t* node, mtr_t* mtr)
{
	ut_ad(mtr && base && node);
	ut_ad(base != node);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node, MTR_MEMO_PAGE_X_FIX));
	const ulint len = flst_get_len(base, mtr);
	fil_addr_t first_addr = flst_get_first(base, mtr);
	ulint space;
	fil_addr_t node_addr;
	buf_ptr_get_fsp_addr(node, &space, &node_addr);
	// If the list is not empty, call flst_insert_before
	if (len != 0) {
		flst_node_t* first_node;
		if (first_addr.page == node_addr.page) {
			first_node = page_align(node) + first_addr.boffset;
		} else {
			const ulint zip_size = fil_space_get_zip_size(space);
			first_node = fut_get_ptr(space, zip_size, first_addr, RW_X_LATCH, mtr);
		}
		flst_insert_before(base, node, first_node, mtr);
	} else {
		// else call flst_add_to_empty
		flst_add_to_empty(base, node, mtr);
	}
}

/// \brief Inserts a node after another in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node1 node to insert after
/// \param [in] node2 node to add
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_insert_after(flst_base_node_t* base, flst_node_t* node1, flst_node_t* node2, mtr_t* mtr)
{
	ut_ad(mtr && node1 && node2 && base);
	ut_ad(base != node1);
	ut_ad(base != node2);
	ut_ad(node2 != node1);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node1, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node2, MTR_MEMO_PAGE_X_FIX));
	ulint space;
	fil_addr_t node1_addr;
	fil_addr_t node2_addr;
	buf_ptr_get_fsp_addr(node1, &space, &node1_addr);
	buf_ptr_get_fsp_addr(node2, &space, &node2_addr);
	fil_addr_t node3_addr = flst_get_next_addr(node1, mtr);
	// Set prev and next fields of node2
	flst_write_addr(node2 + FLST_PREV, node1_addr, mtr);
	flst_write_addr(node2 + FLST_NEXT, node3_addr, mtr);
	if (!fil_addr_is_null(node3_addr)) {
		// Update prev field of node3
		const ulint zip_size = fil_space_get_zip_size(space);
		const flst_node_t* node3 = fut_get_ptr(space, zip_size, node3_addr, RW_X_LATCH, mtr);
		flst_write_addr(node3 + FLST_PREV, node2_addr, mtr);
	} else {
		// node1 was last in list: update last field in base
		flst_write_addr(base + FLST_LAST, node2_addr, mtr);
	}
	// Set next field of node1
	flst_write_addr(node1 + FLST_NEXT, node2_addr, mtr);
	// Update len of base node
	const ulint len = flst_get_len(base, mtr);
	mlog_write_ulint(base + FLST_LEN, len + 1, MLOG_4BYTES, mtr);
}

/// \brief Inserts a node before another in a list.
/// \param [in] base pointer to base node of list
/// \param [in] node2 node to insert
/// \param [in] node3 node to insert before
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_insert_before(flst_base_node_t* base, flst_node_t* node2, flst_node_t* node3, mtr_t* mtr)
{
	ut_ad(mtr && node2 && node3 && base);
	ut_ad(base != node2);
	ut_ad(base != node3);
	ut_ad(node2 != node3);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node2, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node3, MTR_MEMO_PAGE_X_FIX));
	ulint space;
	fil_addr_t node2_addr;
	fil_addr_t node3_addr;
	buf_ptr_get_fsp_addr(node2, &space, &node2_addr);
	buf_ptr_get_fsp_addr(node3, &space, &node3_addr);
	fil_addr_t node1_addr = flst_get_prev_addr(node3, mtr);
	// Set prev and next fields of node2
	flst_write_addr(node2 + FLST_PREV, node1_addr, mtr);
	flst_write_addr(node2 + FLST_NEXT, node3_addr, mtr);
	if (!fil_addr_is_null(node1_addr)) {
		const ulint zip_size = fil_space_get_zip_size(space);
		// Update next field of node1
		const flst_node_t* node1 = fut_get_ptr(space, zip_size, node1_addr, RW_X_LATCH, mtr);
		flst_write_addr(node1 + FLST_NEXT, node2_addr, mtr);
	} else {
		// node3 was first in list: update first field in base
		flst_write_addr(base + FLST_FIRST, node2_addr, mtr);
	}
	// Set prev field of node3
	flst_write_addr(node3 + FLST_PREV, node2_addr, mtr);
	// Update len of base node
	const ulint len = flst_get_len(base, mtr);
	mlog_write_ulint(base + FLST_LEN, len + 1, MLOG_4BYTES, mtr);
}

/// \brief Removes a node.
/// \param [in] base pointer to base node of list
/// \param [in] node2 node to remove
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_remove(flst_base_node_t* base, flst_node_t* node2, mtr_t* mtr)
{
	ut_ad(mtr && node2 && base);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node2, MTR_MEMO_PAGE_X_FIX));
	ulint space;
	fil_addr_t node2_addr;
	buf_ptr_get_fsp_addr(node2, &space, &node2_addr);
	const ulint zip_size = fil_space_get_zip_size(space);
	fil_addr_t node1_addr = flst_get_prev_addr(node2, mtr);
	fil_addr_t node3_addr = flst_get_next_addr(node2, mtr);
	if (!fil_addr_is_null(node1_addr)) {
		// Update next field of node1
		flst_node_t* node1;
		if (node1_addr.page == node2_addr.page) {
			node1 = page_align(node2) + node1_addr.boffset;
		} else {
			node1 = fut_get_ptr(space, zip_size, node1_addr, RW_X_LATCH, mtr);
		}
		ut_ad(node1 != node2);
		flst_write_addr(node1 + FLST_NEXT, node3_addr, mtr);
	} else {
		// node2 was first in list: update first field in base
		flst_write_addr(base + FLST_FIRST, node3_addr, mtr);
	}
	if (!fil_addr_is_null(node3_addr)) {
		// Update prev field of node3
		flst_node_t* node3;
		if (node3_addr.page == node2_addr.page) {
			node3 = page_align(node2) + node3_addr.boffset;
		} else {
			node3 = fut_get_ptr(space, zip_size, node3_addr, RW_X_LATCH, mtr);
		}
		ut_ad(node2 != node3);
		flst_write_addr(node3 + FLST_PREV, node1_addr, mtr);
	} else {
		// node2 was last in list: update last field in base
		flst_write_addr(base + FLST_LAST, node1_addr, mtr);
	}
	// Update len of base node
	const ulint len = flst_get_len(base, mtr);
	ut_ad(len > 0);
	mlog_write_ulint(base + FLST_LEN, len - 1, MLOG_4BYTES, mtr);
}

/// \brief Cuts off the tail of the list, including the node given. The number of nodes which will be removed must be provided by the caller, as this function does not measure the length of the tail.
/// \param [in] base pointer to base node of list
/// \param [in] node2 first node to remove
/// \param [in] n_nodes number of nodes to remove, must be >= 1
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_cut_end(flst_base_node_t* base, flst_node_t* node2, ulint n_nodes, mtr_t* mtr)
{
	ut_ad(mtr && node2 && base);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node2, MTR_MEMO_PAGE_X_FIX));
	ut_ad(n_nodes > 0);
	ulint space;
	fil_addr_t node2_addr;
	buf_ptr_get_fsp_addr(node2, &space, &node2_addr);
	fil_addr_t node1_addr = flst_get_prev_addr(node2, mtr);
	if (!fil_addr_is_null(node1_addr)) {
		// Update next field of node1
		flst_node_t* node1;
		if (node1_addr.page == node2_addr.page) {
			node1 = page_align(node2) + node1_addr.boffset;
		} else {
			node1 = fut_get_ptr(space, fil_space_get_zip_size(space), node1_addr, RW_X_LATCH, mtr);
		}
		flst_write_addr(node1 + FLST_NEXT, fil_addr_null, mtr);
	} else {
		// node2 was first in list: update the field in base
		flst_write_addr(base + FLST_FIRST, fil_addr_null, mtr);
	}
	flst_write_addr(base + FLST_LAST, node1_addr, mtr);
	// Update len of base node
	const ulint len = flst_get_len(base, mtr);
	ut_ad(len >= n_nodes);
	mlog_write_ulint(base + FLST_LEN, len - n_nodes, MLOG_4BYTES, mtr);
}

/// \brief Cuts off the tail of the list, not including the given node. The number of nodes which will be removed must be provided by the caller, as this function does not measure the length of the tail.
/// \param [in] base pointer to base node of list
/// \param [in] node2 first node not to remove
/// \param [in] n_nodes number of nodes to remove
/// \param [in] mtr mini-transaction handle
IB_INTERN void flst_truncate_end(flst_base_node_t* base, flst_node_t* node2, ulint n_nodes, mtr_t* mtr)
{
	ut_ad(mtr && node2 && base);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node2, MTR_MEMO_PAGE_X_FIX));
	if (n_nodes == 0) {
		ut_ad(fil_addr_is_null(flst_get_next_addr(node2, mtr)));
		return;
	}
	ulint space;
	fil_addr_t node2_addr;
	buf_ptr_get_fsp_addr(node2, &space, &node2_addr);
	// Update next field of node2
	flst_write_addr(node2 + FLST_NEXT, fil_addr_null, mtr);
	flst_write_addr(base + FLST_LAST, node2_addr, mtr);
	// Update len of base node
	const ulint len = flst_get_len(base, mtr);
	ut_ad(len >= n_nodes);
	mlog_write_ulint(base + FLST_LEN, len - n_nodes, MLOG_4BYTES, mtr);
}

/// \brief Validates a file-based list.
/// \param [in] base pointer to base node of list
/// \param [in] mtr1 mtr
/// \return TRUE if ok
IB_INTERN ibool flst_validate(const flst_base_node_t* base, mtr_t* mtr1)
{
	ut_ad(base);
	ut_ad(mtr_memo_contains_page(mtr1, base, MTR_MEMO_PAGE_X_FIX));
	// We use two mini-transaction handles: the first is used to lock the base node, and prevent other threads from modifying the list.
	// The second is used to traverse the list. We cannot run the second mtr without committing it at times, because if the list is long, then the x-locked pages could fill the buffer resulting in a deadlock.
	// Find out the space id
	ulint space;
	fil_addr_t base_addr;
	buf_ptr_get_fsp_addr(base, &space, &base_addr);
	const ulint zip_size = fil_space_get_zip_size(space);
	const ulint len = flst_get_len(base, mtr1);
	fil_addr_t node_addr = flst_get_first(base, mtr1);
	for (ulint i = 0; i < len; i++) {
		mtr_t mtr2;
		mtr_start(&mtr2);
		const flst_node_t* node = fut_get_ptr(space, zip_size, node_addr, RW_X_LATCH, &mtr2);
		node_addr = flst_get_next_addr(node, &mtr2);
		mtr_commit(&mtr2); // Commit mtr2 each round to prevent buffer becoming full
	}
	ut_a(fil_addr_is_null(node_addr));
	node_addr = flst_get_last(base, mtr1);
	for (ulint i = 0; i < len; i++) {
		mtr_t mtr2;
		mtr_start(&mtr2);
		const flst_node_t* node = fut_get_ptr(space, zip_size, node_addr, RW_X_LATCH, &mtr2);
		node_addr = flst_get_prev_addr(node, &mtr2);
		mtr_commit(&mtr2); // Commit mtr2 each round to prevent buffer becoming full
	}
	ut_a(fil_addr_is_null(node_addr));
	return(TRUE);
}

/// \brief Prints info of a file-based list.
/// \param [in] base pointer to base node of list
/// \param [in] mtr mtr
IB_INTERN void flst_print(const flst_base_node_t* base, mtr_t* mtr)
{
	ut_ad(base && mtr);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	const buf_frame_t* frame = page_align((byte*) base);
	const ulint len = flst_get_len(base, mtr);
	ib_log(state, "FILE-BASED LIST:\nBase node in space %lu page %lu byte offset %lu; len %lu\n", (ulong) page_get_space_id(frame), (ulong) page_get_page_no(frame), (ulong) page_offset(base), (ulong) len);
}

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

#ifndef IB_HOTBACKUP

static void flst_add_to_empty(flst_base_node_t* base, flst_node_t* node, mtr_t* mtr)
{
	ut_ad(mtr && base && node);
	ut_ad(base != node);
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));
	ut_ad(mtr_memo_contains_page(mtr, node, MTR_MEMO_PAGE_X_FIX));
	const ulint len = flst_get_len(base, mtr);
	ut_a(len == 0);
	ulint space;
	fil_addr_t node_addr;
	buf_ptr_get_fsp_addr(node, &space, &node_addr);
	// Update first and last fields of base node
	flst_write_addr(base + FLST_FIRST, node_addr, mtr);
	flst_write_addr(base + FLST_LAST, node_addr, mtr);
	// Set prev and next fields of node to add
	flst_write_addr(node + FLST_PREV, fil_addr_null, mtr);
	flst_write_addr(node + FLST_NEXT, fil_addr_null, mtr);
	// Update len of base node
	mlog_write_ulint(base + FLST_LEN, len + 1, MLOG_4BYTES, mtr);
}
#endif
