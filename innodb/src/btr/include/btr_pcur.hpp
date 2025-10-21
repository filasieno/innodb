// Copyright (c) 1996, 2010, Innobase Oy. All Rights Reserved.
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

/// \file btr_pcur.hpp
/// \brief The index tree persistent cursor
/// \details Originally created by Heikki Tuuri in 2/23/1996
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#pragma once

#include "univ.i"
#include "dict_dict.hpp"
#include "data_data.hpp"
#include "mtr_mtr.hpp"
#include "page_cur.hpp"
#include "btr_cur.hpp"
#include "btr_btr.hpp"
#include "btr_types.hpp"

// Relative positions for a stored cursor position
constinit ulint BTR_PCUR_ON = 1;
constinit ulint BTR_PCUR_BEFORE = 2;
constinit ulint BTR_PCUR_AFTER = 3;
// Note that if the tree is not empty, btr_pcur_store_position does not use the following, but only uses the above three alternatives, where the position is stored relative to a specific record: this makes implementation of a scroll cursor easier
constinit ulint BTR_PCUR_BEFORE_FIRST_IN_TREE = 4;	// in an empty tree
constinit ulint BTR_PCUR_AFTER_LAST_IN_TREE = 5;	// in an empty tree

/// \brief Allocates memory for a persistent cursor object and initializes the cursor.
/// \return own: persistent cursor
IB_INTERN btr_pcur_t* btr_pcur_create(void);
/// \brief Frees the memory for a persistent cursor object.
/// \param [in] cursor persistent cursor
IB_INTERN void btr_pcur_free(btr_pcur_t* cursor);
/// \brief Copies the stored position of a pcur to another pcur.
/// \param [in] pcur_receive pcur which will receive the position info
/// \param [in] pcur_donate pcur from which the info is copied
IB_INTERN void btr_pcur_copy_stored_position(btr_pcur_t* pcur_receive, btr_pcur_t* pcur_donate);
/// \brief Sets the old_rec_buf field to NULL.
/// \param [in] pcur persistent cursor
IB_INLINE void btr_pcur_init(btr_pcur_t* pcur);
/// \brief Initializes and opens a persistent cursor to an index tree.
/// \details It should be closed with btr_pcur_close.
/// \param [in] dict_index dict_index
/// \param [in] tuple tuple on which search done
/// \param [in] mode PAGE_CUR_L, ...; NOTE that if the search is made using a unique prefix of a record, mode should be PAGE_CUR_LE, not PAGE_CUR_GE, as the latter may end up on the previous page from the record!
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...
/// \param [in] cursor memory buffer for persistent cursor
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INLINE void btr_pcur_open_func(dict_index_t* dict_index, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_pcur_t* cursor, const char* file, ulint line, mtr_t* mtr);
#define btr_pcur_open(i,t,md,l,c,m)				\
	btr_pcur_open_func(i,t,md,l,c,__FILE__,__LINE__,m)
/// \brief Opens an persistent cursor to an index tree without initializing the cursor.
/// \param [in] dict_index dict_index
/// \param [in] tuple tuple on which search done
/// \param [in] mode PAGE_CUR_L, ...; NOTE that if the search is made using a unique prefix of a record, mode should be PAGE_CUR_LE, not PAGE_CUR_GE, as the latter may end up on the previous page of the record!
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...; NOTE that if has_search_latch != 0 then we maybe do not acquire a latch on the cursor page, but assume that the caller uses his btr search latch to protect the record!
/// \param [in] cursor memory buffer for persistent cursor
/// \param [in] has_search_latch latch mode the caller currently has on btr_search_latch: RW_S_LATCH, or 0
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INLINE void btr_pcur_open_with_no_init_func(dict_index_t* dict_index, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_pcur_t* cursor, ulint has_search_latch, const char* file, ulint line, mtr_t* mtr);
#define btr_pcur_open_with_no_init(ix,t,md,l,cur,has,m)			\
	btr_pcur_open_with_no_init_func(ix,t,md,l,cur,has,__FILE__,__LINE__,m)

/// \brief Opens a persistent cursor at either end of an index.
/// \param [in] from_left TRUE if open to the low end, FALSE if to the high end
/// \param [in] index index
/// \param [in] latch_mode latch mode
/// \param [in] pcur cursor
/// \param [in] do_init TRUE if should be initialized
/// \param [in] mtr mtr
IB_INLINE void btr_pcur_open_at_index_side(ibool from_left, dict_index_t* index, ulint latch_mode, btr_pcur_t* pcur, ibool do_init, mtr_t* mtr);
/// \brief Gets the up_match value for a pcur after a search.
/// \param [in] cursor memory buffer for persistent cursor
/// \return number of matched fields at the cursor or to the right if search mode was PAGE_CUR_GE, otherwise undefined
IB_INLINE ulint btr_pcur_get_up_match(btr_pcur_t* cursor);
/// \brief Gets the low_match value for a pcur after a search.
/// \param [in] cursor memory buffer for persistent cursor
/// \return number of matched fields at the cursor or to the right if search mode was PAGE_CUR_LE, otherwise undefined
IB_INLINE ulint btr_pcur_get_low_match(btr_pcur_t* cursor);
/// \brief If mode is PAGE_CUR_G or PAGE_CUR_GE, opens a persistent cursor on the first user record satisfying the search condition.
/// \details In the case PAGE_CUR_L or PAGE_CUR_LE, on the last user record. If no such user record exists, then in the first case sets the cursor after last in tree, and in the latter case before first in tree. The latching mode must be BTR_SEARCH_LEAF or BTR_MODIFY_LEAF.
/// \param [in] index index
/// \param [in] tuple tuple on which search done
/// \param [in] mode PAGE_CUR_L, ...
/// \param [in] latch_mode BTR_SEARCH_LEAF or BTR_MODIFY_LEAF
/// \param [in] cursor memory buffer for persistent cursor
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_pcur_open_on_user_rec_func(dict_index_t* index, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_pcur_t* cursor, const char* file, ulint line, mtr_t* mtr);
#define btr_pcur_open_on_user_rec(i,t,md,l,c,m)				\
	btr_pcur_open_on_user_rec_func(i,t,md,l,c,__FILE__,__LINE__,m)
/// \brief Positions a cursor at a randomly chosen position within a B-tree.
/// \param [in] dict_index dict_index
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...
/// \param [in,out] cursor B-tree pcur
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INLINE void btr_pcur_open_at_rnd_pos_func(dict_index_t* dict_index, ulint latch_mode, btr_pcur_t* cursor, const char* file, ulint line, mtr_t* mtr);
#define btr_pcur_open_at_rnd_pos(i,l,c,m)				\
	btr_pcur_open_at_rnd_pos_func(i,l,c,__FILE__,__LINE__,m)
/// \brief Frees the possible memory heap of a persistent cursor and sets the latch mode of the persistent cursor to BTR_NO_LATCHES.
/// \param [in] cursor persistent cursor
IB_INLINE void btr_pcur_close(btr_pcur_t* cursor);
/// \brief The position of the cursor is stored by taking an initial segment of the record the cursor is positioned on.
/// \details Before, or after, and copying it to the cursor data structure, or just setting a flag if the cursor id before the first in an EMPTY tree, or after the last in an EMPTY tree. NOTE that the page where the cursor is positioned must not be empty if the index tree is not totally empty!
/// \param [in] cursor persistent cursor
/// \param [in] mtr mtr
IB_INTERN void btr_pcur_store_position(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Restores the stored position of a persistent cursor bufferfixing the page and obtaining the specified latches.
/// \details If the cursor position was saved when the (1) cursor was positioned on a user record: this function restores the position to the last record LESS OR EQUAL to the stored record; (2) cursor was positioned on a page infimum record: restores the position to the last record LESS than the user record which was the successor of the page infimum; (3) cursor was positioned on the page supremum: restores to the first record GREATER than the user record which was the predecessor of the supremum. (4) cursor was positioned before the first or after the last in an empty tree: restores to before first or after the last in the tree.
/// \return TRUE if the cursor position was stored when it was on a user record and it can be restored on a user record whose ordering fields are identical to the ones of the original user record
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...
/// \param [in] cursor detached persistent cursor
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN ibool btr_pcur_restore_position_func(ulint latch_mode, btr_pcur_t* cursor, const char* file, ulint line, mtr_t* mtr);
#define btr_pcur_restore_position(l,cur,mtr)				\
	btr_pcur_restore_position_func(l,cur,__FILE__,__LINE__,mtr)
/// \brief If the latch mode of the cursor is BTR_LEAF_SEARCH or BTR_LEAF_MODIFY, releases the page latch and bufferfix reserved by the cursor.
/// \details NOTE! In the case of BTR_LEAF_MODIFY, there should not exist changes made by the current mini-transaction to the data protected by the cursor latch, as then the latch must not be released until mtr_commit.
/// \param [in] cursor persistent cursor
/// \param [in] mtr mtr
IB_INTERN void btr_pcur_release_leaf(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Gets the rel_pos field for a cursor whose position has been stored.
/// \param [in] cursor persistent cursor
/// \return BTR_PCUR_ON, ...
IB_INLINE ulint btr_pcur_get_rel_pos(const btr_pcur_t* cursor);
/// \brief Sets the mtr field for a pcur.
/// \param [in] cursor persistent cursor
/// \param [in] mtr mtr
IB_INLINE void btr_pcur_set_mtr(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Gets the mtr field for a pcur.
/// \param [in] cursor persistent cursor
/// \return mtr
IB_INLINE mtr_t* btr_pcur_get_mtr(btr_pcur_t* cursor);
/// \brief Commits the mtr and sets the pcur latch mode to BTR_NO_LATCHES, that is, the cursor becomes detached.
/// \details If there have been modifications to the page where pcur is positioned, this can be used instead of btr_pcur_release_leaf. Function btr_pcur_store_position should be used before calling this, if restoration of cursor is wanted later.
/// \param [in] pcur persistent cursor
/// \param [in] mtr mtr to commit
IB_INLINE void btr_pcur_commit_specify_mtr(btr_pcur_t* pcur, mtr_t* mtr);
/// \brief Tests if a cursor is detached: that is the latch mode is BTR_NO_LATCHES.
/// \param [in] pcur persistent cursor
/// \return TRUE if detached
IB_INLINE ibool btr_pcur_is_detached(btr_pcur_t* pcur);
/// \brief Moves the persistent cursor to the next record in the tree.
/// \details If no records are left, the cursor stays 'after last in tree'.
/// \param [in] cursor persistent cursor; NOTE that the function may release the page latch
/// \param [in] mtr mtr
/// \return TRUE if the cursor was not after last in tree
IB_INLINE ibool btr_pcur_move_to_next(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Moves the persistent cursor to the previous record in the tree.
/// \details If no records are left, the cursor stays 'before first in tree'.
/// \param [in] cursor persistent cursor; NOTE that the function may release the page latch
/// \param [in] mtr mtr
/// \return TRUE if the cursor was not before first in tree
IB_INLINE ibool btr_pcur_move_to_prev(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Moves the persistent cursor to the last record on the same page.
/// \param [in] cursor persistent cursor
/// \param [in] mtr mtr
IB_INLINE void btr_pcur_move_to_last_on_page(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Moves the persistent cursor to the next user record in the tree.
/// \details If no user records are left, the cursor ends up 'after last in tree'.
/// \return TRUE if the cursor moved forward, ending on a user record
/// \param [in] cursor persistent cursor; NOTE that the function may release the page latch
/// \param [in] mtr mtr
IB_INLINE ibool btr_pcur_move_to_next_user_rec(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Moves the persistent cursor to the prev user record in the tree.
/// \details If no user records are left, the cursor ends up 'before first in tree'.
/// \param [in] cursor persistent cursor; NOTE that the function may release the page latch
/// \param [in] mtr mtr
/// \return TRUE if the cursor moved backward, ending on a user record
IB_INLINE ibool btr_pcur_move_to_prev_user_rec(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Moves the persistent cursor to the first record on the next page.
/// \details Releases the latch on the current page, and bufferunfixes it. Note that there must not be modifications on the current page, as then the x-latch can be released only in mtr_commit.
/// \param [in] cursor persistent cursor; must be on the last record of the current page
/// \param [in] mtr mtr
IB_INTERN void btr_pcur_move_to_next_page(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Moves the persistent cursor backward if it is on the first record of the page.
/// \details Releases the latch on the current page, and bufferunfixes it. Note that to prevent a possible deadlock, the operation first stores the position of the cursor, releases the leaf latch, acquires necessary latches and restores the cursor position again before returning. The alphabetical position of the cursor is guaranteed to be sensible on return, but it may happen that the cursor is not positioned on the last record of any page, because the structure of the tree may have changed while the cursor had no latches.
/// \param [in] cursor persistent cursor, must be on the first record of the current page
/// \param [in] mtr mtr
IB_INTERN void btr_pcur_move_backward_from_page(btr_pcur_t* cursor, mtr_t* mtr);
#ifdef IB_DEBUG
/// \brief Returns the btr cursor component of a persistent cursor.
/// \param [in] cursor persistent cursor
/// \return pointer to btr cursor component
IB_INLINE btr_cur_t* btr_pcur_get_btr_cur(const btr_pcur_t* cursor);
/// \brief Returns the page cursor component of a persistent cursor.
/// \param [in] cursor persistent cursor
/// \return pointer to page cursor component
IB_INLINE page_cur_t* btr_pcur_get_page_cur(const btr_pcur_t* cursor);
#else /* IB_DEBUG */
# define btr_pcur_get_btr_cur(cursor) (&(cursor)->btr_cur)
# define btr_pcur_get_page_cur(cursor) (&(cursor)->btr_cur.page_cur)
#endif /* IB_DEBUG */
/// \brief Returns the page of a persistent cursor.
/// \param [in] cursor persistent cursor
/// \return pointer to the page
IB_INLINE page_t* btr_pcur_get_page(btr_pcur_t* cursor);
/// \brief Returns the buffer block of a persistent cursor.
/// \param [in] cursor persistent cursor
/// \return pointer to the block
IB_INLINE buf_block_t* btr_pcur_get_block(btr_pcur_t* cursor);
/// \brief Returns the record of a persistent cursor.
/// \param [in] cursor persistent cursor
/// \return pointer to the record
IB_INLINE rec_t* btr_pcur_get_rec(btr_pcur_t* cursor);
/// \brief Checks if the persistent cursor is on a user record.
/// \param [in] cursor persistent cursor
IB_INLINE ibool btr_pcur_is_on_user_rec(const btr_pcur_t* cursor);
/// \brief Checks if the persistent cursor is after the last user record on a page.
/// \param [in] cursor persistent cursor
IB_INLINE ibool btr_pcur_is_after_last_on_page(const btr_pcur_t* cursor);
/// \brief Checks if the persistent cursor is before the first user record on a page.
/// \param [in] cursor persistent cursor
IB_INLINE ibool btr_pcur_is_before_first_on_page(const btr_pcur_t* cursor);
/// \brief Checks if the persistent cursor is before the first user record in the index tree.
/// \param [in] cursor persistent cursor
/// \param [in] mtr mtr
IB_INLINE ibool btr_pcur_is_before_first_in_tree(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Checks if the persistent cursor is after the last user record in the index tree.
/// \param [in] cursor persistent cursor
/// \param [in] mtr mtr
IB_INLINE ibool btr_pcur_is_after_last_in_tree(btr_pcur_t* cursor, mtr_t* mtr);
/// \brief Moves the persistent cursor to the next record on the same page.
/// \param [in,out] cursor persistent cursor
IB_INLINE void btr_pcur_move_to_next_on_page(btr_pcur_t* cursor);
/// \brief Moves the persistent cursor to the previous record on the same page.
/// \param [in,out] cursor persistent cursor
IB_INLINE void btr_pcur_move_to_prev_on_page(btr_pcur_t* cursor);



// TODO: currently, the state can be BTR_PCUR_IS_POSITIONED, though it really should be BTR_PCUR_WAS_POSITIONED, because we have no obligation to commit the cursor with mtr; similarly latch_mode may be out of date. This can lead to problems if btr_pcur is not used the right way; all current code should be ok.
constinit ulint BTR_PCUR_IS_POSITIONED = 1997660512;
constinit ulint BTR_PCUR_WAS_POSITIONED = 1187549791;
constinit ulint BTR_PCUR_NOT_POSITIONED = 1328997689;

constinit ulint BTR_PCUR_OLD_STORED = 908467085;
constinit ulint BTR_PCUR_OLD_NOT_STORED = 122766467;

#ifndef IB_DO_NOT_INLINE
#include "btr_pcur.inl"
#endif
