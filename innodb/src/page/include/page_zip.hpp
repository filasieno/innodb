// Copyright (c) 2005, 2009, Innobase Oy. All Rights Reserved.
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

/// \file page_zip.hpp
/// \brief Compressed page interface
/// \details Originally created by Marko Makela in June 2005
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#ifdef IB_MATERIALIZE
#undef IB_INLINE
#define IB_INLINE
#endif

#include "mtr_types.hpp"
#include "page_types.hpp"
#include "buf_types.hpp"
#include "dict_types.hpp"
#include "trx_types.hpp"
#include "mem_mem.hpp"



/** Number of bits needed for representing different compressed page sizes */
constinit ulint PAGE_ZIP_SSIZE_BITS = 3;

/** log2 of smallest compressed page size */
constinit ulint PAGE_ZIP_MIN_SIZE_SHIFT = 10;

/** Smallest compressed page size */
constinit ulint PAGE_ZIP_MIN_SIZE = (1 << PAGE_ZIP_MIN_SIZE_SHIFT);

/// \brief Determine the size of a compressed page in bytes.
/// \return size in bytes

IB_INLINE ulint page_zip_get_size(const page_zip_des_t* page_zip) __attribute__((nonnull, pure));
/// \brief Set the size of a compressed page in bytes.
/// \param [in,out] page_zip compressed page
/// \param [in] size size in bytes
IB_INLINE void page_zip_set_size(page_zip_des_t* page_zip, ulint size);

#ifndef IB_HOTBACKUP

/// \brief Determine if a record is so big that it needs to be stored externally.
/// \return FALSE if the entire record can be stored locally on the page
IB_INLINE ibool page_zip_rec_needs_ext(ulint rec_size, ulint comp, ulint n_fields, ulint zip_size) __attribute__((const));

/// \brief Determine the guaranteed free space on an empty page.
/// \return minimum payload size on the page
IB_INTERN ulint page_zip_empty_size(ulint n_fields, ulint zip_size) __attribute__((const));

#endif // !IB_HOTBACKUP

/// \brief Initialize a compressed page descriptor.
/// \param [in,out] page_zip compressed page descriptor
IB_INLINE void page_zip_des_init(page_zip_des_t* page_zip);

/// \brief Configure the zlib allocator to use the given memory heap.
/// \param [in,out] stream zlib stream
/// \param [in] heap memory heap to use
IB_INTERN void page_zip_set_alloc(void* stream, mem_heap_t* heap);

/// \brief Compress a page.
/// \param [in] page_zip size; out: data, n_blobs, m_start, m_end, m_nonempty
/// \param [in] page uncompressed page
/// \param [in] index index of the B-tree node
/// \param [in] mtr mini-transaction, or NULL
/// \return TRUE on success, FALSE on failure; page_zip will be left intact on failure.
IB_INTERN ibool page_zip_compress(page_zip_des_t* page_zip, const page_t* page, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull(1,2,3)));

/// \brief Decompress a page.
/// \param [in] page_zip data, ssize
/// \param [out] page_zip m_start, m_end, m_nonempty, n_blobs
/// \param [out] page uncompressed page, may be trashed
/// \param [in] all TRUE=decompress the whole page; FALSE=verify but do not copy some page header fields that should not change after page creation
/// \return TRUE on success, FALSE on failure
/// \details This function should tolerate errors on the compressed page. Instead of letting assertions fail, it will return FALSE if an inconsistency is detected.
IB_INTERN ibool page_zip_decompress(page_zip_des_t* page_zip, page_t* page, ibool all) __attribute__((nonnull(1,2)));

#ifdef IB_DEBUG

/// \brief Validate a compressed page descriptor.
/// \return TRUE if ok
IB_INLINE ibool page_zip_simple_validate(const page_zip_des_t* page_zip);

#endif // IB_DEBUG

#ifdef IB_ZIP_DEBUG

/// \brief Check that the compressed and decompressed pages match.
/// \param [in] page_zip compressed page
/// \param [in] page uncompressed page
/// \param [in] sloppy FALSE=strict, TRUE=ignore the MIN_REC_FLAG
/// \return TRUE if valid, FALSE if not
IB_INTERN ibool page_zip_validate_low(const page_zip_des_t* page_zip, const page_t* page, ibool sloppy) __attribute__((nonnull));

/// \brief Check that the compressed and decompressed pages match.
/// \param [in] page_zip compressed page
/// \param [in] page uncompressed page
/// \return TRUE if valid, FALSE if not
IB_INTERN ibool page_zip_validate(const page_zip_des_t* page_zip, const page_t* page) __attribute__((nonnull));

#endif // IB_ZIP_DEBUG

/// \brief Determine how big record can be inserted without recompressing the page.
/// \return a positive number indicating the maximum size of a record whose insertion is guaranteed to succeed, or zero or negative
IB_INLINE lint page_zip_max_ins_size(const page_zip_des_t* page_zip, ibool is_clust) __attribute__((nonnull, pure));

/// \brief Determine if enough space is available in the modification log.
/// \return TRUE if page_zip_write_rec() will succeed
IB_INLINE ibool page_zip_available(const page_zip_des_t* page_zip, ibool is_clust, ulint length, ulint create) __attribute__((nonnull, pure));

/// \brief Write data to the uncompressed header portion of a page.
/// \details The data must already have been written to the uncompressed page.
/// \param [in,out] page_zip compressed page
/// \param [in] str address on the uncompressed page
/// \param [in] length length of the data
/// \param [in] mtr mini-transaction, or NULL
IB_INLINE void page_zip_write_header(page_zip_des_t* page_zip, const byte* str, ulint length, mtr_t* mtr) __attribute__((nonnull(1,2)));

/// \brief Write an entire record on the compressed page.
/// \details The data must already have been written to the uncompressed page.
/// \param [in,out] page_zip compressed page
/// \param [in] rec record being written
/// \param [in] index the index the record belongs to
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] create nonzero=insert, zero=update
IB_INTERN void page_zip_write_rec(page_zip_des_t* page_zip, const byte* rec, dict_index_t* index, const ulint* offsets, ulint create) __attribute__((nonnull));

/// \brief Parses a log record of writing a BLOB pointer of a record.
/// \param [in] ptr redo log buffer
/// \param [in] end_ptr redo log buffer end
/// \param [in,out] page uncompressed page
/// \param [in,out] page_zip compressed page
/// \return end of log record or NULL
IB_INTERN byte* page_zip_parse_write_blob_ptr(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip);

/// \brief Write a BLOB pointer of a record on the leaf page of a clustered index.
/// \details The information must already have been updated on the uncompressed page.
/// \param [in,out] page_zip compressed page
/// \param [in,out] rec record whose data is being written
/// \param [in] index index of the page
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] n column index
/// \param [in] mtr mini-transaction handle, or NULL if no logging is needed
IB_INTERN void page_zip_write_blob_ptr(page_zip_des_t* page_zip, const byte* rec, dict_index_t* index, const ulint* offsets, ulint n, mtr_t* mtr) __attribute__((nonnull(1,2,3,4)));

/// \brief Parses a log record of writing the node pointer of a record.
/// \param [in] ptr redo log buffer
/// \param [in] end_ptr redo log buffer end
/// \param [in,out] page uncompressed page
/// \param [in,out] page_zip compressed page
/// \return end of log record or NULL
IB_INTERN byte* page_zip_parse_write_node_ptr(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip);

/// \brief Write the node pointer of a record on a non-leaf compressed page.
/// \param [in,out] page_zip compressed page
/// \param [in,out] rec record
/// \param [in] size data size of rec
/// \param [in] ptr node pointer
/// \param [in] mtr mini-transaction, or NULL
IB_INTERN void page_zip_write_node_ptr(page_zip_des_t* page_zip, byte* rec, ulint size, ulint ptr, mtr_t* mtr) __attribute__((nonnull(1,2)));

/// \brief Write the trx_id and roll_ptr of a record on a B-tree leaf node page.
/// \param [in,out] page_zip compressed page
/// \param [in,out] rec record
/// \param [in] offsets rec_get_offsets(rec, index)
/// \param [in] trx_id_col column number of TRX_ID in rec
/// \param [in] trx_id transaction identifier
/// \param [in] roll_ptr roll_ptr
IB_INTERN void page_zip_write_trx_id_and_roll_ptr(page_zip_des_t* page_zip, byte* rec, const ulint* offsets, ulint trx_id_col, trx_id_t trx_id, roll_ptr_t roll_ptr) __attribute__((nonnull));

/// \brief Write the "deleted" flag of a record on a compressed page.
/// \details The flag must already have been written on the uncompressed page.
/// \param [in,out] page_zip compressed page
/// \param [in] rec record on the uncompressed page
/// \param [in] flag the deleted flag (nonzero=TRUE)
IB_INTERN void page_zip_rec_set_deleted(page_zip_des_t* page_zip, const byte* rec, ulint flag) __attribute__((nonnull));

/// \brief Write the "owned" flag of a record on a compressed page.
/// \details The n_owned field must already have been written on the uncompressed page.
/// \param [in,out] page_zip compressed page
/// \param [in] rec record on the uncompressed page
/// \param [in] flag the owned flag (nonzero=TRUE)
IB_INTERN void page_zip_rec_set_owned(page_zip_des_t* page_zip, const byte* rec, ulint flag) __attribute__((nonnull));

/// \brief Insert a record to the dense page directory.
/// \param [in,out] page_zip compressed page
/// \param [in] prev_rec record after which to insert
/// \param [in] free_rec record from which rec was allocated, or NULL
/// \param [in] rec record to insert
IB_INTERN void page_zip_dir_insert(page_zip_des_t* page_zip, const byte* prev_rec, const byte* free_rec, byte* rec);

/// \brief Shift the dense page directory and the array of BLOB pointers when a record is deleted.
/// \param [in,out] page_zip compressed page
/// \param [in] rec deleted record
/// \param [in] index index of rec
/// \param [in] offsets rec_get_offsets(rec)
/// \param [in] free previous start of the free list
IB_INTERN void page_zip_dir_delete(page_zip_des_t* page_zip, byte* rec, dict_index_t* index, const ulint* offsets, const byte* free) __attribute__((nonnull(1,2,3,4)));

/// \brief Add a slot to the dense page directory.
/// \param [in,out] page_zip compressed page
/// \param [in] is_clustered nonzero for clustered index, zero for others
IB_INTERN void page_zip_dir_add_slot(page_zip_des_t* page_zip, ulint is_clustered) __attribute__((nonnull));

/// \brief Parses a log record of writing to the header of a page.
/// \param [in] ptr redo log buffer
/// \param [in] end_ptr redo log buffer end
/// \param [in,out] page uncompressed page
/// \param [in,out] page_zip compressed page
/// \return end of log record or NULL
IB_INTERN byte* page_zip_parse_write_header(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip);

/// \brief Write data to the uncompressed header portion of a page.
/// \details The data must already have been written to the uncompressed page. However, the data portion of the uncompressed page may differ from the compressed page when a record is being inserted in page_cur_insert_rec_low().
/// \param [in,out] page_zip compressed page
/// \param [in] str address on the uncompressed page
/// \param [in] length length of the data
/// \param [in] mtr mini-transaction, or NULL
IB_INLINE void page_zip_write_header(page_zip_des_t* page_zip, const byte* str, ulint length, mtr_t* mtr) __attribute__((nonnull(1,2)));

#ifndef IB_HOTBACKUP
/// \brief Write a log record of writing to the uncompressed header portion of a page.
/// \param [in] data data on the uncompressed page
/// \param [in] length length of the data
/// \param [in] mtr mini-transaction
IB_INTERN void page_zip_write_header_log(const byte* data, ulint length, mtr_t* mtr);
#endif

/// \brief Reorganize and compress a page.
/// \param [in,out] block page with compressed page; on the compressed page, in: size; out: data, n_blobs, m_start, m_end, m_nonempty
/// \param [in] index index of the B-tree node
/// \param [in] mtr mini-transaction
/// \return TRUE on success, FALSE on failure; page_zip will be left intact on failure, but page will be overwritten.
/// \details This is a low-level operation for compressed pages, to be used when page_zip_compress() fails. On success, a redo log entry MLOG_ZIP_PAGE_COMPRESS will be written. The function btr_page_reorganize() should be preferred whenever possible. IMPORTANT: if page_zip_reorganize() is invoked on a leaf page of a non-clustered index, the caller must update the insert buffer free bits in the same mini-transaction in such a way that the modification will be redo-logged.
IB_INTERN ibool page_zip_reorganize(buf_block_t* block, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull));


#ifndef IB_HOTBACKUP

/// \brief Copy the records of a page byte for byte.
/// \details Do not copy the page header or trailer, except those B-tree header fields that are directly related to the storage of records. Also copy PAGE_MAX_TRX_ID. NOTE: The caller must update the lock table and the adaptive hash index.
/// \param [out] page_zip copy of src_zip (n_blobs, m_start, m_end, m_nonempty, data[0..size-1])
/// \param [out] page copy of src
/// \param [in] src_zip compressed page
/// \param [in] src page
/// \param [in] index index of the B-tree
/// \param [in] mtr mini-transaction
IB_INTERN void page_zip_copy_recs(page_zip_des_t* page_zip, page_t* page, const page_zip_des_t* src_zip, const page_t* src, dict_index_t* index, mtr_t* mtr) __attribute__((nonnull(1,2,3,4)));

#endif // !IB_HOTBACKUP 


/// \brief Parses a log record of compressing an index page.
/// \param [in] ptr buffer
/// \param [in] end_ptr buffer end
/// \param [out] page uncompressed page
/// \param [out] page_zip compressed page
/// \return end of log record or NULL
IB_INTERN byte* page_zip_parse_compress(byte* ptr, byte* end_ptr, page_t* page, page_zip_des_t* page_zip) __attribute__((nonnull(1,2)));

/// \brief Calculate the compressed page checksum.
/// \param [in] data compressed page
/// \param [in] size size of compressed page
/// \return page checksum
IB_INTERN ulint page_zip_calc_checksum(const void* data, ulint size) __attribute__((nonnull));

/// \brief Reset variables.
IB_INTERN void page_zip_var_init(void);



#ifndef IB_HOTBACKUP

/** Check if a pointer to an uncompressed page matches a compressed page.
@param ptr	pointer to an uncompressed page frame
@param page_zip	compressed page descriptor
@return		TRUE if ptr and page_zip refer to the same block */
#define PAGE_ZIP_MATCH(ptr, page_zip) (buf_frame_get_page_zip(ptr) == (page_zip))

#else // !IB_HOTBACKUP

/** Check if a pointer to an uncompressed page matches a compressed page.
@param ptr	pointer to an uncompressed page frame
@param page_zip	compressed page descriptor
@return		TRUE if ptr and page_zip refer to the same block */
#define PAGE_ZIP_MATCH(ptr, page_zip) (page_align(ptr) + IB_PAGE_SIZE == (page_zip)->data)

#endif // !IB_HOTBACKUP

#ifdef IB_MATERIALIZE
	#undef IB_INLINE
	#define IB_INLINE IB_INLINE_ORIGINAL
#endif // IB_MATERIALIZE

#ifndef IB_DO_NOT_INLINE
	#include "page_zip.inl"
#endif
