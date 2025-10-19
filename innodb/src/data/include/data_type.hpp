// Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA

/// \file data_type.hpp
/// \brief Data types
/// \details Originally created by Heikki Tuuri in 1/16/1996
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "defs.hpp"

// SQL data type struct
typedef struct dtype_struct dtype_t;

extern ulint data_client_default_charset_coll;

constinit ulint DATA_CLIENT_LATIN1_SWEDISH_CHARSET_COLL = 8;
constinit ulint DATA_CLIENT_BINARY_CHARSET_COLL = 63;


// The 'MAIN TYPE' of a column

// Main data type codes
/// \brief Character varying of the latin1_swedish_ci charset-collation
constinit ulint DATA_VARCHAR = 1; 
/// \brief Fixed length character of the latin1_swedish_ci charset-collation
constinit ulint DATA_CHAR = 2;
/// \brief binary string of fixed length
constinit ulint DATA_FIXBINARY = 3;
/// \brief binary string
constinit ulint DATA_BINARY = 4;
/// \brief binary large object, or a TEXT type; if prtype & DATA_BINARY_TYPE == 0, then this is actually a TEXT column (or a BLOB created with < 4.0.14; since column prefix indexes came only in 4.0.14, the missing flag in BLOBs created before that does not cause any harm)
constinit ulint DATA_BLOB = 5;
/// \brief integer: can be any size 1 - 8 bytes
constinit ulint DATA_INT = 6;
/// \brief address of the child page in node pointer
constinit ulint DATA_SYS_CHILD = 7;
/// \brief system column
constinit ulint DATA_SYS = 8;

// Data types >= DATA_FLOAT must be compared using the whole field, not as binary strings

/// \brief C (float) floating point value.
constinit ulint DATA_FLOAT = 9;
/// \brief C (double) floating point value.
constinit ulint DATA_DOUBLE = 10;
/// \brief decimal number stored as an ASCII string
constinit ulint DATA_DECIMAL = 11;
/// \brief any charset varying length char
constinit ulint DATA_VARCLIENT = 12;
/// \brief any charset fixed length char
constinit ulint DATA_CLIENT = 13;

/// \brief NOTE that 4.1.1 used DATA_CLIENT and DATA_VARCLIENT for all character sets, and the charset-collation for tables created with it can also be latin1_swedish_ci
constinit ulint DATA_MTYPE_MAX = 63;

// The 'PRECISE TYPE' of a column
// User tables have the following convention:
// - In the least significant byte in the precise type we store the user type
// code (not applicable for system columns).
// - In the second least significant byte we OR flags DATA_NOT_NULL,
// DATA_UNSIGNED, DATA_BINARY_TYPE.
// - In the third least significant byte of the precise type of string types we
// store the user charset-collation code. In DATA_BLOB columns created with
// < 4.0.14 we do not actually know if it is a BLOB or a TEXT column. Since there
// are no indexes on prefixes of BLOB or TEXT columns in < 4.0.14, this is no
// problem, though.
// If the stored charset code is 0 in the system table SYS_COLUMNS
// of InnoDB, that means that the default charset of this installation
// should be used.
// When loading a table definition from the system tables to the InnoDB data
// dictionary cache in main memory, if the stored charset-collation is 0, and
// the type is a non-binary string, replace that 0 by the default
// charset-collation code of the installation. In short, in old tables, the
// charset-collation code in the system tables on disk can be 0, but in
// in-memory data structures (dtype_t), the charset-collation code is
// always != 0 for non-binary string types.
// In new tables, in binary string types, the charset-collation code is the
// user code for the 'binary charset', that is, != 0.
// For binary string types and for DATA_CHAR, DATA_VARCHAR, and for those
// DATA_BLOB which are binary or have the charset-collation latin1_swedish_ci,
// InnoDB performs all comparisons internally, without resorting to the user
// comparison functions. This is to save CPU time.
// InnoDB's own internal system tables have different precise types for their
// columns, and for them the precise type is usually not used at all.

constinit ulint DATA_ENGLISH = 4; // English language character string: only used for InnoDB's own system tables
constinit ulint DATA_ERROR = 111; // Used for error checking and debugging

constinit ulint DATA_CLIENT_TYPE_MASK = 255; // AND with this mask to extract the user type from the precise type

// Precise data types for system columns and the length of those columns;
// NOTE: the values must run from 0 up in the order given! All codes must
// be less than 256
constinit ulint DATA_ROW_ID = 0; // row id: a dulint
constinit ulint DATA_ROW_ID_LEN = 6; // stored length for row id

constinit ulint DATA_TRX_ID = 1; // transaction id: 6 bytes
constinit ulint DATA_TRX_ID_LEN = 6;

constinit ulint DATA_ROLL_PTR = 2; // rollback data pointer: 7 bytes
constinit ulint DATA_ROLL_PTR_LEN = 7;

constinit ulint DATA_N_SYS_COLS = 3; // number of system columns defined above

constinit ulint DATA_SYS_PRTYPE_MASK = 0xF; // mask to extract the above from prtype

// Flags ORed to the precise data type
constinit ulint DATA_NOT_NULL = 256; // this is ORed to the precise type when the column is declared as NOT NULL
constinit ulint DATA_UNSIGNED = 512; // this id ORed to the precise type when we have an unsigned integer type
constinit ulint DATA_BINARY_TYPE = 1024; // if the data type is a binary character string, this is ORed to the precise type.
constinit ulint DATA_CUSTOM_TYPE = 2048; // first custom type starts here

// This many bytes we need to store the type information affecting the alphabetical order for a single field and decide the storage size of an SQL null
constinit ulint DATA_ORDER_NULL_TYPE_BUF_SIZE = 4;
// In the >= 4.1.x storage format we add 2 bytes more so that we can also store the charset-collation number; one byte is left unused, though
constinit ulint DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE = 6;

#ifndef IB_HOTBACKUP

/// \brief Determine how many bytes the first n characters of the given string occupy.
/// \details If the string is shorter than n characters, returns the number of bytes the characters in the string occupy.
/// \param [in] prtype precise type
/// \param [in] mbminlen minimum length of a multi-byte character
/// \param [in] mbmaxlen maximum length of a multi-byte character
/// \param [in] prefix_len length of the requested prefix, in characters, multiplied by dtype_get_mbmaxlen(dtype)
/// \param [in] data_len length of str (in bytes)
/// \param [in] str the string whose prefix length is being determined
/// \return length of the prefix, in bytes
IB_INTERN ulint dtype_get_at_most_n_mbchars(ulint prtype, ulint mbminlen, ulint mbmaxlen, ulint prefix_len, ulint data_len, const char *str);

#endif // !IB_HOTBACKUP

/// \brief Checks if a data main type is a string type. Also a BLOB is considered a string type.
/// \return TRUE if string type
/// \param mtype InnoDB main data type code: DATA_CHAR, ...
IB_INTERN ibool dtype_is_string_type(ulint mtype);

/// \brief Checks if a type is a binary string type. 
/// \note tables created with < 4.0.14, we do not know if a DATA_BLOB column is a BLOB or a TEXT column. For
/// those DATA_BLOB columns this function currently returns FALSE.
/// \return TRUE if binary string type
/// \param mtype main data type
/// \param prtype precise type
IB_INTERN ibool dtype_is_binary_string_type(ulint mtype, ulint prtype);

/// \brief Checks if a type is a non-binary string type. That is, dtype_is_string_type is
/// TRUE and dtype_is_binary_string_type is FALSE. Note that for tables created
/// with < 4.0.14, we do not know if a DATA_BLOB column is a BLOB or a TEXT column.
/// For those DATA_BLOB columns this function currently returns TRUE.
/// \return TRUE if non-binary string type
/// \param mtype main data type
/// \param prtype precise type
IB_INTERN ibool dtype_is_non_binary_string_type(ulint mtype, ulint prtype);

/// \brief Sets a data type structure.
/// \param [in] type type struct to init
/// \param [in] mtype main data type
/// \param [in] prtype precise type
/// \param [in] len precision of type

IB_INLINE void dtype_set(dtype_t* type, ulint mtype, ulint prtype, ulint len);

/// \brief Copies a data type structure.
IB_INLINE void dtype_copy(dtype_t* type1, const dtype_t* type2);

/// \brief Gets the SQL main data type.
/// \param [in] type data type
/// \return SQL main data type
IB_INLINE ulint dtype_get_mtype(const dtype_t* type);

/// \brief Gets the precise data type.
/// \param [in] type data type
/// \return precise data type
IB_INLINE ulint dtype_get_prtype(const dtype_t* type);

#ifndef IB_HOTBACKUP

/// \brief Compute the mbminlen and mbmaxlen members of a data type structure.
/// \param [in] mtype main type
/// \param [in] prtype precise type (and collation)
/// \param [out] mbminlen minimum length of a multi-byte character
/// \param [out] mbmaxlen maximum length of a multi-byte character
IB_INLINE void dtype_get_mblen(ulint mtype, ulint prtype, ulint *mbminlen, ulint *mbmaxlen);

/// \brief Gets the user charset-collation code for user string types.
/// \param [in] prtype precise data type
/// \return user charset-collation code
IB_INLINE ulint dtype_get_charset_coll(ulint prtype);

/// \brief Forms a precise type from the < 4.1.2 format precise type plus the charset-collation code.
/// \param [in] old_prtype the user type code and the flags DATA_BINARY_TYPE etc.
/// \param [in] charset_coll user charset-collation code
/// \return precise type, including the charset-collation code
IB_INTERN ulint dtype_form_prtype(ulint old_prtype, ulint charset_coll);

/// \brief Gets the type length.
/// \param [in] type data type
/// \return fixed length of the type, in bytes, or 0 if variable-length
IB_INLINE ulint dtype_get_len(const dtype_t* type);

/// \brief Gets the minimum length of a character, in bytes.
/// \param [in] type data type
/// \return minimum length of a char, in bytes, or 0 if this is not a character type
IB_INLINE ulint dtype_get_len(const dtype_t* type);

/// \brief Gets the minimum length of a character, in bytes.
/// \param [in] type data type
/// \return minimum length of a char, in bytes, or 0 if this is not a character type
IB_INLINE ulint dtype_get_mbminlen(const dtype_t* type);

/// \brief Gets the maximum length of a character, in bytes.
/// \param [in] type type
/// \return maximum length of a char, in bytes, or 0 if this is not a character type
IB_INLINE ulint dtype_get_mbmaxlen(const dtype_t* type);                                      

/// \brief Gets the padding character code for the type.
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \return padding character code, or ULINT_UNDEFINED if no padding specified
IB_INLINE ulint dtype_get_pad_char(ulint mtype, ulint prtype); 

/// \brief Returns the minimum size of a data type.
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \param [in] len length
/// \param [in] mbminlen minimum length of a multibyte char
/// \param [in] mbmaxlen maximum length of a multibyte char
/// \return minimum size
IB_INLINE ulint dtype_get_min_size_low(ulint mtype, ulint prtype, ulint len, ulint mbminlen, ulint mbmaxlen);

/// \brief Returns the maximum size of a data type.
/// \details Note: types in system tables may be incomplete and return incorrect information.
/// \param [in] mtype main type
/// \param [in] len length
/// \return maximum size
IB_INLINE ulint dtype_get_max_size_low(ulint mtype, ulint len);

/// \brief Reads to a type the stored information which determines its alphabetical ordering and the storage size of an SQL NULL value.
/// \param [in] type type struct
/// \param [in] buf buffer for stored type order info
IB_INLINE void dtype_read_for_order_and_null_size(dtype_t *type, const byte *buf);

/// \brief Stores for a type the information which determines its alphabetical ordering and the storage size of an SQL NULL value. This is the 4.1.x storage format.
/// \param [in] buf buffer for DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE bytes where we store the info
/// \param [in] type type struct
/// \param [in] prefix_len prefix length to replace type->len, or 0
IB_INLINE void dtype_new_store_for_order_and_null_size(byte *buf, const dtype_t *type, ulint prefix_len);

/// \brief Reads to a type the stored information which determines its alphabetical ordering and the storage size of an SQL NULL value. This is the 4.1.x storage format.
/// \param [in] type type struct
/// \param [in] buf buffer for stored type order info
IB_INLINE void dtype_new_read_for_order_and_null_size(dtype_t *type, const byte *buf);

/// \brief Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a type.
/// \details For fixed length types it is the fixed length of the type, otherwise 0.
/// \param [in] type type
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT
/// \return SQL null storage size in ROW_FORMAT=REDUNDANT
IB_INLINE ulint dtype_get_sql_null_size(const dtype_t *type, ulint comp);

/// \brief Validates a data type structure.
/// \param [in] type type struct to validate
/// \return TRUE if ok
IB_INTERN ibool dtype_validate(const dtype_t *type);

/// \brief Prints a data type structure.
/// \param [in] type type
IB_INTERN void dtype_print(const dtype_t* type);

/// \brief Gets the user type code from a dtype.
/// \param [in] type type struct 
/// \return	type code; this is NOT an InnoDB type code!
IB_INLINE ulint dtype_get_attrib(const dtype_t *type );

/// \brief Reset dtype variables
IB_INTERN void dtype_var_init(innodb_state* state);

#endif // !IB_HOTBACKUP

/// \brief Returns the size of a fixed size data type, 0 if not a fixed size type.
/// \param [in] mtype main type
/// \param [in] prtype precise type
/// \param [in] len length
/// \param [in] mbminlen minimum length of a multibyte char
/// \param [in] mbmaxlen maximum length of a multibyte char
/// \param [in] comp nonzero=ROW_FORMAT=COMPACT
/// \return fixed size, or 0
IB_INLINE ulint dtype_get_fixed_size_low(ulint mtype, ulint prtype, ulint len, ulint mbminlen, ulint mbmaxlen, ulint comp);

#ifndef IB_DO_NOT_INLINE
    #include "data_type.inl"
#endif

// end of file
