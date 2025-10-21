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

/// \file rem_cmp.hpp
/// \brief Comparison services for records
/// \details Originally created by Heikki Tuuri in 7/1/1994
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"
#include "data_data.hpp"
#include "data_type.hpp"
#include "dict_dict.hpp"
#include "rem_rec.hpp"


/// \brief Returns TRUE if two columns are equal for comparison purposes.
/// \param [in] col1 column 1
/// \param [in] col2 column 2
/// \param [in] check_charsets whether to check charsets
/// \return TRUE if the columns are considered equal in comparisons
IB_INTERN ibool cmp_cols_are_equal(const dict_col_t* col1, const dict_col_t* col2, ibool check_charsets);

/// \brief This function is used to compare two data fields for which we know the data type.
/// \param [in] cmp_ctx client compare context
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \param [in] data1 data field (== a pointer to a memory buffer)
/// \param [in] len1 data field length or IB_SQL_NULL
/// \param [in] data2 data field (== a pointer to a memory buffer)
/// \param [in] len2 data field length or IB_SQL_NULL
/// \return 1, 0, -1, if data1 is greater, equal, less than data2, respectively
IB_INLINE int cmp_data_data(void* cmp_ctx, ulint mtype, ulint prtype, const byte* data1, ulint len1, const byte* data2, ulint len2);

/// \brief This function is used to compare two data fields for which we know the data type.
/// \param [in] cmp_ctx client compare context
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \param [in] data1 data field (== a pointer to a memory buffer)
/// \param [in] len1 data field length or IB_SQL_NULL
/// \param [in] data2 data field (== a pointer to a memory buffer)
/// \param [in] len2 data field length or IB_SQL_NULL
/// \return 1, 0, -1, if data1 is greater, equal, less than data2, respectively
IB_INTERN int cmp_data_data_slow(void* cmp_ctx, ulint mtype, ulint prtype, const byte* data1, ulint len1, const byte* data2, ulint len2);

/// \brief This function is used to compare two dfields where at least the first has its data type field set.
/// \param [in] cmp_ctx client compare context
/// \param [in] dfield1 data field; must have type field set
/// \param [in] dfield2 data field
/// \return 1, 0, -1, if dfield1 is greater, equal, less than dfield2, respectively
IB_INLINE int cmp_dfield_dfield(void* cmp_ctx, const dfield_t* dfield1, const dfield_t* dfield2);

/// \brief This function is used to compare a data tuple to a physical record.
/// \param [in] cmp_ctx client compare context
/// \param [in] dtuple data tuple
/// \param [in] rec physical record which differs from dtuple in some of the common fields, or which has an equal number or more fields than dtuple
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in,out] matched_fields number of already completely matched fields; when function returns, contains the value for current comparison
/// \param [in,out] matched_bytes number of already matched bytes within the first field not completely matched; when function returns, contains the value for current comparison
/// \return 1, 0, -1, if dtuple is greater, equal, less than rec, respectively, when only the common first fields are compared, or until the first externally stored field in rec
/// \details Only dtuple->n_fields_cmp first fields are taken into account for the data tuple! If we denote by n = n_fields_cmp, then rec must have either m >= n fields, or it must differ from dtuple in some of the m fields rec has. If rec has an externally stored field we do not compare it but return with value 0 if such a comparison should be made.
IB_INTERN int cmp_dtuple_rec_with_match(void* cmp_ctx, const dtuple_t* dtuple, const rec_t* rec, const ulint* offsets, ulint* matched_fields, ulint* matched_bytes);

/// \brief Compares a data tuple to a physical record.
/// \param [in] cmp_ctx client compare context
/// \param [in] dtuple data tuple
/// \param [in] rec physical record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return 1, 0, -1, if dtuple is greater, equal, less than rec, respectively
/// \see cmp_dtuple_rec_with_match
IB_INTERN int cmp_dtuple_rec(void* cmp_ctx, const dtuple_t* dtuple, const rec_t* rec, const ulint* offsets);

/// \brief Checks if a dtuple is a prefix of a record.
/// \param [in] cmp_ctx client compare context
/// \param [in] dtuple data tuple
/// \param [in] rec physical record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return TRUE if prefix
/// \details The last field in dtuple is allowed to be a prefix of the corresponding field in the record.
IB_INTERN ibool cmp_dtuple_is_prefix_of_rec(void* cmp_ctx, const dtuple_t* dtuple, const rec_t* rec, const ulint* offsets);

/// \brief Compare two physical records that contain the same number of columns, none of which are stored externally.
/// \param [in] rec1 physical record
/// \param [in] rec2 physical record
/// \param [in] offsets1 rec_get_offsets(rec1, ...)
/// \param [in] offsets2 rec_get_offsets(rec2, ...)
/// \param [in] index data dictionary index
/// \return 1, 0, -1 if rec1 is greater, equal, less, respectively, than rec2
IB_INTERN int cmp_rec_rec_simple(const rec_t* rec1, const rec_t* rec2, const ulint* offsets1, const ulint* offsets2, const dict_index_t* index);

/// \brief This function is used to compare two physical records.
/// \param [in] rec1 physical record
/// \param [in] rec2 physical record
/// \param [in] offsets1 rec_get_offsets(rec1, index)
/// \param [in] offsets2 rec_get_offsets(rec2, index)
/// \param [in] index data dictionary index
/// \param [in,out] matched_fields number of already completely matched fields; when the function returns, contains the value the for current comparison
/// \param [in,out] matched_bytes number of already matched bytes within the first field not completely matched; when the function returns, contains the value for the current comparison
/// \return 1, 0, -1 if rec1 is greater, equal, less, respectively
/// \details Only the common first fields are compared, and if an externally stored field is encountered, then 0 is returned.
IB_INTERN int cmp_rec_rec_with_match(const rec_t* rec1, const rec_t* rec2, const ulint* offsets1, const ulint* offsets2, dict_index_t* index, ulint* matched_fields, ulint* matched_bytes);

/// \brief This function is used to compare two physical records.
/// \param [in] rec1 physical record
/// \param [in] rec2 physical record
/// \param [in] offsets1 rec_get_offsets(rec1, index)
/// \param [in] offsets2 rec_get_offsets(rec2, index)
/// \param [in] index data dictionary index
/// \return 1, 0 , -1 if rec1 is greater, equal, less, respectively, than rec2; only the common first fields are compared
/// \details Only the common first fields are compared.
IB_INLINE int cmp_rec_rec(const rec_t* rec1, const rec_t* rec2, const ulint* offsets1, const ulint* offsets2, dict_index_t* index);


#ifndef IB_DO_NOT_INLINE
	#include "rem_cmp.inl"
#endif
