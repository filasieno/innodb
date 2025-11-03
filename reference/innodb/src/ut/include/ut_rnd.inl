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
@file include/ut0rnd.ic
Random numbers and hashing

Created 5/30/1994 Heikki Tuuri
*******************************************************************/

#define UT_HASH_RANDOM_MASK	1463735687
#define UT_HASH_RANDOM_MASK2	1653893711
#define UT_RND1			151117737
#define UT_RND2			119785373
#define UT_RND3			 85689495
#define UT_RND4			 76595339
#define UT_SUM_RND2		 98781234
#define UT_SUM_RND3		126792457
#define UT_SUM_RND4		 63498502
#define UT_XOR_RND1		187678878
#define UT_XOR_RND2		143537923

extern	ulint	 ut_rnd_ulint_counter;

IB_INLINE
void
ut_rnd_set_seed(
/*============*/
	ulint	 seed)
{
	ut_rnd_ulint_counter = seed;
}

IB_INLINE
ulint
ut_rnd_gen_next_ulint(
/*==================*/
	ulint	rnd)
{
	ulint	n_bits;

	n_bits = 8 * sizeof(ulint);

	rnd = UT_RND2 * rnd + UT_SUM_RND3;
	rnd = UT_XOR_RND1 ^ rnd;
	rnd = (rnd << 20) + (rnd >> (n_bits - 20));
	rnd = UT_RND3 * rnd + UT_SUM_RND4;
	rnd = UT_XOR_RND2 ^ rnd;
	rnd = (rnd << 20) + (rnd >> (n_bits - 20));
	rnd = UT_RND1 * rnd + UT_SUM_RND2;

	return(rnd);
}

IB_INLINE
ulint
ut_rnd_gen_ulint(void)
/*==================*/
{
	ulint	rnd;
	ulint	n_bits;

	n_bits = 8 * sizeof(ulint);

	ut_rnd_ulint_counter = UT_RND1 * ut_rnd_ulint_counter + UT_RND2;

	rnd = ut_rnd_gen_next_ulint(ut_rnd_ulint_counter);

	return(rnd);
}

IB_INLINE
ulint
ut_rnd_interval(
/*============*/
	ulint	low,
	ulint	high)
{
	ulint	rnd;

	ut_ad(high >= low);

	if (low == high) {

		return(low);
	}

	rnd = ut_rnd_gen_ulint();

	return(low + (rnd % (high - low + 1)));
}

IB_INLINE
ibool
ut_rnd_gen_ibool(void)
/*=================*/
{
	ulint	 x;

	x = ut_rnd_gen_ulint();

	if (((x >> 20) + (x >> 15)) & 1) {

		return(TRUE);
	}

	return(FALSE);
}

IB_INLINE
ulint
ut_hash_ulint(
/*==========*/
	ulint	 key,
	ulint	 table_size)
{
	ut_ad(table_size);
	key = key ^ UT_HASH_RANDOM_MASK2;

	return(key % table_size);
}

IB_INLINE
ulint
ut_fold_ulint_pair(
/*===============*/
	ulint	n1,
	ulint	n2)
{
	return(((((n1 ^ n2 ^ UT_HASH_RANDOM_MASK2) << 8) + n1)
		^ UT_HASH_RANDOM_MASK) + n2);
}

IB_INLINE
ulint
ut_fold_dulint(
/*===========*/
	dulint	d)
{
	return(ut_fold_ulint_pair(ut_dulint_get_low(d),
				  ut_dulint_get_high(d)));
}

IB_INLINE
ulint
ut_fold_string(
/*===========*/
	const char*	str)
{
	ulint	fold = 0;

	ut_ad(str);

	while (*str != '\0') {
		fold = ut_fold_ulint_pair(fold, (ulint)(*str));
		str++;
	}

	return(fold);
}

IB_INLINE
ulint
ut_fold_binary(
/*===========*/
	const byte*	str,
	ulint		len)
{
	const byte*	str_end	= str + len;
	ulint		fold = 0;

	ut_ad(str || !len);

	while (str < str_end) {
		fold = ut_fold_ulint_pair(fold, (ulint)(*str));

		str++;
	}

	return(fold);
}
