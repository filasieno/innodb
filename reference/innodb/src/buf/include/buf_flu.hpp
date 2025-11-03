// Copyright (c) 1995, 2025, Innobase Oy. All Rights Reserved.
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

/// \file buf_flu.hpp
/// \brief The database buffer pool flush algorithm
/// \details Originally created by Heikki Tuuri in 11/5/1995
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "ut_byte.hpp"
#ifndef IB_HOTBACKUP
#include "mtr_types.hpp"
#include "buf_types.hpp"

/// \brief Remove a block from the flush list of modified blocks.
/// \param [in] bpage pointer to the block in question
IB_INTERN void buf_flush_remove(buf_page_t* bpage);
/// \brief Relocates a buffer control block on the flush_list.
/// \details Note that it is assumed that the contents of bpage has already been copied to dpage.
/// \param [in,out] bpage control block being moved
/// \param [in,out] dpage destination block
IB_INTERN void buf_flush_relocate_on_flush_list(buf_page_t* bpage, buf_page_t* dpage);
/// \brief Updates the flush system data structures when a write is completed.
/// \param [in] bpage pointer to the block in question
IB_INTERN void buf_flush_write_complete(buf_page_t* bpage);
/// \brief Flushes pages from the end of the LRU list if there is too small a margin of replaceable pages there.
IB_INTERN void buf_flush_free_margin(void);
#endif /* !IB_HOTBACKUP */
/// \brief Initializes a page for writing to the tablespace.
/// \param [in,out] page page
/// \param [in,out] page_zip_ compressed page, or NULL
/// \param [in] newest_lsn newest modification lsn to the page
IB_INTERN void buf_flush_init_for_writing(byte* page, void* page_zip_, ib_uint64_t newest_lsn);
#ifndef IB_HOTBACKUP
/// \brief This utility flushes dirty blocks from the end of the LRU list or flush_list.
/// \details NOTE 1: in the case of an LRU flush the calling thread may own latches to pages: to avoid deadlocks, this function must be written so that it cannot end up waiting for these latches! NOTE 2: in the case of a flush list flush, the calling thread is not allowed to own any latches on pages!
/// \param [in] flush_type BUF_FLUSH_LRU or BUF_FLUSH_LIST; if BUF_FLUSH_LIST, then the caller must not own any latches on pages
/// \param [in] min_n wished minimum mumber of blocks flushed (it is not guaranteed that the actual number is that big, though)
/// \param [in] lsn_limit in the case BUF_FLUSH_LIST all blocks whose oldest_modification is smaller than this should be flushed (if their number does not exceed min_n), otherwise ignored
/// \return number of blocks for which the write request was queued; ULINT_UNDEFINED if there was a flush of the same type already running
IB_INTERN ulint buf_flush_batch(enum buf_flush flush_type, ulint min_n, ib_uint64_t lsn_limit);
/// \brief Waits until a flush batch of the given type ends.
/// \param [in] type BUF_FLUSH_LRU or BUF_FLUSH_LIST
IB_INTERN void buf_flush_wait_batch_end(enum buf_flush type);
/// \brief This function should be called at a mini-transaction commit, if a page was modified in it.
/// \details Puts the block to the list of modified blocks, if it not already in it.
/// \param [in] block block which is modified
/// \param [in] mtr mtr
IB_INLINE void buf_flush_note_modification(buf_block_t* block, mtr_t* mtr);
/// \brief This function should be called when recovery has modified a buffer page.
/// \param [in] block block which is modified
/// \param [in] start_lsn start lsn of the first mtr in a set of mtr's
/// \param [in] end_lsn end lsn of the last mtr in the set of mtr's
IB_INLINE void buf_flush_recv_note_modification(buf_block_t* block, ib_uint64_t start_lsn, ib_uint64_t end_lsn);
/// \brief Returns TRUE if the file page block is immediately suitable for replacement, i.e., transition FILE_PAGE => NOT_USED allowed.
/// \param [in] bpage buffer control block, must be buf_page_in_file(bpage) and in the LRU list
/// \return TRUE if can replace immediately
IB_INTERN ibool buf_flush_ready_for_replace(buf_page_t* bpage);

// Statistics for selecting flush rate based on redo log generation speed.
// These statistics are generated for heuristics used in estimating the rate at which we should flush the dirty blocks to avoid bursty IO activity. Note that the rate of flushing not only depends on how many dirty pages we have in the buffer pool but it is also a function of how much redo the workload is generating and at what rate.

struct buf_flush_stat_struct
{
	ib_uint64_t	redo;		// amount of redo generated.
	ulint		n_flushed;	// number of pages flushed.
};

// Statistics for selecting flush rate of dirty pages.
typedef struct buf_flush_stat_struct buf_flush_stat_t;
/// \brief Update the historical stats that we are collecting for flush rate heuristics at the end of each interval.
IB_INTERN void buf_flush_stat_update(void);
/// \brief Determines the fraction of dirty pages that need to be flushed based on the speed at which we generate redo log.
/// \details Note that if redo log is generated at significant rate without a corresponding increase in the number of dirty pages (for example, an in-memory workload) it can cause IO bursts of flushing. This function implements heuristics to avoid this burstiness.
/// \return number of dirty pages to be flushed / second
IB_INTERN ulint buf_flush_get_desired_flush_rate(void);

#if defined IB_DEBUG || defined IB_BUF_DEBUG
/// \brief Validates the flush list.
/// \return TRUE if ok
IB_INTERN ibool buf_flush_validate(void);
#endif /* IB_DEBUG || IB_BUF_DEBUG */

/// \brief Initialize the red-black tree to speed up insertions into the flush_list during recovery process.
/// \details Should be called at the start of recovery process before any page has been read/written.
IB_INTERN void buf_flush_init_flush_rbt(void);

/// \brief Frees up the red-black tree.
IB_INTERN void buf_flush_free_flush_rbt(void);

// When buf_flush_free_margin is called, it tries to make this many blocks available to replacement in the free list and at the end of the LRU list (to make sure that a read-ahead batch can be read efficiently in a single sweep).
#define BUF_FLUSH_FREE_BLOCK_MARGIN	(5 + BUF_READ_AHEAD_AREA)
// Extra margin to apply above BUF_FLUSH_FREE_BLOCK_MARGIN
#define BUF_FLUSH_EXTRA_MARGIN		(BUF_FLUSH_FREE_BLOCK_MARGIN / 4 + 100)
#endif /* !IB_HOTBACKUP */

#ifndef IB_DO_NOT_INLINE
#include "buf0flu.inl"
#endif
