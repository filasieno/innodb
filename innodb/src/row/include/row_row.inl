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

/// \file row_row.inl
/// \brief Inline implementations for general row routines
/// \details Originally created on 4/20/1996 by Heikki Tuuri. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "dict_dict.hpp"
#include "rem_rec.hpp"
#include "trx_undo.hpp"

IB_INLINE trx_id_t row_get_rec_trx_id(const rec_t* rec, dict_index_t* dict_index, const ulint* offsets)
{
	ulint offset = dict_index->trx_id_offset;
	ut_ad(dict_index_is_clust(dict_index));
	ut_ad(rec_offs_validate(rec, dict_index, offsets));
	if (!offset) {
		offset = row_get_trx_id_offset(rec, dict_index, offsets);
	}
	return trx_read_trx_id(rec + offset);
}

IB_INLINE roll_ptr_t row_get_rec_roll_ptr(const rec_t* rec, dict_index_t* dict_index, const ulint* offsets)
{
	ulint offset = dict_index->trx_id_offset;
	ut_ad(dict_index_is_clust(dict_index));
	ut_ad(rec_offs_validate(rec, dict_index, offsets));
	if (!offset) {
		offset = row_get_trx_id_offset(rec, dict_index, offsets);
	}
	return trx_read_roll_ptr(rec + offset + DATA_TRX_ID_LEN);
}

IB_INLINE void row_build_row_ref_fast(dtuple_t* ref, const ulint* map, const rec_t* rec, const ulint* offsets)
{	
	const byte*	field;
	ulint len;
	ulint ref_len = dtuple_get_n_fields(ref);
	ut_ad(rec_offs_validate(rec, NULL, offsets));
	ut_ad(!rec_offs_any_extern(offsets));
	for (ulint i = 0; i < ref_len; i++) {
		dfield_t* dfield = dtuple_get_nth_field(ref, i);
		ulint field_no = *(map + i);
		if (field_no != ULINT_UNDEFINED) {
			field = rec_get_nth_field(rec, offsets, field_no, &len);
			dfield_set_data(dfield, field, len);
		}
	}
}
