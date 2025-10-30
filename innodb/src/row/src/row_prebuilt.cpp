
// Copyright (c) 1997, 2010, Innobase Oy. All Rights Reserved.
// Copyright (c) 2008, Google Inc.
//
// Portions of this file contain modifications contributed and copyrighted by
// Google, Inc. Those modifications are gratefully acknowledged and are described
// briefly in the InnoDB documentation. The contributions by Google are
// incorporated with their permission, and subject to the conditions contained in
// the file COPYING.Google.
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

/// \file row_prebuilt.cpp
/// \brief Row select prebuilt structure function.
/// \details Originally created on 02/03/2009 by Sunny Bains. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "row_ins.hpp"
#include "row_prebuilt.hpp"

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------
#include "pars_pars.hpp"
#include "que_que.hpp"
#include "row_merge.hpp"

IB_INTERN row_prebuilt_t* row_prebuilt_create(dict_table_t* table)
{
    mem_heap_t* heap = IB_MEM_HEAP_CREATE(128);
    row_prebuilt_t* prebuilt = mem_heap_zalloc(heap, sizeof(row_prebuilt_t));
    prebuilt->magic_n = ROW_PREBUILT_ALLOCATED;
    prebuilt->magic_n2 = ROW_PREBUILT_ALLOCATED;
    prebuilt->heap = heap;
    prebuilt->table = table;
    prebuilt->sql_stat_start = TRUE;
    prebuilt->pcur = btr_pcur_create();
    prebuilt->clust_pcur = btr_pcur_create();
    prebuilt->select_lock_type = LOCK_NONE;
    prebuilt->search_tuple = dtuple_create(heap, 2 * dict_table_get_n_cols(table));
    dict_index_t* clust_index = dict_table_get_first_index(table);
    // Make sure that search_tuple is long enough for clustered index
    ut_a(2 * dict_table_get_n_cols(table) >= clust_index->n_fields);
    ulint ref_len = dict_index_get_n_unique(clust_index);
    dtuple_t* ref = dtuple_create(heap, ref_len);
    dict_index_copy_types(ref, clust_index, ref_len);
    prebuilt->clust_ref = ref;
    ib_row_cache_t* row_cache = &prebuilt->row_cache;
    row_cache->n_max = FETCH_CACHE_SIZE;
    row_cache->n_size = row_cache->n_max;
    ulint sz = sizeof(*row_cache->ptr) * row_cache->n_max;
    row_cache->heap = IB_MEM_HEAP_CREATE(sz);
    row_cache->ptr = mem_heap_zalloc(row_cache->heap, sz);
    return prebuilt;
}

IB_INTERN void row_prebuilt_free(row_prebuilt_t* prebuilt, ibool dict_locked)
{
    ulint i;
    if (prebuilt->magic_n != ROW_PREBUILT_ALLOCATED || prebuilt->magic_n2 != ROW_PREBUILT_ALLOCATED) {
        ib_log(state, "InnoDB: Error: trying to free a corrupt\nInnoDB: table handle. Magic n %lu, magic n2 %lu, table name", (ulong) prebuilt->magic_n, (ulong) prebuilt->magic_n2);
        ut_print_name(state->stream, NULL, TRUE, prebuilt->table->name);
        ib_log(state, "\n");
        UT_ERROR;
    }
    prebuilt->magic_n = ROW_PREBUILT_FREED;
    prebuilt->magic_n2 = ROW_PREBUILT_FREED;
    btr_pcur_free(prebuilt->pcur);
    btr_pcur_free(prebuilt->clust_pcur);
    if (prebuilt->sel_graph) {
        que_graph_free_recursive(prebuilt->sel_graph);
    }
    if (prebuilt->old_vers_heap) {
        IB_MEM_HEAP_FREE(prebuilt->old_vers_heap);
    }
    ib_row_cache_t* row_cache = &prebuilt->row_cache;
    for (i = 0; i < row_cache->n_max; i++) {
        ib_cached_row_t* row = &row_cache->ptr[i];
        if (row->ptr != NULL) {
            IB_MEM_FREE(row->ptr);
        }
    }
    IB_MEM_HEAP_FREE(row_cache->heap);
    if (prebuilt->table != NULL) {
        dict_table_decrement_handle_count(prebuilt->table, dict_locked);
    }
    IB_MEM_HEAP_FREE(prebuilt->heap);
}

IB_INTERN void row_prebuilt_reset(row_prebuilt_t* prebuilt)
{
    ut_a(prebuilt->magic_n == ROW_PREBUILT_ALLOCATED);
    ut_a(prebuilt->magic_n2 == ROW_PREBUILT_ALLOCATED);
    prebuilt->sql_stat_start = TRUE;
    prebuilt->client_has_locked = FALSE;
    prebuilt->need_to_access_clustered = FALSE;
    prebuilt->index_usable = FALSE;
    prebuilt->simple_select = FALSE;
    prebuilt->select_lock_type = LOCK_NONE;
    if (prebuilt->old_vers_heap) {
        IB_MEM_HEAP_FREE(prebuilt->old_vers_heap);
        prebuilt->old_vers_heap = NULL;
    }
    prebuilt->trx = NULL;
    if (prebuilt->sel_graph) {
        prebuilt->sel_graph->trx = NULL;
    }
}

IB_INTERN void row_prebuilt_update_trx(row_prebuilt_t* prebuilt, trx_t* trx)
{
    ut_a(trx != NULL);
    if (trx->magic_n != TRX_MAGIC_N) {
        ib_log(state, "InnoDB: Error: trying to use a corrupt\nInnoDB: trx handle. Magic n %lu\n", (ulong) trx->magic_n);
        UT_ERROR;
    } else if (prebuilt->magic_n != ROW_PREBUILT_ALLOCATED) {
        ib_log(state, "InnoDB: Error: trying to use a corrupt\nInnoDB: table handle. Magic n %lu, table name", (ulong) prebuilt->magic_n);
        ut_print_name(state->stream, NULL, TRUE, prebuilt->table->name);
        ib_log(state, "\n");
        UT_ERROR;
    } else {
        prebuilt->trx = trx;
        if (prebuilt->sel_graph) {
            prebuilt->sel_graph->trx = trx;
        }
        prebuilt->index_usable = row_merge_is_index_usable(prebuilt->trx, prebuilt->index);
    }
}
