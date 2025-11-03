// Copyright (c) 1994, 2025, Innobase Oy. All Rights Reserved.
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

/// \file ut_byte.hpp
/// \brief Utilities for byte operations
///
/// Created 1/20/1994 Heikki Tuuri



#pragma once

#include "defs.hpp"

/// Type definition for a 64-bit unsigned integer, which works also in 32-bit machines. 
/// NOTE! Access the fields only with the accessor functions. 
/// This definition appears here only for the compiler to know the size of a dulint. 
struct dulint
{
	ulint high;	//!< most significant 32 bits
	ulint low;	//!< least significant 32 bits
};

/// Zero value for a dulint
extern const dulint	ut_dulint_zero;

/// Maximum value for a dulint
extern const dulint	ut_dulint_max;

/// \brief Creates a 64-bit dulint out of two ulints.
/// \param high High-order 32 bits.
/// \param low  Low-order 32 bits.
/// \return Created dulint.
IB_INLINE dulint ut_dulint_create(ulint high, ulint low);

/// \brief Gets the high-order 32 bits of a dulint.
/// \param d Dulint.
/// \return 32 bits in ulint.
IB_INLINE ulint ut_dulint_get_high(dulint d);

/// \brief Gets the low-order 32 bits of a dulint.
/// \param d Dulint.
/// \return 32 bits in ulint.
IB_INLINE ulint ut_dulint_get_low(dulint d);

/// \brief Converts a dulint (a struct of 2 ulints) to ib_int64_t, which is a 64-bit integer type.
/// \param d Dulint.
/// \return Value in ib_int64_t type.
IB_INLINE ib_int64_t ut_conv_dulint_to_longlong(dulint d);

/// \brief Tests if a dulint is zero.
/// \param a Dulint.
/// \return TRUE if zero.
IB_INLINE ibool ut_dulint_is_zero(dulint a);

/// \brief Compares two dulints.
/// \param a Dulint.
/// \param b Dulint.
/// \return -1 if a < b, 0 if a == b, 1 if a > b.
IB_INLINE int ut_dulint_cmp(dulint a, dulint b);

/// \brief Adds a ulint to a dulint.
/// \param a Dulint.
/// \param b Ulint.
/// \return Sum a + b.
IB_INLINE dulint ut_dulint_add(dulint a, ulint b);

/// \brief Subtracts a ulint from a dulint.
/// \param a Dulint.
/// \param b Ulint.
/// \return a - b.
IB_INLINE dulint ut_dulint_subtract(dulint a, ulint b); 

/// \brief Rounds a dulint downward to a multiple of a power of 2.
/// \param [in] n Dulint.
/// \param [in] align_no Align by this number which must be a power of 2.
/// \return Rounded value.
IB_INLINE dulint ut_dulint_align_down(dulint n, ulint align_no);

/// \brief Rounds a dulint upward to a multiple of a power of 2.
/// \param [in] n Dulint.
/// \param [in] align_no Align by this number which must be a power of 2.
/// \return Rounded value.
IB_INLINE dulint ut_dulint_align_up(dulint n, ulint align_no);


/// \brief Rounds a dulint downward to a multiple of a power of 2.
/// \param [in] n Dulint.
/// \param [in] align_no Align by this number which must be a power of 2.
/// \return Rounded value.
IB_INLINE ib_uint64_t ut_uint64_align_down(ib_uint64_t n, ulint align_no);

/// \brief Rounds a dulint upward to a multiple of a power of 2.
/// \param [in] n Dulint.
/// \param [in] align_no Align by this number which must be a power of 2.
/// \return Rounded value.
IB_INLINE ib_uint64_t ut_uint64_align_up(ib_uint64_t n, ulint align_no);

/// \brief Increments a dulint variable by 1.
/// \param [in,out] D Dulint.
/// \return Rounded value.
#define UT_DULINT_INC(D) \
{ \
	if ((D).low == 0xFFFFFFFFUL) {\
		(D).high = (D).high + 1;\
		(D).low = 0;\
	} else {\
		(D).low = (D).low + 1;\
	}\
}

/// \brief Increments a dulint variable by 1.
/// \param [in,out] D Dulint.
/// \return Rounded value.
#define UT_DULINT_INC(D) \
{ \
	if ((D).low == 0xFFFFFFFFUL) {\
		(D).high = (D).high + 1;\
		(D).low = 0;\
	} else {\
		(D).low = (D).low + 1;\
	}\
}

/// \brief Increments a dulint variable by 1.
/// \param [in,out] D Dulint.
/// \return Rounded value.
#define UT_DULINT_INC(D)\
{\
	if ((D).low == 0xFFFFFFFFUL) {\
		(D).high = (D).high + 1;\
		(D).low = 0;\
	} else {\
		(D).low = (D).low + 1;\
	}\
}

/// \brief Tests if two dulints are equal.
/// \param [in] D1 Dulint.
/// \param [in] D2 Dulint.
/// \return TRUE if equal.
#define UT_DULINT_EQ(D1, D2) (((D1).low == (D2).low) && ((D1).high == (D2).high))

#ifdef notdefined

/// \brief Sort function for dulint arrays.
/// \param arr in/out: array to be sorted
/// \param aux_arr in/out: auxiliary array (same size as arr)
/// \param low in: low bound of sort interval, inclusive
/// \param high in: high bound of sort interval, noninclusive
IB_INTERN void ut_dulint_sort(dulint* arr, dulint* aux_arr, ulint low, ulint high);

#endif // notdefined

/// \brief Rounds up a pointer to the nearest aligned address.
/// \param ptr Pointer.
/// \param align_no Align by this number.
/// \return Aligned pointer.
IB_INLINE void* ut_align(const void* ptr, ulint align_no);

/// \brief Rounds down a pointer to the nearest aligned address.
/// \param ptr Pointer.
/// \param align_no Align by this number.
/// \return Aligned pointer.
IB_INLINE void* ut_align_down(const void* ptr, ulint align_no) __attribute__((const));

/// \brief Computes the offset of a pointer from the nearest aligned address.
/// \param [in] ptr Pointer.
/// \param [in] align_no Align by this number.
/// \return Distance from aligned pointer.
IB_INLINE ulint ut_align_offset(const void* ptr, ulint align_no) __attribute__((const));

/// \brief Gets the nth bit of a ulint.
/// \param a Ulint.
/// \param n Nth bit requested.
/// \return TRUE if nth bit is 1; 0th bit is defined to be the least significant */
IB_INLINE ibool ut_bit_get_nth(ulint a, ulint n);

/// \brief Sets the nth bit of a ulint.
/// \param a Ulint.
/// \param n Nth bit requested.
/// \param val Value for the bit to set.
/// \return The ulint with the bit set as requested.
IB_INLINE ulint ut_bit_set_nth(ulint a, ulint	n, ibool val);

#ifndef IB_DO_NOT_INLINE
  #include "ut_byte.inl"
#endif

#endif
