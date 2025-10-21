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

/// \file rem_rec.inl
/// \brief Record manager
/// \details Originally created by Heikki Tuuri on 5/30/1994
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "mach_data.hpp"
#include "ut_byte.hpp"
#include "dict_dict.hpp"

constinit ulint REC_OFFS_COMPACT = ((ulint) 1 << 31);
constinit ulint REC_OFFS_SQL_NULL = ((ulint) 1 << 31);
constinit ulint REC_OFFS_EXTERNAL = ((ulint) 1 << 30);
constinit ulint REC_OFFS_MASK = (REC_OFFS_EXTERNAL - 1);

/* Offsets of the bit-fields in an old-style record. NOTE! In the table the
most significant bytes and bits are written below less significant.

	(1) byte offset		(2) bit usage within byte
	downward from
	origin ->	1	8 bits pointer to next record
			2	8 bits pointer to next record
			3	1 bit short flag
				7 bits number of fields
			4	3 bits number of fields
				5 bits heap number
			5	8 bits heap number
			6	4 bits n_owned
				4 bits info bits
*/

/* Offsets of the bit-fields in a new-style record. NOTE! In the table the
most significant bytes and bits are written below less significant.

	(1) byte offset		(2) bit usage within byte
	downward from
	origin ->	1	8 bits relative offset of next record
			2	8 bits relative offset of next record
				  the relative offset is an unsigned 16-bit
				  integer:
				  (offset_of_next_record
				   - offset_of_this_record) mod 64Ki,
				  where mod is the modulo as a non-negative
				  number;
				  we can calculate the offset of the next
				  record with the formula:
				  relative_offset + offset_of_this_record
				  mod IB_PAGE_SIZE
			3	3 bits status:
					000=conventional record
					001=node pointer record (inside B-tree)
					010=infimum record
					011=supremum record
					1xx=reserved
				5 bits heap number
			4	8 bits heap number
			5	4 bits n_owned
				4 bits info bits
*/

/* We list the byte offsets from the origin of the record, the mask,
and the shift needed to obtain each bit-field of the record. */

constinit ulint REC_NEXT = 2;
constinit ulint REC_NEXT_MASK = 0xFFFFUL;
constinit ulint REC_NEXT_SHIFT = 0;

constinit ulint REC_OLD_SHORT = 3;	/* This is single byte bit-field */
constinit ulint REC_OLD_SHORT_MASK = 0x1UL;
constinit ulint REC_OLD_SHORT_SHIFT = 0;

constinit ulint REC_OLD_N_FIELDS = 4;
constinit ulint REC_OLD_N_FIELDS_MASK = 0x7FEUL;
constinit ulint REC_OLD_N_FIELDS_SHIFT = 1;

constinit ulint REC_NEW_STATUS = 3;	/* This is single byte bit-field */
constinit ulint REC_NEW_STATUS_MASK = 0x7UL;
constinit ulint REC_NEW_STATUS_SHIFT = 0;

constinit ulint REC_OLD_HEAP_NO = 5;
constinit ulint REC_HEAP_NO_MASK = 0xFFF8UL;
#if 0 /* defined in rem0rec.h for use of page0zip.c */
constinit ulint REC_NEW_HEAP_NO = 4;
constinit ulint REC_HEAP_NO_SHIFT = 3;
#endif

constinit ulint REC_OLD_N_OWNED = 6;	/* This is single byte bit-field */
constinit ulint REC_NEW_N_OWNED = 5;	/* This is single byte bit-field */
constinit ulint REC_N_OWNED_MASK = 0xFUL;
constinit ulint REC_N_OWNED_SHIFT = 0;

constinit ulint REC_OLD_INFO_BITS = 6;	/* This is single byte bit-field */
constinit ulint REC_NEW_INFO_BITS = 5;	/* This is single byte bit-field */
constinit ulint REC_INFO_BITS_MASK = 0xF0UL;
constinit ulint REC_INFO_BITS_SHIFT = 0;

/* The following masks are used to filter the SQL null bit from
one-byte and two-byte offsets */

constinit ulint REC_1BYTE_SQL_NULL_MASK = 0x80UL;
constinit ulint REC_2BYTE_SQL_NULL_MASK = 0x8000UL;

/* In a 2-byte offset the second most significant bit denotes
a field stored to another page: */

constinit ulint REC_2BYTE_EXTERN_MASK = 0x4000UL;

#if REC_OLD_SHORT_MASK << (8 * (REC_OLD_SHORT - 3)) \
		^ REC_OLD_N_FIELDS_MASK << (8 * (REC_OLD_N_FIELDS - 4)) \
		^ REC_HEAP_NO_MASK << (8 * (REC_OLD_HEAP_NO - 4)) \
		^ REC_N_OWNED_MASK << (8 * (REC_OLD_N_OWNED - 3)) \
		^ REC_INFO_BITS_MASK << (8 * (REC_OLD_INFO_BITS - 3)) \
		^ 0xFFFFFFFFUL
# error "sum of old-style masks != 0xFFFFFFFFUL"
#endif
#if REC_NEW_STATUS_MASK << (8 * (REC_NEW_STATUS - 3)) \
		^ REC_HEAP_NO_MASK << (8 * (REC_NEW_HEAP_NO - 4)) \
		^ REC_N_OWNED_MASK << (8 * (REC_NEW_N_OWNED - 3)) \
		^ REC_INFO_BITS_MASK << (8 * (REC_NEW_INFO_BITS - 3)) \
		^ 0xFFFFFFUL
# error "sum of new-style masks != 0xFFFFFFUL"
#endif

/***********************************************************//**
Sets the value of the ith field SQL null bit of an old-style record. */
/// \brief Sets the value of the ith field SQL null bit of an old-style record.
/// \param [in] rec record
/// \param [in] i ith field
/// \param [in] val value to set
IB_INTERN void rec_set_nth_field_null_bit(rec_t* rec, ulint i, ibool val);

/// \brief Sets an old-style record field to SQL null.
/// \details The physical size of the field is not changed.
/// \param [in] rec record
/// \param [in] n index of the field
IB_INTERN void rec_set_nth_field_sql_null(rec_t* rec, ulint n);

/// \brief Gets a bit field from within 1 byte.
/// \param [in] rec pointer to record origin
/// \param [in] offs offset from the origin down
/// \param [in] mask mask used to filter bits
/// \param [in] shift shift right applied after masking
/// \return bit field value
IB_INLINE ulint rec_get_bit_field_1(const rec_t* rec, ulint offs, ulint mask, ulint shift)
{
	ut_ad(rec);

	return((mach_read_from_1(rec - offs) & mask) >> shift);
}

/// \brief Sets a bit field within 1 byte.
/// \param [in] rec pointer to record origin
/// \param [in] val value to set
/// \param [in] offs offset from the origin down
/// \param [in] mask mask used to filter bits
/// \param [in] shift shift right applied after masking
IB_INLINE void rec_set_bit_field_1(rec_t* rec, ulint val, ulint offs, ulint mask, ulint shift) {
	ut_ad(rec);
	ut_ad(offs <= REC_N_OLD_EXTRA_BYTES);
	ut_ad(mask);
	ut_ad(mask <= 0xFFUL);
	ut_ad(((mask >> shift) << shift) == mask);
	ut_ad(((val << shift) & mask) == (val << shift));
    mach_write_to_1(rec - offs, (mach_read_from_1(rec - offs) & ~mask) | (val << shift));
}

/// \brief Gets a bit field from within 2 bytes.
/// \param [in] rec pointer to record origin
/// \param [in] offs offset from the origin down
/// \param [in] mask mask used to filter bits
/// \param [in] shift shift right applied after masking
IB_INLINE ulint rec_get_bit_field_2(const rec_t* rec, ulint offs, ulint mask, ulint shift) {
	ut_ad(rec);
    return (mach_read_from_2(rec - offs) & mask) >> shift;
}

/// \brief Sets a bit field within 2 bytes.
/// \param [in] rec pointer to record origin
/// \param [in] val value to set
/// \param [in] offs offset from the origin down
/// \param [in] mask mask used to filter bits
/// \param [in] shift shift right applied after masking
IB_INLINE void rec_set_bit_field_2(rec_t* rec, ulint val, ulint offs, ulint mask, ulint shift) {
	ut_ad(rec);
	ut_ad(offs <= REC_N_OLD_EXTRA_BYTES);
	ut_ad(mask > 0xFFUL);
	ut_ad(mask <= 0xFFFFUL);
	ut_ad((mask >> shift) & 1);
	ut_ad(0 == ((mask >> shift) & ((mask >> shift) + 1)));
	ut_ad(((mask >> shift) << shift) == mask);
	ut_ad(((val << shift) & mask) == (val << shift));
    mach_write_to_2(rec - offs, (mach_read_from_2(rec - offs) & ~mask) | (val << shift));
}

/// \brief The following function is used to get the pointer of the next chained record on the same page.
/// \details The following function is used to get the pointer of the next chained record on the same page.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return pointer to the next chained record, or NULL if none
IB_INLINE const rec_t* rec_get_next_ptr_const(const rec_t* rec, ulint comp) {
    ulint field_value = mach_read_from_2(rec - REC_NEXT);
	ut_ad(REC_NEXT_MASK == 0xFFFFUL);
	ut_ad(REC_NEXT_SHIFT == 0);
	if (IB_UNLIKELY(field_value == 0)) {
        return NULL;
	}
	if (IB_EXPECT(comp, REC_OFFS_COMPACT)) {
#if IB_PAGE_SIZE <= 32768
        // Note that for 64 KiB pages, field_value can 'wrap around' and the debug assertion is not valid
        // In the following assertion, field_value is interpreted as signed 16-bit integer in 2's complement arithmetics. If all platforms defined int16_t in the standard headers, the expression could be written simpler as (int16_t) field_value + ut_align_offset(...) < IB_PAGE_SIZE
        ut_ad((field_value >= 32768 ? field_value - 65536 : field_value) + ut_align_offset(rec, IB_PAGE_SIZE) < IB_PAGE_SIZE);
#endif
        // There must be at least REC_N_NEW_EXTRA_BYTES + 1 between each record.
        ut_ad((field_value > REC_N_NEW_EXTRA_BYTES && field_value < 32768) || field_value < (ib_uint16_t) -REC_N_NEW_EXTRA_BYTES);
        return (byte*) ut_align_down(rec, IB_PAGE_SIZE) + ut_align_offset(rec + field_value, IB_PAGE_SIZE);
	} else {
		ut_ad(field_value < IB_PAGE_SIZE);
        return (byte*) ut_align_down(rec, IB_PAGE_SIZE) + field_value;
    }
}

/// \brief The following function is used to get the pointer of the next chained record on the same page.
/// \details The following function is used to get the pointer of the next chained record on the same page.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return pointer to the next chained record, or NULL if none
IB_INLINE rec_t* rec_get_next_ptr(rec_t* rec, ulint comp) {
    return (rec_t*) rec_get_next_ptr_const(rec, comp);
}

/// \brief The following function is used to get the offset of the next chained record on the same page.
/// \details The following function is used to get the offset of the next chained record on the same page.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return the page offset of the next chained record, or 0 if none
IB_INLINE ulint rec_get_next_offs(const rec_t* rec, ulint comp) {
    ulint field_value;
#if REC_NEXT_MASK != 0xFFFFUL
# error "REC_NEXT_MASK != 0xFFFFUL"
#endif
#if REC_NEXT_SHIFT
# error "REC_NEXT_SHIFT != 0"
#endif
	field_value = mach_read_from_2(rec - REC_NEXT);
	if (IB_EXPECT(comp, REC_OFFS_COMPACT)) {
#if IB_PAGE_SIZE <= 32768
        // Note that for 64 KiB pages, field_value can 'wrap around' and the debug assertion is not valid
        // In the following assertion, field_value is interpreted as signed 16-bit integer in 2's complement arithmetics. If all platforms defined int16_t in the standard headers, the expression could be written simpler as (int16_t) field_value + ut_align_offset(...) < IB_PAGE_SIZE
        ut_ad((field_value >= 32768 ? field_value - 65536 : field_value) + ut_align_offset(rec, IB_PAGE_SIZE) < IB_PAGE_SIZE);
#endif
		if (IB_UNLIKELY(field_value == 0)) {
            return 0;
        }
        // There must be at least REC_N_NEW_EXTRA_BYTES + 1 between each record.
        ut_ad((field_value > REC_N_NEW_EXTRA_BYTES && field_value < 32768) || field_value < (ib_uint16_t) -REC_N_NEW_EXTRA_BYTES);
        return ut_align_offset(rec + field_value, IB_PAGE_SIZE);
	} else {
		ut_ad(field_value < IB_PAGE_SIZE);
        return field_value;
    }
}

/// \brief The following function is used to set the next record offset field of an old-style record.
/// \param [in] rec old-style physical record
/// \param [in] next offset of the next record
IB_INLINE void rec_set_next_offs_old(rec_t* rec, ulint next) {
	ut_ad(rec);
	ut_ad(IB_PAGE_SIZE > next);
#if REC_NEXT_MASK != 0xFFFFUL
# error "REC_NEXT_MASK != 0xFFFFUL"
#endif
#if REC_NEXT_SHIFT
# error "REC_NEXT_SHIFT != 0"
#endif
	mach_write_to_2(rec - REC_NEXT, next);
}

/// \brief The following function is used to set the next record offset field of a new-style record.
/// \param [in,out] rec new-style physical record
/// \param [in] next offset of the next record
IB_INLINE void rec_set_next_offs_new(rec_t* rec, ulint next) {
    ulint field_value;
	ut_ad(rec);
	ut_ad(IB_PAGE_SIZE > next);
	if (IB_UNLIKELY(!next)) {
		field_value = 0;
	} else {
        // The following two statements calculate next - offset_of_rec mod 64Ki, where mod is the modulo as a non-negative number
        field_value = (ulint) ((lint) next - (lint) ut_align_offset(rec, IB_PAGE_SIZE));
		field_value &= REC_NEXT_MASK;
	}
	mach_write_to_2(rec - REC_NEXT, field_value);
}

/// \brief The following function is used to get the number of fields in an old-style record.
/// \param [in] rec physical record
/// \return number of data fields
IB_INLINE ulint rec_get_n_fields_old(const rec_t* rec) {
    ulint ret = rec_get_bit_field_2(rec, REC_OLD_N_FIELDS, REC_OLD_N_FIELDS_MASK, REC_OLD_N_FIELDS_SHIFT);
	ut_ad(rec);
	ut_ad(ret <= REC_MAX_N_FIELDS);
	ut_ad(ret > 0);
    return ret;
}

/// \brief The following function is used to set the number of fields in an old-style record.
/// \param [in] rec physical record
/// \param [in] n_fields the number of fields
IB_INLINE void rec_set_n_fields_old(rec_t* rec, ulint n_fields) {
	ut_ad(rec);
	ut_ad(n_fields <= REC_MAX_N_FIELDS);
	ut_ad(n_fields > 0);
    rec_set_bit_field_2(rec, n_fields, REC_OLD_N_FIELDS, REC_OLD_N_FIELDS_MASK, REC_OLD_N_FIELDS_SHIFT);
}

/// \brief The following function retrieves the status bits of a new-style record.
/// \param [in] rec physical record
/// \return status bits
IB_INLINE ulint rec_get_status(const rec_t* rec) {
    ulint ret = rec_get_bit_field_1(rec, REC_NEW_STATUS, REC_NEW_STATUS_MASK, REC_NEW_STATUS_SHIFT);
	ut_ad(rec);
	ut_ad((ret & ~REC_NEW_STATUS_MASK) == 0);
    return ret;
}

/// \brief The following function is used to get the number of fields in a record.
/// \param [in] rec physical record
/// \param [in] dict_index record descriptor
/// \return number of data fields
IB_INLINE ulint rec_get_n_fields(const rec_t* rec, const dict_index_t* dict_index) {
	ut_ad(rec);
	ut_ad(dict_index);
	if (!dict_table_is_comp(dict_index->table)) {
        return rec_get_n_fields_old(rec);
	}
	switch (rec_get_status(rec)) {
	case REC_STATUS_ORDINARY:
        return dict_index_get_n_fields(dict_index);
	case REC_STATUS_NODE_PTR:
        return dict_index_get_n_unique_in_tree(dict_index) + 1;
	case REC_STATUS_INFIMUM:
	case REC_STATUS_SUPREMUM:
        return 1;
	default:
		UT_ERROR;
        return ULINT_UNDEFINED;
    }
}

/// \brief The following function is used to get the number of records owned by the previous directory record.
/// \param [in] rec old-style physical record
/// \return number of owned records
IB_INLINE ulint rec_get_n_owned_old(const rec_t* rec) {
    return rec_get_bit_field_1(rec, REC_OLD_N_OWNED, REC_N_OWNED_MASK, REC_N_OWNED_SHIFT);
}

/// \brief The following function is used to set the number of owned records.
/// \param [in] rec old-style physical record
/// \param [in] n_owned the number of owned
IB_INLINE void rec_set_n_owned_old(rec_t* rec, ulint n_owned) {
    rec_set_bit_field_1(rec, n_owned, REC_OLD_N_OWNED, REC_N_OWNED_MASK, REC_N_OWNED_SHIFT);
}

/// \brief The following function is used to get the number of records owned by the previous directory record.
/// \param [in] rec new-style physical record
/// \return number of owned records
IB_INLINE ulint rec_get_n_owned_new(const rec_t* rec) {
    return rec_get_bit_field_1(rec, REC_NEW_N_OWNED, REC_N_OWNED_MASK, REC_N_OWNED_SHIFT);
}

/// \brief The following function is used to set the number of owned records.
/// \param [in,out] rec new-style physical record
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] n_owned the number of owned
IB_INLINE void rec_set_n_owned_new(rec_t* rec, page_zip_des_t* page_zip, ulint n_owned) {
    rec_set_bit_field_1(rec, n_owned, REC_NEW_N_OWNED, REC_N_OWNED_MASK, REC_N_OWNED_SHIFT);
#ifdef WITH_ZIP
    if (IB_LIKELY_NULL(page_zip) && IB_LIKELY(rec_get_status(rec) != REC_STATUS_SUPREMUM)) {
		page_zip_rec_set_owned(page_zip, rec, n_owned);
	}
#endif /* WITH_ZIP */
}

/// \brief The following function is used to retrieve the info bits of a record.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return info bits
IB_INLINE ulint rec_get_info_bits(const rec_t* rec, ulint comp) {
    return rec_get_bit_field_1(rec, comp ? REC_NEW_INFO_BITS : REC_OLD_INFO_BITS, REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
}

/// \brief The following function is used to set the info bits of a record.
/// \param [in] rec old-style physical record
/// \param [in] bits info bits
IB_INLINE void rec_set_info_bits_old(rec_t* rec, ulint bits) {
    rec_set_bit_field_1(rec, bits, REC_OLD_INFO_BITS, REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
}
/// \brief The following function is used to set the info bits of a record.
/// \param [in,out] rec new-style physical record
/// \param [in] bits info bits
IB_INLINE void rec_set_info_bits_new(rec_t* rec, ulint bits) {
    rec_set_bit_field_1(rec, bits, REC_NEW_INFO_BITS, REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
}

/// \brief The following function is used to set the status bits of a new-style record.
/// \param [in,out] rec physical record
/// \param [in] bits info bits
IB_INLINE void rec_set_status(rec_t* rec, ulint bits) {
    rec_set_bit_field_1(rec, bits, REC_NEW_STATUS, REC_NEW_STATUS_MASK, REC_NEW_STATUS_SHIFT);
}

/// \brief The following function is used to retrieve the info and status bits of a record. (Only compact records have status bits.)
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return info bits
IB_INLINE ulint rec_get_info_and_status_bits(const rec_t* rec, ulint comp) {
    ulint bits;
#if (REC_NEW_STATUS_MASK >> REC_NEW_STATUS_SHIFT) & (REC_INFO_BITS_MASK >> REC_INFO_BITS_SHIFT)
# error "REC_NEW_STATUS_MASK and REC_INFO_BITS_MASK overlap"
#endif
	if (IB_EXPECT(comp, REC_OFFS_COMPACT)) {
		bits = rec_get_info_bits(rec, TRUE) | rec_get_status(rec);
	} else {
		bits = rec_get_info_bits(rec, FALSE);
		ut_ad(!(bits & ~(REC_INFO_BITS_MASK >> REC_INFO_BITS_SHIFT)));
	}
    return bits;
}
/// \brief The following function is used to set the info and status bits of a record. (Only compact records have status bits.)
/// \param [in,out] rec physical record
/// \param [in] bits info bits
IB_INLINE void rec_set_info_and_status_bits(rec_t* rec, ulint bits) {
#if (REC_NEW_STATUS_MASK >> REC_NEW_STATUS_SHIFT) & (REC_INFO_BITS_MASK >> REC_INFO_BITS_SHIFT)
# error "REC_NEW_STATUS_MASK and REC_INFO_BITS_MASK overlap"
#endif
	rec_set_status(rec, bits & REC_NEW_STATUS_MASK);
	rec_set_info_bits_new(rec, bits & ~REC_NEW_STATUS_MASK);
}

/// \brief The following function tells if record is delete marked.
/// \param [in] rec physical record
/// \param [in] comp nonzero=compact page format
/// \return nonzero if delete marked
IB_INLINE ulint rec_get_deleted_flag(const rec_t* rec, ulint comp) {
	if (IB_EXPECT(comp, REC_OFFS_COMPACT)) {
        return IB_UNLIKELY(rec_get_bit_field_1(rec, REC_NEW_INFO_BITS, REC_INFO_DELETED_FLAG, REC_INFO_BITS_SHIFT));
	} else {
        return IB_UNLIKELY(rec_get_bit_field_1(rec, REC_OLD_INFO_BITS, REC_INFO_DELETED_FLAG, REC_INFO_BITS_SHIFT));
    }
}

/// \brief The following function is used to set the deleted bit.
/// \param [in] rec old-style physical record
/// \param [in] flag nonzero if delete marked
IB_INLINE void rec_set_deleted_flag_old(rec_t* rec, ulint flag) {
    ulint val = rec_get_info_bits(rec, FALSE);
	if (flag) {
		val |= REC_INFO_DELETED_FLAG;
	} else {
		val &= ~REC_INFO_DELETED_FLAG;
	}
	rec_set_info_bits_old(rec, val);
}
/// \brief The following function is used to set the deleted bit.
/// \param [in,out] rec new-style physical record
/// \param [in,out] page_zip compressed page, or NULL
/// \param [in] flag nonzero if delete marked
IB_INLINE void rec_set_deleted_flag_new(rec_t* rec, page_zip_des_t* page_zip, ulint flag) {
    ulint val = rec_get_info_bits(rec, TRUE);
	if (flag) {
		val |= REC_INFO_DELETED_FLAG;
	} else {
		val &= ~REC_INFO_DELETED_FLAG;
	}
	rec_set_info_bits_new(rec, val);
#ifdef WITH_ZIP
	if (IB_LIKELY_NULL(page_zip)) {
		page_zip_rec_set_deleted(page_zip, rec, flag);
	}
#endif /* WITH_ZIP */
}

/// \brief The following function tells if a new-style record is a node pointer.
/// \param [in] rec physical record
/// \return TRUE if node pointer
IB_INLINE ibool rec_get_node_ptr_flag(const rec_t* rec) {
    return REC_STATUS_NODE_PTR == rec_get_status(rec);
}

/// \brief The following function is used to get the order number of an old-style record in the heap of the index page.
/// \param [in] rec physical record
/// \return heap order number
IB_INLINE ulint rec_get_heap_no_old(const rec_t* rec) {
    return rec_get_bit_field_2(rec, REC_OLD_HEAP_NO, REC_HEAP_NO_MASK, REC_HEAP_NO_SHIFT);
}
/// \brief The following function is used to set the heap number field in an old-style record.
/// \param [in] rec physical record
/// \param [in] heap_no the heap number
IB_INLINE void rec_set_heap_no_old(rec_t* rec, ulint heap_no) {
    rec_set_bit_field_2(rec, heap_no, REC_OLD_HEAP_NO, REC_HEAP_NO_MASK, REC_HEAP_NO_SHIFT);
}
/// \brief The following function is used to get the order number of a new-style record in the heap of the index page.
/// \param [in] rec physical record
/// \return heap order number
IB_INLINE ulint rec_get_heap_no_new(const rec_t* rec) {
    return rec_get_bit_field_2(rec, REC_NEW_HEAP_NO, REC_HEAP_NO_MASK, REC_HEAP_NO_SHIFT);
}
/// \brief The following function is used to set the heap number field in a new-style record.
/// \param [in,out] rec physical record
/// \param [in] heap_no the heap number
IB_INLINE void rec_set_heap_no_new(rec_t* rec, ulint heap_no) {
    rec_set_bit_field_2(rec, heap_no, REC_NEW_HEAP_NO, REC_HEAP_NO_MASK, REC_HEAP_NO_SHIFT);
}

/// \brief The following function is used to test whether the data offsets in the record are stored in one-byte or two-byte format.
/// \param [in] rec physical record
/// \return TRUE if 1-byte form
IB_INLINE ibool rec_get_1byte_offs_flag(const rec_t* rec) {
#if TRUE != 1
#error "TRUE != 1"
#endif
    return rec_get_bit_field_1(rec, REC_OLD_SHORT, REC_OLD_SHORT_MASK, REC_OLD_SHORT_SHIFT);
}
/// \brief The following function is used to set the 1-byte offsets flag.
/// \param [in] rec physical record
/// \param [in] flag TRUE if 1byte form
IB_INLINE void rec_set_1byte_offs_flag(rec_t* rec, ibool flag) {
#if TRUE != 1
#error "TRUE != 1"
#endif
	ut_ad(flag <= TRUE);
    rec_set_bit_field_1(rec, flag, REC_OLD_SHORT, REC_OLD_SHORT_MASK, REC_OLD_SHORT_SHIFT);
}

/// \brief Returns the offset of nth field end if the record is stored in the 1-byte offsets form. If the field is SQL null, the flag is ORed in the returned value.
/// \param [in] rec record
/// \param [in] n field index
/// \return offset of the start of the field, SQL null flag ORed
IB_INLINE ulint rec_1_get_field_end_info(const rec_t* rec, ulint n) {
	ut_ad(rec_get_1byte_offs_flag(rec));
	ut_ad(n < rec_get_n_fields_old(rec));
    return mach_read_from_1(rec - (REC_N_OLD_EXTRA_BYTES + n + 1));
}
/// \brief Returns the offset of nth field end if the record is stored in the 2-byte offsets form. If the field is SQL null, the flag is ORed in the returned value.
/// \param [in] rec record
/// \param [in] n field index
/// \return offset of the start of the field, SQL null flag and extern storage flag ORed
IB_INLINE ulint rec_2_get_field_end_info(const rec_t* rec, ulint n) {
	ut_ad(!rec_get_1byte_offs_flag(rec));
	ut_ad(n < rec_get_n_fields_old(rec));
    return mach_read_from_2(rec - (REC_N_OLD_EXTRA_BYTES + 2 * n + 2));
}

// Get the base address of offsets. The extra_size is stored at this position, and following positions hold the end offsets of the fields.
#define rec_offs_base(offsets) (offsets + REC_OFFS_HEADER_SIZE)

/// \brief The following function returns the number of allocated elements for an array of offsets.
/// \param [in] offsets array for rec_get_offsets()
/// \return number of elements
IB_INLINE ulint rec_offs_get_n_alloc(const ulint* offsets) {
    ulint n_alloc = offsets[0];
	ut_ad(offsets);
	ut_ad(n_alloc > REC_OFFS_HEADER_SIZE);
	IB_MEM_ASSERT_W(offsets, n_alloc * sizeof *offsets);
    return n_alloc;
}
/// \brief The following function sets the number of allocated elements for an array of offsets.
/// \param [out] offsets array for rec_get_offsets(), must be allocated
/// \param [in] n_alloc number of elements
IB_INLINE void rec_offs_set_n_alloc(ulint* offsets, ulint n_alloc) {
	ut_ad(offsets);
	ut_ad(n_alloc > REC_OFFS_HEADER_SIZE);
	IB_MEM_ASSERT_AND_ALLOC(offsets, n_alloc * sizeof *offsets);
	offsets[0] = n_alloc;
}

/// \brief The following function returns the number of fields in a record.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return number of fields
IB_INLINE ulint rec_offs_n_fields(const ulint* offsets) {
    ulint n_fields = offsets[1];
	ut_ad(offsets);
	ut_ad(n_fields > 0);
	ut_ad(n_fields <= REC_MAX_N_FIELDS);
    ut_ad(n_fields + REC_OFFS_HEADER_SIZE <= rec_offs_get_n_alloc(offsets));
    return n_fields;
}

/// \brief Validates offsets returned by rec_get_offsets().
/// \param [in] rec record or NULL
/// \param [in] dict_index record descriptor or NULL
/// \param [in] offsets array returned by rec_get_offsets()
/// \return TRUE if valid
IB_INLINE ibool rec_offs_validate(const rec_t* rec, const dict_index_t* dict_index, const ulint* offsets) {
    ulint i = rec_offs_n_fields(offsets);
    ulint last = ULINT_MAX;
    ulint comp = *rec_offs_base(offsets) & REC_OFFS_COMPACT;
	if (rec) {
		ut_ad((ulint) rec == offsets[2]);
		if (!comp) {
			ut_a(rec_get_n_fields_old(rec) >= i);
		}
	}
	if (dict_index) {
		ut_ad((ulint) dict_index == offsets[3]);
		ulint max_n_fields = ut_max(dict_index_get_n_fields(dict_index), dict_index_get_n_unique_in_tree(dict_index) + 1);

		if (comp && rec) {
			switch (rec_get_status(rec)) {
			case REC_STATUS_ORDINARY:
				break;
			case REC_STATUS_NODE_PTR:
                max_n_fields = dict_index_get_n_unique_in_tree(dict_index) + 1;
				break;
			case REC_STATUS_INFIMUM:
			case REC_STATUS_SUPREMUM:
				max_n_fields = 1;
				break;
			default:
				UT_ERROR;
			}
		}
        // dict_index->n_def == 0 for dummy indexes if !comp
		ut_a(!comp || dict_index->n_def);
		ut_a(!dict_index->n_def || i <= max_n_fields);
	}
	while (i--) {
        ulint curr = rec_offs_base(offsets)[1 + i] & REC_OFFS_MASK;
		ut_a(curr <= last);
		last = curr;
	}
    return TRUE;
}
#ifdef IB_DEBUG
/// \brief Updates debug data in offsets, in order to avoid bogus rec_offs_validate() failures.
/// \param [in] rec record
/// \param [in] index record descriptor
/// \param [in] offsets array returned by rec_get_offsets()
IB_INLINE void rec_offs_make_valid(const rec_t* rec, const dict_index_t* index, ulint* offsets) {
	ut_ad(rec);
	ut_ad(index);
	ut_ad(offsets);
	ut_ad(rec_get_n_fields(rec, index) >= rec_offs_n_fields(offsets));
	offsets[2] = (ulint) rec;
	offsets[3] = (ulint) index;
}
#endif /* IB_DEBUG */

/// \brief The following function is used to get an offset to the nth data field in a record.
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n index of the field
/// \param [out] len length of the field; IB_SQL_NULL if SQL null
/// \return offset from the origin of rec
IB_INLINE ulint rec_get_nth_field_offs(const ulint* offsets, ulint n, ulint* len) {
	ut_ad(n < rec_offs_n_fields(offsets));
	ut_ad(len);
	ulint offs;
	if (IB_UNLIKELY(n == 0)) {
		offs = 0;
	} else {
		offs = rec_offs_base(offsets)[n] & REC_OFFS_MASK;
	}
	ulint length = rec_offs_base(offsets)[1 + n];
	if (length & REC_OFFS_SQL_NULL) {
		length = IB_SQL_NULL;
	} else {
		length &= REC_OFFS_MASK;
		length -= offs;
	}
	*len = length;
	return offs;
}

/// \brief Determine if the offsets are for a record in the new compact format.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return nonzero if compact format
IB_INLINE ulint rec_offs_comp(const ulint* offsets) {
	ut_ad(rec_offs_validate(NULL, NULL, offsets));
    return *rec_offs_base(offsets) & REC_OFFS_COMPACT;
}
/// \brief Determine if the offsets are for a record containing externally stored columns.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return nonzero if externally stored
IB_INLINE ulint rec_offs_any_extern(const ulint* offsets) {
	ut_ad(rec_offs_validate(NULL, NULL, offsets));
    return IB_UNLIKELY(*rec_offs_base(offsets) & REC_OFFS_EXTERNAL);
}
/// \brief Returns nonzero if the extern bit is set in nth field of rec.
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n nth field
/// \return nonzero if externally stored
IB_INLINE ulint rec_offs_nth_extern(const ulint* offsets, ulint n) {
	ut_ad(rec_offs_validate(NULL, NULL, offsets));
	ut_ad(n < rec_offs_n_fields(offsets));
    return IB_UNLIKELY(rec_offs_base(offsets)[1 + n] & REC_OFFS_EXTERNAL);
}
/// \brief Returns nonzero if the SQL NULL bit is set in nth field of rec.
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n nth field
/// \return nonzero if SQL NULL
IB_INLINE ulint rec_offs_nth_sql_null(const ulint* offsets, ulint n) {
	ut_ad(rec_offs_validate(NULL, NULL, offsets));
	ut_ad(n < rec_offs_n_fields(offsets));
    return IB_UNLIKELY(rec_offs_base(offsets)[1 + n] & REC_OFFS_SQL_NULL);
}

/// \brief Gets the physical size of a field.
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n nth field
/// \return length of field
IB_INLINE ulint rec_offs_nth_size(const ulint* offsets, ulint n) {
	ut_ad(rec_offs_validate(NULL, NULL, offsets));
	ut_ad(n < rec_offs_n_fields(offsets));
	if (!n) {
        return rec_offs_base(offsets)[1 + n] & REC_OFFS_MASK;
    }
    return (rec_offs_base(offsets)[1 + n] - rec_offs_base(offsets)[n]) & REC_OFFS_MASK;
}
/// \brief Returns the number of extern bits set in a record.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return number of externally stored fields
IB_INLINE ulint rec_offs_n_extern(const ulint* offsets) {
    ulint n = 0;
    if (rec_offs_any_extern(offsets)) {
        for (ulint i = rec_offs_n_fields(offsets); i--; ) {
            if (rec_offs_nth_extern(offsets, i)) {
                n++;
            }
        }
    }
    return n;
}

/// \brief Returns the offset of n - 1th field end if the record is stored in the 1-byte offsets form. If the field is SQL null, the flag is ORed in the returned value. This function and the 2-byte counterpart are defined here because the C-compiler was not able to sum negative and positive constant offsets, and warned of constant arithmetic overflow within the compiler.
/// \param [in] rec record
/// \param [in] n field index
/// \return offset of the start of the PREVIOUS field, SQL null flag ORed
IB_INLINE ulint rec_1_get_prev_field_end_info(const rec_t* rec, ulint n) {
	ut_ad(rec_get_1byte_offs_flag(rec));
	ut_ad(n <= rec_get_n_fields_old(rec));
    return mach_read_from_1(rec - (REC_N_OLD_EXTRA_BYTES + n));
}
/// \brief Returns the offset of n - 1th field end if the record is stored in the 2-byte offsets form. If the field is SQL null, the flag is ORed in the returned value.
/// \param [in] rec record
/// \param [in] n field index
/// \return offset of the start of the PREVIOUS field, SQL null flag ORed
IB_INLINE ulint rec_2_get_prev_field_end_info(const rec_t* rec, ulint n) {
	ut_ad(!rec_get_1byte_offs_flag(rec));
	ut_ad(n <= rec_get_n_fields_old(rec));
    return mach_read_from_2(rec - (REC_N_OLD_EXTRA_BYTES + 2 * n));
}

/// \brief Sets the field end info for the nth field if the record is stored in the 1-byte format.
/// \param [in] rec record
/// \param [in] n field index
/// \param [in] info value to set
IB_INLINE void rec_1_set_field_end_info(rec_t* rec, ulint n, ulint info) {
	ut_ad(rec_get_1byte_offs_flag(rec));
	ut_ad(n < rec_get_n_fields_old(rec));
	mach_write_to_1(rec - (REC_N_OLD_EXTRA_BYTES + n + 1), info);
}
/// \brief Sets the field end info for the nth field if the record is stored in the 2-byte format.
/// \param [in] rec record
/// \param [in] n field index
/// \param [in] info value to set
IB_INLINE void rec_2_set_field_end_info(rec_t* rec, ulint n, ulint info) {
	ut_ad(!rec_get_1byte_offs_flag(rec));
	ut_ad(n < rec_get_n_fields_old(rec));
	mach_write_to_2(rec - (REC_N_OLD_EXTRA_BYTES + 2 * n + 2), info);
}

/// \brief Returns the offset of nth field start if the record is stored in the 1-byte offsets form.
/// \param [in] rec record
/// \param [in] n field index
/// \return offset of the start of the field
IB_INLINE ulint rec_1_get_field_start_offs(const rec_t* rec, ulint n) {
	ut_ad(rec_get_1byte_offs_flag(rec));
	ut_ad(n <= rec_get_n_fields_old(rec));
	if (n == 0) {
        return 0;
    }
    return rec_1_get_prev_field_end_info(rec, n) & ~REC_1BYTE_SQL_NULL_MASK;
}
/// \brief Returns the offset of nth field start if the record is stored in the 2-byte offsets form.
/// \param [in] rec record
/// \param [in] n field index
/// \return offset of the start of the field
IB_INLINE ulint rec_2_get_field_start_offs(const rec_t* rec, ulint n) {
	ut_ad(!rec_get_1byte_offs_flag(rec));
	ut_ad(n <= rec_get_n_fields_old(rec));
	if (n == 0) {
        return 0;
    }
    return rec_2_get_prev_field_end_info(rec, n) & ~(REC_2BYTE_SQL_NULL_MASK | REC_2BYTE_EXTERN_MASK);
}

/// \brief The following function is used to read the offset of the start of a data field in the record. The start of an SQL null field is the end offset of the previous non-null field, or 0, if none exists. If n is the number of the last field + 1, then the end offset of the last field is returned.
/// \param [in] rec record
/// \param [in] n field index
/// \return offset of the start of the field
IB_INLINE ulint rec_get_field_start_offs(const rec_t* rec, ulint n) {
	ut_ad(rec);
	ut_ad(n <= rec_get_n_fields_old(rec));
	if (n == 0) {
        return 0;
	}
	if (rec_get_1byte_offs_flag(rec)) {
        return rec_1_get_field_start_offs(rec, n);
    }
    return rec_2_get_field_start_offs(rec, n);
}

/// \brief Gets the physical size of an old-style field. Also an SQL null may have a field of size > 0, if the data type is of a fixed size.
/// \param [in] rec record
/// \param [in] n index of the field
/// \return field size in bytes
IB_INLINE ulint rec_get_nth_field_size(const rec_t* rec, ulint n) {
    ulint os = rec_get_field_start_offs(rec, n);
    ulint next_os = rec_get_field_start_offs(rec, n + 1);
	ut_ad(next_os - os < IB_PAGE_SIZE);
    return next_os - os;
}

/// \brief This is used to modify the value of an already existing field in a record. The previous value must have exactly the same size as the new value. If len is IB_SQL_NULL then the field is treated as an SQL null. For records in ROW_FORMAT=COMPACT (new-style records), len must not be IB_SQL_NULL unless the field already is SQL null.
/// \param [in] rec record
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n index number of the field
/// \param [in] data pointer to the data if not SQL null
/// \param [in] len length of the data or IB_SQL_NULL
IB_INLINE void rec_set_nth_field(rec_t* rec, const ulint* offsets, ulint n, const void* data, ulint len) {
	ut_ad(rec);
	ut_ad(rec_offs_validate(rec, NULL, offsets));
	if (IB_UNLIKELY(len == IB_SQL_NULL)) {
		if (!rec_offs_nth_sql_null(offsets, n)) {
			ut_a(!rec_offs_comp(offsets));
			rec_set_nth_field_sql_null(rec, n);
		}
		return;
	}
	ulint len2;
	byte* data2 = rec_get_nth_field(rec, offsets, n, &len2);
	if (len2 == IB_SQL_NULL) {
		ut_ad(!rec_offs_comp(offsets));
		rec_set_nth_field_null_bit(rec, n, FALSE);
		ut_ad(len == rec_get_nth_field_size(rec, n));
	} else {
		ut_ad(len2 == len);
	}
	ut_memcpy(data2, data, len);
}

/// \brief The following function returns the data size of an old-style physical record, that is the sum of field lengths. SQL null fields are counted as length 0 fields. The value returned by the function is the distance from record origin to record end in bytes.
/// \param [in] rec physical record
/// \return size
IB_INLINE ulint rec_get_data_size_old(const rec_t* rec) {
	ut_ad(rec);
    return rec_get_field_start_offs(rec, rec_get_n_fields_old(rec));
}
/// \brief The following function sets the number of fields in offsets.
/// \param [in,out] offsets array returned by rec_get_offsets()
/// \param [in] n_fields number of fields
IB_INLINE void rec_offs_set_n_fields(ulint* offsets, ulint n_fields) {
	ut_ad(offsets);
	ut_ad(n_fields > 0);
	ut_ad(n_fields <= REC_MAX_N_FIELDS);
    ut_ad(n_fields + REC_OFFS_HEADER_SIZE <= rec_offs_get_n_alloc(offsets));
	offsets[1] = n_fields;
}

/// \brief The following function returns the data size of a physical record, that is the sum of field lengths. SQL null fields are counted as length 0 fields. The value returned by the function is the distance from record origin to record end in bytes.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return size
IB_INLINE ulint rec_offs_data_size(const ulint* offsets) {
    ut_ad(rec_offs_validate(NULL, NULL, offsets));
    ulint size = rec_offs_base(offsets)[rec_offs_n_fields(offsets)] & REC_OFFS_MASK;
    ut_ad(size < IB_PAGE_SIZE);
    return size;
}
/// \brief Returns the total size of record minus data size of record. The value returned by the function is the distance from record start to record origin in bytes.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return size
IB_INLINE ulint rec_offs_extra_size(const ulint* offsets) {
    ut_ad(rec_offs_validate(NULL, NULL, offsets));
    ulint size = *rec_offs_base(offsets) & ~(REC_OFFS_COMPACT | REC_OFFS_EXTERNAL);
    ut_ad(size < IB_PAGE_SIZE);
    return size;
}
/// \brief Returns the total size of a physical record.
/// \param [in] offsets array returned by rec_get_offsets()
/// \return size
IB_INLINE ulint rec_offs_size(const ulint* offsets) {
    return rec_offs_data_size(offsets) + rec_offs_extra_size(offsets);
}

/// \brief Returns a pointer to the end of the record.
/// \param [in] rec pointer to record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return pointer to end
IB_INLINE byte* rec_get_end(rec_t* rec, const ulint* offsets) {
	ut_ad(rec_offs_validate(rec, NULL, offsets));
    return rec + rec_offs_data_size(offsets);
}
/// \brief Returns a pointer to the start of the record.
/// \param [in] rec pointer to record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return pointer to start
IB_INLINE byte* rec_get_start(rec_t* rec, const ulint* offsets) {
	ut_ad(rec_offs_validate(rec, NULL, offsets));
    return rec - rec_offs_extra_size(offsets);
}

/// \brief Copies a physical record to a buffer.
/// \param [in] buf buffer
/// \param [in] rec physical record
/// \param [in] offsets array returned by rec_get_offsets()
/// \return pointer to the origin of the copy
IB_INLINE rec_t* rec_copy(void* buf, const rec_t* rec, const ulint* offsets) {
    ulint extra_len = rec_offs_extra_size(offsets);
    ulint data_len = rec_offs_data_size(offsets);
	ut_ad(rec && buf);
	ut_ad(rec_offs_validate((rec_t*) rec, NULL, offsets));
	ut_ad(rec_validate(rec, offsets));
	ut_memcpy(buf, rec - extra_len, extra_len + data_len);
    return (byte*)buf + extra_len;
}

/// \brief Returns the extra size of an old-style physical record if we know its data size and number of fields.
/// \param [in] data_size data size
/// \param [in] n_fields number of fields
/// \param [in] n_ext number of externally stored columns
/// \return extra size
IB_INLINE ulint rec_get_converted_extra_size(ulint data_size, ulint n_fields, ulint n_ext) {
	if (!n_ext && data_size <= REC_1BYTE_OFFS_LIMIT) {
        return REC_N_OLD_EXTRA_BYTES + n_fields;
    }
    return REC_N_OLD_EXTRA_BYTES + 2 * n_fields;
}

/// \brief The following function returns the size of a data tuple when converted to a physical record.
/// \param [in] dict_index record descriptor
/// \param [in] dtuple data tuple
/// \param [in] n_ext number of externally stored columns
/// \return size
IB_INLINE ulint rec_get_converted_size(dict_index_t* dict_index, const dtuple_t* dtuple, ulint n_ext) {
    ut_ad(dict_index);
    ut_ad(dtuple);
    ut_ad(dtuple_check_typed(dtuple));
    ut_ad(dict_index->type & DICT_UNIVERSAL || dtuple_get_n_fields(dtuple) == (((dtuple_get_info_bits(dtuple) & REC_NEW_STATUS_MASK) == REC_STATUS_NODE_PTR) ? dict_index_get_n_unique_in_tree(dict_index) + 1 : dict_index_get_n_fields(dict_index)));
    if (dict_table_is_comp(dict_index->table)) {
        return rec_get_converted_size_comp(dict_index, dtuple_get_info_bits(dtuple) & REC_NEW_STATUS_MASK, dtuple->fields, dtuple->n_fields, NULL);
    }
    ulint data_size = dtuple_get_data_size(dtuple, 0);
    ulint extra_size = rec_get_converted_extra_size(data_size, dtuple_get_n_fields(dtuple), n_ext);
    return data_size + extra_size;
}

#ifndef IB_HOTBACKUP
/// \brief Folds a prefix of a physical record to a ulint. Folds only existing fields, that is, checks that we do not run out of the record.
/// \param [in] rec the physical record
/// \param [in] offsets array returned by rec_get_offsets()
/// \param [in] n_fields number of complete fields to fold
/// \param [in] n_bytes number of bytes to fold in an incomplete last field
/// \param [in] tree_id index tree id
/// \return the folded value
IB_INLINE ulint rec_fold(const rec_t* rec, const ulint* offsets, ulint n_fields, ulint n_bytes, dulint tree_id) {
    ut_ad(rec_offs_validate(rec, NULL, offsets));
    ut_ad(rec_validate(rec, offsets));
    ut_ad(n_fields + n_bytes > 0);
    ulint n_fields_rec = rec_offs_n_fields(offsets);
    ut_ad(n_fields <= n_fields_rec);
    ut_ad(n_fields < n_fields_rec || n_bytes == 0);
    if (n_fields > n_fields_rec) {
        n_fields = n_fields_rec;
    }
    if (n_fields == n_fields_rec) {
        n_bytes = 0;
    }
    ulint fold = ut_fold_dulint(tree_id);
    for (ulint i = 0; i < n_fields; i++) {
        ulint len;
        const byte* data = rec_get_nth_field(rec, offsets, i, &len);
        if (len != IB_SQL_NULL) {
            fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
        }
    }
    if (n_bytes > 0) {
        ulint len;
        const byte* data = rec_get_nth_field(rec, offsets, n_fields, &len);
        if (len != IB_SQL_NULL) {
            if (len > n_bytes) {
                len = n_bytes;
            }
            fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
        }
    }
    return fold;
}
#endif /* !IB_HOTBACKUP */
