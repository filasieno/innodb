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

/// \file row_ext.inl
/// \brief Inline implementations for caching of externally stored column prefixes
/// \details Originally created by Marko Makela in September 2006. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "rem_types.hpp"
#include "btr_types.hpp"

IB_INLINE
const byte*
row_ext_lookup_ith(
	const row_ext_t*	ext,
	ulint			i,
	ulint*			len)
{
	ut_ad(ext);
	ut_ad(len);
	ut_ad(i < ext->n_ext);
	*len = ext->len[i];
	if (IB_UNLIKELY(*len == 0)) {
		// The BLOB could not be fetched to the cache.
		return field_ref_zero;
	} else {
		return ext->buf + i * REC_MAX_INDEX_COL_LEN;
	}
}

IB_INLINE
const byte*
row_ext_lookup(
	const row_ext_t*	ext,
	ulint			col,
	ulint*			len)
{
	ulint	i;
	ut_ad(ext);
	ut_ad(len);
	for (i = 0; i < ext->n_ext; i++) {
		if (col == ext->ext[i]) {
			return row_ext_lookup_ith(ext, i, len);
		}
	}
	return NULL;
}
