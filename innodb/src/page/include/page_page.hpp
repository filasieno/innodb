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

/// \file page_page.hpp
/// \brief Index page routines
/// \details Originally created by Heikki Tuuri in 2/2/1994
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#pragma once

#include "univ.i"

#include "page_types.hpp"
#include "fil_fil.hpp"
#include "buf_buf.hpp"
#include "data_data.hpp"
#include "dict_dict.hpp"
#include "rem_rec.hpp"
#include "fsp_fsp.hpp"
#include "mtr_mtr.hpp"

#ifdef IB_MATERIALIZE
#undef IB_INLINE
#define IB_INLINE
#endif

/*            PAGE HEADER
            ===========

Index page header starts at the first offset left free by the FIL-module */

typedef    byte        page_header_t;

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

constinit ulint PAGE_HEADER = FSEG_PAGE_DATA;    /* index page header starts at this offset */
constinit ulint PAGE_N_DIR_SLOTS = 0;    /* number of slots in page directory */
constinit ulint PAGE_HEAP_TOP = 2;    /* pointer to record heap top */
constinit ulint PAGE_N_HEAP = 4;    /* number of records in the heap, bit 15=flag: new-style compact page format */
constinit ulint PAGE_FREE = 6;    /* pointer to start of page free record list */
constinit ulint PAGE_GARBAGE = 8;    /* number of bytes in deleted records */
constinit ulint PAGE_LAST_INSERT = 10;    /* pointer to the last inserted record, or NULL if this info has been reset by a delete, for example */
constinit ulint PAGE_DIRECTION = 12;    /* last insert direction: PAGE_LEFT, ... */
constinit ulint PAGE_N_DIRECTION = 14;    /* number of consecutive inserts to the same direction */
constinit ulint PAGE_N_RECS = 16;    /* number of user records on the page */
constinit ulint PAGE_MAX_TRX_ID = 18;    /* highest id of a trx which may have modified a record on the page; a dulint; defined only in secondary indexes and in the insert buffer tree; NOTE: this may be modified only when the thread has an x-latch to the page, and ALSO an x-latch to btr_search_latch if there is a hash index to the page! */
constinit ulint PAGE_HEADER_PRIV_END = 26;    /* end of private data structure of the page header which are set in a page create */
constinit ulint PAGE_LEVEL = 26;    /* level of the node in an index tree; the leaf level is the level 0.  This field should not be written to after page creation. */
constinit ulint PAGE_INDEX_ID = 28;    /* index id where the page belongs. This field should not be written to after page creation. */
constinit ulint PAGE_BTR_SEG_LEAF = 36;    /* file segment header for the leaf pages in a B-tree: defined only on the root page of a B-tree, but not in the root of an ibuf tree */
constinit ulint PAGE_BTR_IBUF_FREE_LIST = PAGE_BTR_SEG_LEAF;
constinit ulint PAGE_BTR_IBUF_FREE_LIST_NODE = PAGE_BTR_SEG_LEAF;    /* in the place of PAGE_BTR_SEG_LEAF and _TOP there is a free list base node if the page is the root page of an ibuf tree, and at the same place is the free list node if the page is in a free list */
constinit ulint PAGE_BTR_SEG_TOP = (36 + FSEG_HEADER_SIZE);    /* file segment header for the non-leaf pages in a B-tree: defined only on the root page of a B-tree, but not in the root of an ibuf tree */
constinit ulint PAGE_DATA = (PAGE_HEADER + 36 + 2 * FSEG_HEADER_SIZE);    /* start of data on the page */
constinit ulint PAGE_OLD_INFIMUM = (PAGE_DATA + 1 + REC_N_OLD_EXTRA_BYTES);    /* offset of the page infimum record on an old-style page */
constinit ulint PAGE_OLD_SUPREMUM = (PAGE_DATA + 2 + 2 * REC_N_OLD_EXTRA_BYTES + 8);    /* offset of the page supremum record on an old-style page */
constinit ulint PAGE_OLD_SUPREMUM_END = (PAGE_OLD_SUPREMUM + 9);    /* offset of the page supremum record end on an old-style page */
constinit ulint PAGE_NEW_INFIMUM = (PAGE_DATA + REC_N_NEW_EXTRA_BYTES);    /* offset of the page infimum record on a new-style compact page */
constinit ulint PAGE_NEW_SUPREMUM = (PAGE_DATA + 2 * REC_N_NEW_EXTRA_BYTES + 8);    /* offset of the page supremum record on a new-style compact page */
constinit ulint PAGE_NEW_SUPREMUM_END = (PAGE_NEW_SUPREMUM + 8);    /* offset of the page supremum record end on a new-style compact page */

/* Heap numbers */
constinit ulint PAGE_HEAP_NO_INFIMUM = 0;    /* page infimum */
constinit ulint PAGE_HEAP_NO_SUPREMUM = 1;    /* page supremum */
constinit ulint PAGE_HEAP_NO_USER_LOW = 2;    /* first user record in creation (insertion) order, not necessarily collation order; this record may have been deleted */

/* Directions of cursor movement */
constinit ulint PAGE_LEFT = 1;
constinit ulint PAGE_RIGHT = 2;
constinit ulint PAGE_SAME_REC = 3;
constinit ulint PAGE_SAME_PAGE = 4;
constinit ulint PAGE_NO_DIRECTION = 5;

/*            PAGE DIRECTORY
            ==============
*/

typedef    byte            page_dir_slot_t;
typedef page_dir_slot_t        page_dir_t;

/* Offset of the directory start down from the page end. We call the slot with the highest file address directory start, as it points to the first record in the list of records. */
constinit ulint PAGE_DIR = FIL_PAGE_DATA_END;

/* We define a slot in the page directory as two bytes */
constinit ulint PAGE_DIR_SLOT_SIZE = 2;

/* The offset of the physically lower end of the directory, counted from page end, when the page is empty */
constinit ulint PAGE_EMPTY_DIR_START = (PAGE_DIR + 2 * PAGE_DIR_SLOT_SIZE);

/* The maximum and minimum number of records owned by a directory slot. The number may drop below the minimum in the first and the last slot in the directory. */
constinit ulint PAGE_DIR_SLOT_MAX_N_OWNED = 8;
constinit ulint PAGE_DIR_SLOT_MIN_N_OWNED = 4;


/// \brief Gets the start of a page.
/// \return start of the page
/// \param [in] ptr pointer to page frame
IB_INLINE page_t* page_align(const void* ptr) __attribute__((const));

/// \brief Gets the offset within a page.
/// \return offset from the start of the page
/// \param [in] ptr pointer to page frame
IB_INLINE ulint page_offset(const void* ptr) __attribute__((const));

/// \brief Returns the max trx id field value.
/// \param [in] page page
IB_INLINE trx_id_t page_get_max_trx_id(const page_t* page);

/// \brief Sets the max trx id field value.
/// \param [in,out] block page
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] trx_id transaction id
/// \param [in,out] mtr mini-transaction, or NULL
IB_INTERN void page_set_max_trx_id(buf_block_t* block, page_zip_des_t* page_zip, trx_id_t trx_id, mtr_t* mtr);

/// \brief Sets the max trx id field value if trx_id is bigger than the previous value.
/// \param [in,out] block page
/// \param [in,out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] trx_id transaction id
/// \param [in,out] mtr mini-transaction
IB_INLINE void page_update_max_trx_id(buf_block_t* block, page_zip_des_t* page_zip, trx_id_t trx_id, mtr_t* mtr);

/// \brief Reads the given header field.
/// \param [in] page page
/// \param [in] field PAGE_LEVEL, ...
IB_INLINE ulint page_header_get_field(const page_t* page, ulint field);

/// \brief Sets the given header field.
/// \param [in,out] page page
/// \param [in,out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] field PAGE_N_DIR_SLOTS, ...
/// \param [in] val value
IB_INLINE void page_header_set_field(page_t* page, page_zip_des_t* page_zip, ulint field, ulint val);

/// \brief Returns the offset stored in the given header field.
/// \return offset from the start of the page, or 0
/// \param [in] page page
/// \param [in] field PAGE_FREE, ...
IB_INLINE ulint page_header_get_offs(const page_t* page, ulint field) __attribute__((nonnull, pure));

/*************************************************************//**
Returns the pointer stored in the given header field, or NULL. */
#define page_header_get_ptr(page, field)            \
    (page_header_get_offs(page, field)            \
     ? page + page_header_get_offs(page, field) : NULL)

	 /// \brief Sets the pointer stored in the given header field.
/// \param [in] page page
/// \param [in,out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in,out] field PAGE_FREE, ...
/// \param [in] ptr pointer or NULL
IB_INLINE void page_header_set_ptr(page_t* page, page_zip_des_t* page_zip, ulint field, const byte* ptr);
#ifndef IB_HOTBACKUP

/// \brief Resets the last insert info field in the page header. Writes to mlog about this operation.
/// \param [in] page page
/// \param [in,out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] mtr mtr
IB_INLINE void page_header_reset_last_insert(page_t* page, page_zip_des_t* page_zip, mtr_t* mtr);
#endif /* !IB_HOTBACKUP */

/// \brief Gets the offset of the first record on the page.
/// \return offset of the first record in record list, relative from page
/// \param [in] page page which must have record(s)
IB_INLINE ulint page_get_infimum_offset(const page_t* page);

/// \brief Gets the offset of the last record on the page.
/// \return offset of the last record in record list, relative from page
/// \param [in] page page which must have record(s)
IB_INLINE ulint page_get_supremum_offset(const page_t* page);
#define page_get_infimum_rec(page) ((page) + page_get_infimum_offset(page))
#define page_get_supremum_rec(page) ((page) + page_get_supremum_offset(page))

/// \brief Returns the middle record of record list. If there are an even number of records in the list, returns the first record of upper half-list.
/// \return middle record
/// \param [in] page page
IB_INTERN rec_t* page_get_middle_rec(page_t* page);
#ifndef IB_HOTBACKUP

/// \brief Compares a data tuple to a physical record.
/// \details Differs from the function cmp_dtuple_rec_with_match in the way that the record must reside on an index page, and also page infimum and supremum records can be given in the parameter rec. These are considered as the negative infinity and the positive infinity in the alphabetical order.
/// \return 1, 0, -1, if dtuple is greater, equal, less than rec, respectively, when only the common first fields are compared
/// \param [in] cmp_ctx client compare context
/// \param [in] dtuple data tuple
/// \param [in] rec physical record on a page; may also be page infimum or supremum, in which case matched-parameter values below are not affected
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in,out] matched_fields number of already completely matched fields; when function returns contains the value for current comparison
/// \param [in,out] matched_bytes number of already matched bytes within the first field not completely matched; when function returns contains the value for current comparison
IB_INLINE int page_cmp_dtuple_rec_with_match(void* cmp_ctx, const dtuple_t* dtuple, const rec_t* rec, const ulint* offsets, ulint* matched_fields, ulint* matched_bytes);
#endif /* !IB_HOTBACKUP */

/// \brief Gets the page number.
/// \return page number
/// \param [in] page page
IB_INLINE ulint page_get_page_no(const page_t* page);

/// \brief Gets the tablespace identifier.
/// \param [in] page page
/// \return space id
IB_INLINE ulint page_get_space_id(const page_t* page);

/// \brief Gets the number of user records on page (the infimum and supremum records are not user records).
/// \param [in] page index page
/// \return number of user records
IB_INLINE ulint page_get_n_recs(const page_t* page);

/// \brief Returns the number of records before the given record in chain. The number includes infimum and supremum records.
/// \param [in] rec the physical record
/// \return number of records
IB_INTERN ulint page_rec_get_n_recs_before(const rec_t* rec);

/// \brief Gets the number of records in the heap.
/// \param [in] page index page
/// \return number of user records
IB_INLINE ulint page_dir_get_n_heap(const page_t* page);

/// \brief Sets the number of records in the heap.
/// \param [in,out] page index page
/// \param [in,out] page_zip compressed page whose uncompressed part will be updated, or NULL. Note that the size of the dense page directory in the compressed page trailer is n_heap * PAGE_ZIP_DIR_SLOT_SIZE.
/// \param [in] n_heap number of records
IB_INLINE void page_dir_set_n_heap(page_t* page, page_zip_des_t* page_zip, ulint n_heap);

/// \brief Gets the number of dir slots in directory.
/// \param [in] page index page
/// \return number of slots
IB_INLINE ulint page_dir_get_n_slots(const page_t* page);

/// \brief Sets the number of dir slots in directory.
/// \param [in,out] page page
/// \param [in,out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in] n_slots number of slots
IB_INLINE void page_dir_set_n_slots(page_t* page, page_zip_des_t* page_zip, ulint n_slots);
#ifdef IB_DEBUG
	
/// \brief Gets pointer to nth directory slot.
	/// \param [in] page index page
	/// \param [in] n position
	/// \return pointer to dir slot
	IB_INLINE page_dir_slot_t* page_dir_get_nth_slot(const page_t* page, ulint n);
#else /* IB_DEBUG */
	#define page_dir_get_nth_slot(page, n) ((page) + IB_PAGE_SIZE - PAGE_DIR - (n + 1) * PAGE_DIR_SLOT_SIZE)
#endif /* IB_DEBUG */

/// \brief Used to check the consistency of a record on a page.
/// \param [in] rec record
/// \return TRUE if succeed
IB_INLINE ibool page_rec_check(const rec_t* rec);

/// \brief Gets the record pointed to by a directory slot.
/// \param [in] slot directory slot
/// \return pointer to record
IB_INLINE const rec_t* page_dir_slot_get_rec(const page_dir_slot_t* slot);

/// \brief This is used to set the record offset in a directory slot.
/// \param [in] slot directory slot
/// \param [in] rec record on the page
IB_INLINE void page_dir_slot_set_rec(page_dir_slot_t* slot, rec_t* rec);

/// \brief Gets the number of records owned by a directory slot.
/// \param [in] slot page directory slot
/// \return number of records
IB_INLINE ulint page_dir_slot_get_n_owned(const page_dir_slot_t* slot);

/// \brief This is used to set the owned records field of a directory slot.
/// \param [in,out] slot directory slot
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] n number of records owned by the slot
IB_INLINE void page_dir_slot_set_n_owned(page_dir_slot_t* slot, page_zip_des_t* page_zip, ulint n);

/// \brief Calculates the space reserved for directory slots of a given number of records. The exact value is a fraction number n * PAGE_DIR_SLOT_SIZE / PAGE_DIR_SLOT_MIN_N_OWNED, and it is rounded upwards to an integer.
/// \param [in] n_recs number of records
/// \return space reserved for directory slots
IB_INLINE ulint page_dir_calc_reserved_space(ulint n_recs);

/// \brief Looks for the directory slot which owns the given record.
/// \param [in] rec the physical record
/// \return the directory slot number
IB_INTERN ulint page_dir_find_owner_slot(const rec_t* rec);

/// \brief Determine whether the page is in new-style compact format.
/// \param [in] page index page
/// \return nonzero if the page is in compact format, zero if it is in old-style format
IB_INLINE ulint page_is_comp(const page_t* page);

/// \brief TRUE if the record is on a page in compact format.
/// \param [in] rec record
/// \return nonzero if in compact format
IB_INLINE ulint page_rec_is_comp(const rec_t* rec);

/// \brief Returns the heap number of a record.
/// \param [in] rec the physical record
/// \return heap number
IB_INLINE ulint page_rec_get_heap_no(const rec_t* rec);

/// \brief Determine whether the page is a B-tree leaf.
/// \param [in] page page
/// \return TRUE if the page is a B-tree leaf
IB_INLINE ibool page_is_leaf(const page_t* page) __attribute__((nonnull, pure));

/// \brief Gets the pointer to the next record on the page.
/// \param [in] rec pointer to record
/// \param [in] comp nonzero=compact page layout
/// \return pointer to next record
IB_INLINE const rec_t* page_rec_get_next_low(const rec_t* rec, ulint comp);

/// \brief Gets the pointer to the next record on the page.
/// \param [in] rec pointer to record
/// \return pointer to next record
IB_INLINE rec_t* page_rec_get_next(rec_t* rec);

/// \brief Gets the pointer to the next record on the page.
/// \param [in] rec pointer to record
/// \return pointer to next record
IB_INLINE const rec_t* page_rec_get_next_const(const rec_t* rec);

/// \brief Sets the pointer to the next record on the page.
/// \param [in] rec pointer to record, must not be page supremum
/// \param [in] next pointer to next record, must not be page infimum
IB_INLINE void page_rec_set_next(rec_t* rec, rec_t* next);

/// \brief Gets the pointer to the previous record.
/// \param [in] rec pointer to record, must not be page infimum
/// \return pointer to previous record
IB_INLINE const rec_t* page_rec_get_prev_const(const rec_t* rec);

/// \brief Gets the pointer to the previous record.
/// \param [in] rec pointer to record, must not be page infimum
/// \return pointer to previous record
IB_INLINE rec_t* page_rec_get_prev(rec_t* rec);

/// \brief TRUE if the record is a user record on the page.
/// \param [in] offset record offset on page
/// \return TRUE if a user record
IB_INLINE ibool page_rec_is_user_rec_low(ulint offset) __attribute__((const));

/// \brief TRUE if the record is the supremum record on a page.
/// \param [in] offset record offset on page
/// \return TRUE if the supremum record
IB_INLINE ibool page_rec_is_supremum_low(ulint offset) __attribute__((const));

/// \brief TRUE if the record is the infimum record on a page.
/// \param [in] offset record offset on page
/// \return TRUE if the infimum record
IB_INLINE ibool page_rec_is_infimum_low(ulint offset) __attribute__((const));


/// \brief TRUE if the record is a user record on the page.
/// \param [in] rec record
/// \return TRUE if a user record
IB_INLINE ibool page_rec_is_user_rec(const rec_t* rec) __attribute__((const));

/// \brief TRUE if the record is the supremum record on a page.
/// \param [in] rec record
/// \return TRUE if the supremum record
IB_INLINE ibool page_rec_is_supremum(const rec_t* rec) __attribute__((const));


/// \brief TRUE if the record is the infimum record on a page.
/// \param [in] rec record
/// \return TRUE if the infimum record
IB_INLINE ibool page_rec_is_infimum(const rec_t* rec) __attribute__((const));

/// \brief Looks for the record which owns the given record.
/// \param [in] rec the physical record
/// \return the owner record
IB_INLINE rec_t* page_rec_find_owner_rec(rec_t* rec);

/// \brief This is a low-level operation which is used in a database index creation to update the page number of a created B-tree to a data dictionary record.
/// \param [in] rec record to update
/// \param [in] i index of the field to update
/// \param [in] page_no value to write
/// \param [in] mtr mtr
IB_INTERN void page_rec_write_index_page_no(rec_t* rec, ulint i, ulint page_no, mtr_t* mtr);

/// \brief Returns the maximum combined size of records which can be inserted on top of record heap.
/// \param [in] page index page
/// \param [in] n_recs number of records
/// \return maximum combined size for inserted records
IB_INLINE ulint page_get_max_insert_size(const page_t* page, ulint n_recs);

/// \brief Returns the maximum combined size of records which can be inserted on top of record heap if page is first reorganized.
/// \param [in] page index page
/// \param [in] n_recs number of records
/// \return maximum combined size for inserted records
IB_INLINE ulint page_get_max_insert_size_after_reorganize(const page_t* page, ulint n_recs);

/// \brief Calculates free space if a page is emptied.
/// \param [in] comp nonzero=compact page format
/// \return free space
IB_INLINE ulint page_get_free_space_of_empty(ulint comp) __attribute__((const));

/// \brief Returns the base extra size of a physical record. This is the size of the fixed header, independent of the record size.
/// \param [in] rec physical record
/// \return REC_N_NEW_EXTRA_BYTES or REC_N_OLD_EXTRA_BYTES
IB_INLINE ulint page_rec_get_base_extra_size(const rec_t* rec);

/// \brief Returns the sum of the sizes of the records in the record list excluding the infimum and supremum records.
/// \param [in] page index page
/// \return data in bytes
IB_INLINE ulint page_get_data_size(const page_t* page);

/// \brief Allocates a block of memory from the head of the free list of an index page.
/// \param [in,out] page index page
/// \param [in,out] page_zip compressed page with enough space available for inserting the record, or NULL
/// \param [in] next_rec pointer to the new head of the free record list
/// \param [in] need number of bytes allocated
IB_INLINE void page_mem_alloc_free(page_t* page, page_zip_des_t* page_zip, rec_t* next_rec, ulint need);

/// \brief Allocates a block of memory from the heap of an index page.
/// \param [in,out] page index page
/// \param [in,out] page_zip compressed page with enough space available for inserting the record, or NULL
/// \param [in] need total number of bytes needed
/// \param [out] heap_no this contains the heap number of the allocated record if allocation succeeds
/// \return pointer to start of allocated buffer, or NULL if allocation fails
IB_INTERN byte* page_mem_alloc_heap(page_t* page, page_zip_des_t* page_zip, ulint need, ulint* heap_no);

/// \brief Puts a record to free list.
/// \param [in,out] page index page
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] rec pointer to the (origin of) record
/// \param [in] index index of rec
/// \param [in] offsets array returned by rec_get_offsets()
IB_INLINE void page_mem_free(page_t* page, page_zip_des_t* page_zip, rec_t* rec, dict_index_t* index, const ulint* offsets);

/// \brief Create an uncompressed B-tree index page.
/// \param [in] block a buffer block where the page is created
/// \param [in] mtr mini-transaction handle
/// \param [in] comp nonzero=compact page format
/// \return pointer to the page
IB_INTERN page_t* page_create(buf_block_t* block, mtr_t* mtr, ulint comp);

/// \brief Create a compressed B-tree index page.
/// \param [in,out] block a buffer frame where the page is created
/// \param [in] index the index of the page
/// \param [in] level the B-tree level of the page
/// \param [in] mtr mini-transaction handle
/// \return pointer to the page
IB_INTERN page_t* page_create_zip(buf_block_t* block, dict_index_t* index, ulint level, mtr_t* mtr);


/// \brief Differs from page_copy_rec_list_end, because this function does not touch the lock table and max trx id on page or compress the page.
/// \param [in] new_block index page to copy to
/// \param [in] block index page of rec
/// \param [in] rec record on page
/// \param [in] index record descriptor
/// \param [in] mtr mtr
IB_INTERN void page_copy_rec_list_end_no_locks(buf_block_t* new_block, buf_block_t* block, rec_t* rec, dict_index_t* index, mtr_t* mtr);

/// \brief Copies records from page to new_page, from the given record onward, including that record. Infimum and supremum records are not copied. The records are copied to the start of the record list on new_page.
/// \param [in,out] new_block index page to copy to
/// \param [in] block index page containing rec
/// \param [in] rec record on page
/// \param [in] index record descriptor
/// \param [in] mtr mtr
/// \return pointer to the original successor of the infimum record on new_page, or NULL on zip overflow (new_block will be decompressed)
IB_INTERN rec_t* page_copy_rec_list_end(buf_block_t* new_block, buf_block_t* block, rec_t* rec, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull));

/// \brief Copies records from page to new_page, up to the given record, NOT including that record. Infimum and supremum records are not copied. The records are copied to the end of the record list on new_page.
/// \param [in,out] new_block index page to copy to
/// \param [in] block index page containing rec
/// \param [in] rec record on page
/// \param [in] index record descriptor
/// \param [in] mtr mtr
/// \return pointer to the original predecessor of the supremum record on new_page, or NULL on zip overflow (new_block will be decompressed)
IB_INTERN rec_t* page_copy_rec_list_start(buf_block_t* new_block, buf_block_t* block, rec_t* rec, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull));

/// \brief Deletes records from a page from a given record onward, including that record. The infimum and supremum records are not deleted.
/// \param [in] rec pointer to record on page
/// \param [in] block buffer block of the page
/// \param [in] index record descriptor
/// \param [in] n_recs number of records to delete, or ULINT_UNDEFINED if not known
/// \param [in] size the sum of the sizes of the records in the end of the chain to delete, or ULINT_UNDEFINED if not known
/// \param [in] mtr mtr
IB_INTERN void page_delete_rec_list_end(rec_t* rec, buf_block_t* block, dict_index_t* index, ulint n_recs, ulint size, mtr_t* mtr) __attribute__((nonnull));

/// \brief Deletes records from page, up to the given record, NOT including that record. Infimum and supremum records are not deleted.
/// \param [in] rec record on page
/// \param [in] block buffer block of the page
/// \param [in] index record descriptor
/// \param [in] mtr mtr
IB_INTERN void page_delete_rec_list_start(rec_t* rec, buf_block_t* block, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull));

/// \brief Moves record list end to another page. Moved records include split_rec.
/// \param [in,out] new_block index page where to move
/// \param [in] block index page from where to move
/// \param [in] split_rec first record to move
/// \param [in] index record descriptor
/// \param [in] mtr mtr
/// \return TRUE on success; FALSE on compression failure (new_block will be decompressed)
IB_INTERN ibool page_move_rec_list_end(buf_block_t* new_block, buf_block_t* block, rec_t* split_rec, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull(1, 2, 4, 5)));

/// \brief Moves record list start to another page. Moved records do not include split_rec.
/// \param [in,out] new_block index page where to move
/// \param [in,out] block page containing split_rec
/// \param [in] split_rec first record not to move
/// \param [in] index record descriptor
/// \param [in] mtr mtr
/// \return TRUE on success; FALSE on compression failure
IB_INTERN ibool page_move_rec_list_start(buf_block_t* new_block, buf_block_t* block, rec_t* split_rec, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull(1, 2, 4, 5)));

/// \brief Splits a directory slot which owns too many records.
/// \param [in] page index page
/// \param [in,out] page_zip compressed page whose uncompressed part will be written, or NULL
/// \param [in] slot_no the directory slot
IB_INTERN void page_dir_split_slot(page_t* page, page_zip_des_t* page_zip, ulint slot_no) __attribute__((nonnull(1)));

/// \brief Tries to balance the given directory slot with too few records with the upper neighbor, so that there are at least the minimum number of records owned by the slot; this may result in the merging of two slots.
/// \param [in,out] page index page
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] slot_no the directory slot
IB_INTERN void page_dir_balance_slot(page_t* page, page_zip_des_t* page_zip, ulint slot_no) __attribute__((nonnull(1)));

/// \brief Parses a log record of a record list end or start deletion.
/// \param [in] type MLOG_LIST_END_DELETE, MLOG_LIST_START_DELETE, MLOG_COMP_LIST_END_DELETE or MLOG_COMP_LIST_START_DELETE
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in,out] block buffer block or NULL
/// \param [in] index record descriptor
/// \param [in] mtr mtr or NULL
/// \return end of log record or NULL
IB_INTERN byte* page_parse_delete_rec_list(byte type, byte* ptr, byte* end_ptr, buf_block_t* block, dict_index_t* index, mtr_t* mtr);

/// \brief Parses a redo log record of creating a page.
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in] comp nonzero=compact page format
/// \param [in] block block or NULL
/// \param [in] mtr mtr or NULL
/// \return end of log record or NULL
IB_INTERN byte* page_parse_create(byte* ptr, byte* end_ptr, ulint comp, buf_block_t* block, mtr_t* mtr);

/// \brief Prints record contents including the data relevant only in the index page context.
/// \param [in] rec physical record
/// \param [in] offsets record descriptor
IB_INTERN void page_rec_print(const rec_t* rec, const ulint* offsets);

/// \brief This is used to print the contents of the directory for debugging purposes.
/// \param [in] page index page
/// \param [in] pr_n print n first and n last entries
IB_INTERN void page_dir_print(page_t* page, ulint pr_n);

/// \brief This is used to print the contents of the page record list for debugging purposes.
/// \param [in] block index page
/// \param [in] index dictionary index of the page
/// \param [in] pr_n print n first and n last entries
IB_INTERN void page_print_list(buf_block_t* block, dict_index_t* index, ulint pr_n);

/// \brief Prints the info in a page header.
/// \param [in] page index page
IB_INTERN void page_header_print(const page_t* page);

/// \brief This is used to print the contents of the page for debugging purposes.
/// \param [in] block index page
/// \param [in] index dictionary index of the page
/// \param [in] dn print dn first and last entries in directory
/// \param [in] rn print rn first and last records in directory
IB_INTERN void page_print(buf_block_t* block, dict_index_t* index, ulint dn, ulint rn);

/***************************************************************//**
The following is used to validate a record on a page. This function
differs from rec_validate as it can also check the n_owned field and
the heap_no field.
/// \return    TRUE if ok */
IB_INTERN
ibool
page_rec_validate(
/*==============*/
    rec_t*        rec,    /*!< in: physical record */
    const ulint*    offsets);/*!< in: array returned by rec_get_offsets() */
/***************************************************************//**
Checks that the first directory slot points to the infimum record and
the last to the supremum. This function is intended to track if the
bug fixed in 4.0.14 has caused corruption to users' databases. */
IB_INTERN
void
page_check_dir(
/*===========*/
    const page_t*    page);    /*!< in: index page */
/***************************************************************//**
This function checks the consistency of an index page when we do not
know the index. This is also resilient so that this should never crash
even if the page is total garbage.
/// \return    TRUE if ok */
IB_INTERN
ibool
page_simple_validate_old(
/*=====================*/
    page_t*    page);    /*!< in: old-style index page */
/***************************************************************//**
This function checks the consistency of an index page when we do not
know the index. This is also resilient so that this should never crash
even if the page is total garbage.
/// \return    TRUE if ok */
IB_INTERN
ibool
page_simple_validate_new(
/*=====================*/
    page_t*    block);    /*!< in: new-style index page */
/***************************************************************//**
This function checks the consistency of an index page.
/// \return    TRUE if ok */
IB_INTERN
ibool
page_validate(
/*==========*/
    page_t*        page,    /*!< in: index page */
    dict_index_t*    index);    /*!< in: data dictionary index containing
                the page record type definition */
/// \brief Looks in the page record list for a record with the given heap number.
/// \param [in] page index page
/// \param [in] heap_no heap number
/// \return record, NULL if not found
const rec_t* page_find_rec_with_heap_no(const page_t* page, ulint heap_no);

/// \brief Determine if a record is so big that it needs to be stored externally.
/// \param [in] rec_size length of the record in bytes
/// \param [in] comp nonzero=compact format
/// \param [in] n_fields number of fields in the record; ignored if zip_size == 0
/// \param [in] zip_size compressed page size in bytes, or 0
/// \return FALSE if the entire record can be stored locally on the page
IB_INLINE ibool page_rec_needs_ext(ulint rec_size, ulint comp, ulint n_fields, ulint zip_size);

#ifdef IB_MATERIALIZE
#undef IB_INLINE
#define IB_INLINE IB_INLINE_ORIGINAL
#endif

#ifndef IB_DO_NOT_INLINE
	#include "page_page.inl"
#endif
