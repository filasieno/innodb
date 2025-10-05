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

/// @file ut/ut0byte.c
/// @brief Byte utilities
///
/// Created 5/11/1994 Heikki Tuuri

#include "ut_byte.hpp"

#ifdef UNIV_NONINL
#include "ut_byte.inl"
#endif

/// @brief Zero value for a dulint
///
UNIV_INTERN const dulint ut_dulint_zero = { 0, 0 };

/// \brief Maximum value for a dulint
///
UNIV_INTERN const dulint ut_dulint_max = { 0xFFFFFFFFUL, 0xFFFFFFFFUL };

#ifdef notdefined /* unused code */

#include "ut_sort.hpp"

/// \brief Sort function for dulint arrays.
/// \param arr in/out: array to be sorted
/// \param aux_arr in/out: auxiliary array (same size as arr)
/// \param low in: low bound of sort interval, inclusive
/// \param high in: high bound of sort interval, noninclusive
UNIV_INTERN void ut_dulint_sort(dulint *arr, dulint *aux_arr, ulint low, ulint high)
{
	UT_SORT_FUNCTION_BODY(ut_dulint_sort, arr, aux_arr, low, high, ut_dulint_cmp);
}
#endif	  // notdefined
