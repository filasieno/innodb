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

/// \file fsp_fsp.hpp
/// \brief File space management
/// \details Originally created by Heikki Tuuri in 12/18/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"

#include "mtr_mtr.hpp"
#include "fut_lst.hpp"
#include "ut_byte.hpp"
#include "page_types.hpp"
#include "fsp_types.hpp"


/// \brief Initializes the file space system.
IB_INTERN void fsp_init(void);

/// \brief Gets the current free limit of the system tablespace.
/// \details The free limit means the place of the first page which has never been put to the free list for allocation. The space above that address is initialized to zero. Sets also the global variable log_fsp_current_free_limit.
/// \return free limit in megabytes
IB_INTERN ulint fsp_header_get_free_limit(void);

/// \brief Gets the size of the system tablespace from the tablespace header.
/// \details If we do not have an auto-extending data file, this should be equal to the size of the data files. If there is an auto-extending data file, this can be smaller.
/// \return size in pages
IB_INTERN ulint fsp_header_get_tablespace_size(void);

/// \brief Reads the file space size stored in the header page.
/// \return tablespace size stored in the space header
/// \param [in] page header page (page 0 in the tablespace)
IB_INTERN ulint fsp_get_size_low(page_t* page);

/// \brief Reads the space id from the first page of a tablespace.
/// \return space id, ULINT UNDEFINED if error
/// \param [in] page first page of a tablespace
IB_INTERN ulint fsp_header_get_space_id(const page_t* page);

/// \brief Reads the space flags from the first page of a tablespace.
/// \return flags
/// \param [in] page first page of a tablespace
IB_INTERN ulint fsp_header_get_flags(const page_t* page);

/// \brief Reads the compressed page size from the first page of a tablespace.
/// \return compressed page size in bytes, or 0 if uncompressed
/// \param [in] page first page of a tablespace
IB_INTERN ulint fsp_header_get_zip_size(const page_t* page);

/// \brief Writes the space id and compressed page size to a tablespace header.
/// \details This function is used past the buffer pool when we in fil0fil.c create a new single-table tablespace.
/// \param [in,out] page first page in the space
/// \param [in] space_id space id
/// \param [in] flags tablespace flags (FSP_SPACE_FLAGS): 0, or table->flags if newer than COMPACT
IB_INTERN void fsp_header_init_fields(page_t* page, ulint space_id, ulint flags);

/// \brief Initializes the space header of a new created space and creates also the insert buffer tree root if space == 0.
/// \param [in] space space id
/// \param [in] size current size in blocks
/// \param [in] mtr mini-transaction handle
IB_INTERN void fsp_header_init(ulint space, ulint size, mtr_t* mtr);

/// \brief Increases the space size field of a space.
/// \param [in] space space id
/// \param [in] size_inc size increment in pages
/// \param [in] mtr mini-transaction handle
IB_INTERN void fsp_header_inc_size(ulint space, ulint size_inc, mtr_t* mtr);

/// \brief Creates a new segment.
/// \return the block where the segment header is placed, x-latched, NULL if could not create segment because of lack of space
/// \param [in] space space id
/// \param [in] page page where the segment header is placed: if this is != 0, the page must belong to another segment, if this is 0, a new page will be allocated and it will belong to the created segment
/// \param [in] byte_offset byte offset of the created segment header on the page
/// \param [in] mtr mtr
IB_INTERN buf_block_t* fseg_create(ulint space, ulint page, ulint byte_offset, mtr_t* mtr);

/// \brief Creates a new segment.
/// \return the block where the segment header is placed, x-latched, NULL if could not create segment because of lack of space
/// \param [in] space space id
/// \param [in] page page where the segment header is placed: if this is != 0, the page must belong to another segment, if this is 0, a new page will be allocated and it will belong to the created segment
/// \param [in] byte_offset byte offset of the created segment header on the page
/// \param [in] has_done_reservation TRUE if the caller has already done the reservation for the pages with fsp_reserve_free_extents (at least 2 extents: one for the inode and the other for the segment) then there is no need to do the check for this individual operation
/// \param [in] mtr mtr
IB_INTERN buf_block_t* fseg_create_general(ulint space, ulint page, ulint byte_offset, ibool has_done_reservation, mtr_t* mtr);

/// \brief Calculates the number of pages reserved by a segment, and how many pages are currently used.
/// \return number of reserved pages
/// \param [in] header segment header
/// \param [out] used number of pages used (<= reserved)
/// \param [in] mtr mtr handle
IB_INTERN ulint fseg_n_reserved_pages(fseg_header_t* header, ulint* used, mtr_t* mtr);

/// \brief Allocates a single free page from a segment. This function implements the intelligent allocation strategy which tries to minimize file space fragmentation.
/// \return the allocated page offset FIL_NULL if no page could be allocated
/// \param [in] seg_header segment header
/// \param [in] hint hint of which page would be desirable
/// \param [in] direction if the new page is needed because of an index page split, and records are inserted there in order, into which direction they go alphabetically: FSP_DOWN, FSP_UP, FSP_NO_DIR
/// \param [in] mtr mtr handle
IB_INTERN ulint fseg_alloc_free_page(fseg_header_t* seg_header, ulint hint, byte direction, mtr_t* mtr);

/// \brief Allocates a single free page from a segment. This function implements the intelligent allocation strategy which tries to minimize file space fragmentation.
/// \return allocated page offset, FIL_NULL if no page could be allocated
/// \param [in] seg_header segment header
/// \param [in] hint hint of which page would be desirable
/// \param [in] direction if the new page is needed because of an index page split, and records are inserted there in order, into which direction they go alphabetically: FSP_DOWN, FSP_UP, FSP_NO_DIR
/// \param [in] has_done_reservation TRUE if the caller has already done the reservation for the page with fsp_reserve_free_extents, then there is no need to do the check for this individual page
/// \param [in] mtr mtr handle
IB_INTERN ulint fseg_alloc_free_page_general(fseg_header_t* seg_header, ulint hint, byte direction, ibool has_done_reservation, mtr_t* mtr);

/// \brief Reserves free pages from a tablespace. All mini-transactions which may use several pages from the tablespace should call this function beforehand and reserve enough free extents so that they certainly will be able to do their operation, like a B-tree page split, fully. Reservations must be released with function fil_space_release_free_extents!
/// \details The alloc_type below has the following meaning: FSP_NORMAL means an operation which will probably result in more space usage, like an insert in a B-tree; FSP_UNDO means allocation to undo logs: if we are deleting rows, then this allocation will in the long run result in less space usage (after a purge); FSP_CLEANING means allocation done in a physical record delete (like in a purge) or other cleaning operation which will result in less space usage in the long run. We prefer the latter two types of allocation: when space is scarce, FSP_NORMAL allocations will not succeed, but the latter two allocations will succeed, if possible. The purpose is to avoid dead end where the database is full but the user cannot free any space because these freeing operations temporarily reserve some space. Single-table tablespaces whose size is < 32 pages are a special case. In this function we would liberally reserve several 64 page extents for every page split or merge in a B-tree. But we do not want to waste disk space if the table only occupies < 32 pages. That is why we apply different rules in that special case, just ensuring that there are 3 free pages available.
/// \return TRUE if we were able to make the reservation
/// \param [out] n_reserved number of extents actually reserved; if we return TRUE and the tablespace size is < 64 pages, then this can be 0, otherwise it is n_ext
/// \param [in] space space id
/// \param [in] n_ext number of extents to reserve
/// \param [in] alloc_type FSP_NORMAL, FSP_UNDO, or FSP_CLEANING
/// \param [in] mtr mtr
IB_INTERN ibool fsp_reserve_free_extents(ulint* n_reserved, ulint space, ulint n_ext, ulint alloc_type, mtr_t* mtr);

/// \brief This function should be used to get information on how much we still will be able to insert new data to the database without running out the tablespace. Only free extents are taken into account and we also subtract the safety margin required by the above function fsp_reserve_free_extents.
/// \return available space in kB
/// \param [in] space space id
IB_INTERN ib_uint64_t fsp_get_available_space_in_free_extents(ulint space);

/// \brief Frees a single page of a segment.
/// \param [in] seg_header segment header
/// \param [in] space space id
/// \param [in] page page offset
/// \param [in] mtr mtr handle
IB_INTERN void fseg_free_page(fseg_header_t* seg_header, ulint space, ulint page, mtr_t* mtr);

/// \brief Frees part of a segment. This function can be used to free a segment by repeatedly calling this function in different mini-transactions. Doing the freeing in a single mini-transaction might result in too big a mini-transaction.
/// \return TRUE if freeing completed
/// \param [in,out] header segment header; NOTE: if the header resides on the first page of the frag list of the segment, this pointer becomes obsolete after the last freeing step
/// \param [in] mtr mtr
IB_INTERN ibool fseg_free_step(fseg_header_t* header, mtr_t* mtr);

/// \brief Frees part of a segment. Differs from fseg_free_step because this function leaves the header page unfreed.
/// \return TRUE if freeing completed, except the header page
/// \param [in] header segment header which must reside on the first fragment page of the segment
/// \param [in] mtr mtr
IB_INTERN ibool fseg_free_step_not_header(fseg_header_t* header, mtr_t* mtr);

/// \brief Checks if a page address is an extent descriptor page address.
/// \return TRUE if a descriptor page
/// \param [in] zip_size compressed page size in bytes; 0 for uncompressed pages
/// \param [in] page_no page number
IB_INLINE ibool fsp_descr_page(ulint zip_size, ulint page_no);

/// \brief Parses a redo log record of a file page init.
/// \return end of log record or NULL
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in] block block or NULL
IB_INTERN byte* fsp_parse_init_file_page(byte* ptr, byte* end_ptr, buf_block_t* block);

/// \brief Validates the file space system and its segments.
/// \return TRUE if ok
/// \param [in] space space id
IB_INTERN ibool fsp_validate(ulint space);

/// \brief Prints info of a file space.
/// \param [in] space space id
IB_INTERN void fsp_print(ulint space);

#ifdef IB_DEBUG

/// \brief Validates a segment.
/// \return TRUE if ok
/// \param [in] header segment header
/// \param [in] mtr mtr
IB_INTERN ibool fseg_validate(fseg_header_t* header, mtr_t* mtr);
#endif /* IB_DEBUG */

#ifdef IB_BTR_PRINT

/// \brief Writes info of a segment.
/// \param [in] header segment header
/// \param [in] mtr mtr
IB_INTERN void fseg_print(fseg_header_t* header, mtr_t* mtr);

#endif /* IB_BTR_PRINT */

#ifndef IB_DO_NOT_INLINE
    #include "fsp_fsp.inl"
#endif
