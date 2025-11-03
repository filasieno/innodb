
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

/// \file page_cur.cpp
/// \brief The page cursor
/// \details Originally created on 10/4/1994 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "defs.hpp"

#include "page_zip.hpp"
#include "page_cur.hpp"

#ifdef IB_DO_NOT_INLINE
    #include "page_cur.inl"
#endif

#include "mtr_log.hpp"
#include "log_recv.hpp"
#include "ut_ut.hpp"

#ifndef IB_HOTBACKUP

#include "rem_cmp.hpp"

#ifdef PAGE_CUR_ADAPT

#ifdef IB_SEARCH_PERF_STAT
    static ulint page_cur_short_succ = 0;
#endif // IB_SEARCH_PERF_STAT

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

#ifdef PAGE_CUR_ADAPT

/// \brief Linear Congruential Generator pseudo-random number generator for page cursor operations
/// \return A 64-bit pseudo-random number
static ib_uint64_t page_cur_lcg_prng(void);

/// \brief Tries to use a search shortcut based on the last insert position
/// \param [in] block buffer block
/// \param [in] index record descriptor
/// \param [in] tuple search key
/// \param [out] iup_matched_fields matched fields in upper record
/// \param [out] iup_matched_bytes matched bytes in upper record
/// \param [out] ilow_matched_fields matched fields in lower record
/// \param [out] ilow_matched_bytes matched bytes in lower record
/// \param [out] cursor page cursor
/// \return TRUE if the shortcut search was successful
IB_INLINE ibool page_cur_try_search_shortcut(const buf_block_t* block, const dict_index_t* index, const dtuple_t* tuple, ulint* iup_matched_fields, ulint* iup_matched_bytes, ulint* ilow_matched_fields, ulint* ilow_matched_bytes, page_cur_t* cursor);

#endif

#ifdef PAGE_CUR_LE_OR_EXTENDS

/// \brief Checks if a record field extends beyond the search tuple
/// \param [in] tuple search key
/// \param [in] rec record
/// \param [in] offsets record offsets
/// \param [in] n field number
/// \return TRUE if the record field extends beyond the tuple field
static ibool page_cur_rec_field_extends(const dtuple_t* tuple, const rec_t* rec, const ulint* offsets, ulint n);

#endif

#ifndef IB_HOTBACKUP

/// \brief Writes a redo log record of a record insert
/// \param [in] insert_rec inserted record
/// \param [in] rec_size size of the inserted record
/// \param [in] cursor_rec cursor record after which inserted
/// \param [in] index record descriptor
/// \param [in] mtr mini-transaction
static void page_cur_insert_rec_write_log(rec_t* insert_rec, ulint rec_size, rec_t* cursor_rec, dict_index_t* index, mtr_t* mtr);

/// \brief Inserts a record on a compressed page after reorganization
/// \param [in,out] current_rec pointer to current record after which the new record is inserted
/// \param [in] block buffer block
/// \param [in] index record descriptor
/// \param [in] rec inserted record
/// \param [in] page uncompressed page
/// \param [in] page_zip compressed page
/// \param [in] mtr mini-transaction, or NULL
/// \return pointer to inserted record, or NULL on failure
static rec_t* page_cur_insert_rec_zip_reorg(rec_t** current_rec, buf_block_t* block, dict_index_t* index, rec_t* rec, page_t* page, page_zip_des_t* page_zip, mtr_t* mtr);

/// \brief Writes log record for copying record list to created page
/// \param [in] page index page
/// \param [in] index record descriptor
/// \param [in] mtr mtr
/// \return pointer to log buffer
IB_INLINE byte* page_copy_rec_list_to_created_page_write_log(page_t* page, dict_index_t* index, mtr_t* mtr);

/// \brief Writes log record of a record deletion
/// \param [in] rec record to be deleted
/// \param [in] index record descriptor
/// \param [in] mtr mini-transaction handle
IB_INLINE void page_cur_delete_rec_write_log(rec_t* rec, dict_index_t* index, mtr_t* mtr);

#endif

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

#ifdef PAGE_CUR_ADAPT

/// \brief Linear Congruential Generator pseudo-random number generator for page cursor operations
/// \return A 64-bit pseudo-random number
static ib_uint64_t page_cur_lcg_prng(void)
{
    static const ulint LCG_a = 1103515245;
    static const ulint LCG_c = 12345;
    static ib_uint64_t lcg_current = 0;
    static ibool initialized = FALSE;
    if (!initialized) {
        lcg_current = (ib_uint64_t) ut_time_us(NULL);
        initialized = TRUE;
    }
    // No need to "% 2^64" explicitly because lcg_current is 64 bit and this will be done anyway
    lcg_current = LCG_a * lcg_current + LCG_c;
    return lcg_current;
}

IB_INLINE ibool page_cur_try_search_shortcut(const buf_block_t* block, const dict_index_t* index, const dtuple_t* tuple, ulint* iup_matched_fields, ulint* iup_matched_bytes, ulint* ilow_matched_fields, ulint* ilow_matched_bytes, page_cur_t* cursor)
{
    ulint low_match;
    ulint low_bytes;
    ulint up_match;
    ulint up_bytes;
#ifdef IB_SEARCH_DEBUG
    page_cur_t cursor2;
#endif
    ibool success = FALSE;
    const page_t* page = buf_block_get_frame(block);
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    ut_ad(dtuple_check_typed(tuple));
    const rec_t* rec = page_header_get_ptr(page, PAGE_LAST_INSERT);
    offsets = rec_get_offsets(rec, index, offsets, dtuple_get_n_fields(tuple), &heap);
    ut_ad(rec);
    ut_ad(page_rec_is_user_rec(rec));
    ut_pair_min(&low_match, &low_bytes, *ilow_matched_fields, *ilow_matched_bytes, *iup_matched_fields, *iup_matched_bytes);
    up_match = low_match;
    up_bytes = low_bytes;
    if (page_cmp_dtuple_rec_with_match(index->cmp_ctx, tuple, rec, offsets, &low_match, &low_bytes) < 0) {
        goto exit_func;
    }
    const rec_t* next_rec = page_rec_get_next_const(rec);
    offsets = rec_get_offsets(next_rec, index, offsets, dtuple_get_n_fields(tuple), &heap);
    if (page_cmp_dtuple_rec_with_match(index->cmp_ctx, tuple, next_rec, offsets, &up_match, &up_bytes) >= 0) {
        goto exit_func;
    }
    page_cur_position(rec, block, cursor);
#ifdef IB_SEARCH_DEBUG
    page_cur_search_with_match(block, index, tuple, PAGE_CUR_DBG, iup_matched_fields, iup_matched_bytes, ilow_matched_fields, ilow_matched_bytes, &cursor2);
    ut_a(cursor2.rec == cursor->rec);
    if (!page_rec_is_supremum(next_rec)) {
        ut_a(*iup_matched_fields == up_match);
        ut_a(*iup_matched_bytes == up_bytes);
    }
    ut_a(*ilow_matched_fields == low_match);
    ut_a(*ilow_matched_bytes == low_bytes);
#endif
    if (!page_rec_is_supremum(next_rec)) {
        *iup_matched_fields = up_match;
        *iup_matched_bytes = up_bytes;
    }
    *ilow_matched_fields = low_match;
    *ilow_matched_bytes = low_bytes;
#ifdef IB_SEARCH_PERF_STAT
    page_cur_short_succ++;
#endif // IB_SEARCH_PERF_STAT
    success = TRUE;
exit_func:
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
    return success;
}

#endif // PAGE_CUR_ADAPT

#ifdef PAGE_CUR_LE_OR_EXTENDS

static ibool page_cur_rec_field_extends(const dtuple_t* tuple, const rec_t* rec, const ulint* offsets, ulint n)
{
    ut_ad(rec_offs_validate(rec, NULL, offsets));
    const dfield_t* dfield = dtuple_get_nth_field(tuple, n);
    const dtype_t* type = dfield_get_type(dfield);
    ulint rec_f_len;
    const byte* rec_f = rec_get_nth_field(rec, offsets, n, &rec_f_len);
    if (type->mtype == DATA_VARCHAR || type->mtype == DATA_CHAR || type->mtype == DATA_FIXBINARY || type->mtype == DATA_BINARY || type->mtype == DATA_BLOB || type->mtype == DATA_VARCLIENT || type->mtype == DATA_CLIENT) {
        if (dfield_get_len(dfield) != IB_SQL_NULL && rec_f_len != IB_SQL_NULL && rec_f_len >= dfield_get_len(dfield) && !cmp_data_data_slow(type->mtype, type->prtype, dfield_get_data(dfield), dfield_get_len(dfield), rec_f, dfield_get_len(dfield))) {
            return TRUE;
        }
    }
    return FALSE;
}
#endif // PAGE_CUR_LE_OR_EXTENDS

IB_INTERN void page_cur_search_with_match(const buf_block_t* block, const dict_index_t* index, const dtuple_t* tuple, ulint mode, ulint* iup_matched_fields, ulint* iup_matched_bytes, ulint* ilow_matched_fields, ulint* ilow_matched_bytes, page_cur_t* cursor)
{
    const page_t* page = buf_block_get_frame(block);
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    ut_ad(block && tuple && iup_matched_fields && iup_matched_bytes && ilow_matched_fields && ilow_matched_bytes && cursor);
    ut_ad(dtuple_validate(tuple));
#ifdef IB_DEBUG
#ifdef PAGE_CUR_DBG
    if (mode != PAGE_CUR_DBG)
#endif // PAGE_CUR_DBG
#ifdef PAGE_CUR_LE_OR_EXTENDS
        if (mode != PAGE_CUR_LE_OR_EXTENDS)
#endif // PAGE_CUR_LE_OR_EXTENDS
                ut_ad(mode == PAGE_CUR_L || mode == PAGE_CUR_LE || mode == PAGE_CUR_G || mode == PAGE_CUR_GE);
#endif // IB_DEBUG
#ifdef IB_ZIP_DEBUG
    const page_zip_des_t* page_zip = buf_block_get_page_zip(block);
            ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG
    page_check_dir(page);
#ifdef PAGE_CUR_ADAPT
    if (page_is_leaf(page) && (mode == PAGE_CUR_LE) && (page_header_get_field(page, PAGE_N_DIRECTION) > 3) && (page_header_get_ptr(page, PAGE_LAST_INSERT)) && (page_header_get_field(page, PAGE_DIRECTION) == PAGE_RIGHT)) {
        if (page_cur_try_search_shortcut(block, index, tuple, iup_matched_fields, iup_matched_bytes, ilow_matched_fields, ilow_matched_bytes, cursor)) {
            return;
        }
    }
#ifdef PAGE_CUR_DBG
    if (mode == PAGE_CUR_DBG) {
        mode = PAGE_CUR_LE;
    }
#endif
#endif
    // The following flag does not work for non-latin1 char sets because cmp_full_field does not tell how many bytes matched

#ifdef PAGE_CUR_LE_OR_EXTENDS
    ut_a(mode != PAGE_CUR_LE_OR_EXTENDS);
#endif // PAGE_CUR_LE_OR_EXTENDS

    // If mode PAGE_CUR_G is specified, we are trying to position the cursor to answer a query of the form "tuple < X", where tuple is the input parameter, and X denotes an arbitrary physical record on the page.
    // We want to position the cursor on the first X which satisfies the condition.
    ulint up_matched_fields = *iup_matched_fields;
    ulint up_matched_bytes = *iup_matched_bytes;
    ulint low_matched_fields = *ilow_matched_fields;
    ulint low_matched_bytes = *ilow_matched_bytes;
    // Perform binary search. First the search is done through the page directory, after that as a linear search in the list of records owned by the upper limit directory slot.
    ulint low = 0;
    ulint up = page_dir_get_n_slots(page) - 1;
    // Perform binary search until the lower and upper limit directory slots come to the distance 1 of each other
    while (up - low > 1) {
        ulint mid = (low + up) / 2;
        const page_dir_slot_t* slot = page_dir_get_nth_slot(page, mid);
        const rec_t* mid_rec = page_dir_slot_get_rec(slot);
        ulint cur_matched_fields;
        ulint cur_matched_bytes;
        ut_pair_min(&cur_matched_fields, &cur_matched_bytes, low_matched_fields, low_matched_bytes, up_matched_fields, up_matched_bytes);
        offsets = rec_get_offsets(mid_rec, index, offsets, dtuple_get_n_fields_cmp(tuple), &heap);
        int cmp = cmp_dtuple_rec_with_match(index->cmp_ctx, tuple, mid_rec, offsets, &cur_matched_fields, &cur_matched_bytes);
        if (IB_LIKELY(cmp > 0)) {
low_slot_match:
            low = mid;
            low_matched_fields = cur_matched_fields;
            low_matched_bytes = cur_matched_bytes;
        } else if (IB_EXPECT(cmp, -1)) {
#ifdef PAGE_CUR_LE_OR_EXTENDS
            if (mode == PAGE_CUR_LE_OR_EXTENDS && page_cur_rec_field_extends(tuple, mid_rec, offsets, cur_matched_fields)) {
                goto low_slot_match;
            }
#endif // PAGE_CUR_LE_OR_EXTENDS
up_slot_match:
            up = mid;
            up_matched_fields = cur_matched_fields;
            up_matched_bytes = cur_matched_bytes;
        } else if (mode == PAGE_CUR_G || mode == PAGE_CUR_LE
#ifdef PAGE_CUR_LE_OR_EXTENDS
               || mode == PAGE_CUR_LE_OR_EXTENDS
#endif /* PAGE_CUR_LE_OR_EXTENDS */
               ) {
            goto low_slot_match;
        } else {
            goto up_slot_match;
        }
    }
    const page_dir_slot_t* slot = page_dir_get_nth_slot(page, low);
    const rec_t* low_rec = page_dir_slot_get_rec(slot);
    slot = page_dir_get_nth_slot(page, up);
    const rec_t* up_rec = page_dir_slot_get_rec(slot);
    // Perform linear search until the upper and lower records come to distance 1 of each other.
    while (page_rec_get_next_const(low_rec) != up_rec) {
        const rec_t* mid_rec = page_rec_get_next_const(low_rec);
        ulint cur_matched_fields;
        ulint cur_matched_bytes;
        ut_pair_min(&cur_matched_fields, &cur_matched_bytes, low_matched_fields, low_matched_bytes, up_matched_fields, up_matched_bytes);
        offsets = rec_get_offsets(mid_rec, index, offsets, dtuple_get_n_fields_cmp(tuple), &heap);
        int cmp = cmp_dtuple_rec_with_match(index->cmp_ctx, tuple, mid_rec, offsets, &cur_matched_fields, &cur_matched_bytes);
        if (IB_LIKELY(cmp > 0)) {
low_rec_match:
            low_rec = mid_rec;
            low_matched_fields = cur_matched_fields;
            low_matched_bytes = cur_matched_bytes;
        } else if (IB_EXPECT(cmp, -1)) {
#ifdef PAGE_CUR_LE_OR_EXTENDS
            if (mode == PAGE_CUR_LE_OR_EXTENDS && page_cur_rec_field_extends(tuple, mid_rec, offsets, cur_matched_fields)) {
                goto low_rec_match;
            }
#endif // PAGE_CUR_LE_OR_EXTENDS
up_rec_match:
            up_rec = mid_rec;
            up_matched_fields = cur_matched_fields;
            up_matched_bytes = cur_matched_bytes;
        } else if (mode == PAGE_CUR_G || mode == PAGE_CUR_LE
#ifdef PAGE_CUR_LE_OR_EXTENDS
               || mode == PAGE_CUR_LE_OR_EXTENDS
#endif // PAGE_CUR_LE_OR_EXTENDS
               ) {
            goto low_rec_match;
        } else {
            goto up_rec_match;
        }
    }
#ifdef IB_SEARCH_DEBUG
    // Check that the lower and upper limit records have the right alphabetical order compared to tuple.
    ulint dbg_matched_fields = 0;
    ulint dbg_matched_bytes = 0;
    offsets = rec_get_offsets(low_rec, index, offsets, ULINT_UNDEFINED, &heap);
    int dbg_cmp = page_cmp_dtuple_rec_with_match(index->cmp_ctx, tuple, low_rec, offsets, &dbg_matched_fields, &dbg_matched_bytes);
    if (mode == PAGE_CUR_G) {
        ut_a(dbg_cmp >= 0);
    } else if (mode == PAGE_CUR_GE) {
        ut_a(dbg_cmp == 1);
    } else if (mode == PAGE_CUR_L) {
        ut_a(dbg_cmp == 1);
    } else if (mode == PAGE_CUR_LE) {
        ut_a(dbg_cmp >= 0);
    }
    if (!page_rec_is_infimum(low_rec)) {
        ut_a(low_matched_fields == dbg_matched_fields);
        ut_a(low_matched_bytes == dbg_matched_bytes);
    }
    ulint dbg_matched_fields2 = 0;
    ulint dbg_matched_bytes2 = 0;
    offsets = rec_get_offsets(up_rec, index, offsets, ULINT_UNDEFINED, &heap);
    int dbg_cmp2 = page_cmp_dtuple_rec_with_match(index->cmp_ctx, tuple, up_rec, offsets, &dbg_matched_fields2, &dbg_matched_bytes2);
    if (mode == PAGE_CUR_G) {
        ut_a(dbg_cmp2 == -1);
    } else if (mode == PAGE_CUR_GE) {
        ut_a(dbg_cmp2 <= 0);
    } else if (mode == PAGE_CUR_L) {
        ut_a(dbg_cmp2 <= 0);
    } else if (mode == PAGE_CUR_LE) {
        ut_a(dbg_cmp2 == -1);
    }
    if (!page_rec_is_supremum(up_rec)) {
        ut_a(up_matched_fields == dbg_matched_fields2);
        ut_a(up_matched_bytes == dbg_matched_bytes2);
    }
#endif
    if (mode <= PAGE_CUR_GE) {
        page_cur_position(up_rec, block, cursor);
    } else {
        page_cur_position(low_rec, block, cursor);
    }
    *iup_matched_fields = up_matched_fields;
    *iup_matched_bytes = up_matched_bytes;
    *ilow_matched_fields = low_matched_fields;
    *ilow_matched_bytes = low_matched_bytes;
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
}

/// \brief Position cursor on a random user record on the page
/// \param [in] block buffer block
/// \param [out] cursor page cursor to position
IB_INTERN void page_cur_open_on_rnd_user_rec(buf_block_t* block, page_cur_t* cursor)
{
    ulint n_recs = page_get_n_recs(buf_block_get_frame(block));
    page_cur_set_before_first(block, cursor);
    if (IB_UNLIKELY(n_recs == 0)) {
        return;
    }
    ulint rnd = (ulint) (page_cur_lcg_prng() % n_recs);
    do {
        page_cur_move_to_next(cursor);
    } while (rnd--);
}

static void page_cur_insert_rec_write_log(rec_t* insert_rec, ulint rec_size, rec_t* cursor_rec, dict_index_t* index, mtr_t* mtr)
{
    ut_a(rec_size < IB_PAGE_SIZE);
    ut_ad(page_align(insert_rec) == page_align(cursor_rec));
    ut_ad(!page_rec_is_comp(insert_rec) == !dict_table_is_comp(index->table));
    ulint extra_size;
    ulint cur_extra_size;
    ulint cur_rec_size;
    {
        mem_heap_t* heap = NULL;
        ulint cur_offs_[REC_OFFS_NORMAL_SIZE];
        ulint ins_offs_[REC_OFFS_NORMAL_SIZE];
        ulint* cur_offs;
        ulint* ins_offs;
        rec_offs_init(cur_offs_);
        rec_offs_init(ins_offs_);
        cur_offs = rec_get_offsets(cursor_rec, index, cur_offs_, ULINT_UNDEFINED, &heap);
        ins_offs = rec_get_offsets(insert_rec, index, ins_offs_, ULINT_UNDEFINED, &heap);
        extra_size = rec_offs_extra_size(ins_offs);
        cur_extra_size = rec_offs_extra_size(cur_offs);
        ut_ad(rec_size == rec_offs_size(ins_offs));
        cur_rec_size = rec_offs_size(cur_offs);
        if (IB_LIKELY_NULL(heap)) {
            IB_MEM_HEAP_FREE(heap);
        }
    }
    const byte* ins_ptr = insert_rec - extra_size;
    ulint i = 0;
    if (cur_extra_size == extra_size) {
        ulint min_rec_size = ut_min(cur_rec_size, rec_size);
        const byte* cur_ptr = cursor_rec - cur_extra_size;
        // Find out the first byte in insert_rec which differs from cursor_rec; skip the bytes in the record info
        do {
            if (*ins_ptr == *cur_ptr) {
                i++;
                ins_ptr++;
                cur_ptr++;
            } else if ((i < extra_size) && (i >= extra_size - page_rec_get_base_extra_size(insert_rec))) {
                i = extra_size;
                ins_ptr = insert_rec;
                cur_ptr = cursor_rec;
            } else {
                break;
            }
        } while (i < min_rec_size);
    }
    byte* log_ptr;
    const byte* log_end;
    if (mtr_get_log_mode(mtr) != MTR_LOG_SHORT_INSERTS) {
        if (page_rec_is_comp(insert_rec)) {
            log_ptr = mlog_open_and_write_index(mtr, insert_rec, index, MLOG_COMP_REC_INSERT, 2 + 5 + 1 + 5 + 5 + MLOG_BUF_MARGIN);
            if (IB_UNLIKELY(!log_ptr)) {
                // Logging in mtr is switched off during crash recovery: in that case mlog_open returns NULL
                return;
            }
        } else {
            log_ptr = mlog_open(mtr, 11 + 2 + 5 + 1 + 5 + 5 + MLOG_BUF_MARGIN);
            if (IB_UNLIKELY(!log_ptr)) {
                // Logging in mtr is switched off during crash recovery: in that case mlog_open returns NULL
                return;
            }
            log_ptr = mlog_write_initial_log_record_fast(insert_rec, MLOG_REC_INSERT, log_ptr, mtr);
        }
        log_end = &log_ptr[2 + 5 + 1 + 5 + 5 + MLOG_BUF_MARGIN];
        // Write the cursor rec offset as a 2-byte ulint
        mach_write_to_2(log_ptr, page_offset(cursor_rec));
        log_ptr += 2;
    } else {
        log_ptr = mlog_open(mtr, 5 + 1 + 5 + 5 + MLOG_BUF_MARGIN);
        if (!log_ptr) {
            // Logging in mtr is switched off during crash recovery: in that case mlog_open returns NULL
            return;
        }
        log_end = &log_ptr[5 + 1 + 5 + 5 + MLOG_BUF_MARGIN];
    }
    if (page_rec_is_comp(insert_rec)) {
        if (IB_UNLIKELY(rec_get_info_and_status_bits(insert_rec, TRUE) != rec_get_info_and_status_bits(cursor_rec, TRUE))) {
            goto need_extra_info;
        }
    } else {
        if (IB_UNLIKELY(rec_get_info_and_status_bits(insert_rec, FALSE) != rec_get_info_and_status_bits(cursor_rec, FALSE))) {
            goto need_extra_info;
        }
    }
    if (extra_size != cur_extra_size || rec_size != cur_rec_size) {
need_extra_info:
        // Write the record end segment length and the extra info storage flag
        log_ptr += mach_write_compressed(log_ptr, 2 * (rec_size - i) + 1);
        // Write the info bits
        mach_write_to_1(log_ptr, rec_get_info_and_status_bits(insert_rec, page_rec_is_comp(insert_rec)));
        log_ptr++;
        // Write the record origin offset
        log_ptr += mach_write_compressed(log_ptr, extra_size);
        // Write the mismatch index
        log_ptr += mach_write_compressed(log_ptr, i);
        ut_a(i < IB_PAGE_SIZE);
        ut_a(extra_size < IB_PAGE_SIZE);
    } else {
        // Write the record end segment length and the extra info storage flag
        log_ptr += mach_write_compressed(log_ptr, 2 * (rec_size - i));
    }
    // Write to the log the inserted index record end segment which differs from the cursor record
    rec_size -= i;
    if (log_ptr + rec_size <= log_end) {
        memcpy(log_ptr, ins_ptr, rec_size);
        mlog_close(mtr, log_ptr + rec_size);
    } else {
        mlog_close(mtr, log_ptr);
        ut_a(rec_size < IB_PAGE_SIZE);
        mlog_catenate_string(mtr, ins_ptr, rec_size);
    }
}

#else // !IB_HOTBACKUP 

#define page_cur_insert_rec_write_log(ins_rec,size,cur,index,mtr) ((void) 0)

#endif // !IB_HOTBACKUP 

IB_INTERN byte* page_cur_parse_insert_rec(ibool is_short, byte* ptr, byte* end_ptr, buf_block_t* block, dict_index_t* index, mtr_t* mtr)
{
    byte* ptr2 = ptr;
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);
    page_t* page = block ? buf_block_get_frame(block) : NULL;
    rec_t* cursor_rec;
    if (is_short) {
        cursor_rec = page_rec_get_prev(page_get_supremum_rec(page));
    } else {
        // Read the cursor rec offset as a 2-byte ulint
        if (IB_UNLIKELY(end_ptr < ptr + 2)) {
            return NULL;
        }
        ulint offset = mach_read_from_2(ptr);
        ptr += 2;
        cursor_rec = page + offset;
        if (IB_UNLIKELY(offset >= IB_PAGE_SIZE)) {
            recv_sys->found_corrupt_log = TRUE;
            return NULL;
        }
    }
    ulint end_seg_len;
    ptr = mach_parse_compressed(ptr, end_ptr, &end_seg_len);
    if (ptr == NULL) {
        return NULL;
    }
    if (IB_UNLIKELY(end_seg_len >= IB_PAGE_SIZE << 1)) {
        recv_sys->found_corrupt_log = TRUE;
        return NULL;
    }
    ulint info_and_status_bits = 0; // remove warning
    ulint origin_offset = 0;
    ulint mismatch_index = 0;
    if (end_seg_len & 0x1UL) {
        // Read the info bits
        if (end_ptr < ptr + 1) {
            return NULL;
        }
        info_and_status_bits = mach_read_from_1(ptr);
        ptr++;
        ptr = mach_parse_compressed(ptr, end_ptr, &origin_offset);
        if (ptr == NULL) {
            return NULL;
        }
        ut_a(origin_offset < IB_PAGE_SIZE);
        ptr = mach_parse_compressed(ptr, end_ptr, &mismatch_index);
        if (ptr == NULL) {
            return NULL;
        }
        ut_a(mismatch_index < IB_PAGE_SIZE);
    }
    if (IB_UNLIKELY(end_ptr < ptr + (end_seg_len >> 1))) {
        return NULL;
    }
    if (!block) {
        return ptr + (end_seg_len >> 1);
    }
    ut_ad(!!page_is_comp(page) == dict_table_is_comp(index->table));
    ut_ad(!buf_block_get_page_zip(block) || page_is_comp(page));
    // Read from the log the inserted index record end segment which differs from the cursor record
    offsets = rec_get_offsets(cursor_rec, index, offsets, ULINT_UNDEFINED, &heap);
    if (!(end_seg_len & 0x1UL)) {
        info_and_status_bits = rec_get_info_and_status_bits(cursor_rec, page_is_comp(page));
        origin_offset = rec_offs_extra_size(offsets);
        mismatch_index = rec_offs_size(offsets) - (end_seg_len >> 1);
    }
    end_seg_len >>= 1;
    byte buf1[1024];
    byte* buf;
    if (mismatch_index + end_seg_len < sizeof buf1) {
        buf = buf1;
    } else {
        buf = IB_MEM_ALLOC(mismatch_index + end_seg_len);
    }
    // Build the inserted record to buf
    if (IB_UNLIKELY(mismatch_index >= IB_PAGE_SIZE)) {
        ib_log(state, "Is short %lu, info_and_status_bits %lu, offset %lu, o_offset %lu\nmismatch index %lu, end_seg_len %lu\nparsed len %lu\n", (ulong) is_short, (ulong) info_and_status_bits, (ulong) page_offset(cursor_rec), (ulong) origin_offset, (ulong) mismatch_index, (ulong) end_seg_len, (ulong) (ptr - ptr2));
        ib_log(state, "Dump of 300 bytes of log:\n");
        ut_print_buf(state->stream, ptr2, 300);
        ib_log(state, "\n");
        buf_page_print(page, 0);
        UT_ERROR;
    }
    ut_memcpy(buf, rec_get_start(cursor_rec, offsets), mismatch_index);
    ut_memcpy(buf + mismatch_index, ptr, end_seg_len);
    if (page_is_comp(page)) {
        rec_set_info_and_status_bits(buf + origin_offset, info_and_status_bits);
    } else {
        rec_set_info_bits_old(buf + origin_offset, info_and_status_bits);
    }
    page_cur_t cursor;
    page_cur_position(cursor_rec, block, &cursor);
    offsets = rec_get_offsets(buf + origin_offset, index, offsets, ULINT_UNDEFINED, &heap);
    if (IB_UNLIKELY(!page_cur_rec_insert(&cursor, buf + origin_offset, index, offsets, mtr))) {
        // The redo log record should only have been written after the write was successful.
        UT_ERROR;
    }
    if (buf != buf1) {
        IB_MEM_FREE(buf);
    }
    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }
    return ptr + end_seg_len;
}

IB_INTERN rec_t* page_cur_insert_rec_low(rec_t* current_rec, dict_index_t* index, const rec_t* rec, ulint* offsets, mtr_t* mtr)
{
    ut_ad(rec_offs_validate(rec, index, offsets));
    page_t* page = page_align(current_rec);
    ut_ad(dict_table_is_comp(index->table) == (ibool) !!page_is_comp(page));
    ut_ad(!page_rec_is_supremum(current_rec));
    // 1. Get the size of the physical record in the page
    ulint rec_size = rec_offs_size(offsets);
#ifdef IB_DEBUG_VALGRIND
    {
        const void* rec_start = rec - rec_offs_extra_size(offsets);
        ulint extra_size = rec_offs_extra_size(offsets) - (rec_offs_comp(offsets) ? REC_N_NEW_EXTRA_BYTES : REC_N_OLD_EXTRA_BYTES);
        // All data bytes of the record must be valid. The variable-length header must be valid.
        IB_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
        IB_MEM_ASSERT_RW(rec_start, extra_size);
    }
#endif //  IB_DEBUG_VALGRIND 
    // 2. Try to find suitable space from page memory management
    rec_t* free_rec = page_header_get_ptr(page, PAGE_FREE);
    if (IB_LIKELY_NULL(free_rec)) {
        // Try to allocate from the head of the free list.
        ulint foffsets_[REC_OFFS_NORMAL_SIZE];
        ulint* foffsets = foffsets_;
        mem_heap_t* heap = NULL;
        rec_offs_init(foffsets_);
        foffsets = rec_get_offsets(free_rec, index, foffsets, ULINT_UNDEFINED, &heap);
        if (rec_offs_size(foffsets) < rec_size) {
            if (IB_LIKELY_NULL(heap)) {
                IB_MEM_HEAP_FREE(heap);
            }
            goto use_heap;
        }
        ulint heap_no;
        byte* insert_buf = free_rec - rec_offs_extra_size(foffsets);
        if (page_is_comp(page)) {
            heap_no = rec_get_heap_no_new(free_rec);
            page_mem_alloc_free(page, NULL, rec_get_next_ptr(free_rec, TRUE), rec_size);
        } else {
            heap_no = rec_get_heap_no_old(free_rec);
            page_mem_alloc_free(page, NULL, rec_get_next_ptr(free_rec, FALSE), rec_size);
        }
        if (IB_LIKELY_NULL(heap)) {
            IB_MEM_HEAP_FREE(heap);
        }
    } else {
use_heap:
        free_rec = NULL;
        insert_buf = page_mem_alloc_heap(page, NULL, rec_size, &heap_no);
        if (IB_UNLIKELY(insert_buf == NULL)) {
            return NULL;
        }
    }
    // 3. Create the record
    rec_t* insert_rec = rec_copy(insert_buf, rec, offsets);
    rec_offs_make_valid(insert_rec, index, offsets);

    // 4. Insert the record in the linked list of records
    ut_ad(current_rec != insert_rec);

    {
        // next record after current before the insertion
        rec_t* next_rec = page_rec_get_next(current_rec);
#ifdef IB_DEBUG
        if (page_is_comp(page)) {
            ut_ad(rec_get_status(current_rec) <= REC_STATUS_INFIMUM);
            ut_ad(rec_get_status(insert_rec) < REC_STATUS_INFIMUM);
            ut_ad(rec_get_status(next_rec) != REC_STATUS_INFIMUM);
        }
#endif
        page_rec_set_next(insert_rec, next_rec);
        page_rec_set_next(current_rec, insert_rec);
    }

    page_header_set_field(page, NULL, PAGE_N_RECS, 1 + page_get_n_recs(page));
    // 5. Set the n_owned field in the inserted record to zero, and set the heap_no field
    if (page_is_comp(page)) {
        rec_set_n_owned_new(insert_rec, NULL, 0);
        rec_set_heap_no_new(insert_rec, heap_no);
    } else {
        rec_set_n_owned_old(insert_rec, 0);
        rec_set_heap_no_old(insert_rec, heap_no);
    }

    IB_MEM_ASSERT_RW(rec_get_start(insert_rec, offsets), rec_offs_size(offsets));
    // 6. Update the last insertion info in page header

    rec_t* last_insert = page_header_get_ptr(page, PAGE_LAST_INSERT);
    ut_ad(!last_insert || !page_is_comp(page) || rec_get_node_ptr_flag(last_insert) == rec_get_node_ptr_flag(insert_rec));

    if (IB_UNLIKELY(last_insert == NULL)) {
        page_header_set_field(page, NULL, PAGE_DIRECTION, PAGE_NO_DIRECTION);
        page_header_set_field(page, NULL, PAGE_N_DIRECTION, 0);
    } else if ((last_insert == current_rec) && (page_header_get_field(page, PAGE_DIRECTION) != PAGE_LEFT)) {
        page_header_set_field(page, NULL, PAGE_DIRECTION, PAGE_RIGHT);
        page_header_set_field(page, NULL, PAGE_N_DIRECTION, page_header_get_field(page, PAGE_N_DIRECTION) + 1);
    } else if ((page_rec_get_next(insert_rec) == last_insert) && (page_header_get_field(page, PAGE_DIRECTION) != PAGE_RIGHT)) {
        page_header_set_field(page, NULL, PAGE_DIRECTION, PAGE_LEFT);
        page_header_set_field(page, NULL, PAGE_N_DIRECTION, page_header_get_field(page, PAGE_N_DIRECTION) + 1);
    } else {
        page_header_set_field(page, NULL, PAGE_DIRECTION, PAGE_NO_DIRECTION);
        page_header_set_field(page, NULL, PAGE_N_DIRECTION, 0);
    }

    page_header_set_ptr(page, NULL, PAGE_LAST_INSERT, insert_rec);

    // 7. It remains to update the owner record.
    {
        rec_t* owner_rec = page_rec_find_owner_rec(insert_rec);
        ulint n_owned;
        if (page_is_comp(page)) {
            n_owned = rec_get_n_owned_new(owner_rec);
            rec_set_n_owned_new(owner_rec, NULL, n_owned + 1);
        } else {
            n_owned = rec_get_n_owned_old(owner_rec);
            rec_set_n_owned_old(owner_rec, n_owned + 1);
        }

        // 8. Now we have incremented the n_owned field of the owner record. If the number exceeds PAGE_DIR_SLOT_MAX_N_OWNED, we have to split the corresponding directory slot in two.

        if (IB_UNLIKELY(n_owned == PAGE_DIR_SLOT_MAX_N_OWNED)) {
            page_dir_split_slot(page, NULL, page_dir_find_owner_slot(owner_rec));
        }
    }

    // 9. Write log record of the insert
    if (IB_LIKELY(mtr != NULL)) {
        page_cur_insert_rec_write_log(insert_rec, rec_size, current_rec, index, mtr);
    }

    return insert_rec;
}

/// \brief Recompress or reorganize and recompress the page for insert on compressed pages
/// \param [in,out] current_rec pointer to current record after which the new record is inserted
/// \param [in] block buffer block
/// \param [in] index record descriptor
/// \param [in] rec inserted record
/// \param [in] page uncompressed page
/// \param [in] page_zip compressed page
/// \param [in] mtr mini-transaction, or NULL
/// \return pointer to inserted record, or NULL on failure
static rec_t* page_cur_insert_rec_zip_reorg(rec_t** current_rec, buf_block_t* block, dict_index_t* index, rec_t* rec, page_t* page, page_zip_des_t* page_zip, mtr_t* mtr)
{
    // Recompress or reorganize and recompress the page.
    if (IB_LIKELY(page_zip_compress(page_zip, page, index, mtr))) {
        return rec;
    }

    // Before trying to reorganize the page, store the number of preceding records on the page.
    ulint pos = page_rec_get_n_recs_before(rec);

    if (page_zip_reorganize(block, index, mtr)) {
        /* The page was reorganized: Find rec by seeking to pos,
        and update *current_rec. */
        rec = page + PAGE_NEW_INFIMUM;

        while (--pos) {
            rec = page + rec_get_next_offs(rec, TRUE);
        }

        *current_rec = rec;
        rec = page + rec_get_next_offs(rec, TRUE);

        return rec;
    }

    // Out of space: restore the page
    if (!page_zip_decompress(page_zip, page, FALSE)) {
        UT_ERROR; // Memory corrupted?
    }
    ut_ad(page_validate(page, index));
    return NULL;
}

/// \brief Insert a record into a compressed page, reorganizing if needed
/// \param [in,out] current_rec pointer to current record after which the new record is inserted
/// \param [in] block buffer block
/// \param [in] index record descriptor
/// \param [in] rec inserted record
/// \param [in] offsets offsets of the inserted record
/// \param [in] mtr mini-transaction, or NULL
/// \return pointer to inserted record, or NULL on failure
IB_INTERN rec_t* page_cur_insert_rec_zip(rec_t** current_rec, buf_block_t* block, dict_index_t* index, const rec_t* rec, ulint* offsets, mtr_t* mtr)
{
    page_zip_des_t* page_zip = buf_block_get_page_zip(block);
    ut_ad(page_zip);
    ut_ad(rec_offs_validate(rec, index, offsets));
    page_t* page = page_align(*current_rec);
    ut_ad(dict_table_is_comp(index->table));
    ut_ad(page_is_comp(page));
    ut_ad(!page_rec_is_supremum(*current_rec));

#ifdef IB_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page));
#endif // IB_ZIP_DEBUG

    // 1. Get the size of the physical record in the page
    ulint rec_size = rec_offs_size(offsets);
#ifdef IB_DEBUG_VALGRIND
    {
        const void* rec_start = rec - rec_offs_extra_size(offsets);
        ulint extra_size = rec_offs_extra_size(offsets) - (rec_offs_comp(offsets) ? REC_N_NEW_EXTRA_BYTES : REC_N_OLD_EXTRA_BYTES);

        // All data bytes of the record must be valid.
        IB_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
        // The variable-length header must be valid.
        IB_MEM_ASSERT_RW(rec_start, extra_size);
    }
#endif // IB_DEBUG_VALGRIND

    // 2. Try to find suitable space from page memory management
    if (!page_zip_available(page_zip, dict_index_is_clust(index), rec_size, 1)) {
        // Try compressing the whole page afterwards.
        rec_t* insert_rec = page_cur_insert_rec_low(*current_rec, index, rec, offsets, NULL);
        if (IB_LIKELY(insert_rec != NULL)) {
            insert_rec = page_cur_insert_rec_zip_reorg(current_rec, block, index, insert_rec, page, page_zip, mtr);
        }
        return insert_rec;
    }

    rec_t* free_rec = page_header_get_ptr(page, PAGE_FREE);
    if (IB_LIKELY_NULL(free_rec)) {
        // Try to allocate from the head of the free list.
        ulint foffsets_[REC_OFFS_NORMAL_SIZE];
        ulint* foffsets = foffsets_;
        mem_heap_t* heap = NULL;

        rec_offs_init(foffsets_);

        foffsets = rec_get_offsets(free_rec, index, foffsets, ULINT_UNDEFINED, &heap);
        if (rec_offs_size(foffsets) < rec_size) {
too_small:
            if (IB_LIKELY_NULL(heap)) {
                IB_MEM_HEAP_FREE(heap);
            }
            goto use_heap;
        }

        byte* insert_buf = free_rec - rec_offs_extra_size(foffsets);
        // On compressed pages, do not relocate records from the free list. If extra_size would grow, use the heap.
        lint extra_size_diff = rec_offs_extra_size(offsets) - rec_offs_extra_size(foffsets);
        if (IB_UNLIKELY(extra_size_diff < 0)) {
            // Add an offset to the extra_size.
            if (rec_offs_size(foffsets) < rec_size - extra_size_diff) {
                goto too_small;
            }
            insert_buf -= extra_size_diff;
        } else if (IB_UNLIKELY(extra_size_diff)) {
            // Do not allow extra_size to grow
            goto too_small;
        }

        ulint heap_no = rec_get_heap_no_new(free_rec);
        page_mem_alloc_free(page, page_zip, rec_get_next_ptr(free_rec, TRUE), rec_size);

        if (!page_is_leaf(page)) {
            // Zero out the node pointer of free_rec, in case it will not be overwritten by insert_rec.
            ut_ad(rec_size > REC_NODE_PTR_SIZE);
            if (rec_offs_extra_size(foffsets) + rec_offs_data_size(foffsets) > rec_size) {
                memset(rec_get_end(free_rec, foffsets) - REC_NODE_PTR_SIZE, 0, REC_NODE_PTR_SIZE);
            }
        } else if (dict_index_is_clust(index)) {
            // Zero out the DB_TRX_ID and DB_ROLL_PTR columns of free_rec, in case it will not be overwritten by insert_rec.

            ulint trx_id_col = dict_index_get_sys_col_pos(index, DATA_TRX_ID);
            ut_ad(trx_id_col > 0);
            ut_ad(trx_id_col != ULINT_UNDEFINED);

            ulint len;
            ulint trx_id_offs = rec_get_nth_field_offs(foffsets, trx_id_col, &len);
            ut_ad(len == DATA_TRX_ID_LEN);

            if (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN + trx_id_offs + rec_offs_extra_size(foffsets) > rec_size) {
                // We will have to zero out the DB_TRX_ID and DB_ROLL_PTR, because they will not be fully overwritten by insert_rec.
                memset(free_rec + trx_id_offs, 0, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
            }

            ut_ad(free_rec + trx_id_offs + DATA_TRX_ID_LEN == rec_get_nth_field(free_rec, foffsets, trx_id_col + 1, &len));
            ut_ad(len == DATA_ROLL_PTR_LEN);
        }

        if (IB_LIKELY_NULL(heap)) {
            IB_MEM_HEAP_FREE(heap);
        }
    } else {
use_heap:
        free_rec = NULL;
        insert_buf = page_mem_alloc_heap(page, page_zip, rec_size, &heap_no);

        if (IB_UNLIKELY(insert_buf == NULL)) {
            return NULL;
        }

        page_zip_dir_add_slot(page_zip, dict_index_is_clust(index));
    }

    // 3. Create the record
    insert_rec = rec_copy(insert_buf, rec, offsets);
    rec_offs_make_valid(insert_rec, index, offsets);

    // 4. Insert the record in the linked list of records
    ut_ad(*current_rec != insert_rec);

    {
        // next record after current before the insertion */
        rec_t* next_rec = page_rec_get_next(*current_rec);
        ut_ad(rec_get_status(*current_rec) <= REC_STATUS_INFIMUM);
        ut_ad(rec_get_status(insert_rec) < REC_STATUS_INFIMUM);
        ut_ad(rec_get_status(next_rec) != REC_STATUS_INFIMUM);

        page_rec_set_next(insert_rec, next_rec);
        page_rec_set_next(*current_rec, insert_rec);
    }

    page_header_set_field(page, page_zip, PAGE_N_RECS, 1 + page_get_n_recs(page));

    // 5. Set the n_owned field in the inserted record to zero, and set the heap_no field
    rec_set_n_owned_new(insert_rec, NULL, 0);
    rec_set_heap_no_new(insert_rec, heap_no);
    IB_MEM_ASSERT_RW(rec_get_start(insert_rec, offsets), rec_offs_size(offsets));
    page_zip_dir_insert(page_zip, *current_rec, free_rec, insert_rec);

    // 6. Update the last insertion info in page header

    rec_t* last_insert = page_header_get_ptr(page, PAGE_LAST_INSERT);
    ut_ad(!last_insert || rec_get_node_ptr_flag(last_insert) == rec_get_node_ptr_flag(insert_rec));

    if (IB_UNLIKELY(last_insert == NULL)) {
        page_header_set_field(page, page_zip, PAGE_DIRECTION, PAGE_NO_DIRECTION);
        page_header_set_field(page, page_zip, PAGE_N_DIRECTION, 0);
    } else if ((last_insert == *current_rec) && (page_header_get_field(page, PAGE_DIRECTION) != PAGE_LEFT)) {
        page_header_set_field(page, page_zip, PAGE_DIRECTION, PAGE_RIGHT);
        page_header_set_field(page, page_zip, PAGE_N_DIRECTION, page_header_get_field(page, PAGE_N_DIRECTION) + 1);
    } else if ((page_rec_get_next(insert_rec) == last_insert) && (page_header_get_field(page, PAGE_DIRECTION) != PAGE_RIGHT)) {
        page_header_set_field(page, page_zip, PAGE_DIRECTION, PAGE_LEFT);
        page_header_set_field(page, page_zip, PAGE_N_DIRECTION, page_header_get_field(page, PAGE_N_DIRECTION) + 1);
    } else {
        page_header_set_field(page, page_zip, PAGE_DIRECTION, PAGE_NO_DIRECTION);
        page_header_set_field(page, page_zip, PAGE_N_DIRECTION, 0);
    }

    page_header_set_ptr(page, page_zip, PAGE_LAST_INSERT, insert_rec);

    // 7. It remains to update the owner record.
    {
        rec_t* owner_rec = page_rec_find_owner_rec(insert_rec);
        ulint n_owned;

        n_owned = rec_get_n_owned_new(owner_rec);
        rec_set_n_owned_new(owner_rec, page_zip, n_owned + 1);

        // 8. Now we have incremented the n_owned field of the owner record. If the number exceeds PAGE_DIR_SLOT_MAX_N_OWNED, we have to split the corresponding directory slot in two.
        if (IB_UNLIKELY(n_owned == PAGE_DIR_SLOT_MAX_N_OWNED)) {
            page_dir_split_slot( page, page_zip, page_dir_find_owner_slot(owner_rec));
        }
    }
    page_zip_write_rec(page_zip, insert_rec, index, offsets, 1);

    // 9. Write log record of the insert
    if (IB_LIKELY(mtr != NULL)) {
        page_cur_insert_rec_write_log(insert_rec, rec_size, *current_rec, index, mtr);
    }

    return insert_rec;
}

#ifndef IB_HOTBACKUP
/// \brief Write redo log for copying record list to a created page
/// \param [in] page index page
/// \param [in] index record descriptor
/// \param [in] mtr mini-transaction
/// \return pointer to log buffer
IB_INLINE byte* page_copy_rec_list_to_created_page_write_log(page_t* page, dict_index_t* index, mtr_t* mtr)
{
    byte* log_ptr;
    ut_ad(!!page_is_comp(page) == dict_table_is_comp(index->table));
    log_ptr = mlog_open_and_write_index(mtr, page, index, page_is_comp(page) ? MLOG_COMP_LIST_END_COPY_CREATED : MLOG_LIST_END_COPY_CREATED, 4);
    if (IB_LIKELY(log_ptr != NULL)) {
        mlog_close(mtr, log_ptr + 4);
    }
    return log_ptr;
}
#endif /* !IB_HOTBACKUP */

/// \brief Parse redo log for copying a record list to a created page
/// \param [in] ptr buffer pointer
/// \param [in] end_ptr buffer end pointer
/// \param [in] block page or NULL
/// \param [in] index record descriptor
/// \param [in] mtr mini-transaction or NULL
/// \return pointer to end of parsed data, or NULL on error
IB_INTERN byte* page_parse_copy_rec_list_to_created_page(byte* ptr, byte* end_ptr, buf_block_t* block, dict_index_t* index, mtr_t* mtr)
{
    if (ptr + 4 > end_ptr) {
        return NULL;
    }
    ulint log_data_len = mach_read_from_4(ptr);
    ptr += 4;
    byte* rec_end = ptr + log_data_len;
    if (rec_end > end_ptr) {
        return NULL;
    }
    if (!block) {
        return rec_end;
    }
    while (ptr < rec_end) {
        ptr = page_cur_parse_insert_rec(TRUE, ptr, end_ptr, block, index, mtr);
    }
    ut_a(ptr == rec_end);

    page_t* page = buf_block_get_frame(block);
    page_zip_des_t* page_zip = buf_block_get_page_zip(block);
    page_header_set_ptr(page, page_zip, PAGE_LAST_INSERT, NULL);
    page_header_set_field(page, page_zip, PAGE_DIRECTION, PAGE_NO_DIRECTION);
    page_header_set_field(page, page_zip, PAGE_N_DIRECTION, 0);

    return rec_end;
}

#ifndef IB_HOTBACKUP

IB_INTERN void page_copy_rec_list_end_to_created_page(
    page_t*        new_page,    /*!< in/out: index page to copy to */
    rec_t*        rec,        /*!< in: first record to copy */
    dict_index_t*    index,        /*!< in: record descriptor */
    mtr_t*        mtr)        /*!< in: mtr */
{
    mem_heap_t* heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    ulint* offsets = offsets_;
    rec_offs_init(offsets_);

    ut_ad(page_dir_get_n_heap(new_page) == PAGE_HEAP_NO_USER_LOW);
    ut_ad(page_align(rec) != new_page);
    ut_ad(page_rec_is_comp(rec) == page_is_comp(new_page));

    if (page_rec_is_infimum(rec)) {
        rec = page_rec_get_next(rec);
    }
    if (page_rec_is_supremum(rec)) {
        return;
    }

#ifdef IB_DEBUG
    // To pass the debug tests we have to set these dummy values in the debug version
    page_dir_set_n_slots(new_page, NULL, IB_PAGE_SIZE / 2);
    page_header_set_ptr(new_page, NULL, PAGE_HEAP_TOP, new_page + IB_PAGE_SIZE - 1);
#endif

    byte* log_ptr = page_copy_rec_list_to_created_page_write_log(new_page, index, mtr);
    ulint log_data_len = dyn_array_get_data_size(&(mtr->log));
    // Individual inserts are logged in a shorter form
    ulint log_mode = mtr_set_log_mode(mtr, MTR_LOG_SHORT_INSERTS);

    rec_t* prev_rec = page_get_infimum_rec(new_page);
    byte* heap_top;
    if (page_is_comp(new_page)) {
        heap_top = new_page + PAGE_NEW_SUPREMUM_END;
    } else {
        heap_top = new_page + PAGE_OLD_SUPREMUM_END;
    }
    ulint count = 0;
    ulint slot_index = 0;
    ulint n_recs = 0;
    page_dir_slot_t* slot;

    do {
        offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);
        rec_t* insert_rec = rec_copy(heap_top, rec, offsets);

        if (page_is_comp(new_page)) {
            rec_set_next_offs_new(prev_rec, page_offset(insert_rec));
            rec_set_n_owned_new(insert_rec, NULL, 0);
            rec_set_heap_no_new(insert_rec, PAGE_HEAP_NO_USER_LOW + n_recs);
        } else {
            rec_set_next_offs_old(prev_rec, page_offset(insert_rec));
            rec_set_n_owned_old(insert_rec, 0);
            rec_set_heap_no_old(insert_rec, PAGE_HEAP_NO_USER_LOW + n_recs);
        }

        count++;
        n_recs++;
        if (IB_UNLIKELY(count == (PAGE_DIR_SLOT_MAX_N_OWNED + 1) / 2)) {
            slot_index++;
            slot = page_dir_get_nth_slot(new_page, slot_index);
            page_dir_slot_set_rec(slot, insert_rec);
            page_dir_slot_set_n_owned(slot, NULL, count);
            count = 0;
        }
        ulint rec_size = rec_offs_size(offsets);
        ut_ad(heap_top < new_page + IB_PAGE_SIZE);
        heap_top += rec_size;
        page_cur_insert_rec_write_log(insert_rec, rec_size, prev_rec, index, mtr);
        prev_rec = insert_rec;
        rec = page_rec_get_next(rec);
    } while (!page_rec_is_supremum(rec));

    if ((slot_index > 0) && (count + 1 + (PAGE_DIR_SLOT_MAX_N_OWNED + 1) / 2 <= PAGE_DIR_SLOT_MAX_N_OWNED)) {
        // We can merge the two last dir slots. This operation is here to make this function imitate exactly the equivalent task made using page_cur_insert_rec, which we use in database recovery to reproduce the task performed by this function. To be able to check the correctness of recovery, it is good that it imitates exactly.

        count += (PAGE_DIR_SLOT_MAX_N_OWNED + 1) / 2;

        page_dir_slot_set_n_owned(slot, NULL, 0);

        slot_index--;
    }

    if (IB_LIKELY_NULL(heap)) {
        IB_MEM_HEAP_FREE(heap);
    }

    log_data_len = dyn_array_get_data_size(&(mtr->log)) - log_data_len;

    ut_a(log_data_len < 100 * IB_PAGE_SIZE);

    if (IB_LIKELY(log_ptr != NULL)) {
        mach_write_to_4(log_ptr, log_data_len);
    }

    if (page_is_comp(new_page)) {
        rec_set_next_offs_new(insert_rec, PAGE_NEW_SUPREMUM);
    } else {
        rec_set_next_offs_old(insert_rec, PAGE_OLD_SUPREMUM);
    }

    slot = page_dir_get_nth_slot(new_page, 1 + slot_index);

    page_dir_slot_set_rec(slot, page_get_supremum_rec(new_page));
    page_dir_slot_set_n_owned(slot, NULL, count + 1);

    page_dir_set_n_slots(new_page, NULL, 2 + slot_index);
    page_header_set_ptr(new_page, NULL, PAGE_HEAP_TOP, heap_top);
    page_dir_set_n_heap(new_page, NULL, PAGE_HEAP_NO_USER_LOW + n_recs);
    page_header_set_field(new_page, NULL, PAGE_N_RECS, n_recs);

    page_header_set_ptr(new_page, NULL, PAGE_LAST_INSERT, NULL);
    page_header_set_field(new_page, NULL, PAGE_DIRECTION, PAGE_NO_DIRECTION);
    page_header_set_field(new_page, NULL, PAGE_N_DIRECTION, 0);

    // Restore the log mode

    mtr_set_log_mode(mtr, log_mode);
}

/// \brief Write redo log record for a record delete
/// \param [in] rec record to be deleted
/// \param [in] index record descriptor
/// \param [in] mtr mini-transaction handle
IB_INLINE void page_cur_delete_rec_write_log(rec_t* rec, dict_index_t* index, mtr_t* mtr)
{
    byte* log_ptr;
    ut_ad(!!page_rec_is_comp(rec) == dict_table_is_comp(index->table));
    log_ptr = mlog_open_and_write_index(mtr, rec, index, page_rec_is_comp(rec) ? MLOG_COMP_REC_DELETE : MLOG_REC_DELETE, 2);
    if (!log_ptr) {
        // Logging in mtr is switched off during crash recovery: in that case mlog_open returns NULL
        return;
    }
    // Write the cursor rec offset as a 2-byte ulint
    mach_write_to_2(log_ptr, page_offset(rec));
    mlog_close(mtr, log_ptr + 2);
}
#else // !IB_HOTBACKUP

    #define page_cur_delete_rec_write_log(rec,index,mtr) ((void) 0)
#endif // !IB_HOTBACKUP

/// \brief Parse redo log for a record delete on a page
/// \param [in] ptr buffer pointer
/// \param [in] end_ptr buffer end
/// \param [in] block page or NULL
/// \param [in] index record descriptor
/// \param [in] mtr mini-transaction or NULL
/// \return pointer to end of parsed data, or NULL on error
IB_INTERN byte* page_cur_parse_delete_rec(byte* ptr, byte* end_ptr, buf_block_t* block, dict_index_t* index, mtr_t* mtr)
{
    ulint offset;
    page_cur_t cursor;

    if (end_ptr < ptr + 2) {
        return NULL;
    }

    // Read the cursor rec offset as a 2-byte ulint
    offset = mach_read_from_2(ptr);
    ptr += 2;

    ut_a(offset <= IB_PAGE_SIZE);

    if (block) {
        page_t* page = buf_block_get_frame(block);
        mem_heap_t* heap = NULL;
        ulint offsets_[REC_OFFS_NORMAL_SIZE];
        rec_t* rec = page + offset;
        rec_offs_init(offsets_);

        page_cur_position(rec, block, &cursor);
        ut_ad(!buf_block_get_page_zip(block) || page_is_comp(page));

        page_cur_delete_rec(&cursor, index, rec_get_offsets(rec, index, offsets_, ULINT_UNDEFINED, &heap), mtr);
        if (IB_LIKELY_NULL(heap)) {
            IB_MEM_HEAP_FREE(heap);
        }
    }

    return ptr;
}

/// \brief Delete the record at the cursor and update page directory/ownership
/// \param [in,out] cursor a page cursor
/// \param [in] index record descriptor
/// \param [in] offsets rec_get_offsets(cursor->rec, index)
/// \param [in] mtr mini-transaction handle
IB_INTERN void page_cur_delete_rec(page_cur_t* cursor, dict_index_t* index, const ulint* offsets, mtr_t* mtr)
{
    ut_ad(cursor && mtr);
    page_t* page = page_cur_get_page(cursor);
    page_zip_des_t* page_zip = page_cur_get_page_zip(cursor);
    // page_zip_validate() will fail here when btr_cur_pessimistic_delete() invokes btr_set_min_rec_mark(). Then, both "page_zip" and "page" would have the min-rec-mark set on the smallest user record, but "page" would additionally have it set on the smallest-but-one record. Because sloppy page_zip_validate_low() only ignores min-rec-flag differences in the smallest user record, it cannot be used here either.
    rec_t* current_rec = cursor->rec;
    ut_ad(rec_offs_validate(current_rec, index, offsets));
    ut_ad(!!page_is_comp(page) == dict_table_is_comp(index->table));
    // The record must not be the supremum or infimum record.
    ut_ad(page_rec_is_user_rec(current_rec));
    // Save to local variables some data associated with current_rec
    ulint cur_slot_no = page_dir_find_owner_slot(current_rec);
    page_dir_slot_t* cur_dir_slot = page_dir_get_nth_slot(page, cur_slot_no);
    ulint cur_n_owned = page_dir_slot_get_n_owned(cur_dir_slot);
    // 0. Write the log record
    page_cur_delete_rec_write_log(current_rec, index, mtr);
    // 1. Reset the last insert info in the page header and increment the modify clock for the frame
    page_header_set_ptr(page, page_zip, PAGE_LAST_INSERT, NULL);
    // The page gets invalid for optimistic searches: increment the frame modify clock
    buf_block_modify_clock_inc(page_cur_get_block(cursor));
    // 2. Find the next and the previous record. Note that the cursor is left at the next record.
    ut_ad(cur_slot_no > 0);
    page_dir_slot_t* prev_slot = page_dir_get_nth_slot(page, cur_slot_no - 1);
    rec_t* rec = (rec_t*) page_dir_slot_get_rec(prev_slot);
    // rec now points to the record of the previous directory slot. Look for the immediate predecessor of current_rec in a loop.
    rec_t* prev_rec = NULL;
    while (current_rec != rec) {
        prev_rec = rec;
        rec = page_rec_get_next(rec);
    }
    page_cur_move_to_next(cursor);
    rec_t* next_rec = cursor->rec;
    // 3. Remove the record from the linked list of records
    page_rec_set_next(prev_rec, next_rec);
    // 4. If the deleted record is pointed to by a dir slot, update the record pointer in slot. In the following if-clause we assume that prev_rec is owned by the same slot, i.e., PAGE_DIR_SLOT_MIN_N_OWNED >= 2.
#if PAGE_DIR_SLOT_MIN_N_OWNED < 2
# error "PAGE_DIR_SLOT_MIN_N_OWNED < 2"
#endif
    ut_ad(cur_n_owned > 1);
    if (current_rec == page_dir_slot_get_rec(cur_dir_slot)) {
        page_dir_slot_set_rec(cur_dir_slot, prev_rec);
    }
    // 5. Update the number of owned records of the slot
    page_dir_slot_set_n_owned(cur_dir_slot, page_zip, cur_n_owned - 1);
    // 6. Free the memory occupied by the record
    page_mem_free(page, page_zip, current_rec, index, offsets);
    // 7. Now we have decremented the number of owned records of the slot. If the number drops below PAGE_DIR_SLOT_MIN_N_OWNED, we balance the slots.
    if (IB_UNLIKELY(cur_n_owned <= PAGE_DIR_SLOT_MIN_N_OWNED)) {
        page_dir_balance_slot(page, page_zip, cur_slot_no);
    }
#ifdef IB_ZIP_DEBUG
    ut_a(!page_zip || page_zip_validate(page_zip, page));
#endif /* IB_ZIP_DEBUG */
}

#ifdef IB_COMPILE_TEST_FUNCS

/// \brief Print the first n numbers, generated by page_cur_lcg_prng() to make sure (visually) that it works properly.
/// \param [in] n print first n numbers
void test_page_cur_lcg_prng(int n)
{
    for (int i = 0; i < n; i++) {
        unsigned long long rnd = page_cur_lcg_prng();
        printf("%llu\t%%2=%llu %%3=%llu %%5=%llu %%7=%llu %%11=%llu\n", rnd, rnd % 2, rnd % 3, rnd % 5, rnd % 7, rnd % 11);
    }
}

#endif // IB_COMPILE_TEST_FUNCS
