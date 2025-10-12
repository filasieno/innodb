/*****************************************************************************

Copyright (c) 1994, 2009, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/**************************************************************//**
@file include/ut0byte.ic
Utilities for byte operations

Created 5/30/1994 Heikki Tuuri
*******************************************************************/

IB_INLINE dulint ut_dulint_create(ulint	high, ulint	low)
{
	dulint res;
	ut_ad(high <= 0xFFFFFFFF);
	ut_ad(low <= 0xFFFFFFFF);
	res.high = high;
	res.low	 = low;
	return res;
}

IB_INLINE ulint ut_dulint_get_high(dulint d)
{
	return d.high;
}

IB_INLINE ulint ut_dulint_get_low(dulint d)
{
	return d.low;
}

IB_INLINE ib_int64_t ut_conv_dulint_to_longlong(dulint	d)
{
	return (ib_int64_t)d.low + (((ib_int64_t)d.high) << 32);
}

IB_INLINE ibool ut_dulint_is_zero(dulint a)
{
	if ((a.low == 0) && (a.high == 0)) {
		return TRUE;
	}
	return FALSE ;
}

IB_INLINE int ut_dulint_cmp(dulint a, dulint	b)
{
	if (a.high > b.high) {
		return(1);
	} else if (a.high < b.high) {
		return(-1);
	} else if (a.low > b.low) {
		return(1);
	} else if (a.low < b.low) {
		return(-1);
	} else {
		return(0);
	}
}

IB_INLINE dulint ut_dulint_add(dulint a, ulint b)
{
	if (0xFFFFFFFFUL - b >= a.low) {
		a.low += b;
		return a;
	}
	a.low = a.low - (0xFFFFFFFFUL - b) - 1;
	a.high++;
	return a;
}

IB_INLINE dulint ut_dulint_subtract(dulint a, ulint b)
{
	if (a.low >= b) {
		a.low -= b;
		return a;
	}
	b -= a.low + 1;
	a.low = 0xFFFFFFFFUL - b;
	ut_ad(a.high > 0);
	a.high--;
	return a;
}

IB_INLINE dulint ut_dulint_align_down(dulint n, ulint align_no)
{
	ut_ad(align_no > 0);
	ut_ad(((align_no - 1) & align_no) == 0);
	ulint low = ut_dulint_get_low(n);
	ulint high = ut_dulint_get_high(n);
	low = low & ~(align_no - 1);
	return ut_dulint_create(high, low);
}

IB_INLINE dulint ut_dulint_align_up(dulint n, ulint align_no)
{
	return ut_dulint_align_down(ut_dulint_add(n, align_no - 1), align_no);
}

IB_INLINE ib_uint64_t ut_uint64_align_down(ib_uint64_t n, ulint align_no)
{
	ut_ad(align_no > 0);
	ut_ad(ut_is_2pow(align_no));
	return n & ~((ib_uint64_t) align_no - 1);
}

IB_INLINE ib_uint64_t ut_uint64_align_up(ib_uint64_t n, ulint align_no)
{
	ut_ad(align_no > 0);
	ut_ad(ut_is_2pow(align_no));

	ib_uint64_t	align_1 = (ib_uint64_t) align_no - 1;
	return (n + align_1) & ~align_1;
}

IB_INLINE void* ut_align(const void* ptr, ulint align_no)
{
	ut_ad(align_no > 0);
	ut_ad(((align_no - 1) & align_no) == 0);
	ut_ad(ptr);
	ut_ad(sizeof(void*) == sizeof(ulint));
	return (void*)((((ulint)ptr) + align_no - 1) & ~(align_no - 1));
}

IB_INLINE void* ut_align_down(const void ptr, ulint align_no)
{
	ut_ad(align_no > 0);
	ut_ad(((align_no - 1) & align_no) == 0);
	ut_ad(ptr);
	ut_ad(sizeof(void*) == sizeof(ulint));

	return((void*)((((ulint)ptr)) & ~(align_no - 1)));
}

IB_INLINE ulint ut_align_offset(const void*	ptr, ulint align_no)
{
	ut_ad(align_no > 0);
	ut_ad(((align_no - 1) & align_no) == 0);
	ut_ad(ptr);
	ut_ad(sizeof(void*) == sizeof(ulint));
	return ((ulint)ptr) & (align_no - 1);
}

IB_INLINE ibool ut_bit_get_nth(ulint a, ulint n)
{
	ut_ad(n < 8 * sizeof(ulint));
	return 1 & (a >> n);
}

IB_INLINE ulint ut_bit_set_nth(ulint a, ulint n, ibool val)
{
	ut_ad(n < 8 * sizeof(ulint));
	if (val) {
		return(((ulint) 1 << n) | a);
	} else {
		return(~((ulint) 1 << n) & a);
	}
}
