// Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.
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

/// \file row_row.hpp
/// \brief General row routines
/// \details Originally created on 4/20/1996 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "data_data.hpp"
#include "dict_types.hpp"
#include "trx_types.hpp"
#include "que_types.hpp"
#include "mtr_mtr.hpp"
#include "rem_types.hpp"
#include "read_types.hpp"
#include "row_types.hpp"
#include "btr_types.hpp"

/// \brief Gets the offset of the trx id field, in bytes relative to the origin of a clustered index record.
/// \param [in] rec record
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \return offset of DATA_TRX_ID
IB_INTERN ulint row_get_trx_id_offset(const rec_t* rec, dict_index_t* index, const ulint* offsets);

/// \brief Reads the trx id field from a clustered index record.
/// \param [in] rec record
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \return value of the field
IB_INLINE trx_id_t row_get_rec_trx_id(const rec_t* rec, dict_index_t* index, const ulint* offsets);

/// \brief Reads the roll pointer field from a clustered index record.
/// \param [in] rec record
/// \param [in] index clustered index
/// \param [in] offsets rec_get_offsets(rec, index)
/// \return value of the field
IB_INLINE roll_ptr_t row_get_rec_roll_ptr(const rec_t* rec, dict_index_t* index, const ulint* offsets);

/// \brief When an insert or purge to a table is performed, this function builds the entry to be inserted into or purged from an index on the table.
/// \details unavailable and ext != NULL
/// \param [in] row row which should be inserted or purged
/// \param [in] ext externally stored column prefixes, or NULL
/// \param [in] index index on the table
/// \param [in] heap memory heap from which the memory for the index entry is allocated
/// \return index entry which should be inserted or purged, or NULL if the externally stored columns in the clustered index record are
IB_INTERN dtuple_t* row_build_index_entry(const dtuple_t* row, row_ext_t* ext, dict_index_t* index, mem_heap_t* heap);

/// \brief An inverse function to row_build_index_entry.
/// \details Builds a row from a record in a clustered index.
/// \param type ROW_COPY_POINTERS or ROW_COPY_DATA; the latter copies also the data fields to heap while the first only places pointers to data fields on the index page, and thus is more efficient
/// \param index clustered index
/// \param rec record in the clustered index; NOTE: in the case ROW_COPY_POINTERS the data fields in the row will point directly into this record, therefore, the buffer page of this record must be at least s-latched and the latch held as long as the row dtuple is used!
/// \param offsets rec_get_offsets(rec,index) or NULL, in which case this function will invoke rec_get_offsets()
/// \param col_table table, to check which externally stored columns occur in the ordering columns of an index, or NULL if index->table should be consulted instead; the user columns in this table should be the same columns as in index->table
/// \param ext out, own: cache of externally stored column prefixes, or NULL
/// \param heap memory heap from which the memory needed is allocated
/// \return own: row built; see the NOTE below!
IB_INTERN dtuple_t* row_build(ulint type, const dict_index_t* index, const rec_t* rec, const ulint* offsets, const dict_table_t* col_table, row_ext_t** ext, mem_heap_t* heap);

/// \brief Converts an index record to a typed data tuple.
/// \param rec record in the index
/// \param index index
/// \param offsets rec_get_offsets(rec, index)
/// \param n_ext out: number of externally stored columns
/// \param heap memory heap from which the memory needed is allocated
/// \return index entry built; does not set info_bits, and the data fields in the entry will point directly to rec
IB_INTERN dtuple_t* row_rec_to_index_entry_low(const rec_t* rec, const dict_index_t* index, const ulint* offsets, ulint* n_ext, mem_heap_t* heap);

/// \brief Converts an index record to a typed data tuple.
/// \details NOTE that externally stored (often big) fields are NOT copied to heap.
/// \param type ROW_COPY_DATA, or ROW_COPY_POINTERS: the former copies also the data fields to heap as the latter only places pointers to data fields on the index page
/// \param rec record in the index; NOTE: in the case ROW_COPY_POINTERS the data fields in the row will point directly into this record, therefore, the buffer page of this record must be at least s-latched and the latch held as long as the dtuple is used!
/// \param index index
/// \param offsets in/out: rec_get_offsets(rec)
/// \param n_ext out: number of externally stored columns
/// \param heap memory heap from which the memory needed is allocated
/// \return own: index entry built; see the NOTE below!
IB_INTERN dtuple_t* row_rec_to_index_entry(ulint type, const rec_t* rec, const dict_index_t* index, ulint* offsets, ulint* n_ext, mem_heap_t* heap);

/// \brief Builds from a secondary index record a row reference with which we can search the clustered index record.
/// \param type ROW_COPY_DATA, or ROW_COPY_POINTERS: the former copies also the data fields to heap, whereas the latter only places pointers to data fields on the index page
/// \param index secondary index
/// \param rec record in the index; NOTE: in the case ROW_COPY_POINTERS the data fields in the row will point directly into this record, therefore, the buffer page of this record must be at least s-latched and the latch held as long as the row reference is used!
/// \param heap memory heap from which the memory needed is allocated
/// \return own: row reference built; see the NOTE below!
IB_INTERN dtuple_t* row_build_row_ref(ulint type, dict_index_t* index, const rec_t* rec, mem_heap_t* heap);
/// \brief Builds from a secondary index record a row reference with which we can search the clustered index record.
/// \param ref in/out: row reference built; see the NOTE below!
/// \param rec record in the index; NOTE: the data fields in ref will point directly into this record, therefore, the buffer page of this record must be at least s-latched and the latch held as long as the row reference is used!
/// \param index secondary index
/// \param offsets rec_get_offsets(rec, index) or NULL
/// \param trx transaction
IB_INTERN void row_build_row_ref_in_tuple(dtuple_t* ref, const rec_t* rec, const dict_index_t* index, ulint* offsets, trx_t* trx);
/// \brief Builds from a secondary index record a row reference with which we can search the clustered index record.
/// \param ref in/out: typed data tuple where the reference is built
/// \param map array of field numbers in rec telling how ref should be built from the fields of rec
/// \param rec record in the index; must be preserved while ref is used, as we do not copy field values to heap
/// \param offsets array returned by rec_get_offsets()
IB_INLINE void row_build_row_ref_fast(dtuple_t* ref, const ulint* map, const rec_t* rec, const ulint* offsets);
/// \brief Searches the clustered index record for a row, if we have the row reference.
/// \return TRUE if found
IB_INTERN ibool row_search_on_row_ref(btr_pcur_t* pcur, ulint mode, const dict_table_t* table, const dtuple_t* ref, mtr_t* mtr);
/// \brief Fetches the clustered index record for a secondary index record.
/// \details The latches on the secondary index record are preserved.
/// \return record or NULL, if no record found
IB_INTERN rec_t* row_get_clust_rec(ulint mode, const rec_t* rec, dict_index_t* index, dict_index_t** clust_index, mtr_t* mtr);
/// \brief Searches an index record.
/// \return TRUE if found
IB_INTERN ibool row_search_index_entry(dict_index_t* index, const dtuple_t* entry, ulint mode, btr_pcur_t* pcur, mtr_t* mtr);


constinit ulint ROW_COPY_DATA = 1;
constinit ulint ROW_COPY_POINTERS = 2;

/* The allowed latching order of index records is the following:
(1) a secondary index record ->
(2) the clustered index record ->
(3) rollback segment data for the clustered index record.

No new latches may be obtained while the kernel mutex is reserved.
However, the kernel mutex can be reserved while latches are owned. */

/*******************************************************************//**
Formats the raw data in "data" (in InnoDB on-disk format) using
"dict_field" and writes the result to "buf".
Not more than "buf_size" bytes are written to "buf".
The result is always NUL-terminated (provided buf_size is positive) and the
number of bytes that were written to "buf" is returned (including the
terminating NUL).
@return	number of bytes that were written */
IB_INTERN
ulint
row_raw_format(
/*===========*/
	const char*		data,		/*!< in: raw data */
	ulint			data_len,	/*!< in: raw data length
						in bytes */
	const dict_field_t*	dict_field,	/*!< in: index field */
	char*			buf,		/*!< out: output buffer */
	ulint			buf_size);	/*!< in: output buffer size
						in bytes */

#ifndef IB_DO_NOT_INLINE
#include "row_row.inl"
#endif

#endif
