// Copyright (c) 1994, 2010, Innobase Oy. All Rights Reserved.
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

/// \file btr_cur.hpp
/// \brief The index tree cursor
/// \details Originally created by Heikki Tuuri in 10/16/1994
/// \author Fabio N. Filasieno
/// \date 21/10/2025

#pragma once

// -----------------------------------------------------------------------------------------
// includes
// -----------------------------------------------------------------------------------------

#include "univ.i"
#include "dict_dict.hpp"
#include "page_cur.hpp"
#include "btr_types.hpp"

// -----------------------------------------------------------------------------------------
// macro constants
// -----------------------------------------------------------------------------------------

/* Mode flags for btr_cur operations; these can be ORed */
constinit ulint BTR_NO_UNDO_LOG_FLAG = 1;
constinit ulint BTR_NO_LOCKING_FLAG = 2;
constinit ulint BTR_KEEP_SYS_FLAG = 4;

#ifndef IB_HOTBACKUP
#include "que_types.hpp"
#include "row_types.hpp"
#include "ha_ha.hpp"

#define BTR_CUR_ADAPT
#define BTR_CUR_HASH_ADAPT

#ifdef IB_DEBUG
/*********************************************************//**
Returns the page cursor component of a tree cursor.
@return    pointer to page cursor component */
IB_INLINE
page_cur_t*
btr_cur_get_page_cur(
/*=================*/
    const btr_cur_t*    cursor);/*!< in: tree cursor */
#else /* IB_DEBUG */
# define btr_cur_get_page_cur(cursor) (&(cursor)->page_cur)
#endif /* IB_DEBUG */
/// \brief Returns the buffer block on which the tree cursor is positioned.
/// \return pointer to buffer block
IB_INLINE buf_block_t* btr_cur_get_block(btr_cur_t* cursor);
/// \brief Returns the record pointer of a tree cursor.
/// \return pointer to record
IB_INLINE rec_t* btr_cur_get_rec(btr_cur_t* cursor);
/// \brief Returns the compressed page on which the tree cursor is positioned.
/// \return pointer to compressed page, or NULL if the page is not compressed
IB_INLINE page_zip_des_t* btr_cur_get_page_zip(btr_cur_t* cursor);
/// \brief Invalidates a tree cursor by setting record pointer to NULL.
/// \param [in] cursor tree cursor
IB_INLINE void btr_cur_invalidate(btr_cur_t* cursor);
/// \brief Returns the page of a tree cursor.
/// \return pointer to page
IB_INLINE page_t* btr_cur_get_page(btr_cur_t* cursor);
/// \brief Returns the index of a cursor.
/// \return index
IB_INLINE dict_index_t* btr_cur_get_index(btr_cur_t* cursor);
/// \brief Positions a tree cursor at a given record.
/// \param [in] index index
/// \param [in] rec record in tree
/// \param [in] block buffer block of rec
/// \param [in] cursor cursor
IB_INLINE void btr_cur_position(dict_index_t* index, rec_t* rec, buf_block_t* block, btr_cur_t* cursor);

// -----------------------------------------------------------------------------------------
// function declarations
// -----------------------------------------------------------------------------------------

/// \brief Searches an index tree and positions a tree cursor on a given level.
/// \details NOTE: n_fields_cmp in tuple must be set so that it cannot be compared to node pointer page number fields on the upper levels of the tree! Note that if mode is PAGE_CUR_LE, which is used in inserts, then cursor->up_match and cursor->low_match both will have sensible values. If mode is PAGE_CUR_GE, then up_match will a have a sensible value.
/// \param [in] index index
/// \param [in] level the tree level of search
/// \param [in] tuple data tuple; NOTE: n_fields_cmp in tuple must be set so that it cannot get compared to the node ptr page number field!
/// \param [in] mode PAGE_CUR_L, ...; NOTE that if the search is made using a unique prefix of a record, mode should be PAGE_CUR_LE, not PAGE_CUR_GE, as the latter may end up on the previous page of the record! Inserts should always be made using PAGE_CUR_LE to search the position!
/// \param [in] latch_mode BTR_SEARCH_LEAF, ..., ORed with BTR_INSERT and BTR_ESTIMATE; cursor->left_block is used to store a pointer to the left neighbor page, in the cases BTR_SEARCH_PREV and BTR_MODIFY_PREV; NOTE that if has_search_latch is != 0, we maybe do not have a latch set on the cursor page, we assume the caller uses his search latch to protect the record!
/// \param [in,out] cursor tree cursor; the cursor page is s- or x-latched, but see also above!
/// \param [in] has_search_latch latch mode the caller currently has on btr_search_latch: RW_S_LATCH, or 0
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_cur_search_to_nth_level(dict_index_t* index, ulint level, const dtuple_t* tuple, ulint mode, ulint latch_mode, btr_cur_t* cursor, ulint has_search_latch, const char* file, ulint line, mtr_t* mtr);
/// \brief Opens a cursor at either end of an index.
/// \param [in] from_left TRUE if open to the low end, FALSE if to the high end
/// \param [in] index index
/// \param [in] latch_mode latch mode
/// \param [in] cursor cursor
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_cur_open_at_index_side_func(ibool from_left, dict_index_t* index, ulint latch_mode, btr_cur_t* cursor, const char* file, ulint line, mtr_t* mtr);
#define btr_cur_open_at_index_side(f,i,l,c,m)                \
	btr_cur_open_at_index_side_func(f,i,l,c,__FILE__,__LINE__,m)
/// \brief Positions a cursor at a randomly chosen position within a B-tree.
/// \param [in] index index
/// \param [in] latch_mode BTR_SEARCH_LEAF, ...
/// \param [in,out] cursor B-tree cursor
/// \param [in] file file name
/// \param [in] line line where called
/// \param [in] mtr mtr
IB_INTERN void btr_cur_open_at_rnd_pos_func(dict_index_t* index, ulint latch_mode, btr_cur_t* cursor, const char* file, ulint line, mtr_t* mtr);
#define btr_cur_open_at_rnd_pos(i,l,c,m)                \
	btr_cur_open_at_rnd_pos_func(i,l,c,__FILE__,__LINE__,m)
/// \brief Tries to perform an insert to a page in an index tree, next to cursor.
/// \details It is assumed that mtr holds an x-latch on the page. The operation does not succeed if there is too little space on the page. If there is just one record on the page, the insert will always succeed; this is to prevent trying to split a page with just one record.
/// \param [in] flags undo logging and locking flags: if not zero, the parameters index and thr should be specified
/// \param [in] cursor cursor on page after which to insert; cursor stays valid
/// \param [in,out] entry entry to insert
/// \param [out] rec pointer to inserted record if succeed
/// \param [out] big_rec big rec vector whose fields have to be stored externally by the caller, or NULL
/// \param [in] n_ext number of externally stored columns
/// \param [in] thr query thread or NULL
/// \param [in] mtr mtr; if this function returns DB_SUCCESS on a leaf page of a secondary index in a compressed tablespace, the mtr must be committed before latching any further pages
/// \return DB_SUCCESS, DB_WAIT_LOCK, DB_FAIL, or error number
IB_INTERN ulint btr_cur_optimistic_insert(ulint flags, btr_cur_t* cursor, dtuple_t* entry, rec_t** rec, big_rec_t** big_rec, ulint n_ext, que_thr_t* thr, mtr_t* mtr);
/// \brief Performs an insert on a page of an index tree.
/// \details It is assumed that mtr holds an x-latch on the tree and on the cursor page. If the insert is made on the leaf level, to avoid deadlocks, mtr must also own x-latches to brothers of page, if those brothers exist.
/// \param [in] flags undo logging and locking flags: if not zero, the parameter thr should be specified; if no undo logging is specified, then the caller must have reserved enough free extents in the file space so that the insertion will certainly succeed
/// \param [in] cursor cursor after which to insert; cursor stays valid
/// \param [in,out] entry entry to insert
/// \param [out] rec pointer to inserted record if succeed
/// \param [out] big_rec big rec vector whose fields have to be stored externally by the caller, or NULL
/// \param [in] n_ext number of externally stored columns
/// \param [in] thr query thread or NULL
/// \param [in] mtr mtr
/// \return DB_SUCCESS or error number
IB_INTERN ulint btr_cur_pessimistic_insert(ulint flags, btr_cur_t* cursor, dtuple_t* entry, rec_t** rec, big_rec_t** big_rec, ulint n_ext, que_thr_t* thr, mtr_t* mtr);
/// \brief Updates a record when the update causes no size changes in its fields.
/// \param [in] flags undo logging and locking flags
/// \param [in] cursor cursor on the record to update; cursor stays valid and positioned on the same record
/// \param [in] update update vector
/// \param [in] cmpl_info compiler info on secondary index updates
/// \param [in] thr query thread
/// \param [in] mtr mtr; must be committed before latching any further pages
/// \return DB_SUCCESS or error number
IB_INTERN ulint btr_cur_update_in_place(ulint flags, btr_cur_t* cursor, const upd_t* update, ulint cmpl_info, que_thr_t* thr, mtr_t* mtr);
/// \brief Tries to update a record on a page in an index tree.
/// \details It is assumed that mtr holds an x-latch on the page. The operation does not succeed if there is too little space on the page or if the update would result in too empty a page, so that tree compression is recommended.
/// \param [in] flags undo logging and locking flags
/// \param [in] cursor cursor on the record to update; cursor stays valid and positioned on the same record
/// \param [in] update update vector; this must also contain trx id and roll ptr fields
/// \param [in] cmpl_info compiler info on secondary index updates
/// \param [in] thr query thread
/// \param [in] mtr mtr; must be committed before latching any further pages
/// \return DB_SUCCESS, or DB_OVERFLOW if the updated record does not fit, DB_UNDERFLOW if the page would become too empty, or DB_ZIP_OVERFLOW if there is not enough space left on the compressed page
IB_INTERN ulint btr_cur_optimistic_update(ulint flags, btr_cur_t* cursor, const upd_t* update, ulint cmpl_info, que_thr_t* thr, mtr_t* mtr);
/// \brief Performs an update of a record on a page of a tree.
/// \details It is assumed that mtr holds an x-latch on the tree and on the cursor page. If the update is made on the leaf level, to avoid deadlocks, mtr must also own x-latches to brothers of page, if those brothers exist.
/// \param [in] flags undo logging, locking, and rollback flags
/// \param [in] cursor cursor on the record to update
/// \param [in,out] heap pointer to memory heap, or NULL
/// \param [out] big_rec big rec vector whose fields have to be stored externally by the caller, or NULL
/// \param [in] update update vector; this is allowed also contain trx id and roll ptr fields, but the values in update vector have no effect
/// \param [in] cmpl_info compiler info on secondary index updates
/// \param [in] thr query thread
/// \param [in] mtr mtr; must be committed before latching any further pages
/// \return DB_SUCCESS or error code
IB_INTERN ulint btr_cur_pessimistic_update(ulint flags, btr_cur_t* cursor, mem_heap_t** heap, big_rec_t** big_rec, const upd_t* update, ulint cmpl_info, que_thr_t* thr, mtr_t* mtr);
/// \brief Marks a clustered index record deleted.
/// \details Writes an undo log record to undo log on this delete marking. Writes in the trx id field the id of the deleting transaction, and in the roll ptr field pointer to the undo log record created.
/// \param [in] flags undo logging and locking flags
/// \param [in] cursor cursor
/// \param [in] val value to set
/// \param [in] thr query thread
/// \param [in] mtr mtr
/// \return DB_SUCCESS, DB_LOCK_WAIT, or error number
IB_INTERN ulint btr_cur_del_mark_set_clust_rec(ulint flags, btr_cur_t* cursor, ibool val, que_thr_t* thr, mtr_t* mtr);
/// \brief Sets a secondary index record delete mark to TRUE or FALSE.
/// \param [in] flags locking flag
/// \param [in] cursor cursor
/// \param [in] val value to set
/// \param [in] thr query thread
/// \param [in] mtr mtr
/// \return DB_SUCCESS, DB_LOCK_WAIT, or error number
IB_INTERN ulint btr_cur_del_mark_set_sec_rec(ulint flags, btr_cur_t* cursor, ibool val, que_thr_t* thr, mtr_t* mtr);
/// \brief Clear a secondary index record's delete mark.
/// \details This function is only used by the insert buffer insert merge mechanism.
/// \param [in,out] rec record to delete unmark
/// \param [in,out] page_zip compressed page corresponding to rec, or NULL when the tablespace is uncompressed
/// \param [in] mtr mtr
IB_INTERN void btr_cur_del_unmark_for_ibuf(rec_t* rec, page_zip_des_t* page_zip, mtr_t* mtr);
/// \brief Tries to compress a page of the tree if it seems useful.
/// \details It is assumed that mtr holds an x-latch on the tree and on the cursor page. To avoid deadlocks, mtr must also own x-latches to brothers of page, if those brothers exist. NOTE: it is assumed that the caller has reserved enough free extents so that the compression will always succeed if done!
/// \param [in] cursor cursor on the page to compress; cursor does not stay valid if compression occurs
/// \param [in] mtr mtr
/// \return TRUE if compression occurred
IB_INTERN ibool btr_cur_compress_if_useful(btr_cur_t* cursor, mtr_t* mtr);
/// \brief Removes the record on which the tree cursor is positioned.
/// \details It is assumed that the mtr has an x-latch on the page where the cursor is positioned, but no latch on the whole tree.
/// \param [in] cursor cursor on the record to delete; cursor stays valid: if deletion succeeds, on function exit it points to the successor of the deleted record
/// \param [in] mtr mtr; if this function returns TRUE on a leaf page of a secondary index, the mtr must be committed before latching any further pages
/// \return TRUE if success, i.e., the page did not become too empty
IB_INTERN ibool btr_cur_optimistic_delete(btr_cur_t* cursor, mtr_t* mtr);
/// \brief Removes the record on which the tree cursor is positioned.
/// \details Tries to compress the page if its fillfactor drops below a threshold or if it is the only page on the level. It is assumed that mtr holds an x-latch on the tree and on the cursor page. To avoid deadlocks, mtr must also own x-latches to brothers of page, if those brothers exist.
/// \param [out] err DB_SUCCESS or DB_OUT_OF_FILE_SPACE; the latter may occur because we may have to update node pointers on upper levels, and in the case of variable length keys these may actually grow in size
/// \param [in] has_reserved_extents TRUE if the caller has already reserved enough free extents so that he knows that the operation will succeed
/// \param [in] cursor cursor on the record to delete; if compression does not occur, the cursor stays valid: it points to successor of deleted record on function exit
/// \param [in] rb_ctx rollback context
/// \param [in] mtr mtr
/// \return TRUE if compression occurred
IB_INTERN ibool btr_cur_pessimistic_delete(ulint* err, ibool has_reserved_extents, btr_cur_t* cursor, enum trx_rb_ctx rb_ctx, mtr_t* mtr);
#endif /* !IB_HOTBACKUP */
/// \brief Parses a redo log record of updating a record in-place.
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in,out] page page or NULL
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] index index corresponding to page
/// \return end of log record or NULL
IB_INTERN byte* btr_cur_parse_update_in_place(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip, dict_index_t* index);
/// \brief Parses the redo log record for delete marking or unmarking of a clustered index record.
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in,out] page page or NULL
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] index index corresponding to page
/// \return end of log record or NULL
IB_INTERN byte* btr_cur_parse_del_mark_set_clust_rec(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip, dict_index_t* index);
/// \brief Parses the redo log record for delete marking or unmarking of a secondary index record.
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [in,out] page page or NULL
/// \param [in,out] page_zip compressed page, or NULL
/// \return end of log record or NULL
IB_INTERN byte* btr_cur_parse_del_mark_set_sec_rec(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip);
#ifndef IB_HOTBACKUP
/// \brief Estimates the number of rows in a given index range.
/// \param [in] index index
/// \param [in] tuple1 range start, may also be empty tuple
/// \param [in] mode1 search mode for range start
/// \param [in] tuple2 range end, may also be empty tuple
/// \param [in] mode2 search mode for range end
/// \return estimated number of rows
IB_INTERN ib_int64_t btr_estimate_n_rows_in_range(dict_index_t* index, const dtuple_t* tuple1, ulint mode1, const dtuple_t* tuple2, ulint mode2);
/// \brief Estimates the number of different key values in a given index.
/// \details For each n-column prefix of the index where n <= dict_index_get_n_unique(index). The estimates are stored in the array index->stat_n_diff_key_vals.
/// \param [in] index index
IB_INTERN void btr_estimate_number_of_different_key_vals(dict_index_t* index);
/// \brief Marks not updated extern fields as not-owned by this record.
/// \details The ownership is transferred to the updated record which is inserted elsewhere in the index tree. In purge only the owner of externally stored field is allowed to free the field.
/// \param [in,out] page_zip compressed page whose uncompressed part will be updated, or NULL
/// \param [in,out] rec record in a clustered index
/// \param [in] index index of the page
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] update update vector
/// \param [in] mtr mtr, or NULL if not logged
IB_INTERN void btr_cur_mark_extern_inherited_fields(page_zip_des_t* page_zip, rec_t* rec, dict_index_t* index, const ulint* offsets, const upd_t* update, mtr_t* mtr);
/// \brief The complement of the previous function: in an update entry may inherit some externally stored fields from a record.
/// \details We must mark them as inherited in entry, so that they are not freed in a rollback.
/// \param [in,out] entry updated entry to be inserted to clustered index
/// \param [in] update update vector
IB_INTERN void btr_cur_mark_dtuple_inherited_extern(dtuple_t* entry, const upd_t* update);
/// \brief Marks all extern fields in a dtuple as owned by the record.
/// \param [in,out] entry clustered index entry
IB_INTERN void btr_cur_unmark_dtuple_extern_fields(dtuple_t* entry);
/// \brief Stores the fields in big_rec_vec to the tablespace and puts pointers to them in rec.
/// \details The extern flags in rec will have to be set beforehand. The fields are stored on pages allocated from leaf node file segment of the index tree.
/// \param [in] index index of rec; the index tree MUST be X-latched
/// \param [in,out] rec_block block containing rec
/// \param [in] rec record
/// \param [in] offsets rec_get_offsets(rec, index); the "external storage" flags in offsets will not correspond to rec when this function returns
/// \param [in] big_rec_vec vector containing fields to be stored externally
/// \param [in] local_mtr mtr containing the latch to rec and to the tree
/// \return DB_SUCCESS or error
IB_INTERN ulint btr_store_big_rec_extern_fields(dict_index_t* index, buf_block_t* rec_block, rec_t* rec, const ulint* offsets, big_rec_t* big_rec_vec, mtr_t* local_mtr);
/// \brief Frees the space in an externally stored field to the file space management.
/// \details If the field in data is owned the externally stored field, in a rollback we may have the additional condition that the field must not be inherited.
/// \param [in] index index of the data, the index tree MUST be X-latched; if the tree height is 1, then also the root page must be X-latched! (this is relevant in the case this function is called from purge where 'data' is located on an undo log page, not an index page)
/// \param [in,out] field_ref field reference
/// \param [in] rec record containing field_ref, for page_zip_write_blob_ptr(), or NULL
/// \param [in] offsets rec_get_offsets(rec, index), or NULL
/// \param [in] page_zip compressed page corresponding to rec, or NULL if rec == NULL
/// \param [in] i field number of field_ref; ignored if rec == NULL
/// \param [in] rb_ctx rollback context
/// \param [in] local_mtr mtr containing the latch to data an an X-latch to the index tree
IB_INTERN void btr_free_externally_stored_field(dict_index_t* index, byte* field_ref, const rec_t* rec, const ulint* offsets, page_zip_des_t* page_zip, ulint i, enum trx_rb_ctx rb_ctx, mtr_t* local_mtr);
/// \brief Copies the prefix of an externally stored field of a record.
/// \details The clustered index record must be protected by a lock or a page latch.
/// \param [out] buf the field, or a prefix of it
/// \param [in] len length of buf, in bytes
/// \param [in] zip_size nonzero=compressed BLOB page size, zero for uncompressed BLOBs
/// \param [in] data 'internally' stored part of the field containing also the reference to the external part; must be protected by a lock or a page latch
/// \param [in] local_len length of data, in bytes
/// \return the length of the copied field, or 0 if the column was being or has been deleted
IB_INTERN ulint btr_copy_externally_stored_field_prefix(byte* buf, ulint len, ulint zip_size, const byte* data, ulint local_len);
/// \brief Copies an externally stored field of a record to mem heap.
/// \param [in] rec record in a clustered index; must be protected by a lock or a page latch
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] zip_size nonzero=compressed BLOB page size, zero for uncompressed BLOBs
/// \param [in] no field number
/// \param [out] len length of the field
/// \param [in] heap mem heap
/// \return the field copied to heap
IB_INTERN byte* btr_rec_copy_externally_stored_field(const rec_t* rec, const ulint* offsets, ulint zip_size, ulint no, ulint* len, mem_heap_t* heap);
/// \brief Flags the data tuple fields that are marked as extern storage in the update vector.
/// \details We use this function to remember which fields we must mark as extern storage in a record inserted for an update.
/// \param [in,out] tuple data tuple
/// \param [in] update update vector
/// \param [in] heap memory heap
/// \return number of flagged external columns
IB_INTERN ulint btr_push_update_extern_fields(dtuple_t* tuple, const upd_t* update, mem_heap_t* heap) __attribute__((nonnull));
/// \brief Reset global configuration variables.
IB_INTERN void btr_cur_var_init(void);


/** If pessimistic delete fails because of lack of file space, there
is still a good change of success a little later.  Try this many
times. */
constinit ulint BTR_CUR_RETRY_DELETE_N_TIMES = 100;
/** If pessimistic delete fails because of lack of file space, there
is still a good change of success a little later.  Sleep this many
microseconds between retries. */
constinit ulint BTR_CUR_RETRY_SLEEP_TIME = 50000;

/** The reference in a field for which data is stored on a different page.
The reference is at the end of the 'locally' stored part of the field.
'Locally' means storage in the index record.
We store locally a long enough prefix of each column so that we can determine
the ordering parts of each index record without looking into the externally
stored part. */
/*-------------------------------------- @{ */
constinit ulint BTR_EXTERN_SPACE_ID = 0;
constinit ulint BTR_EXTERN_PAGE_NO = 4;
constinit ulint BTR_EXTERN_OFFSET = 8;
constinit ulint BTR_EXTERN_LEN = 12;
/*-------------------------------------- @} */
/* #define BTR_EXTERN_FIELD_REF_SIZE    20 // moved to btr0types.h */

/** The most significant bit of BTR_EXTERN_LEN (i.e., the most
significant bit of the byte at smallest address) is set to 1 if this
field does not 'own' the externally stored field; only the owner field
is allowed to free the field in purge! */
constinit ulint BTR_EXTERN_OWNER_FLAG = 128;
/** If the second most significant bit of BTR_EXTERN_LEN (i.e., the
second most significant bit of the byte at smallest address) is 1 then
it means that the externally stored field was inherited from an
earlier version of the row.  In rollback we are not allowed to free an
inherited external field. */
constinit ulint BTR_EXTERN_INHERITED_FLAG = 64;

/** Number of searches down the B-tree in btr_cur_search_to_nth_level(). */
extern ulint    btr_cur_n_non_sea;
/** Number of successful adaptive hash index lookups in
btr_cur_search_to_nth_level(). */
extern ulint    btr_cur_n_sea;
/** Old value of btr_cur_n_non_sea.  Copied by
srv_refresh_innodb_monitor_stats().  Referenced by
srv_printf_innodb_monitor(). */
extern ulint    btr_cur_n_non_sea_old;
/** Old value of btr_cur_n_sea.  Copied by
srv_refresh_innodb_monitor_stats().  Referenced by
srv_printf_innodb_monitor(). */
extern ulint    btr_cur_n_sea_old;
#endif /* !IB_HOTBACKUP */

#ifndef IB_DO_NOT_INLINE
#include "btr0cur.inl"
#endif
