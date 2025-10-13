// Copyright (c) 2025 Fabio N. Filsieno. All Rights Reserved.
// Copyright (c) 2010 Stewart Smith. All Rights Reserved.
// Copyright (c) 2008 Oracle. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

/// @file api_ucode.cpp
/// \brief HailDB API Unicode functions

#include "univ.i"
#include "ut_mem.hpp"
#include "api_ucode.hpp"
#include <ctype.h>

#ifdef IB_HAVE_STRINGS_H
  #include <strings.h>
#endif

/**
@file api/api0ucode.c
Determines the connection character set.
@return	connection character set */
IB_INTERN const charset_t* ib_ucode_get_connection_charset()
{
	return NULL;
}

/**
Determines the character set based on id.
FIXME: If the id can't be found then what do we do, return some default ?
@return	character set or NULL */
/*!< in: Charset-collation code */
IB_INTERN const charset_t* ib_ucode_get_charset(ulint id)
{
	return(NULL);
}

/** Get the variable length bounds of the given (multibyte) character set. */
/*!< in: Charset */
/*!< out: min len of a char (in bytes) */
/*!< out: max len of a char (in bytes) */
IB_INTERN void ib_ucode_get_charset_width(const charset_t* cs, ulint* mbminlen, ulint* mbmaxlen)
{
        *mbminlen = *mbmaxlen = 0;
        if (cs) {
			//FIXME
			//*mbminlen = charset_get_minlen(cs);
			//*mbmaxlen = charset_get_maxlen(cs);
		}
}


/// \brief Compare two strings ignoring case.
/// \param p1 First string to compare.
/// \param p2 Second string to compare.
/// \return 0 if equal
int ib_utf8_strcasecmp(const char* p1, const char* p2)
{
	/* FIXME: Call the UTF-8 comparison function. */
	/* FIXME: This should take cs as the parameter. */
	return strcasecmp(p1, p2) ;
}

/// \brief Compare two strings ignoring case.
/// \param p1 First string to compare.
/// \param p2 Second string to compare.
/// \param len Length of strings to compare.
/// \return 0 if equal
int ib_utf8_strncasecmp(const char* p1, const char* p2, ulint len)
{
	/* FIXME: Call the UTF-8 comparison function. */
	/* FIXME: This should take cs as the parameter. */
	/* FIXME: Which function?  Note that this is locale-dependent.
	For example, there is a capital dotted i and a lower-case
	dotless I (U+0130 and U+0131, respectively).  In many other
	locales, I=i but not in Turkish. */
	return(strncasecmp(p1, p2, len));
}

/// \brief Makes all characters in a NUL-terminated UTF-8 string lower case.
/// \param a String to put in lower case.
IB_INTERN void ib_utf8_casedown(char* a)
{
	/* FIXME: Call the UTF-8 tolower() equivalent. */
	/* FIXME: Is this function really needed?  The proper
	implementation is locale-dependent.  In Turkish, the
	lower-case counterpart of the upper-case I (U+0049, one byte)
	is the dotless i (U+0131, two bytes in UTF-8).  That cannot
	even be converted in place. */
	while (*a) {
		*a = tolower(*a);
		++a;
	}
}

/// \brief Converts an identifier to a table name.
/// \param cs The 'from' character set.
/// \param to Converted identifier.
/// \param from Identifier to convert.
/// \param len Length of 'to', in bytes; should be at least 5 * strlen(to) + 1.
IB_INTERN void ib_utf8_convert_from_table_id(const charset_t*cs, char* to, const char* from, ulint len)
{
	/* FIXME: why 5*strlen(to)+1?  That is a relic from the MySQL
	5.1 filename safe encoding that encodes some chars in
	four-digit hexadecimal notation, such as @0023.  Do we even
	need this function?  Could the files be named by table id or
	something? */
	/* FIXME: Call the UTF-8 equivalent */
	strncpy(to, from, len);
}


/// \brief Converts an identifier to UTF-8.
/// \param cs The 'from' character set.
/// \param to Converted identifier.
/// \param from Identifier to convert.
/// \param len Length of 'to', in bytes; should be at least 3 * strlen(to) + 1.
IB_INTERN
void ib_utf8_convert_from_id(
	const charset_t*cs,
	char* to,
	const char* from,
	ulint len)
{
	/* FIXME: why 3*strlen(to)+1?  I suppose that it comes from
	MySQL, where the connection charset can be 8-bit, such as
	the "latin1" (really Windows Code Page 1252).  Converting
	that to UTF-8 can take 1..3 characters per byte. */
	/* FIXME: Do we even need this function?  Can't we just assume
	that the connection character encoding always is UTF-8?  (We
	may still want to support different collations for UTF-8.) */
	/* FIXME: Call the UTF-8 equivalent */
	strncpy(to, from, len);
}


/// \brief Test whether a UTF-8 character is a space or not.
/// \param cs Character set.
/// \param c Character to test.
/// \return TRUE if isspace(c)
IB_INTERN int ib_utf8_isspace(const charset_t* cs, char c)
{
	/* FIXME: Call the equivalent UTF-8 function. */
	/* FIXME: Do we really need this function?  This is needed by
	the InnoDB foreign key parser in MySQL, because U+00A0 is a
	space in the MySQL connection charset latin1 but not in
	utf8. */
	return isspace(c) ;
}

/// \brief This function is used to find the storage length in bytes of the characters that will fit into prefix_len bytes.
/// \param cs Character set.
/// \param prefix_len Prefix length in bytes.
/// \param str_len Length of the string in bytes.
/// \param str Character string.
/// \return Number of bytes required to copy the characters that will fit into prefix_len bytes.
IB_INTERN ulint ib_ucode_get_storage_size(const charset_t* cs, ulint prefix_len, ulint str_len, const char* str)	
{
	//  FIXME: Do we really need this function?  Can't we assume
	// that all strings are UTF-8?  (We still may want to support
	// different collations.) 
	return ut_min(prefix_len, str_len);
}

