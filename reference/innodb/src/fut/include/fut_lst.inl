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

/// \file fut_lst.inl
/// \brief File-based list utilities inline implementations
/// \details Originally created by Heikki Tuuri in 11/28/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "fut_fut.hpp"
#include "mtr_log.hpp"
#include "buf_buf.hpp"

// -----------------------------------------------------------------------------------------
// type definitions
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

// We define the field offsets of a node for the list
constinit ulint FLST_PREV = 0; // 6-byte address of the previous list element; the page part of address is FIL_NULL, if no previous element
constinit ulint FLST_NEXT = FIL_ADDR_SIZE; // 6-byte address of the next list element; the page part of address is FIL_NULL, if no next element

// We define the field offsets of a base node for the list
constinit ulint FLST_LEN = 0; // 32-bit list length field
constinit ulint FLST_FIRST = 4; // 6-byte address of the first element of the list; undefined if empty list
constinit ulint FLST_LAST = 4 + FIL_ADDR_SIZE; // 6-byte address of the last element of the list; undefined if empty list

// -----------------------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Writes a file address.
/// \param [in] faddr pointer to file faddress
/// \param [in] addr file address
/// \param [in] mtr mini-transaction handle
IB_INLINE void flst_write_addr(fil_faddr_t* faddr, fil_addr_t addr, mtr_t* mtr)
{
	ut_ad(faddr && mtr);
	ut_ad(mtr_memo_contains_page(mtr, faddr, MTR_MEMO_PAGE_X_FIX));
	ut_a(addr.page == FIL_NULL || addr.boffset >= FIL_PAGE_DATA);
	ut_a(ut_align_offset(faddr, IB_PAGE_SIZE) >= FIL_PAGE_DATA);

	mlog_write_ulint(faddr + FIL_ADDR_PAGE, addr.page, MLOG_4BYTES, mtr);
	mlog_write_ulint(faddr + FIL_ADDR_BYTE, addr.boffset, MLOG_2BYTES, mtr);
}

/// \brief Reads a file address.
/// \param [in] faddr pointer to file faddress
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_read_addr(const fil_faddr_t* faddr, mtr_t* mtr)
{
	ut_ad(faddr && mtr);

	fil_addr_t addr;
	addr.page = mtr_read_ulint(faddr + FIL_ADDR_PAGE, MLOG_4BYTES, mtr);
	addr.boffset = mtr_read_ulint(faddr + FIL_ADDR_BYTE, MLOG_2BYTES, mtr);
	ut_a(addr.page == FIL_NULL || addr.boffset >= FIL_PAGE_DATA);
	ut_a(ut_align_offset(faddr, IB_PAGE_SIZE) >= FIL_PAGE_DATA);
	return addr;
}

/// \brief Initializes a list base node.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
IB_INLINE void flst_init(flst_base_node_t* base, mtr_t* mtr)
{
	ut_ad(mtr_memo_contains_page(mtr, base, MTR_MEMO_PAGE_X_FIX));

	mlog_write_ulint(base + FLST_LEN, 0, MLOG_4BYTES, mtr);
	flst_write_addr(base + FLST_FIRST, fil_addr_null, mtr);
	flst_write_addr(base + FLST_LAST, fil_addr_null, mtr);
}

/// \brief Gets list length.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
/// \return length
IB_INLINE ulint flst_get_len(const flst_base_node_t* base, mtr_t* mtr)
{
	return mtr_read_ulint(base + FLST_LEN, MLOG_4BYTES, mtr);
}

/// \brief Gets list first node address.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_first(const flst_base_node_t* base, mtr_t* mtr)
{
	return flst_read_addr(base + FLST_FIRST, mtr);
}

/// \brief Gets list last node address.
/// \param [in] base pointer to base node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_last(const flst_base_node_t* base, mtr_t* mtr)
{
	return flst_read_addr(base + FLST_LAST, mtr);
}

/// \brief Gets list next node address.
/// \param [in] node pointer to node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_next_addr(const flst_node_t* node, mtr_t* mtr)
{
	return flst_read_addr(node + FLST_NEXT, mtr);
}

/// \brief Gets list prev node address.
/// \param [in] node pointer to node
/// \param [in] mtr mini-transaction handle
/// \return file address
IB_INLINE fil_addr_t flst_get_prev_addr(const flst_node_t* node, mtr_t* mtr)
{
	return flst_read_addr(node + FLST_PREV, mtr);
}
