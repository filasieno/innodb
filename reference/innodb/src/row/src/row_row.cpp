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

/// \file row_row.cpp
/// \brief General row routines
/// \details Originally created on 4/20/1996 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "row_row.hpp"

#ifdef IB_DO_NOT_INLINE
#include "row_row.inl"
#endif

#include "data_type.hpp"
#include "dict_dict.hpp"
#include "btr_btr.hpp"
#include "api_ucode.hpp"
#include "mach_data.hpp"
#include "trx_rseg.hpp"
#include "trx_trx.hpp"
#include "trx_roll.hpp"
#include "trx_undo.hpp"
#include "trx_purge.hpp"
#include "trx_rec.hpp"
#include "que_que.hpp"
#include "row_ext.hpp"
#include "row_upd.hpp"
#include "rem_cmp.hpp"
#include "read_read.hpp"
#include "ut_mem.hpp"

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

IB_INTERN ulint row_get_trx_id_offset(const rec_t* rec __attribute__((unused)), dict_index_t* index, const ulint* offsets)
{
    ulint pos = dict_index_get_sys_col_pos(index, DATA_TRX_ID);
    ulint offset;
    ulint len;
    ut_ad(dict_index_is_clust(index));
    ut_ad(rec_offs_validate(rec, index, offsets));
    offset = rec_get_nth_field_offs(offsets, pos, &len);
    ut_ad(len == DATA_TRX_ID_LEN);
    return offset;
}

IB_INTERN dtuple_t* row_build_index_entry(const dtuple_t* row, row_ext_t* ext, dict_index_t* index, mem_heap_t* heap)
{
    dtuple_t*    entry;
    ulint        entry_len = dict_index_get_n_fields(index);
    ulint        i;
    ut_ad(row && index && heap);
    ut_ad(dtuple_check_typed(row));
    entry = dtuple_create(heap, entry_len);
    if (IB_UNLIKELY(index->type & DICT_UNIVERSAL)) {
        dtuple_set_n_fields_cmp(entry, entry_len);
        // There may only be externally stored columns in a clustered index B-tree of a user table.
        ut_a(!ext);
    } else {
        dtuple_set_n_fields_cmp(entry, dict_index_get_n_unique_in_tree(index));
        if (dict_index_is_clust(index)) {
            // Do not fetch externally stored columns to the clustered index. Such columns are handled at a higher level.
            ext = NULL;
        }
    }
    for (i = 0; i < entry_len; i++) {
        const dict_field_t*    ind_field = dict_index_get_nth_field(index, i);
        const dict_col_t*    col = ind_field->col;
        ulint            col_no = dict_col_get_no(col);
        dfield_t*        dfield = dtuple_get_nth_field(entry, i);
        const dfield_t*        dfield2 = dtuple_get_nth_field(row, col_no);
        ulint            len = dfield_get_len(dfield2);
        dfield_copy(dfield, dfield2);
        if (dfield_is_null(dfield)) {
        } else if (IB_LIKELY_NULL(ext)) {
            // See if the column is stored externally.
            const byte*    buf = row_ext_lookup(ext, col_no, &len);
            if (IB_LIKELY_NULL(buf)) {
                if (IB_UNLIKELY(buf == field_ref_zero)) {
                    return NULL;
                }
                dfield_set_data(dfield, buf, len);
            }
        } else if (dfield_is_ext(dfield)) {
            ut_a(len >= BTR_EXTERN_FIELD_REF_SIZE);
            len -= BTR_EXTERN_FIELD_REF_SIZE;
            ut_a(ind_field->prefix_len <= len || dict_index_is_clust(index));
        }
        // If a column prefix index, take only the prefix
        if (ind_field->prefix_len > 0 && !dfield_is_null(dfield)) {
            ut_ad(col->ord_part);
            len = dtype_get_at_most_n_mbchars(col->prtype, col->mbminlen, col->mbmaxlen, ind_field->prefix_len, len, dfield_get_data(dfield));
            dfield_set_len(dfield, len);
        }
    }
    ut_ad(dtuple_check_typed(entry));
    return entry;
}

/// \brief An inverse function to row_build_index_entry.
/// \param [in] type ROW_COPY_POINTERS or ROW_COPY_DATA; the latter copies also the data fields to heap while the first only places pointers to data fields on the index page, and thus is more efficient
/// \param [in] index clustered index
/// \param [in] rec record in the clustered index; NOTE: in the case ROW_COPY_POINTERS the data fields in the row will point directly into this record, therefore, the buffer page of this record must be at least s-latched and the latch held as long as the row dtuple is used!
/// \param [in] offsets rec_get_offsets(rec,index) or NULL, in which case this function will invoke rec_get_offsets()
/// \param [in] col_table table, to check which externally stored columns occur in the ordering columns of an index, or NULL if index->table should be consulted instead
/// \param [out] ext cache of externally stored column prefixes, or NULL
/// \param [in] heap memory heap from which the memory needed is allocated
/// \return own: row built; see the NOTE below!
/// \details Builds a row from a record in a clustered index.
IB_INTERN dtuple_t* row_build(ulint type, const dict_index_t* index, const rec_t* rec, const ulint* offsets, const dict_table_t* col_table, row_ext_t** ext, mem_heap_t* heap)
{
    ulint            offsets_[REC_OFFS_NORMAL_SIZE];
    rec_offs_init(offsets_);
    mem_heap_t*        tmp_heap    = NULL;
    ut_ad(index && rec && heap);
    ut_ad(dict_index_is_clust(index));
    if (!offsets) {
        offsets = rec_get_offsets(rec, index, offsets_, ULINT_UNDEFINED, &tmp_heap);
    } else {
        ut_ad(rec_offs_validate(rec, index, offsets));
    }
    if (type != ROW_COPY_POINTERS) {
        /* Take a copy of rec to heap */
        byte* buf = mem_heap_alloc(heap, rec_offs_size(offsets));
        rec = rec_copy(buf, rec, offsets);
        /* Avoid a debug assertion in rec_offs_validate(). */
        rec_offs_make_valid(rec, index, (ulint*) offsets);
    }
    const dict_table_t* table = index->table;
    ulint row_len = dict_table_get_n_cols(table);
    dtuple_t* row = dtuple_create(heap, row_len);
    dict_table_copy_types(row, table);
    dtuple_set_info_bits(row, rec_get_info_bits(
                     rec, dict_table_is_comp(table)));
    ulint n_fields = rec_offs_n_fields(offsets);
    ulint n_ext_cols = rec_offs_n_extern(offsets);
    ulint* ext_cols = NULL; /* remove warning */
    if (n_ext_cols) {
        ext_cols = mem_heap_alloc(heap, n_ext_cols * sizeof *ext_cols);
    }
    for (ulint i = 0, j = 0; i < n_fields; i++) {
        dict_field_t* ind_field = dict_index_get_nth_field(index, i);
        const dict_col_t* col = dict_field_get_col(ind_field);
        ulint col_no = dict_col_get_no(col);
        dfield_t* dfield = dtuple_get_nth_field(row, col_no);
        ulint len;
        if (ind_field->prefix_len == 0) {
            const byte*    field = rec_get_nth_field(
                rec, offsets, i, &len);
            dfield_set_data(dfield, field, len);
        }
        if (rec_offs_nth_extern(offsets, i)) {
            dfield_set_ext(dfield);
            if (IB_LIKELY_NULL(col_table)) {
                ut_a(col_no
                     < dict_table_get_n_cols(col_table));
                col = dict_table_get_nth_col(
                    col_table, col_no);
            }
            if (col->ord_part) {
                /* We will have to fetch prefixes of
                externally stored columns that are
                referenced by column prefixes. */
                ext_cols[j++] = col_no;
            }
        }
    }
    ut_ad(dtuple_check_typed(row));
    if (j) {
        *ext = row_ext_create(j, ext_cols, row,
                      dict_table_zip_size(index->table),
                      heap);
    } else {
        *ext = NULL;
    }
    if (tmp_heap) {
        IB_MEM_HEAP_FREE(tmp_heap);
    }
    return(row);
}

IB_INTERN dtuple_t* row_rec_to_index_entry_low(const rec_t* rec, const dict_index_t* index, const ulint* offsets, ulint* n_ext, mem_heap_t* heap)
{
    dtuple_t*    entry;
    dfield_t*    dfield;
    ulint        i;
    const byte*    field;
    ulint        len;
    ulint        rec_len;
    ut_ad(rec && heap && index);
    // Because this function may be invoked by row0merge.c on a record whose header is in different format, the check rec_offs_validate(rec, index, offsets) must be avoided here.
    ut_ad(n_ext);
    *n_ext = 0;
    rec_len = rec_offs_n_fields(offsets);
    entry = dtuple_create(heap, rec_len);
    dtuple_set_n_fields_cmp(entry, dict_index_get_n_unique_in_tree(index));
    ut_ad(rec_len == dict_index_get_n_fields(index));
    dict_index_copy_types(entry, index, rec_len);
    for (i = 0; i < rec_len; i++) {
        dfield = dtuple_get_nth_field(entry, i);
        field = rec_get_nth_field(rec, offsets, i, &len);
        dfield_set_data(dfield, field, len);
        if (rec_offs_nth_extern(offsets, i)) {
            dfield_set_ext(dfield);
            (*n_ext)++;
        }
    }
    ut_ad(dtuple_check_typed(entry));
    return entry;
}

/// \brief Converts an index record to a typed data tuple.
/// \param [in] type ROW_COPY_DATA, or ROW_COPY_POINTERS: the former copies also the data fields to heap as the latter only places pointers to data fields on the index page
/// \param [in] rec record in the index; NOTE: in the case ROW_COPY_POINTERS the data fields in the row will point directly into this record, therefore, the buffer page of this record must be at least s-latched and the latch held as long as the dtuple is used!
/// \param [in] index index
/// \param [in,out] offsets rec_get_offsets(rec)
/// \param [out] n_ext number of externally stored columns
/// \param [in] heap memory heap from which the memory needed is allocated
/// \return own: index entry built; see the NOTE below!
/// \details NOTE that externally stored (often big) fields are NOT copied to heap.
IB_INTERN dtuple_t* row_rec_to_index_entry(ulint type, const rec_t* rec, const dict_index_t* index, ulint* offsets, ulint* n_ext, mem_heap_t* heap)
{
    ut_ad(rec && heap && index);
    ut_ad(rec_offs_validate(rec, index, offsets));
    if (type == ROW_COPY_DATA) {
        /* Take a copy of rec to heap */
        byte* buf = mem_heap_alloc(heap, rec_offs_size(offsets));
        rec = rec_copy(buf, rec, offsets);
        /* Avoid a debug assertion in rec_offs_validate(). */
        rec_offs_make_valid(rec, index, offsets);
    }
    dtuple_t* entry = row_rec_to_index_entry_low(rec, index, offsets, n_ext, heap);
    dtuple_set_info_bits(entry,
                 rec_get_info_bits(rec, rec_offs_comp(offsets)));
    return(entry);
}

/*******************************************************************//**
Builds from a secondary index record a row reference with which we can
search the clustered index record.
@return    own: row reference built; see the NOTE below! */
IB_INTERN
dtuple_t*
row_build_row_ref(
/*==============*/
    ulint        type,    /*!< in: ROW_COPY_DATA, or ROW_COPY_POINTERS:
                the former copies also the data fields to
                heap, whereas the latter only places pointers
                to data fields on the index page */
    dict_index_t*    index,    /*!< in: secondary index */
    const rec_t*    rec,    /*!< in: record in the index;
                NOTE: in the case ROW_COPY_POINTERS
                the data fields in the row will point
                directly into this record, therefore,
                the buffer page of this record must be
                at least s-latched and the latch held
                as long as the row reference is used! */
    mem_heap_t*    heap)    /*!< in: memory heap from which the memory
                needed is allocated */
{
    dict_table_t*    table;
    dict_index_t*    clust_index;
    dfield_t*    dfield;
    dtuple_t*    ref;
    const byte*    field;
    ulint        len;
    ulint        ref_len;
    ulint        pos;
    byte*        buf;
    ulint        clust_col_prefix_len;
    ulint        i;
    mem_heap_t*    tmp_heap    = NULL;
    ulint        offsets_[REC_OFFS_NORMAL_SIZE];
    ulint*        offsets        = offsets_;
    rec_offs_init(offsets_);
    ut_ad(index && rec && heap);
    ut_ad(!dict_index_is_clust(index));
    offsets = rec_get_offsets(rec, index, offsets,
                  ULINT_UNDEFINED, &tmp_heap);
    /* Secondary indexes must not contain externally stored columns. */
    ut_ad(!rec_offs_any_extern(offsets));
    if (type == ROW_COPY_DATA) {
        /* Take a copy of rec to heap */
        buf = mem_heap_alloc(heap, rec_offs_size(offsets));
        rec = rec_copy(buf, rec, offsets);
        /* Avoid a debug assertion in rec_offs_validate(). */
        rec_offs_make_valid(rec, index, offsets);
    }
    table = index->table;
    clust_index = dict_table_get_first_index(table);
    ref_len = dict_index_get_n_unique(clust_index);
    ref = dtuple_create(heap, ref_len);
    dict_index_copy_types(ref, clust_index, ref_len);
    for (i = 0; i < ref_len; i++) {
        dfield = dtuple_get_nth_field(ref, i);
        pos = dict_index_get_nth_field_pos(index, clust_index, i);
        ut_a(pos != ULINT_UNDEFINED);
        field = rec_get_nth_field(rec, offsets, pos, &len);
        dfield_set_data(dfield, field, len);
        /* If the primary key contains a column prefix, then the
        secondary index may contain a longer prefix of the same
        column, or the full column, and we must adjust the length
        accordingly. */
                ulint clust_col_prefix_len = dict_index_get_nth_field(
            clust_index, i)->prefix_len;
        if (clust_col_prefix_len > 0) {
            if (len != IB_SQL_NULL) {
                const dtype_t*    dtype
                    = dfield_get_type(dfield);
                dfield_set_len(dfield,
                           dtype_get_at_most_n_mbchars(
                               dtype->prtype,
                               dtype->mbminlen,
                               dtype->mbmaxlen,
                               clust_col_prefix_len,
                               len, (char*) field));
            }
        }
    }
    ut_ad(dtuple_check_typed(ref));
    if (tmp_heap) {
        IB_MEM_HEAP_FREE(tmp_heap);
    }
    return(ref);
}

/// \brief Builds from a secondary index record a row reference with which we can search the clustered index record.
/// \param [in,out] ref row reference built; see the NOTE below!
/// \param [in] rec record in the index; NOTE: the data fields in ref will point directly into this record, therefore, the buffer page of this record must be at least s-latched and the latch held as long as the row reference is used!
/// \param [in] index secondary index
/// \param [in] offsets rec_get_offsets(rec, index) or NULL
/// \param [in] trx transaction
IB_INTERN void row_build_row_ref_in_tuple(dtuple_t* ref, const rec_t* rec, const dict_index_t* index, ulint* offsets, trx_t* trx)
{
    mem_heap_t*        heap        = NULL;
    ulint            offsets_[REC_OFFS_NORMAL_SIZE];
    rec_offs_init(offsets_);
    ut_a(ref);
    ut_a(index);
    ut_a(rec);
    ut_ad(!dict_index_is_clust(index));
    if (IB_UNLIKELY(!index->table)) {
        ib_log(state, "InnoDB: table ");
notfound:
        ut_print_name(state->stream, trx, TRUE, index->table_name);
        ib_log(state, " for index ");
        ut_print_name(state->stream, trx, FALSE, index->name);
        ib_log(state, " not found\n");
        UT_ERROR;
    }
    const dict_index_t* clust_index = dict_table_get_first_index(index->table);
    if (IB_UNLIKELY(!clust_index)) {
        ib_log(state, "InnoDB: clust index for table ");
        goto notfound;
    }
    if (!offsets) {
        offsets = rec_get_offsets(rec, index, offsets_,
                      ULINT_UNDEFINED, &heap);
    } else {
        ut_ad(rec_offs_validate(rec, index, offsets));
    }
    /* Secondary indexes must not contain externally stored columns. */
    ut_ad(!rec_offs_any_extern(offsets));
    ulint ref_len = dict_index_get_n_unique(clust_index);
    ut_ad(ref_len == dtuple_get_n_fields(ref));
    dict_index_copy_types(ref, clust_index, ref_len);
    for (ulint i = 0; i < ref_len; i++) {
        dfield_t* dfield = dtuple_get_nth_field(ref, i);
        ulint pos = dict_index_get_nth_field_pos(index, clust_index, i);
        ut_a(pos != ULINT_UNDEFINED);
        ulint len;
        const byte* field = rec_get_nth_field(rec, offsets, pos, &len);
        dfield_set_data(dfield, field, len);
        /* If the primary key contains a column prefix, then the
        secondary index may contain a longer prefix of the same
        column, or the full column, and we must adjust the length
        accordingly. */
                ulint clust_col_prefix_len = dict_index_get_nth_field(
            clust_index, i)->prefix_len;
        if (clust_col_prefix_len > 0) {
            if (len != IB_SQL_NULL) {
                const dtype_t*    dtype
                    = dfield_get_type(dfield);
                dfield_set_len(dfield,
                           dtype_get_at_most_n_mbchars(
                               dtype->prtype,
                               dtype->mbminlen,
                               dtype->mbmaxlen,
                               clust_col_prefix_len,
                               len, (char*) field));
            }
        }
    }
    ut_ad(dtuple_check_typed(ref));
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
}

/// \brief Searches the clustered index record for a row, if we have the row reference.
/// \param [out] pcur persistent cursor, which must be closed by the caller
/// \param [in] mode BTR_MODIFY_LEAF, ...
/// \param [in] table table
/// \param [in] ref row reference
/// \param [in,out] mtr mtr
/// \return TRUE if found
IB_INTERN ibool row_search_on_row_ref(btr_pcur_t* pcur, ulint mode, const dict_table_t* table, const dtuple_t* ref, mtr_t* mtr)
{
    ut_ad(dtuple_check_typed(ref));
    dict_index_t* index = dict_table_get_first_index(table);
    ut_a(dtuple_get_n_fields(ref) == dict_index_get_n_unique(index));
    btr_pcur_open(index, ref, PAGE_CUR_LE, mode, pcur, mtr);
    ulint low_match = btr_pcur_get_low_match(pcur);
    rec_t* rec = btr_pcur_get_rec(pcur);
    if (page_rec_is_infimum(rec)) {
        return(FALSE);
    }
    if (low_match != dtuple_get_n_fields(ref)) {
        return(FALSE);
    }
    return(TRUE);
}

/// \brief Fetches the clustered index record for a secondary index record.
/// \param [in] mode BTR_MODIFY_LEAF, ...
/// \param [in] rec record in a secondary index
/// \param [in] index secondary index
/// \param [out] clust_index clustered index
/// \param [in] mtr mtr
/// \return record or NULL, if no record found
/// \details The latches on the secondary index record are preserved.
IB_INTERN rec_t* row_get_clust_rec(ulint mode, const rec_t* rec, dict_index_t* index, dict_index_t** clust_index, mtr_t* mtr)
{
    ut_ad(!dict_index_is_clust(index));
    dict_table_t* table = index->table;
    mem_heap_t* heap = IB_MEM_HEAP_CREATE(256);
    dtuple_t* ref = row_build_row_ref(ROW_COPY_POINTERS, index, rec, heap);
    btr_pcur_t pcur;
    ibool found = row_search_on_row_ref(&pcur, mode, table, ref, mtr);
    rec_t* clust_rec = found ? btr_pcur_get_rec(&pcur) : NULL;
    IB_MEM_HEAP_FREE(heap);
    btr_pcur_close(&pcur);
    *clust_index = dict_table_get_first_index(table);
    return(clust_rec);
}

/// \brief Searches an index record.
/// \param [in] index index
/// \param [in] entry index entry
/// \param [in] mode BTR_MODIFY_LEAF, ...
/// \param [in,out] pcur persistent cursor, which must be closed by the caller
/// \param [in] mtr mtr
/// \return TRUE if found
IB_INTERN ibool row_search_index_entry(dict_index_t* index, const dtuple_t* entry, ulint mode, btr_pcur_t* pcur, mtr_t* mtr)
{
    ut_ad(dtuple_check_typed(entry));
    btr_pcur_open(index, entry, PAGE_CUR_LE, mode, pcur, mtr);
    ulint low_match = btr_pcur_get_low_match(pcur);
    rec_t* rec = btr_pcur_get_rec(pcur);
    ulint n_fields = dtuple_get_n_fields(entry);
    return(!page_rec_is_infimum(rec) && low_match == n_fields);
}
