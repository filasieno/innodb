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

/// \file rem_cmp.inl
/// \brief Comparison services for records
/// \details Originally created by Heikki Tuuri on 7/1/1994
/// \author Fabio N. Filasieno
/// \date 20/10/2025

/// \brief This function is used to compare two data fields for which we know the data type.
/// \param [in] cmp_ctx client compare context
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \param [in] data1 data field (== a pointer to a memory buffer)
/// \param [in] len1 data field length or IB_SQL_NULL
/// \param [in] data2 data field (== a pointer to a memory buffer)
/// \param [in] len2 data field length or IB_SQL_NULL
/// \return 1, 0, -1, if data1 is greater, equal, less than data2, respectively
IB_INLINE int cmp_data_data(void* cmp_ctx, ulint mtype, ulint prtype, const byte* data1, ulint len1, const byte* data2, ulint len2)
{
	return cmp_data_data_slow(cmp_ctx, mtype, prtype, data1, len1, data2, len2);
}

/// \brief This function is used to compare two dfields where at least the first has its data type field set.
/// \param [in] cmp_ctx client compare context
/// \param [in] dfield1 data field; must have type field set
/// \param [in] dfield2 data field
/// \return 1, 0, -1, if dfield1 is greater, equal, less than dfield2, respectively
IB_INLINE int cmp_dfield_dfield(void* cmp_ctx, const dfield_t* dfield1, const dfield_t* dfield2)
{
	const dtype_t* type = dfield_get_type(dfield1);

	ut_ad(dfield_check_typed(dfield1));

	return cmp_data_data(cmp_ctx, type->mtype, type->prtype, (const byte*) dfield_get_data(dfield1), dfield_get_len(dfield1), (const byte*) dfield_get_data(dfield2), dfield_get_len(dfield2));
}

/// \brief This function is used to compare two physical records. Only the common first fields are compared.
/// \param [in] rec1 physical record
/// \param [in] rec2 physical record
/// \param [in] offsets1 rec_get_offsets(rec1, index)
/// \param [in] offsets2 rec_get_offsets(rec2, index)
/// \param [in] dict_index data dictionary index
/// \return 1, 0, -1 if rec1 is greater, equal, less, respectively, than rec2; only the common first fields are compared
IB_INLINE int cmp_rec_rec(const rec_t* rec1, const rec_t* rec2, const ulint* offsets1, const ulint* offsets2, dict_index_t* dict_index)
{
	ulint match_f = 0;
	ulint match_b = 0;
	return cmp_rec_rec_with_match(rec1, rec2, offsets1, offsets2, dict_index, &match_f, &match_b);
}
