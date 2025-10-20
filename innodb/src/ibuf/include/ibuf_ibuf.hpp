// Copyright (c) 1997, 2009, Innobase Oy. All Rights Reserved.
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

/// \file ibuf_ibuf.hpp
/// \brief Insert buffer
/// \details Originally created by Heikki Tuuri in 7/19/1997
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"

#include "mtr_mtr.hpp"
#include "dict_mem.hpp"
#include "fsp_fsp.hpp"

#ifndef IB_HOTBACKUP
# include "ibuf0types.h"

/** Combinations of operations that can be buffered.  Because the enum
values are used for indexing innobase_change_buffering_values[], they
should start at 0 and there should not be any gaps. */
typedef enum {
	IBUF_USE_NONE = 0,
	IBUF_USE_INSERT,	/* insert */

	IBUF_USE_COUNT		/* number of entries in ibuf_use_t */
} ibuf_use_t;

/** Operations that can currently be buffered. */
extern ibuf_use_t	ibuf_use;

/** The insert buffer control structure */
extern ibuf_t*		ibuf;

/* The purpose of the insert buffer is to reduce random disk access.
When we wish to insert a record into a non-unique secondary index and
the B-tree leaf page where the record belongs to is not in the buffer
pool, we insert the record into the insert buffer B-tree, indexed by
(space_id, page_no).  When the page is eventually read into the buffer
pool, we look up the insert buffer B-tree for any modifications to the
page, and apply these upon the completion of the read operation.  This
is called the insert buffer merge. */

/* The insert buffer merge must always succeed.  To guarantee this,
the insert buffer subsystem keeps track of the free space in pages for
which it can buffer operations.  Two bits per page in the insert
buffer bitmap indicate the available space in coarse increments.  The
free bits in the insert buffer bitmap must never exceed the free space
on a page.  It is safe to decrement or reset the bits in the bitmap in
a mini-transaction that is committed before the mini-transaction that
affects the free space.  It is unsafe to increment the bits in a
separately committed mini-transaction, because in crash recovery, the
free bits could momentarily be set too high. */

/// \brief Creates the insert buffer data structure at a database startup and initializes the data structures for the insert buffer of each tablespace.
IB_INTERN void ibuf_init_at_db_start(void);
/// \brief Reads the biggest tablespace id from the high end of the insert buffer tree and updates the counter in fil_system.
IB_INTERN void ibuf_update_max_tablespace_id(void);
/// \brief Initializes an ibuf bitmap page.
/// \param [in] block bitmap page
/// \param [in] mtr mtr
IB_INTERN void ibuf_bitmap_page_init(buf_block_t* block, mtr_t* mtr);
/// \brief Resets the free bits of the page in the ibuf bitmap. This is done in a separate mini-transaction, hence this operation does not restrict further work to only ibuf bitmap operations, which would result if the latch to the bitmap page were kept.
/// \details NOTE: The free bits in the insert buffer bitmap must never exceed the free space on a page. It is safe to decrement or reset the bits in the bitmap in a mini-transaction that is committed before the mini-transaction that affects the free space.
/// \param [in] block index page; free bits are set to 0 if the index is a non-clustered non-unique, and page level is 0
IB_INTERN void ibuf_reset_free_bits(buf_block_t* block);
/// \brief Updates the free bits of an uncompressed page in the ibuf bitmap if there is not enough free on the page any more. This is done in a separate mini-transaction, hence this operation does not restrict further work to only ibuf bitmap operations, which would result if the latch to the bitmap page were kept.
/// \details NOTE: The free bits in the insert buffer bitmap must never exceed the free space on a page. It is unsafe to increment the bits in a separately committed mini-transaction, because in crash recovery, the free bits could momentarily be set too high. It is only safe to use this function for decrementing the free bits. Should more free space become available, we must not update the free bits here, because that would break crash recovery.
/// \param [in] block index page to which we have added new records; the free bits are updated if the index is non-clustered and non-unique and the page level is 0, and the page becomes fuller
/// \param [in] max_ins_size value of maximum insert size with reorganize before the latest operation performed to the page
/// \param [in] increase upper limit for the additional space used in the latest operation, if known, or ULINT_UNDEFINED
IB_INLINE void ibuf_update_free_bits_if_full(buf_block_t* block, ulint max_ins_size, ulint increase);
/// \brief Updates the free bits for an uncompressed page to reflect the present state. Does this in the mtr given, which means that the latching order rules virtually prevent any further operations for this OS thread until mtr is committed.
/// \details NOTE: The free bits in the insert buffer bitmap must never exceed the free space on a page. It is safe to set the free bits in the same mini-transaction that updated the page.
/// \param [in] block index page
/// \param [in] max_ins_size value of maximum insert size with reorganize before the latest operation performed to the page
/// \param [in,out] mtr mtr
IB_INTERN void ibuf_update_free_bits_low(const buf_block_t* block, ulint max_ins_size, mtr_t* mtr);
/// \brief Updates the free bits for a compressed page to reflect the present state. Does this in the mtr given, which means that the latching order rules virtually prevent any further operations for this OS thread until mtr is committed.
/// \details NOTE: The free bits in the insert buffer bitmap must never exceed the free space on a page. It is safe to set the free bits in the same mini-transaction that updated the page.
/// \param [in,out] block index page
/// \param [in,out] mtr mtr
IB_INTERN void ibuf_update_free_bits_zip(buf_block_t* block, mtr_t* mtr);
/// \brief Updates the free bits for the two pages to reflect the present state. Does this in the mtr given, which means that the latching order rules virtually prevent any further operations until mtr is committed.
/// \details NOTE: The free bits in the insert buffer bitmap must never exceed the free space on a page. It is safe to set the free bits in the same mini-transaction that updated the pages.
/// \param [in] zip_size compressed page size in bytes; 0 for uncompressed pages
/// \param [in] block1 index page
/// \param [in] block2 index page
/// \param [in] mtr mtr
IB_INTERN void ibuf_update_free_bits_for_two_pages_low(ulint zip_size, buf_block_t* block1, buf_block_t* block2, mtr_t* mtr);
/// \brief A basic partial test if an insert to the insert buffer could be possible and recommended.
/// \param [in] index index where to insert
/// \param [in] ignore_sec_unique if != 0, we should ignore UNIQUE constraint on a secondary index when we decide
IB_INLINE ibool ibuf_should_try(dict_index_t* index, ulint ignore_sec_unique);
/// \brief Returns TRUE if the current OS thread is performing an insert buffer routine.
/// \details For instance, a read-ahead of non-ibuf pages is forbidden by threads that are executing an insert buffer routine.
/// \return TRUE if inside an insert buffer routine
IB_INTERN ibool ibuf_inside(void);
/// \brief Checks if a page address is an ibuf bitmap page (level 3 page) address.
/// \param [in] zip_size compressed page size in bytes; 0 for uncompressed pages
/// \param [in] page_no page number
/// \return TRUE if a bitmap page
IB_INLINE ibool ibuf_bitmap_page(ulint zip_size, ulint page_no);
/// \brief Checks if a page is a level 2 or 3 page in the ibuf hierarchy of pages.
/// \details Must not be called when recv_no_ibuf_operations==TRUE.
/// \param [in] space space id
/// \param [in] zip_size compressed page size in bytes, or 0
/// \param [in] page_no page number
/// \param [in] mtr mtr which will contain an x-latch to the bitmap page if the page is not one of the fixed address ibuf pages, or NULL, in which case a new transaction is created.
/// \return TRUE if level 2 or level 3 page
IB_INTERN ibool ibuf_page(ulint space, ulint zip_size, ulint page_no, mtr_t* mtr);
/// \brief Frees excess pages from the ibuf free list. This function is called when an OS thread calls fsp services to allocate a new file segment, or a new page to a file segment, and the thread did not own the fsp latch before this call.
IB_INTERN void ibuf_free_excess_pages(void);
/// \brief Makes an index insert to the insert buffer, instead of directly to the disk page, if this is possible. Does not do insert if the index is clustered or unique.
/// \param [in] entry index entry to insert
/// \param [in] index index where to insert
/// \param [in] space space id where to insert
/// \param [in] zip_size compressed page size in bytes, or 0
/// \param [in] page_no page number where to insert
/// \param [in] thr query thread
/// \return TRUE if success
IB_INTERN ibool ibuf_insert(const dtuple_t* entry, dict_index_t* index, ulint space, ulint zip_size, ulint page_no, que_thr_t* thr);
/// \brief When an index page is read from a disk to the buffer pool, this function inserts to the page the possible index entries buffered in the insert buffer. The entries are deleted from the insert buffer. If the page is not read, but created in the buffer pool, this function deletes its buffered entries from the insert buffer; there can exist entries for such a page if the page belonged to an index which subsequently was dropped.
/// \param [in] block if page has been read from disk, pointer to the page x-latched, else NULL
/// \param [in] space space id of the index page
/// \param [in] page_no page number of the index page
/// \param [in] zip_size compressed page size in bytes, or 0
/// \param [in] update_ibuf_bitmap normally this is set to TRUE, but if we have deleted or are deleting the tablespace, then we naturally do not want to update a non-existent bitmap page
IB_INTERN void ibuf_merge_or_delete_for_page(buf_block_t* block, ulint space, ulint page_no, ulint zip_size, ibool update_ibuf_bitmap);
/// \brief Deletes all entries in the insert buffer for a given space id. This is used in DISCARD TABLESPACE and IMPORT TABLESPACE.
/// \details NOTE: this does not update the page free bitmaps in the space. The space will become CORRUPT when you call this function!
/// \param [in] space space id
IB_INTERN void ibuf_delete_for_discarded_space(ulint space);
/// \brief Contracts insert buffer trees by reading pages to the buffer pool.
/// \param [in] sync TRUE if the caller wants to wait for the issued read with the highest tablespace address to complete
/// \return a lower limit for the combined size in bytes of entries which will be merged from ibuf trees to the pages read, 0 if ibuf is empty
IB_INTERN ulint ibuf_contract(ibool sync);
/// \brief Contracts insert buffer trees by reading pages to the buffer pool.
/// \param [in] sync TRUE if the caller wants to wait for the issued read with the highest tablespace address to complete
/// \param [in] n_pages try to read at least this many pages to the buffer pool and merge the ibuf contents to them
/// \return a lower limit for the combined size in bytes of entries which will be merged from ibuf trees to the pages read, 0 if ibuf is empty
IB_INTERN ulint ibuf_contract_for_n_pages(ibool sync, ulint n_pages);
#endif /* !IB_HOTBACKUP */
/// \brief Parses a redo log record of an ibuf bitmap page init.
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in] block block or NULL
/// \param [in] mtr mtr or NULL
/// \return end of log record or NULL
IB_INTERN byte* ibuf_parse_bitmap_init(byte* ptr, byte* end_ptr, buf_block_t* block, mtr_t* mtr);
#ifndef IB_HOTBACKUP
#ifdef IB_IBUF_COUNT_DEBUG
/// \brief Gets the ibuf count for a given page.
/// \param [in] space space id
/// \param [in] page_no page number
/// \return number of entries in the insert buffer currently buffered for this page
IB_INTERN ulint ibuf_count_get(ulint space, ulint page_no);
#endif
/// \brief Looks if the insert buffer is empty.
/// \return TRUE if empty
IB_INTERN ibool ibuf_is_empty(void);
/// \brief Prints info of ibuf.
/// \param [in] ib_stram stream where to print
IB_INTERN void ibuf_print(ib_stream_t ib_stram);
/// \brief Reset the variables.
IB_INTERN void ibuf_var_init(void);
/// \brief Closes insert buffer and frees the data structures.
IB_INTERN void ibuf_close(void);

#define IBUF_HEADER_PAGE_NO	FSP_IBUF_HEADER_PAGE_NO
#define IBUF_TREE_ROOT_PAGE_NO	FSP_IBUF_TREE_ROOT_PAGE_NO

#endif /* !IB_HOTBACKUP */

/* The ibuf header page currently contains only the file segment header
for the file segment from which the pages for the ibuf tree are allocated */
#define IBUF_HEADER		PAGE_DATA
#define	IBUF_TREE_SEG_HEADER	0	/* fseg header for ibuf tree */

/* The insert buffer tree itself is always located in space 0. */
constinit ulint IBUF_SPACE_ID = 0;

#ifndef IB_DO_NOT_INLINE
	#include "ibuf_ibuf.inl"
#endif
