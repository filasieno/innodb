// Copyright (c) 2006, 2009, Innobase Oy. All Rights Reserved.
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

/// \file row_ext.cpp
/// \brief Caching of externally stored column prefixes
/// \details Originally created by Marko Makela in September 2006. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "row_ext.hpp"

#ifdef IB_DO_NOT_INLINE
#include "row_ext.inl"
#endif

#include "btr_cur.hpp"

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

static void row_ext_cache_fill(row_ext_t* ext, ulint i, ulint zip_size, const dfield_t* dfield);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// Static helper routine definitions
// -----------------------------------------------------------------------------------------

// Fills the column prefix cache of an externally stored column.
static
void
row_ext_cache_fill(
	row_ext_t*	ext,
	ulint		i,
	ulint		zip_size,
	const dfield_t*	dfield)
{
	const byte*	field	= dfield_get_data(dfield);
	ulint		f_len	= dfield_get_len(dfield);
	byte*		buf	= ext->buf + i * REC_MAX_INDEX_COL_LEN;
	ut_ad(i < ext->n_ext);
	ut_ad(dfield_is_ext(dfield));
	ut_a(f_len >= BTR_EXTERN_FIELD_REF_SIZE);

	// The BLOB pointer is not set: we cannot fetch it
	if (IB_UNLIKELY(!memcmp(field_ref_zero, field + f_len - BTR_EXTERN_FIELD_REF_SIZE, BTR_EXTERN_FIELD_REF_SIZE))) {
		ext->len[i] = 0;
	} else {
		// Fetch at most REC_MAX_INDEX_COL_LEN of the column. The column should be non-empty. However, trx_rollback_or_clean_all_recovered() may try to access a half-deleted BLOB if the server previously crashed during the execution of btr_free_externally_stored_field().
		ext->len[i] = btr_copy_externally_stored_field_prefix(buf, REC_MAX_INDEX_COL_LEN, zip_size, field, f_len);
	}
}

IB_INTERN row_ext_t* row_ext_create(ulint n_ext, const ulint* ext, const dtuple_t* tuple, ulint zip_size, mem_heap_t* heap)
{
	row_ext_t*	ret = mem_heap_alloc(heap, (sizeof *ret) + (n_ext - 1) * sizeof ret->len);
	ut_ad(ut_is_2pow(zip_size));
	ut_ad(zip_size <= IB_PAGE_SIZE);
	ret->n_ext = n_ext;
	ret->ext = ext;
	ret->buf = mem_heap_alloc(heap, n_ext * REC_MAX_INDEX_COL_LEN);
#ifdef IB_DEBUG
	memset(ret->buf, 0xaa, n_ext * REC_MAX_INDEX_COL_LEN);
	IB_MEM_ALLOC(ret->buf, n_ext * REC_MAX_INDEX_COL_LEN);
#endif

	// Fetch the BLOB prefixes
	for (ulint i = 0; i < n_ext; i++) {
		const dfield_t*	dfield = dtuple_get_nth_field(tuple, ext[i]);
		row_ext_cache_fill(ret, i, zip_size, dfield);
	}

	return ret;
}
