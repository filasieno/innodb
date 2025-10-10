// Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details. You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

/// \file mach_data.cpp
/// \brief Utilities for converting data from the database file to the machine
/// format.

/// Updated 10/10/2025 Fabio N. Filasieno
/// Created 11/28/1995 Heikki Tuuri

#include "mach_data.hpp"

#ifdef IB_DO_NOT_INLINE
#include "mach_data.inl"
#endif

IB_INTERN byte *mach_parse_compressed(byte *ptr, byte *end_ptr, ulint *val)
{
	ut_ad(ptr && end_ptr && val);

	if (ptr >= end_ptr) {
		return NULL;
	}
	ulint flag = mach_read_from_1(ptr);
	if (flag < 0x80UL) {
		*val = flag;
		return ptr + 1;
	} else if (flag < 0xC0UL) {
		if (end_ptr < ptr + 2) {
			return NULL;
		}
		*val = mach_read_from_2(ptr) & 0x7FFFUL;
		return (ptr + 2);
	} else if (flag < 0xE0UL) {
		if (end_ptr < ptr + 3) {
			return NULL;
		}

		*val = mach_read_from_3(ptr) & 0x3FFFFFUL;
		return ptr + 3;
	} else if (flag < 0xF0UL) {
		if (end_ptr < ptr + 4) {
			return NULL;
		}
		*val = mach_read_from_4(ptr) & 0x1FFFFFFFUL;
		return ptr + 4;
	} else {
		ut_ad(flag == 0xF0UL);
		if (end_ptr < ptr + 5) {
			return NULL;
		}
		*val = mach_read_from_4(ptr + 1);
		return ptr + 5;
	}
}

IB_INTERN byte *mach_dulint_parse_compressed(byte *ptr, byte *end_ptr, dulint *val)
{
	ut_ad(ptr && end_ptr && val);

	if (end_ptr < ptr + 5) {
		return NULL;
	}
	ulint high = mach_read_compressed(ptr);
	ulint size = mach_get_compressed_size(high);
	ptr += size;
	if (end_ptr < ptr + 4) {
		return NULL;
	}
	ulint low = mach_read_from_4(ptr);
	*val = ut_dulint_create(high, low);
	return ptr + 4;
}
