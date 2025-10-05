// Copyright (c) 2025, Fabio N. Filasieno. All Rights Reserved.
// Copyright (c) 2008, 2025, Innobase Oy. All Rights Reserved.
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

/// @file api_ucode.hpp
/// \brief Unicode character set handling functions

#pragma once

// TODO: Review inclues

/// Opaque type used by the Unicode implementation.
typedef struct charset_struct charset_t;

/// \brief Determines the connection character set.
/// \return Connection character set.
UNIV_INTERN const charset_t* ib_ucode_get_connection_charset(void);

/// \brief Determines the character set based on id.
/// \param id Charset-collation code.
/// \return Connection character set.
UNIV_INTERN const charset_t* ib_ucode_get_charset(ulint id);

/// \brief Get the variable length bounds of the given (multibyte) character set.
/// \param cs Character set.
/// \param mbminlen Min len of a char (in bytes).
/// \param mbmaxlen Max len of a char (in bytes).
UNIV_INTERN void ib_ucode_get_charset_width(const charset_t* cs, ulint* mbminlen, ulint* mbmaxlen);

/// \brief This function is used to find the storage length in bytes of the
/// characters that will fit into prefix_len bytes.
/// \param cs Character set id.
/// \param prefix_len Prefix length in bytes.
/// \param str_len Length of the string in bytes.
/// \param str Character string.
/// \return Number of bytes required to copy the characters that will fit into prefix_len bytes.
UNIV_INTERN ulint ib_ucode_get_storage_size(const charset_t* cs, ulint prefix_len, ulint str_len, const char* str);

/// \brief Compares NUL-terminated UTF-8 strings case insensitively.
/// \param a First string to compare.
/// \param b Second string to compare.
/// \return 0 if a=b, <0 if a<b, >1 if a>b.
UNIV_INTERN int ib_utf8_strcasecmp(const char* a, const char* b);

/// \brief Compares NUL-terminated UTF-8 strings case insensitively.
/// \param a First string to compare.
/// \param b Second string to compare.
/// \param n No. of bytes to compare.
/// \return 0 if a=b, <0 if a<b, >1 if a>b.
UNIV_INTERN int ib_utf8_strncasecmp(const char* a, const char* b, ulint n);

/// \brief Makes all characters in a NUL-terminated UTF-8 string lower case.
/// \param a Str to put in lower case.
UNIV_INTERN void ib_utf8_casedown(char* a);

/// \brief Test whether a UTF-8 character is a space or not.
/// \param cs Character set.
/// \param c Character to test.
/// \return TRUE if isspace(c).
UNIV_INTERN int ib_utf8_isspace(const charset_t* cs, char c);

/// \brief Converts an identifier to a UTF-8 table name.
/// \param cs The 'from' character set.
/// \param to Converted identifier.
/// \param from Identifier to convert.
/// \param to_len Length of 'to', in bytes.
UNIV_INTERN void ib_utf8_convert_from_table_id(const charset_t* cs, char* to, const char* from, ulint to_len);
					
/// \brief Converts an identifier to UTF-8.
/// \param cs The 'from' character set.
/// \param to Converted identifier.
/// \param from Identifier to convert.
/// \param to_len Length of 'to', in bytes; should be at least 3 * strlen(to) + 1.
UNIV_INTERN void ib_utf8_convert_from_id(const charset_t* cs, char* to, const char* from, ulint to_len);

