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

/// \file include/ut0mem.h
/// \brief Memory primitives
/// 5/30/1994 Heikki Tuuri

#pragma once

#include "defs.hpp"
#include <string.h>

#ifndef IB_HOTBACKUP
	#include "os_sync.h"

#endif // !IB_HOTBACKUP

/// \brief Wrapper for memcpy(3).  
/// \details Copy memory area when the source andtarget are not overlapping.
/// \param [in] dest copy to
/// \param [in] sour copy from
/// \param [in] n number of bytes to copy
/// \return dest
IB_INLINE void* ut_memcpy(void* dest, const void* sour, ulint n);

/// \brief Wrapper for memmove(3).  
/// \details Copy memory area when the source andtarget are overlapping.
/// \param [in] dest copy to
/// \param [in] sour copy from
/// \param [in] n number of bytes to copy
/// \return dest
IB_INLINE void* ut_memmove(void* dest, const void* sour, ulint n);

/// \brief Wrapper for memcmp(3).  
/// \details Compare memory areas.
/// \param [in] str1 first memory block to compare
/// \param [in] str2 second memory block to compare
/// \param [in] n number of bytes to compare
/// \return negative, 0, or positive if str1 is smaller, equal, or greater than str2, respectively.
IB_INLINE int ut_memcmp(const void* str1, const void* str2, ulint n);


/// \brief Initializes the mem block list at database startup.
IB_INTERN void ut_mem_init(void);


/// \brief Allocates memory.
/// \details Sets it also to zero if IB_SET_MEM_TO_ZERO is defined and set_to_zero is TRUE.
/// \param [in] n number of bytes to allocate
/// \param [in] set_to_zero TRUE if allocated memory should be set to zero if IB_SET_MEM_TO_ZERO is defined
/// \param [in] assert_on_error If TRUE, we crash the server if the memory cannot be allocated
/// \return Allocated memory
IB_INTERN void* ut_malloc_low(ulint n, ibool set_to_zero, ibool assert_on_error);

/// \brief Allocates memory.
/// \details Sets it also to zero if IB_SET_MEM_TO_ZERO is defined.
/// \param [in] n number of bytes to allocate
/// \return Allocated memory
IB_INTERN void* ut_malloc(ulint n);

#ifndef IB_HOTBACKUP

	/// \brief Tests if malloc of n bytes would succeed. ut_malloc() asserts if memory runs
	/// \details out. It cannot be used if we want to return an error message. Prints to stderr a message if fails.
	/// \param [in] n try to allocate this many bytes
	/// \return TRUE if succeeded
	IB_INTERN ibool ut_test_malloc(ulint n);

#endif // !IB_HOTBACKUP


/// \brief Frees a memory block allocated with ut_malloc.
/// \param [in] ptr memory block
IB_INTERN void ut_free(void* ptr);


#ifndef IB_HOTBACKUP

/// \brief Implements realloc.
/// \details This is needed by /pars/lexyy.c. Otherwise, you should not use this function because the allocation functions in mem0mem.h are the recommended ones in InnoDB.
/// man realloc in Linux, 2004:
///        realloc()  changes the size of the memory block pointed to
///        by ptr to size bytes.  The contents will be  unchanged  to
///        the minimum of the old and new sizes; newly allocated mem�
///        ory will be uninitialized.  If ptr is NULL,  the	 call  is
///        equivalent  to malloc(size); if size is equal to zero, the
///        call is equivalent to free(ptr).	 Unless ptr is	NULL,  it
///        must  have  been	 returned by an earlier call to malloc(),
///        calloc() or realloc().
/// RETURN VALUE
///        realloc() returns a pointer to the newly allocated memory,
///        which is suitably aligned for any kind of variable and may
///        be different from ptr, or NULL if the  request  fails.  If
///        size  was equal to 0, either NULL or a pointer suitable to
///        be passed to free() is returned.	 If realloc()  fails  the
///        original	 block	is  left  untouched  - it is not freed or
///        moved.
/// \param [in] ptr pointer to old block or NULL
/// \param [in] size desired size
/// \return pointer to new mem block or NULL
IB_INTERN void* ut_realloc(void* ptr, ulint size);


/// \brief Frees in shutdown all allocated memory not freed yet.
IB_INTERN void ut_free_all_mem(void);

#endif // !IB_HOTBACKUP

/// \brief Wrapper for strcpy(3).  
/// \details Copy a NUL-terminated string.
/// \param [in] dest copy to
/// \param [in] sour copy from
/// \return dest
IB_INLINE char* ut_strcpy(char* dest, const char* sour);

/// \brief Wrapper for strlen(3).
/// \details Determine the length of a NUL-terminated string.
/// \param [in] str string
/// \return length of the string in bytes, excluding the terminating NUL
IB_INLINE ulint ut_strlen(const char* str);

/// \brief Wrapper for strcmp(3).  Compare NUL-terminated strings.
/// \details Compare NUL-terminated strings.
/// \param [in] str1 first string to compare
/// \param [in] str2 second string to compare
/// \return negative, 0, or positive if str1 is smaller, equal, or greater than str2, respectively.
IB_INLINE int ut_strcmp(const char* str1, const char* str2);

/// \brief Copies up to size - 1 characters from the NUL-terminated string src to dst, NUL-terminating the result. Returns strlen(src), so truncation occurred if the return value >= size.
/// \details Copies up to size - 1 characters from the NUL-terminated string src to dst, NUL-terminating the result. Returns strlen(src), so truncation occurred if the return value >= size.
/// \param [in] dst destination buffer
/// \param [in] src source buffer
/// \param [in] size size of destination buffer
/// \return strlen(src)
IB_INTERN ulint ut_strlcpy(char* dst, const char* src, ulint size);

/// \brief Like ut_strlcpy, but if src doesn't fit in dst completely, copies the last (size - 1) bytes of src, not the first.
/// \details Like ut_strlcpy, but if src doesn't fit in dst completely, copies the last (size - 1) bytes of src, not the first.
/// \param [in] dst destination buffer
/// \param [in] src source buffer
/// \param [in] size size of destination buffer
/// \return strlen(src)
IB_INTERN ulint ut_strlcpy_rev(char* dst, const char* src, ulint size);

/// \brief Compute strlen(ut_strcpyq(str, q)).
/// \details Compute strlen(ut_strcpyq(str, q)).
/// \param [in] str null-terminated string
/// \param [in] q the quote character
/// \return length of the string when quoted
IB_INLINE ulint ut_strlenq(const char* str, char q);

/// \brief Make a quoted copy of a NUL-terminated string.
/// \details Leading and trailing quotes will not be included; only embedded quotes will be escaped. See also ut_strlenq() and ut_memcpyq().
/// \param [in] dest output buffer
/// \param [in] q the quote character
/// \param [in] src string to be quoted
/// \return pointer to end of dest
IB_INTERN char* ut_strcpyq(char* dest, char q, const char* src);

/// \brief Make a quoted copy of a fixed-length string.
/// \details Leading and trailing quotes will not be included; only embedded quotes will be escaped. See also ut_strlenq() and ut_strcpyq().
/// \param [in] dest output buffer
/// \param [in] q the quote character
/// \param [in] src string to be quoted
/// \param [in] len length of src
/// \return pointer to end of dest
IB_INTERN char* ut_memcpyq(char* dest,char q, const char* src, ulint len);

/// \brief Return the number of times s2 occurs in s1. Overlapping instances of s2 are only counted once.
/// \details Overlapping instances of s2 are only counted once.
/// \param [in] s1 string to search in
/// \param [in] s2 string to search for
/// \return the number of times s2 occurs in s1
IB_INTERN ulint ut_strcount(const char*	s1, const char*	s2);

/// \brief Replace every occurrence of s1 in str with s2. Overlapping instances of s1 are only replaced once.
/// \details Overlapping instances of s1 are only replaced once.
/// \param [in] str string to operate on
/// \param [in] s1 string to replace
/// \param [in] s2 string to replace s1 with
/// \return own: modified string, must be freed with IB_MEM_FREE()
IB_INTERN char* ut_strreplace(const char*	str, const char* s1, const char* s2);

/// \brief Converts a raw binary data to a NUL-terminated hex string. 
/// \details The output is truncated if there is not enough space in "hex", make sure "hex_size" is at
/// least (2 * raw_size + 1) if you do not want this to happen. Returns the
/// actual number of characters written to "hex" (including the NUL).
/// \param [in] raw raw data
/// \param [in] raw_size "raw" length in bytes
/// \param [out] hex hex string
/// \param [in] hex_size "hex" size in bytes
/// \return number of chars written 
IB_INLINE ulint ut_raw_to_hex(const void* raw, ulint raw_size, char* hex, ulint hex_size);


/// \brief Adds single quotes to the start and end of string and escapes any quotes by doubling them.
/// \details Returns the number of bytes that were written to "buf" (including the terminating NUL). If buf_size is too small then the trailing bytes from "str" are discarded.
/// \param [in] str string
/// \param [in] str_len string length in bytes
/// \param [out] buf output buffer
/// \param [in] buf_size output buffer size in bytes
/// \return	number of bytes that were written
IB_INLINE ulint ut_str_sql_format(const char* str, ulint str_len, char* buf, ulint buf_size);

/// \brief Reset the variables.
IB_INTERN void ut_mem_var_init(void);

#ifndef IB_DO_NOT_INLINE
  #include "ut_mem.inl"
#endif

#endif
