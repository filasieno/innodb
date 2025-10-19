// Copyright (c) 1996, 2010, Innobase Oy. All Rights Reserved.
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

/// \file data_type.inl
/// \brief Data types
/// \details Originally created by Heikki Tuuri in 1/16/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "api_ucode.hpp"
#include "mach_data.hpp"

#ifndef IB_HOTBACKUP

/// \brief Gets the client charset-collation code for user string types.
/// \param [in] prtype precise data type
/// \return client charset-collation code
IB_INLINE ulint dtype_get_charset_coll(ulint prtype)
{
    return (prtype >> 16) & 0xFFUL;
}

/// \brief Gets the user type code from a dtype.
/// \param [in] type type struct
/// \return User type code; this is NOT an InnoDB type code!
IB_INLINE ulint dtype_get_attrib(const dtype_t* type)
{
    return type->prtype & 0xFFUL;
}

/// \brief Compute the mbminlen and mbmaxlen members of a data type structure.
/// \param [in] mtype main type
/// \param [in] prtype precise type (and collation)
/// \param [out] mbminlen minimum length of a multi-byte character
/// \param [out] mbmaxlen maximum length of a multi-byte character
IB_INLINE void dtype_get_mblen(ulint mtype, ulint prtype, ulint* mbminlen, ulint* mbmaxlen)
{
    if (dtype_is_string_type(mtype)) {
        const charset_t* = ib_ucode_get_charset(dtype_get_charset_coll(prtype));
        ib_ucode_get_charset_width(cs, mbminlen, mbmaxlen);
        ut_ad(*mbminlen <= *mbmaxlen);
        ut_ad(*mbminlen <= 2); /* mbminlen in dtype_t is 0..3 */
        ut_ad(*mbmaxlen < 1 << 3); /* mbmaxlen in dtype_t is 0..7 */
    } else {
        *mbminlen = *mbmaxlen = 0;
    }
}

/// \brief Compute the mbminlen and mbmaxlen members of a data type structure.
/// \param [in,out] type type
IB_INLINE void dtype_set_mblen(dtype_t* type)
{
    ulint mbminlen;
    ulint mbmaxlen;

    dtype_get_mblen(type->mtype, type->prtype, &mbminlen, &mbmaxlen);
    type->mbminlen = mbminlen;
    type->mbmaxlen = mbmaxlen;
    ut_ad(dtype_validate(type));
}

#else // !IB_HOTBACKUP

    #define dtype_set_mblen(type) (void) 0

#endif // !IB_HOTBACKUP

/// \brief Sets a data type structure.
/// \param [in] type type struct to init
/// \param [in] mtype main data type
/// \param [in] prtype precise type
/// \param [in] len precision of type
IB_INLINE void dtype_set(dtype_t* type, ulint mtype, ulint prtype, ulint len)
{
    ut_ad(type);
    ut_ad(mtype <= DATA_MTYPE_MAX);

    type->mtype = mtype;
    type->prtype = prtype;
    type->len = len;
    dtype_set_mblen(type);
}

/// \brief Copies a data type structure.
/// \param [in] type1 type struct to copy to
/// \param [in] type2 type struct to copy from
IB_INLINE void dtype_copy(dtype_t* type1, const dtype_t* type2)
{
    *type1 = *type2;
    ut_ad(dtype_validate(type1));
}

/// \brief Gets the SQL main data type.
/// \param [in] type data type
/// \return SQL main data type
IB_INLINE ulint dtype_get_mtype(const dtype_t* type)
{
    ut_ad(type);
    return type->mtype;
}

/// \brief Gets the precise data type.
/// \param [in] type data type
/// \return precise data type
IB_INLINE ulint dtype_get_prtype(const dtype_t* type)
{
    ut_ad(type);
    return type->prtype;
}

/// \brief Gets the type length.
/// \param [in] type data type
/// \return fixed length of the type, in bytes, or 0 if variable-length
IB_INLINE ulint dtype_get_len(const dtype_t* type)
{
    ut_ad(type);
    return type->len;
}

#ifndef IB_HOTBACKUP

/// \brief Gets the minimum length of a character, in bytes.
/// \param [in] type type
/// \return minimum length of a char, in bytes, or 0 if this is not a character type
IB_INLINE ulint dtype_get_mbminlen(const dtype_t* type)
{
    ut_ad(type);
    return type->mbminlen;
}
/// \brief Gets the maximum length of a character, in bytes.
/// \param [in] type type
/// \return maximum length of a char, in bytes, or 0 if this is not a character type
IB_INLINE ulint dtype_get_mbmaxlen(const dtype_t* type)
{
    ut_ad(type);
    return type->mbmaxlen;
}

/// \brief Gets the padding character code for a type.
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \return padding character code, or ULINT_UNDEFINED if no padding specified
IB_INLINE ulint dtype_get_pad_char(ulint mtype, ulint prtype)
{
    switch (mtype) {
    case DATA_FIXBINARY:
    case DATA_BINARY:
        if (IB_UNLIKELY(dtype_get_charset_coll(prtype) == DATA_CLIENT_BINARY_CHARSET_COLL)) {
            // Starting from 5.0.18, do not pad VARBINARY or BINARY columns.
            return ULINT_UNDEFINED;
        }
        // Fall through
    case DATA_CHAR:
    case DATA_VARCHAR:
    case DATA_CLIENT:
    case DATA_VARCLIENT:
        // Space is the padding character for all char and binary strings, and starting from 5.0.3, also for TEXT strings.
        return 0x20;
    case DATA_BLOB:
        if (!(prtype & DATA_BINARY_TYPE)) {
            return 0x20;
        }
        // Fall through
    default:
        // No padding specified
        return ULINT_UNDEFINED;
    }
}

/// \brief Stores for a type the information which determines its alphabetical ordering and the storage size of an SQL NULL value.
/// \details This is the >= 4.1.x storage format.
/// \param [in] buf buffer for DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE bytes where we store the info
/// \param [in] type type struct
/// \param [in] prefix_len prefix length to replace type->len, or 0
IB_INLINE void dtype_new_store_for_order_and_null_size(byte* buf, const dtype_t* type, ulint prefix_len)
{
    static_assert(DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE == 6, "DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE must be 6");

    ut_ad(type);
    ut_ad(type->mtype >= DATA_VARCHAR);
    ut_ad(type->mtype <= DATA_MYSQL);

    buf[0] = (byte)(type->mtype & 0xFFUL);
    if (type->prtype & DATA_BINARY_TYPE) buf[0] = buf[0] | 128;
    buf[1] = (byte)(type->prtype & 0xFFUL);
    ulint len = prefix_len ? prefix_len : type->len;
    mach_write_to_2(buf + 2, len & 0xFFFFUL);
    ut_ad(dtype_get_charset_coll(type->prtype) < 256);
    mach_write_to_2(buf + 4, dtype_get_charset_coll(type->prtype));
    if (type->prtype & DATA_NOT_NULL) buf[4] |= 128;
}

/// \brief Reads to a type the stored information which determines its alphabetical ordering and the storage size of an SQL NULL value.
/// \details This is the < 4.1.x storage format.
/// \param [in] type type struct
/// \param [in] buf buffer for stored type order info
IB_INLINE void dtype_read_for_order_and_null_size(dtype_t* type, const byte* buf)
{
    static_assert(DATA_ORDER_NULL_TYPE_BUF_SIZE == 4, "DATA_ORDER_NULL_TYPE_BUF_SIZE must be 4");

    type->mtype = buf[0] & 63;
    type->prtype = buf[1];
    if (buf[0] & 128) type->prtype = type->prtype | DATA_BINARY_TYPE;
    type->len = mach_read_from_2(buf + 2);
    type->prtype = dtype_form_prtype(type->prtype, data_client_default_charset_coll);
    dtype_set_mblen(type);
}

/// \brief Reads to a type the stored information which determines its alphabetical ordering and the storage size of an SQL NULL value.
/// \details This is the >= 4.1.x storage format.
/// \param [in] type type struct
/// \param [in] buf buffer for stored type order info
IB_INLINE void dtype_new_read_for_order_and_null_size(dtype_t* type, const byte* buf)
{
    static_assert(DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE == 6, "DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE must be 6");

    type->mtype = buf[0] & 63;
    type->prtype = buf[1];
    if (buf[0] & 128) type->prtype |= DATA_BINARY_TYPE;
    if (buf[4] & 128) type->prtype |= DATA_NOT_NULL;
    type->len = mach_read_from_2(buf + 2);
    ulint charset_coll = mach_read_from_2(buf + 4) & 0x7fff;

    if (dtype_is_string_type(type->mtype)) {
#if 0
        // FIXME: This is probably MySQL specific too.
        ut_a(charset_coll > 0);
        ut_a(charset_coll < 256);

        type->prtype = dtype_form_prtype(type->prtype, charset_coll);
#else
        type->prtype = 1; // FIXME: Hack for testing.
#endif
    }
    dtype_set_mblen(type);
}
#endif // !IB_HOTBACKUP 

/// \brief Returns the size of a fixed size data type, 0 if not a fixed size type.
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \param [in] len length
/// \param [in] mbminlen minimum length of a multibyte char
/// \param [in] mbmaxlen maximum length of a multibyte char
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT
/// \return fixed size, or 0
IB_INLINE ulint dtype_get_fixed_size_low(ulint mtype, ulint prtype, ulint len, ulint mbminlen, ulint mbmaxlen, ulint comp)
{
    switch (mtype) {
    case DATA_SYS:
    if constexpr (IB_DEBUG) switch (prtype & DATA_CLIENT_TYPE_MASK) {
        case DATA_ROW_ID:
            ut_ad(len == DATA_ROW_ID_LEN);
            break;
        case DATA_TRX_ID:
            ut_ad(len == DATA_TRX_ID_LEN);
            break;
        case DATA_ROLL_PTR:
            ut_ad(len == DATA_ROLL_PTR_LEN);
            break;
        default:
            ut_ad(0);
            return 0;
    }
    case DATA_CHAR:
    case DATA_FIXBINARY:
    case DATA_INT:
    case DATA_FLOAT:
    case DATA_DOUBLE:
        return len;
    case DATA_CLIENT:
        if ((prtype & DATA_BINARY_TYPE) || mbminlen == mbmaxlen) {
            return len;
        }
        // fall through for variable-length charsets
    case DATA_VARCHAR:
    case DATA_BINARY:
    case DATA_DECIMAL:
    case DATA_VARCLIENT:
    case DATA_BLOB:
        return 0;
    default:
        UT_ERROR;
    }

    return 0;
}

#ifndef IB_HOTBACKUP

/// \brief Returns the minimum size of a data type.
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \param [in] len length
/// \param [in] mbminlen minimum length of a multibyte char
/// \param [in] mbmaxlen maximum length of a multibyte char
/// \return minimum size
IB_INLINE ulint dtype_get_min_size_low(ulint mtype, ulint prtype, ulint len, ulint mbminlen, ulint mbmaxlen)
{
    switch (mtype) {
    case DATA_SYS:
    if constexpr (IB_DEBUG) switch (prtype & DATA_CLIENT_TYPE_MASK) {
        case DATA_ROW_ID:
            ut_ad(len == DATA_ROW_ID_LEN);
            break;
        case DATA_TRX_ID:
            ut_ad(len == DATA_TRX_ID_LEN);
            break;
        case DATA_ROLL_PTR:
            ut_ad(len == DATA_ROLL_PTR_LEN);
            break;
        default:
            ut_ad(0);
            return 0;
    }
    case DATA_CHAR:
    case DATA_FIXBINARY:
    case DATA_INT:
    case DATA_FLOAT:
    case DATA_DOUBLE:
        return len;
    case DATA_CLIENT:
        if ((prtype & DATA_BINARY_TYPE) || mbminlen == mbmaxlen) {
            return len;
        }
        // this is a variable-length character set
        ut_a(mbminlen > 0);
        ut_a(mbmaxlen > mbminlen);
        ut_a(len % mbmaxlen == 0);
        return len * mbminlen / mbmaxlen;
    case DATA_VARCHAR:
    case DATA_BINARY:
    case DATA_DECIMAL:
    case DATA_VARCLIENT:
    case DATA_BLOB:
        return 0;
    default:
        UT_ERROR;
    }

    return 0;
}

/// \brief Returns the maximum size of a data type.
/// \details Note: types in system tables may be incomplete and return incorrect information.
/// \param [in] mtype main type
/// \param [in] len length
/// \return maximum size
IB_INLINE ulint dtype_get_max_size_low(ulint mtype, ulint len)
{
    switch (mtype) {
    case DATA_SYS:
    case DATA_CHAR:
    case DATA_FIXBINARY:
    case DATA_INT:
    case DATA_FLOAT:
    case DATA_DOUBLE:
    case DATA_CLIENT:
    case DATA_VARCHAR:
    case DATA_BINARY:
    case DATA_DECIMAL:
    case DATA_VARCLIENT:
        return len;
    case DATA_BLOB:
        break;
    default:
        UT_ERROR;
    }

    return ULINT_MAX;
}

#endif // !IB_HOTBACKUP

/// \brief Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a type.
/// \details For fixed length types it is the fixed length of the type, otherwise 0.
/// \param [in] type type
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT
/// \return SQL null storage size in ROW_FORMAT=REDUNDANT
IB_INLINE ulint dtype_get_sql_null_size(const dtype_t* type, ulint comp)
{
    if constexpr (IB_HOTBACKUP) {
        return dtype_get_fixed_size_low(type->mtype, type->prtype, type->len, type->mbminlen, type->mbmaxlen, comp);
    } else {
        return dtype_get_fixed_size_low(type->mtype, type->prtype, type->len, 0, 0, 0);
    }
}
