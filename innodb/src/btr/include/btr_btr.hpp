// Copyright (c) 1994, 2025, Innobase Oy. All Rights Reserved.
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

/// \file btr_btr.hpp
/// \brief The B-tree
/// \details Originally created by Heikki Tuuri in 6/2/1994
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#pragma once

#include "univ.inl"

#include "dict_dict.hpp"
#include "data_data.hpp"
#include "page_cur.hpp"
#include "mtr_mtr.hpp"
#include "btr_types.hpp"

#ifndef IB_HOTBACKUP


/// \brief Gets the root node of a tree and x-latches it.
/// \return root page, x-latched
/// \param [in] index index tree
/// \param [in] mtr mtr
IB_INTERN page_t* btr_root_get(dict_index_t* index, mtr_t* mtr);
/// \brief Gets a buffer page and declares its latching order level.
/// \param [in] space space id
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] page_no page number
/// \param [in] mode latch mode
/// \param [in] mtr mtr
IB_INLINE buf_block_t* btr_block_get(ulint space, ulint zip_size, ulint page_no, ulint mode, mtr_t* mtr);
/// \brief Gets a buffer page and declares its latching order level.
/// \param [in] space space id
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] page_no page number
/// \param [in] mode latch mode
/// \param [in] mtr mtr
IB_INLINE page_t* btr_page_get(ulint space, ulint zip_size, ulint page_no, ulint mode, mtr_t* mtr);
#endif /* !IB_HOTBACKUP */
/// \brief Gets the index id field of a page.
/// \return index id
/// \param [in] page index page
IB_INLINE dulint btr_page_get_index_id(const page_t* page);
#ifndef IB_HOTBACKUP
/// \brief Gets the node level field in an index page.
/// \return level, leaf level == 0
/// \param [in] page index page
IB_INLINE ulint btr_page_get_level_low(const page_t* page);
/// \brief Gets the node level field in an index page.
/// \return level, leaf level == 0
/// \param [in] page index page
/// \param [in] mtr mini-transaction handle
IB_INLINE ulint btr_page_get_level(const page_t* page, mtr_t* mtr);
/// \brief Gets the next index page number.
/// \return next page number
/// \param [in] page index page
/// \param [in] mtr mini-transaction handle
IB_INLINE ulint btr_page_get_next(const page_t* page, mtr_t* mtr);
/// \brief Gets the previous index page number.
/// \return prev page number
/// \param [in] page index page
/// \param [in] mtr mini-transaction handle
IB_INLINE ulint btr_page_get_prev(const page_t* page, mtr_t* mtr);
/// \brief Gets pointer to the previous user record in the tree. It is assumed that the caller has appropriate latches on the page and its neighbor.
/// \return previous user record, NULL if there is none
/// \param [in] rec record on leaf level
/// \param [in] mtr mtr holding a latch on the page, and if needed, also to the previous page
IB_INTERN rec_t* btr_get_prev_user_rec(rec_t* rec, mtr_t* mtr);
/// \brief Gets pointer to the next user record in the tree. It is assumed that the caller has appropriate latches on the page and its neighbor.
/// \return next user record, NULL if there is none
/// \param [in] rec record on leaf level
/// \param [in] mtr mtr holding a latch on the page, and if needed, also to the next page
IB_INTERN rec_t* btr_get_next_user_rec(rec_t* rec, mtr_t* mtr);
/// \brief Releases the latch on a leaf page and bufferunfixes it.
/// \param [in] block buffer block
/// \param [in] latch_mode BTR_SEARCH_LEAF or BTR_MODIFY_LEAF
/// \param [in] mtr mtr
IB_INLINE void btr_leaf_page_release(buf_block_t* block, ulint latch_mode, mtr_t* mtr);
/// \brief Gets the child node file address in a node pointer.
/// \details NOTE: the offsets array must contain all offsets for the record since we read the last field according to offsets and assume that it contains the child page number. In other words offsets must have been retrieved with rec_get_offsets(n_fields=ULINT_UNDEFINED).
/// \return child node address
/// \param [in] rec node pointer record
/// \param [in] offsets array returned by rec_get_offsets()
IB_INLINE ulint btr_node_ptr_get_child_page_no(const rec_t* rec, const ulint* offsets);
/// \brief Creates the root node for a new index tree.
/// \return page number of the created root, FIL_NULL if did not succeed
/// \param [in] type type of the index
/// \param [in] space space where created
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] index_id index id
/// \param [in] index index
/// \param [in] mtr mini-transaction handle
IB_INTERN ulint btr_create(ulint type, ulint space, ulint zip_size, dulint index_id, dict_index_t* index, mtr_t* mtr);
/// \brief Frees a B-tree except the root page, which MUST be freed after this by calling btr_free_root.
/// \param [in] space space where created
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] root_page_no root page number
IB_INTERN void btr_free_but_not_root(ulint space, ulint zip_size, ulint root_page_no);
/// \brief Frees the B-tree root page. Other tree MUST already have been freed.
/// \param [in] space space where created
/// \param [in] zip_size compressed page size in bytes or 0 for uncompressed pages
/// \param [in] root_page_no root page number
/// \param [in] mtr a mini-transaction which has already been started
IB_INTERN void btr_free_root(ulint space, ulint zip_size, ulint root_page_no, mtr_t* mtr);
/// \brief Makes tree one level higher by splitting the root, and inserts the tuple. It is assumed that mtr contains an x-latch on the tree.
/// \details NOTE that the operation of this function must always succeed, we cannot reverse it: therefore enough free disk space must be guaranteed to be available before this function is called.
/// \return inserted record
/// \param [in] cursor cursor at which to insert: must be on the root page; when the function returns, the cursor is positioned on the predecessor of the inserted record
/// \param [in] tuple tuple to insert
/// \param [in] n_ext number of externally stored columns
/// \param [in] mtr mtr
IB_INTERN rec_t* btr_root_raise_and_insert(btr_cur_t* cursor, const dtuple_t* tuple, ulint n_ext, mtr_t* mtr);
/// \brief Reorganizes an index page.
/// \details IMPORTANT: if btr_page_reorganize() is invoked on a compressed leaf page of a non-clustered index, the caller must update the insert buffer free bits in the same mini-transaction in such a way that the modification will be redo-logged.
/// \return TRUE on success, FALSE on failure
/// \param [in] block page to be reorganized
/// \param [in] index record descriptor
/// \param [in] mtr mtr
IB_INTERN ibool btr_page_reorganize(buf_block_t* block, dict_index_t* index, mtr_t* mtr);
/// \brief Decides if the page should be split at the convergence point of inserts converging to left.
/// \return TRUE if split recommended
/// \param [in] cursor cursor at which to insert
/// \param [out] split_rec if split recommended, the first record on upper half page, or NULL if tuple should be first
IB_INTERN ibool btr_page_get_split_rec_to_left(btr_cur_t* cursor, rec_t** split_rec);
/// \brief Decides if the page should be split at the convergence point of inserts converging to right.
/// \return TRUE if split recommended
/// \param [in] cursor cursor at which to insert
/// \param [out] split_rec if split recommended, the first record on upper half page, or NULL if tuple should be first
IB_INTERN ibool btr_page_get_split_rec_to_right(btr_cur_t* cursor, rec_t** split_rec);
/// \brief Splits an index page to halves and inserts the tuple. It is assumed that mtr holds an x-latch to the index tree.
/// \details NOTE: the tree x-latch is released within this function! NOTE that the operation of this function must always succeed, we cannot reverse it: therefore enough free disk space (2 pages) must be guaranteed to be available before this function is called.
/// \return inserted record
/// \param [in] cursor cursor at which to insert; when the function returns, the cursor is positioned on the predecessor of the inserted record
/// \param [in] tuple tuple to insert
/// \param [in] n_ext number of externally stored columns
/// \param [in] mtr mtr
IB_INTERN rec_t* btr_page_split_and_insert(btr_cur_t* cursor, const dtuple_t* tuple, ulint n_ext, mtr_t* mtr);
/// \brief Inserts a data tuple to a tree on a non-leaf level. It is assumed that mtr holds an x-latch on the tree.
/// \param [in] index index
/// \param [in] level level, must be > 0
/// \param [in] tuple the record to be inserted
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_insert_on_non_leaf_level_func(dict_index_t* index, ulint level, dtuple_t* tuple, const char* file, ulint line, mtr_t* mtr);
# define btr_insert_on_non_leaf_level(i,l,t,m) \
    btr_insert_on_non_leaf_level_func(i,l,t,__FILE__,__LINE__,m)
#endif /* !IB_HOTBACKUP */
/// \brief Sets a record as the predefined minimum record.
/// \param [in,out] rec record
/// \param [in] mtr mtr
IB_INTERN void btr_set_min_rec_mark(rec_t* rec, mtr_t* mtr);
#ifndef IB_HOTBACKUP
/// \brief Deletes on the upper level the node pointer to a page.
/// \param [in] index index tree
/// \param [in] block page whose node pointer is deleted
/// \param [in] mtr mtr
IB_INTERN void btr_node_ptr_delete(dict_index_t* index, buf_block_t* block, mtr_t* mtr);
#ifdef IB_DEBUG
/// \brief Checks that the node pointer to a page is appropriate.
/// \return TRUE
/// \param [in] index index tree
/// \param [in] block index page
/// \param [in] mtr mtr
IB_INTERN ibool btr_check_node_ptr(dict_index_t* index, buf_block_t* block, mtr_t* mtr);
#endif /* IB_DEBUG */
/// \brief Tries to merge the page first to the left immediate brother if such a brother exists, and the node pointers to the current page and to the brother reside on the same page. If the left brother does not satisfy these conditions, looks at the right brother. If the page is the only one on that level lifts the records of the page to the father page, thus reducing the tree height. It is assumed that mtr holds an x-latch on the tree and on the page. If cursor is on the leaf level, mtr must also hold x-latches to the brothers, if they exist.
/// \return TRUE on success
/// \param [in] cursor cursor on the page to merge or lift; the page must not be empty: in record delete use btr_discard_page if the page would become empty
/// \param [in] mtr mtr
IB_INTERN ibool btr_compress(btr_cur_t* cursor, mtr_t* mtr);
/// \brief Discards a page from a B-tree. This is used to remove the last record from a B-tree page: the whole page must be removed at the same time. This cannot be used for the root page, which is allowed to be empty.
/// \param [in] cursor cursor on the page to discard: not on the root page
/// \param [in] mtr mtr
IB_INTERN void btr_discard_page(btr_cur_t* cursor, mtr_t* mtr);
#endif /* !IB_HOTBACKUP */
/// \brief Parses the redo log record for setting an index record as the predefined minimum record.
/// \return end of log record or NULL
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in] comp nonzero=compact page format
/// \param [in] page page or NULL
/// \param [in] mtr mtr or NULL
IB_INTERN byte* btr_parse_set_min_rec_mark(byte* ptr, byte* end_ptr, ulint comp, page_t* page, mtr_t* mtr);
/// \brief Parses a redo log record of reorganizing a page.
/// \return end of log record or NULL
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in] index record descriptor
/// \param [in] block page to be reorganized, or NULL
/// \param [in] mtr mtr or NULL
IB_INTERN byte* btr_parse_page_reorganize(byte* ptr, byte* end_ptr, dict_index_t* index, buf_block_t* block, mtr_t* mtr);
#ifndef IB_HOTBACKUP
/// \brief Gets the number of pages in a B-tree.
/// \return number of pages
/// \param [in] index index
/// \param [in] flag BTR_N_LEAF_PAGES or BTR_TOTAL_SIZE
IB_INTERN ulint btr_get_size(dict_index_t* index, ulint flag);
/// \brief Allocates a new file page to be used in an index tree. NOTE: we assume that the caller has made the reservation for free extents!
/// \return new allocated block, x-latched; NULL if out of space
/// \param [in] index index tree
/// \param [in] hint_page_no hint of a good page
/// \param [in] file_direction direction where a possible page split is made
/// \param [in] level level where the page is placed in the tree
/// \param [in] mtr mtr
IB_INTERN buf_block_t* btr_page_alloc(dict_index_t* index, ulint hint_page_no, byte file_direction, ulint level, mtr_t* mtr);
/// \brief Frees a file page used in an index tree. NOTE: cannot free field external storage pages because the page must contain info on its level.
/// \param [in] index index tree
/// \param [in] block block to be freed, x-latched
/// \param [in] mtr mtr
IB_INTERN void btr_page_free(dict_index_t* index, buf_block_t* block, mtr_t* mtr);
/// \brief Frees a file page used in an index tree. Can be used also to BLOB external storage pages, because the page level 0 can be given as an argument.
/// \param [in] index index tree
/// \param [in] block block to be freed, x-latched
/// \param [in] level page level
/// \param [in] mtr mtr
IB_INTERN void btr_page_free_low(dict_index_t* index, buf_block_t* block, ulint level, mtr_t* mtr);
#ifdef IB_BTR_PRINT
/// \brief Prints size info of a B-tree.
/// \param [in] index index tree
IB_INTERN void btr_print_size(dict_index_t* index);
/// \brief Prints directories and other info of all nodes in the index.
/// \param [in] index index
/// \param [in] width print this many entries from start and end
IB_INTERN void btr_print_index(dict_index_t* index, ulint width);
#endif /* IB_BTR_PRINT */
/// \brief Checks the size and number of fields in a record based on the definition of the index.
/// \return TRUE if ok
/// \param [in] rec index record
/// \param [in] index index
/// \param [in] dump_on_error TRUE if the function should print hex dump of record and page on error
IB_INTERN ibool btr_index_rec_validate(const rec_t* rec, const dict_index_t* index, ibool dump_on_error);
/// \brief Checks the consistency of an index tree.
/// \return TRUE if ok
/// \param [in] index index
/// \param [in] trx transaction or NULL
IB_INTERN ibool btr_validate_index(dict_index_t* index, trx_t* trx);

constinit ulint BTR_N_LEAF_PAGES = 1;
constinit ulint BTR_TOTAL_SIZE = 2;

#endif // !IB_HOTBACKUP

#ifndef IB_DO_NOT_INLINE
#include "btr_btr.inl"
#endif

