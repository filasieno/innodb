// Copyright (c) 2009, 2025, Fabio N. Filasieno. All Rights Reserved.
// Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.
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


/// \file include/mach_data.inl
/// \brief Utilities for converting data from the database file to the machine format.

/// Updated by Fabio N. Filasieno
/// Created 11/28/1995 Heikki Tuuri

#include <string.h>

UNIV_INLINE void mach_write_to_1(byte* b, ulint n)
{
	ut_ad(b);
	ut_ad(n <= 0xFFUL);
	b[0] = (byte)n;
}

UNIV_INLINE ulint mach_read_from_1(const byte* b)
{
	ut_ad(b);
	return((ulint)(b[0]));
}


UNIV_INLINE void mach_write_to_2(byte* b, ulint n)
{
	ut_ad(b);
	ut_ad(n <= 0xFFFFUL);

	b[0] = (byte)(n >> 8);
	b[1] = (byte)(n);
}

UNIV_INLINE ulint mach_read_from_2(const byte* b)
{
	ut_ad(b);
	return ((ulint)(b[0]) << 8) + (ulint)(b[1]);
}

UNIV_INLINE ib_uint16_t mach_encode_2(ulint n)
{
	ib_uint16_t	ret;
	ut_ad(2 == sizeof ret);
	mach_write_to_2((byte*) &ret, n);
	return ret;
}

UNIV_INLINE ulint mach_decode_2(ib_uint16_t	n)
{
	ut_ad(2 == sizeof n);
	return mach_read_from_2((const byte*) &n);
}

UNIV_INLINE void mach_write_to_3(byte* b, ulint n)
{
	ut_ad(b);
	ut_ad(n <= 0xFFFFFFUL);

	b[0] = (byte)(n >> 16);
	b[1] = (byte)(n >> 8);
	b[2] = (byte)(n);
}

UNIV_INLINE ulint mach_read_from_3(const byte* b)
{
	ut_ad(b);
	return ((ulint)(b[0]) << 16) + ((ulint)(b[1]) << 8) + (ulint)(b[2]);
}


UNIV_INLINE void mach_write_to_4(byte* b, ulint n)
{
	ut_ad(b);

	b[0] = (byte)(n >> 24);
	b[1] = (byte)(n >> 16);
	b[2] = (byte)(n >> 8);
	b[3] = (byte)n;
}

UNIV_INLINE ulint mach_read_from_4(const byte* b)
{
	ut_ad(b); 
	return ((ulint)(b[0]) << 24) + ((ulint)(b[1]) << 16) + ((ulint)(b[2]) << 8) + (ulint)(b[3]);
}

UNIV_INLINE ulint mach_write_compressed(byte* b, ulint n)
{
	ut_ad(b);
	if (n < 0x80UL) {
		mach_write_to_1(b, n);
		return 1;
	} else if (n < 0x4000UL) {
		mach_write_to_2(b, n | 0x8000UL);
		return 2;
	} else if (n < 0x200000UL) {
		mach_write_to_3(b, n | 0xC00000UL);
		return 3;
	} else if (n < 0x10000000UL) {
		mach_write_to_4(b, n | 0xE0000000UL);
		return 4;
	} else {
		mach_write_to_1(b, 0xF0UL);
		mach_write_to_4(b + 1, n);
		return 5;
	}
}

UNIV_INLINE ulint mach_get_compressed_size(ulint n)
{
	if (n < 0x80UL) {
		return 1;
	} else if (n < 0x4000UL) {
		return 2;
	} else if (n < 0x200000UL) {
		return 3;
	} else if (n < 0x10000000UL) {
		return 4;
	} else {
		return 5;
	}
}

UNIV_INLINE ulint mach_read_compressed(const byte* b)
{
	ut_ad(b);

	ulint flag = mach_read_from_1(b);
	if (flag < 0x80UL) {
		return flag;
	} else if (flag < 0xC0UL) {
		return mach_read_from_2(b) & 0x7FFFUL;
	} else if (flag < 0xE0UL) {
		return mach_read_from_3(b) & 0x3FFFFFUL;
	} else if (flag < 0xF0UL) {
		return mach_read_from_4(b) & 0x1FFFFFFFUL;
	} else {
		ut_ad(flag == 0xF0UL);
		return mach_read_from_4(b + 1) ;
	}
}

UNIV_INLINE void mach_write_to_8(byte* b, dulint n)
{
	ut_ad(b);

	mach_write_to_4(b, ut_dulint_get_high(n));
	mach_write_to_4(b + 4, ut_dulint_get_low(n));
}

UNIV_INLINE void mach_write_ull(byte* b, ib_uint64_t n)
{
	ut_ad(b);

	mach_write_to_4(b, (ulint) (n >> 32));
	mach_write_to_4(b + 4, (ulint) n);
}

UNIV_INLINE dulint mach_read_from_8(const byte*	b)
{
	ut_ad(b);

	ulint high = mach_read_from_4(b);
	ulint low = mach_read_from_4(b + 4);

	return ut_dulint_create(high, low);
}

UNIV_INLINE ib_uint64_t mach_read_ull(const byte* b)
{
	ib_uint64_t ull = ((ib_uint64_t) mach_read_from_4(b)) << 32;
	ib_uint64_t ull |= (ib_uint64_t) mach_read_from_4(b + 4);

	return ull;
}

UNIV_INLINE void mach_write_to_7(byte* b, dulint n)
{
	ut_ad(b);
	mach_write_to_3(b, ut_dulint_get_high(n));
	mach_write_to_4(b + 3, ut_dulint_get_low(n));
}

UNIV_INLINE dulint mach_read_from_7(const byte*	b)
{
	ut_ad(b);
	ulint high = mach_read_from_3(b);
	ulint low = mach_read_from_4(b + 3);
	return ut_dulint_create(high, low);
}

UNIV_INLINE void mach_write_to_6(byte* b, dulint n)
{
	ut_ad(b);
	mach_write_to_2(b, ut_dulint_get_high(n));
	mach_write_to_4(b + 2, ut_dulint_get_low(n));
}

UNIV_INLINE dulint mach_read_from_6(const byte*	b)
{
	ut_ad(b);
	ulint high = mach_read_from_2(b);
	ulint low = mach_read_from_4(b + 2);
	return ut_dulint_create(high, low);
}

UNIV_INLINE ulint mach_dulint_write_compressed(byte* b, dulint n)	
{
	ut_ad(b);
	ulint size = mach_write_compressed(b, ut_dulint_get_high(n));
	mach_write_to_4(b + size, ut_dulint_get_low(n));
	return size + 4;
}

UNIV_INLINE ulint mach_dulint_get_compressed_size(dulint n)	
{
	return(4 + mach_get_compressed_size(ut_dulint_get_high(n)));
}

UNIV_INLINE dulint mach_dulint_read_compressed(const byte*	b)	
{
	ut_ad(b);
	ulint high = mach_read_compressed(b);
	ulint size = mach_get_compressed_size(high);
	ulint low = mach_read_from_4(b + size);
	return ut_dulint_create(high, low);
}

UNIV_INLINE ulint mach_dulint_write_much_compressed(byte* b, dulint n)	
{
	ulint size;
	ut_ad(b);
	if (ut_dulint_get_high(n) == 0) {
		return(mach_write_compressed(b, ut_dulint_get_low(n)));
	}
	*b = (byte)0xFF;
	size = 1 + mach_write_compressed(b + 1, ut_dulint_get_high(n));
	size += mach_write_compressed(b + size, ut_dulint_get_low(n));
	return size;
}

UNIV_INLINE ulint mach_dulint_get_much_compressed_size(dulint n)	
{
	if (0 == ut_dulint_get_high(n)) {
		return mach_get_compressed_size(ut_dulint_get_low(n));
	}
	return 1 + mach_get_compressed_size(ut_dulint_get_high(n)) + mach_get_compressed_size(ut_dulint_get_low(n));
}

UNIV_INLINE dulint mach_dulint_read_much_compressed(const byte* b)	
{
	ut_ad(b);

	ulint high;
	ulint low;
	ulint size;

	if (*b != (byte)0xFF) {
		high = 0;
		size = 0;
	} else {
		high = mach_read_compressed(b + 1);

		size = 1 + mach_get_compressed_size(high);
	}

	low = mach_read_compressed(b + size);

	return ut_dulint_create(high, low);
}

#ifndef UNIV_HOTBACKUP
UNIV_INLINE double mach_double_read(const byte*	b)	
{
	double d;
	byte* ptr = (byte*)&d;
	for (ulint i = 0; i < sizeof(double); i++) {
#ifdef WORDS_BIGENDIAN
		ptr[sizeof(double) - i - 1] = b[i];
#else
		ptr[i] = b[i];
#endif
	}
	return d;
}

UNIV_INLINE void mach_double_ptr_write(byte* b, const byte* ptr)	
{
	for (ulint i = 0; i < sizeof(double); i++) {
#ifdef WORDS_BIGENDIAN
		b[i] = ptr[sizeof(double) - i - 1];
#else
		b[i] = ptr[i];
#endif
	}
}

UNIV_INLINE void mach_double_write(byte* b, double d)	
{
	mach_double_ptr_write(b, (byte*) &d);
}

UNIV_INLINE float mach_float_read(const byte*	b)	
{
	float d;
	ulint i;
	byte* ptr = (byte*)&d;
	for (i = 0; i < sizeof(float); i++) {
#ifdef WORDS_BIGENDIAN
		ptr[sizeof(float) - i - 1] = b[i];
#else
		ptr[i] = b[i];
#endif
	}
	return d;
}

UNIV_INLINE void mach_float_ptr_write(byte* b, const byte* ptr)	
{
	ulint i;
	for (i = 0; i < sizeof(float); i++) {
#ifdef WORDS_BIGENDIAN
		b[i] = ptr[sizeof(float) - i - 1];
#else
		b[i] = ptr[i];
#endif
	}
}

UNIV_INLINE void mach_float_write(byte* b, float d)	
{
	mach_float_ptr_write(b, (byte*)&d);
}

UNIV_INLINE ulint mach_read_from_n_little_endian(const byte* buf, ulint buf_size)	
{
	ulint n	= 0;
	const byte*	ptr;
	ut_ad(buf_size <= sizeof(ulint));
	ut_ad(buf_size > 0);
	ptr = buf + buf_size;
	for (;;) {
		ptr--;
		n = n << 8;
		n += (ulint)(*ptr);
		if (ptr == buf) {
			break;
		}
	}
	return n;
}

UNIV_INLINE void mach_write_to_n_little_endian(byte* dest, ulint dest_size, ulint n)		
{
	ut_ad(dest_size <= sizeof(ulint));
	ut_ad(dest_size > 0);
	byte* end = dest + dest_size;
	for (;;) {
		*dest = (byte)(n & 0xFF);
		n = n >> 8;
		dest++;
		if (dest == end) {
			break;
		}
	}
	ut_ad(n == 0);
}

UNIV_INLINE ulint mach_read_from_2_little_endian(const byte*	buf)		
{
	return((ulint)(*buf) + ((ulint)(*(buf + 1))) * 256);
}

UNIV_INLINE void mach_write_to_2_little_endian(byte* dest, ulint n)		
{
	ut_ad(n < 256 * 256);
	*dest = (byte)(n & 0xFFUL);
	n = n >> 8;
	dest++;
	*dest = (byte)(n & 0xFFUL);
}

UNIV_INLINE void mach_swap_byte_order(byte* dest, const byte* from, ulint len)		
{
	ut_ad(len > 0);
	ut_ad(len <= 8);
	dest += len;
	switch (len & 0x7) {
	case 0: *--dest = *from++;
	case 7: *--dest = *from++;
	case 6: *--dest = *from++;
	case 5: *--dest = *from++;
	case 4: *--dest = *from++;
	case 3: *--dest = *from++;
	case 2: *--dest = *from++;
	case 1: *--dest = *from;
	}
}

UNIV_INLINE void mach_read_int_type(void* dst, const byte* src, ibool usign)		
{
#ifdef WORDS_BIGENDIAN
	memcpy(dst, src, len);
#else
	mach_swap_byte_order(dst, src, len);
	if (usign) {
		*(((byte*) dst) + len - 1) ^= 0x80;
	}
#endif
}

UNIV_INLINE void mach_write_int_type(byte* dest, const byte* src, ulint len, ibool usign)		
{
#ifdef WORDS_BIGENDIAN
	memcpy(dest, src, len);
#else
	mach_swap_byte_order(dest, src, len);
	if (usign) {
		*dest ^=  0x80;
	}
#endif
}

UNIV_INLINE ib_uint64_t mach_read_uint64(const byte* src)		
{
	ib_uint64_t	dst;
	mach_read_int_type(&dst, src, sizeof(dst), TRUE);
	return dst;
}

UNIV_INLINE ib_int64_t mach_read_int64(const byte*	src)		
{
	ib_uint64_t	dst;
	mach_read_int_type(&dst, src, sizeof(dst), FALSE);
	return dst;
}

UNIV_INLINE ib_uint32_t mach_read_uint32(const byte* src)		
{
	ib_uint32_t	dst;
	mach_read_int_type(&dst, src, sizeof(dst), TRUE);
	return dst;
}

UNIV_INLINE ib_int32_t mach_read_int32(const byte* src)		
{
	ib_int32_t dst;
	mach_read_int_type(&dst, src, sizeof(dst), FALSE);
	return dst;
}

UNIV_INLINE void
mach_write_uint64(byte* dest, ib_uint64_t	n)
{
	ut_ad(dest != NULL);

	mach_write_int_type(dest, (const byte*) &n, sizeof(n), TRUE);
}

UNIV_INLINE void mach_write_int64(byte* dest, ib_int64_t n)		
{
	mach_write_int_type(dest, (const byte*) &n, sizeof(n), FALSE);
}

UNIV_INLINE void mach_write_uint32(byte* dest, ib_uint32_t n)		
{
	mach_write_int_type(dest, (const byte*) &n, sizeof(n), TRUE);
}

UNIV_INLINE void mach_write_int32(byte* dest, ib_int32_t n)		
{
	mach_write_int_type(dest, (const byte*) &n, sizeof(n), FALSE);
}

#endif
