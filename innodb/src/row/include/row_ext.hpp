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

/// \file row_ext.hpp
/// \brief Caching of externally stored column prefixes
/// \details Originally created by Marko Makela in September 2006. Refactored to modern documentation and style while preserving original authorship information.
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"
#include "row_types.hpp"
#include "data_types.hpp"
#include "mem_mem.hpp"

/// \brief Creates a cache of column prefixes of externally stored columns.
/// \param [in] n_ext number of externally stored columns
/// \param [in] ext col_no's of externally stored columns in the InnoDB table object, as reported by dict_col_get_no(); NOT relative to the records in the clustered index
/// \param [in] tuple data tuple containing the field references of the externally stored columns; must be indexed by col_no; the clustered index record must be covered by a lock or a page latch to prevent deletion (rollback or purge).
/// \param [in] zip_size compressed page size in bytes, or 0
/// \param [in] heap heap where created
/// \return own: column prefix cache
IB_INTERN row_ext_t* row_ext_create(ulint n_ext, const ulint* ext, const dtuple_t* tuple, ulint zip_size, mem_heap_t* heap);

/// \brief Looks up a column prefix of an externally stored column.
/// \param [in] ext column prefix cache
/// \param [in] i index of ext->ext[]
/// \param [out] len length of prefix, in bytes, at most REC_MAX_INDEX_COL_LEN
/// \return column prefix, or NULL if the column is not stored externally, or pointer to field_ref_zero if the BLOB pointer is unset
IB_INLINE const byte* row_ext_lookup_ith(const row_ext_t* ext, ulint i, ulint* len);
/// \brief Looks up a column prefix of an externally stored column.
/// \param [in] ext column prefix cache
/// \param [in] col column number in the InnoDB table object, as reported by dict_col_get_no(); NOT relative to the records in the clustered index
/// \param [out] len length of prefix, in bytes, at most REC_MAX_INDEX_COL_LEN
/// \return column prefix, or NULL if the column is not stored externally, or pointer to field_ref_zero if the BLOB pointer is unset
IB_INLINE const byte* row_ext_lookup(const row_ext_t* ext, ulint col, ulint* len);

/// \struct row_ext_struct Struct for prefixes of externally stored columns
/// \var ulint row_ext_struct::n_ext
/// \brief Number of externally stored columns
/// \var const ulint* row_ext_struct::ext
/// \brief Column numbers of externally stored columns
/// \var byte* row_ext_struct::buf
/// \brief Backing store of the column prefix cache
/// \var ulint row_ext_struct::len[1]
/// \brief Prefix lengths; 0 if not cached

struct row_ext_struct {
	ulint        n_ext;
	const ulint* ext;
	byte*        buf;
	ulint        len[1];
};

#ifndef IB_DO_NOT_INLINE
	#include "row_ext.inl"
#endif

#endif
