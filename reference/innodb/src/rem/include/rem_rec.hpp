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

/// \file rem_rec.hpp

/// \brief Record manager
/// \details Originally created by Heikki Tuuri on 5/30/1994
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "data_data.hpp"
#include "rem_types.hpp"
#include "mtr_types.hpp"
#include "page_types.hpp"

constinit ulint REC_INFO_MIN_REC_FLAG = 0x10UL;
constinit ulint REC_INFO_DELETED_FLAG = 0x20UL;
constinit ulint REC_N_OLD_EXTRA_BYTES = 6;
constinit ulint REC_N_NEW_EXTRA_BYTES = 5;
constinit ulint REC_STATUS_ORDINARY = 0;
constinit ulint REC_STATUS_NODE_PTR = 1;
constinit ulint REC_STATUS_INFIMUM = 2;
constinit ulint REC_STATUS_SUPREMUM = 3;
constinit ulint REC_NEW_HEAP_NO = 4;
constinit ulint REC_HEAP_NO_SHIFT = 3;
constinit ulint REC_NODE_PTR_SIZE = 4;

#ifdef IB_DEBUG
constinit ulint REC_OFFS_HEADER_SIZE = 4;
#else /* IB_DEBUG */
constinit ulint REC_OFFS_HEADER_SIZE = 2;
#endif /* IB_DEBUG */

constinit ulint REC_OFFS_NORMAL_SIZE = 100;
constinit ulint REC_OFFS_SMALL_SIZE = 10;

/// \brief The following function is used to get the pointer of the next chained record on the same page.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return pointer to the next chained record, or NULL if none
IB_INLINE const rec_t* rec_get_next_ptr_const(const rec_t* rec, ulint comp);

/// \brief Gets the pointer of the next chained record on the same page.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return pointer to the next chained record, or NULL if none
IB_INLINE rec_t* rec_get_next_ptr(rec_t* rec, ulint comp);
/// \brief Gets the offset of the next chained record on the same page.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return the page offset of the next chained record, or 0 if none
IB_INLINE ulint rec_get_next_offs(const rec_t* rec, ulint comp);

/// \brief Sets the next record offset field of an old-style record.
/// \param [in] rec old-style physical record
/// \param [in] next offset of the next record
IB_INLINE void rec_set_next_offs_old(rec_t* rec, ulint next);
/// \brief The following function is used to set the next record offset field of a new-style record.
/// \param [in,out] rec new-style physical record
/// \param [in] next offset of the next record
IB_INLINE void rec_set_next_offs_new(rec_t* rec, ulint next);

/// \brief Gets the number of fields in an old-style record.
/// \param [in] rec physical record
/// \return number of data fields
IB_INLINE ulint rec_get_n_fields_old(const rec_t* rec);
/// \brief Gets the number of fields in a record.
/// \param [in] rec physical record
/// \param [in] index record descriptor
/// \return number of data fields
IB_INLINE ulint rec_get_n_fields(const rec_t* rec, const dict_index_t* index);

/// \brief Gets the number of records owned by the previous directory record.
/// \param [in] rec old-style physical record
/// \return number of owned records
IB_INLINE ulint rec_get_n_owned_old(const rec_t* rec);
/// \brief Sets the number of owned records.
/// \param [in] rec old-style physical record
/// \param [in] n_owned the number of owned
IB_INLINE void rec_set_n_owned_old(rec_t* rec, ulint n_owned);

/// \brief Gets the number of records owned by the previous directory record.
/// \param [in] rec new-style physical record
/// \return number of owned records
IB_INLINE ulint rec_get_n_owned_new(const rec_t* rec);

/// \brief Sets the number of owned records.
/// \param [in,out] rec new-style physical record
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] n_owned the number of owned
IB_INLINE void rec_set_n_owned_new(rec_t* rec, page_zip_des_t* page_zip, ulint n_owned);
/// \brief Retrieves the info bits of a record.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return info bits
IB_INLINE ulint rec_get_info_bits(const rec_t* rec, ulint comp);

/// \brief Sets the info bits of a record.
/// \param [in] rec old-style physical record
/// \param [in] bits info bits
IB_INLINE void rec_set_info_bits_old(rec_t* rec, ulint bits);

/// \brief Sets the info bits of a record.
/// \param [in,out] rec new-style physical record
/// \param [in] bits info bits
IB_INLINE void rec_set_info_bits_new(rec_t* rec, ulint bits);
/// \brief Retrieves the status bits of a new-style record.
/// \param [in] rec physical record
/// \return status bits
IB_INLINE ulint rec_get_status(const rec_t* rec);

/// \brief Sets the status bits of a new-style record.
/// \param [in,out] rec physical record
/// \param [in] bits status bits
IB_INLINE void rec_set_status(rec_t* rec, ulint bits);

/// \brief Retrieves the info and status bits of a record.
/// \details (Only compact records have status bits.)
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return info bits
IB_INLINE ulint rec_get_info_and_status_bits(const rec_t* rec, ulint comp);
/// \brief Sets the info and status bits of a record.
/// \details (Only compact records have status bits.)
/// \param [in,out] rec compact physical record
/// \param [in] bits info bits
IB_INLINE void rec_set_info_and_status_bits(rec_t* rec, ulint bits);

/// \brief The following function tells if record is delete marked.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return nonzero if delete marked
IB_INLINE ulint rec_get_deleted_flag(const rec_t* rec, ulint comp);

/// \brief The following function is used to set the deleted bit.
/// \param [in] rec old-style physical record
/// \param [in] flag nonzero if delete marked
IB_INLINE void rec_set_deleted_flag_old(rec_t* rec, ulint flag);
/// \brief The following function is used to set the deleted bit.
/// \param [in,out] rec new-style physical record
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] flag nonzero if delete marked
IB_INLINE void rec_set_deleted_flag_new(rec_t* rec, page_zip_des_t* page_zip, ulint flag);

/// \brief The following function tells if a new-style record is a node pointer.
/// \param [in] rec physical record
/// \return TRUE if node pointer
IB_INLINE ibool rec_get_node_ptr_flag(const rec_t* rec);

/// \brief The following function is used to get the order number of an old-style record in the heap of the index page.
/// \param [in] rec physical record
/// \return heap order number
IB_INLINE ulint rec_get_heap_no_old(const rec_t* rec);
/// \brief The following function is used to set the heap number field in an old-style record.
/// \param [in] rec physical record
/// \param [in] heap_no the heap number
IB_INLINE void rec_set_heap_no_old(rec_t* rec, ulint heap_no);

/// \brief The following function is used to get the order number of a new-style record in the heap of the index page.
/// \param [in] rec physical record
/// \return heap order number
IB_INLINE ulint rec_get_heap_no_new(const rec_t* rec);

/// \brief The following function is used to set the heap number field in a new-style record.
/// \param [in,out] rec physical record
/// \param [in] heap_no the heap number
IB_INLINE void rec_set_heap_no_new(rec_t* rec, ulint heap_no);
/// \brief The following function is used to test whether the data offsets in the record are stored in one-byte or two-byte format.
/// \param [in] rec physical record
/// \return TRUE if 1-byte form
IB_INLINE ibool rec_get_1byte_offs_flag(const rec_t* rec);

/// \brief Determine how many of the first n columns in a compact physical record are stored externally.
/// \param [in] rec compact physical record
/// \param [in] index record descriptor
/// \param [in] n number of columns to scan
/// \return number of externally stored columns
IB_INTERN ulint rec_get_n_extern_new(const rec_t* rec, dict_index_t* index, ulint n);

/// \brief The following function determines the offsets to each field in the record.
/// \details It can reuse a previously allocated array.
/// \param [in] rec physical record
/// \param [in] index record descriptor
/// \param [in,out] offsets array consisting of offsets[0] allocated elements, or an array from rec_get_offsets(), or NULL
/// \param [in] n_fields maximum number of initialized fields (ULINT_UNDEFINED if all fields)
/// \param [in,out] heap memory heap
/// \param [in] file file name where called
/// \param [in] line line number where called
/// \return the new offsets
IB_INTERN ulint* rec_get_offsets_func(const rec_t* rec, const dict_index_t* index, ulint* offsets, ulint n_fields, mem_heap_t** heap, const char* file, ulint line);

#define rec_get_offsets(rec,index,offsets,n,heap)	rec_get_offsets_func(rec,index,offsets,n,heap,__FILE__,__LINE__)

/// \brief Determines the offset to each field in a leaf-page record in ROW_FORMAT=COMPACT.
/// \details This is a special case of rec_init_offsets() and rec_get_offsets_func().
/// \param [in] rec physical record in ROW_FORMAT=COMPACT
/// \param [in] extra number of bytes to reserve between the record header and the data payload (usually REC_N_NEW_EXTRA_BYTES)
/// \param [in] index record descriptor
/// \param [in,out] offsets array of offsets; in: n=rec_offs_n_fields(offsets)
IB_INTERN void rec_init_offsets_comp_ordinary(const rec_t* rec, ulint extra, const dict_index_t* index, ulint* offsets);

/// \brief Determines the offsets to each field in the record.
/// \details It can reuse a previously allocated array.
/// \param [in] extra the extra bytes of a compact record in reverse order, excluding the fixed-size REC_N_NEW_EXTRA_BYTES
/// \param [in] index record descriptor
/// \param [in] node_ptr nonzero=node pointer, 0=leaf node
/// \param [in,out] offsets array consisting of offsets[0] allocated elements
IB_INTERN void rec_get_offsets_reverse(const byte* extra, const dict_index_t* index, ulint node_ptr, ulint* offsets);

/// \brief Validates offsets returned by rec_get_offsets().
/// \return TRUE if valid
IB_INLINE ibool rec_offs_validate(const rec_t* rec, const dict_index_t* index, const ulint* offsets);
#ifdef IB_DEBUG
/// \brief Updates debug data in offsets, in order to avoid bogus rec_offs_validate() failures.
/// \param [in] rec record
/// \param [in] index record descriptor
/// \param [in] offsets array returned by rec_get_offsets()
IB_INLINE void rec_offs_make_valid(const rec_t* rec, const dict_index_t* index, ulint* offsets);
#else
# define rec_offs_make_valid(rec, index, offsets) ((void) 0)
#endif /* IB_DEBUG */

/// \brief Gets the offset to the nth data field in an old-style record.
/// \return offset to the field
IB_INTERN ulint rec_get_nth_field_offs_old(const rec_t* rec, ulint n, ulint* len);
#define rec_get_nth_field_old(rec, n, len) ((rec) + rec_get_nth_field_offs_old(rec, n, len))

/// \brief Gets the physical size of an old-style field.
/// \details Also an SQL null may have a field of size > 0, if the data type is of a fixed size.
/// \return field size in bytes
IB_INLINE ulint rec_get_nth_field_size(const rec_t* rec, ulint n);

/// \brief Gets an offset to the nth data field in a record.
/// \return offset from the origin of rec
IB_INLINE ulint rec_get_nth_field_offs(const ulint* offsets, ulint n, ulint* len);

#define rec_get_nth_field(rec, offsets, n, len) ((rec) + rec_get_nth_field_offs(offsets, n, len))

/// \brief Determines if the offsets are for a record in the new compact format.
/// \return nonzero if compact format
IB_INLINE ulint rec_offs_comp(const ulint* offsets);

/// \brief Determines if the offsets are for a record containing externally stored columns.
/// \return nonzero if externally stored
IB_INLINE ulint rec_offs_any_extern(const ulint* offsets);

/// \brief Returns nonzero if the extern bit is set in nth field of rec.
/// \return nonzero if externally stored
IB_INLINE ulint rec_offs_nth_extern(const ulint* offsets, ulint n);

/// \brief Returns nonzero if the SQL NULL bit is set in nth field of rec.
/// \return nonzero if SQL NULL
IB_INLINE ulint rec_offs_nth_sql_null(const ulint* offsets, ulint n);

/// \brief Gets the physical size of a field.
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n nth field
/// \return length of field
IB_INLINE ulint rec_offs_nth_size(const ulint* offsets, ulint n);


/// \brief Returns the number of extern bits set in a record.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return number of externally stored fields
IB_INLINE ulint rec_offs_n_extern(const ulint* offsets);

/// \brief This is used to modify the value of an already existing field in a record.
/// \details The previous value must have exactly the same size as the new value. If len is IB_SQL_NULL then the field is treated as an SQL null. For records in ROW_FORMAT=COMPACT (new-style records), len must not be IB_SQL_NULL unless the field already is SQL null.
/// \param [in] rec record
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n index number of the field
/// \param [in] data pointer to the data if not SQL null
/// \param [in] len length of the data or IB_SQL_NULL
IB_INLINE void rec_set_nth_field(rec_t* rec, const ulint* offsets, ulint n, const void* data, ulint len);

/// \brief The following function returns the data size of an old-style physical record, that is the sum of field lengths.
/// \details SQL null fields are counted as length 0 fields. The value returned by the function is the distance from record origin to record end in bytes.
/// \param [in] rec physical record
/// \return size
IB_INLINE ulint rec_get_data_size_old(const rec_t* rec);

/// \brief The following function returns the number of allocated elements for an array of offsets.
/// \param [in] offsets array for rec_get_offsets()
/// \return number of elements
IB_INLINE ulint rec_offs_get_n_alloc(const ulint* offsets);

/// \brief The following function sets the number of allocated elements for an array of offsets.
/// \param [out] offsets array for rec_get_offsets(), must be allocated
/// \param [in] n_alloc number of elements
IB_INLINE void rec_offs_set_n_alloc(ulint* offsets, ulint n_alloc);
#define rec_offs_init(offsets) \
	rec_offs_set_n_alloc(offsets, (sizeof offsets) / sizeof *offsets)

	/// \brief The following function returns the number of fields in a record.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return number of fields
IB_INLINE ulint rec_offs_n_fields(const ulint* offsets);

/// \brief The following function returns the data size of a physical record, that is the sum of field lengths.
/// \details SQL null fields are counted as length 0 fields. The value returned by the function is the distance from record origin to record end in bytes.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return size
IB_INLINE ulint rec_offs_data_size(const ulint* offsets);

/// \brief Returns the total size of record minus data size of record.
/// \details The value returned by the function is the distance from record start to record origin in bytes.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return size
IB_INLINE ulint rec_offs_extra_size(const ulint* offsets);

/// \brief Returns the total size of a physical record.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return size
IB_INLINE ulint rec_offs_size(const ulint* offsets);

/// \brief Returns a pointer to the start of the record.
/// \param [in] rec pointer to record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return pointer to start
IB_INLINE byte* rec_get_start(rec_t* rec, const ulint* offsets);

/// \brief Returns a pointer to the end of the record.
/// \param [in] rec pointer to record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return pointer to end
IB_INLINE byte* rec_get_end(rec_t* rec, const ulint* offsets);

/// \brief Copies a physical record to a buffer.
/// \param [in] buf buffer
/// \param [in] rec physical record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return pointer to the origin of the copy
IB_INLINE rec_t* rec_copy(void* buf, const rec_t* rec, const ulint* offsets);

#ifndef IB_HOTBACKUP

/// \brief Copies the first n fields of a physical record to a new physical record in a buffer.
/// \param [in] rec physical record
/// \param [in] index record descriptor
/// \param [in] n_fields number of fields to copy
/// \param [in/out] buf memory buffer for the copied prefix, or NULL
/// \param [in/out] buf_size buffer size
/// \return own: copied record
IB_INTERN rec_t* rec_copy_prefix_to_buf(const rec_t* rec, const dict_index_t* index, ulint n_fields, byte** buf, ulint* buf_size);

/// \brief Folds a prefix of a physical record to a ulint.
/// \param [in] rec the physical record
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n_fields number of complete fields to fold
/// \param [in] n_bytes number of bytes to fold in an incomplete last field
/// \param [in] tree_id index tree id
/// \return the folded value
IB_INLINE ulint rec_fold(const rec_t* rec, const ulint* offsets, ulint n_fields, ulint n_bytes, dulint tree_id) __attribute__((pure));

#endif // !IB_HOTBACKUP

/// \brief Builds a ROW_FORMAT=COMPACT record out of a data tuple.
/// \param [in] rec origin of record
/// \param [in] extra number of bytes to reserve between the record header and the data payload (normally REC_N_NEW_EXTRA_BYTES)
/// \param [in] index record descriptor
/// \param [in] status status bits of the record
/// \param [in] fields array of data fields
/// \param [in] n_fields number of data fields
IB_INTERN void rec_convert_dtuple_to_rec_comp(rec_t* rec, ulint extra, const dict_index_t* index, ulint status, const dfield_t* fields, ulint n_fields);

/// \brief Builds a physical record out of a data tuple and stores it into the given buffer.
/// \param [in] buf start address of the physical record
/// \param [in] index record descriptor
/// \param [in] dtuple data tuple
/// \param [in] n_ext number of externally stored columns
/// \return pointer to the origin of physical record
IB_INTERN rec_t* rec_convert_dtuple_to_rec(byte* buf, const dict_index_t* index, const dtuple_t* dtuple, ulint n_ext);

/// \brief Returns the extra size of an old-style physical record if we know its data size and number of fields.
/// \param [in] data_size data size
/// \param [in] n_fields number of fields
/// \param [in] n_ext number of externally stored columns
/// \return extra size
IB_INLINE ulint rec_get_converted_extra_size(ulint data_size, ulint n_fields, ulint n_ext) __attribute__((const));


/// \brief Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
/// \param [in] index record descriptor; dict_table_is_comp() is assumed to hold, even if it does not
/// \param [in] fields array of data fields
/// \param [in] n_fields number of data fields
/// \param [out] extra extra size
/// \return total size
IB_INTERN ulint rec_get_converted_size_comp_prefix(const dict_index_t* index, const dfield_t* fields, ulint n_fields, ulint* extra);


/// \brief Determines the size of a data tuple in ROW_FORMAT=COMPACT.
/// \param [in] index record descriptor; dict_table_is_comp() is assumed to hold, even if it does not
/// \param [in] status status bits of the record
/// \param [in] fields array of data fields
/// \param [in] n_fields number of data fields
/// \param [out] extra extra size
/// \return total size
IB_INTERN ulint rec_get_converted_size_comp(const dict_index_t* index, ulint status, const dfield_t* fields, ulint n_fields, ulint* extra);

/// \brief The following function returns the size of a data tuple when converted to a physical record.
/// \param [in] index record descriptor
/// \param [in] dtuple data tuple
/// \param [in] n_ext number of externally stored columns
/// \return size
IB_INLINE ulint rec_get_converted_size(dict_index_t* index, const dtuple_t* dtuple, ulint n_ext);
#ifndef IB_HOTBACKUP

/// \brief Copies the first n fields of a physical record to a data tuple.
/// \details The fields are copied to the memory heap.
/// \param [out] tuple data tuple
/// \param [in] rec physical record
/// \param [in] index record descriptor
/// \param [in] n_fields number of fields to copy
/// \param [in] heap memory heap
IB_INTERN void rec_copy_prefix_to_dtuple(dtuple_t* tuple, const rec_t* rec, const dict_index_t* index, ulint n_fields, mem_heap_t* heap);
#endif /* !IB_HOTBACKUP */

/// \brief Validates the consistency of a physical record.
/// \param [in] rec physical record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return TRUE if ok
IB_INTERN ibool rec_validate(const rec_t* rec, const ulint* offsets);
	

/// \brief Prints an old-style physical record.
/// \param [in] state->stream stream where to print
/// \param [in] rec physical record
IB_INTERN void rec_print_old(innodb_state* state, const rec_t* rec);

#ifndef IB_HOTBACKUP

/// \brief Prints a physical record in ROW_FORMAT=COMPACT.
/// \details Ignores the record header.
/// \param [in] state->stream stream where to print
/// \param [in] rec physical record
/// \param [in] offsets array returned by rec_get_offsets()
IB_INTERN void rec_print_comp(innodb_state* state, const rec_t* rec, const ulint* offsets);


/// \brief Prints a physical record.
/// \param [in] state->stream stream where to print
/// \param [in] rec physical record
/// \param [in] offsets array returned by rec_get_offsets()
IB_INTERN void rec_print_new(innodb_state* state, const rec_t* rec, const ulint* offsets);

/// \brief Prints a physical record.
/// \param [in] state->stream stream where to print
/// \param [in] rec physical record
/// \param [in] index record descriptor
IB_INTERN void rec_print(innodb_state* state, const rec_t* rec, dict_index_t* index);

#endif // IB_HOTBACKUP

constinit ulint REC_INFO_BITS = 6;

// Maximum lengths for the data in a physical record if the offsets are given in one byte (resp. two byte) format. 
constinit ulint REC_1BYTE_OFFS_LIMIT = 0x7FUL;
constinit ulint REC_2BYTE_OFFS_LIMIT = 0x7FFFUL;

// The data size of record must be smaller than this because we reserve two upmost bits in a two byte offset for special purposes
constinit ulint REC_MAX_DATA_SIZE = 16 * 1024;

#ifndef IB_DO_NOT_INLINE
	#include "rem_rec.inl"
#endif
