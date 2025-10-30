// Copyright (c) 1994, 2010, Innobase Oy. All Rights Reserved.
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

/// \file mem_mem.hpp
/// \brief The memory management
/// \details Memory management functionality for InnoDB
/// \author Heikki Tuuri
/// \date 6/9/1994

#pragma once

#include "univ.i"
#include "ut_mem.hpp"
#include "ut_byte.hpp"
#include "ut_rnd.hpp"

#ifndef IB_HOTBACKUP
# include "sync0sync.h"
#endif // IB_HOTBACKUP

#include "ut_lst.hpp"
#include "mach_data.hpp"

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Types 
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/// \brief The info structure stored at the beginning of a heap block
typedef struct mem_block_info_struct mem_block_info_t;

/// \brief A block of a memory heap consists of the info structure followed by an area of memory
typedef mem_block_info_t mem_block_t;

/// \brief A memory heap is a nonempty linear list of memory blocks
typedef mem_block_t	mem_heap_t;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/// \brief allocation from the dynamic memory pool of the C compiler; the most common type
constinit ulint MEM_HEAP_DYNAMIC = 0; 

/// \brief means allocation from the buffer pool
constinit ulint MEM_HEAP_BUFFER = 1;

/// \brief this flag can optionally be ORed to MEM_HEAP_BUFFER, in which case heap->free_block is used in some cases for memory allocations, and if it's NULL, the memory allocation functions can return NULL.
constinit ulint MEM_HEAP_BTR_SEARCH = 2;

/// \brief Magic number for a memory heap block
constinit ulint MEM_BLOCK_MAGIC_N = 764741555;

/// \brief Magic number for a freed memory heap block
constinit ulint MEM_FREED_BLOCK_MAGIC_N = 547711122;

/// \brief Header size for a memory heap block
constinit ulint MEM_BLOCK_HEADER_SIZE = ut_calc_align(sizeof(mem_block_info_t), IB_MEM_ALIGNMENT);

/// \brief used for the first block in the memory heap if the size is not specified, i.e., 0 is given as the parameter in the call of
/// create. The standard size is the maximum (payload) size of the blocks used for
/// allocations of small buffers.

constinit ulint MEM_BLOCK_START_SIZE = 64;
constinit ulint MEM_BLOCK_STANDARD_SIZE = (IB_PAGE_SIZE >= 16384 ? 8000 : MEM_MAX_ALLOC_IN_BUF);

/// \brief If a memory heap is allowed to grow into the buffer pool, the following is the maximum size for a single allocated buffer:
constinit ulint MEM_MAX_ALLOC_IN_BUF = (IB_PAGE_SIZE - 200);

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros 
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define IB_MEM_ALLOC(N)	                    mem_alloc_func((N), NULL, __FILE__, __LINE__)
#define IB_MEM_ALLOC_EX(N,S)                mem_alloc_func((N), (S), __FILE__, __LINE__)
#define IB_MEM_FREE(PTR)	                mem_free_func((PTR), __FILE__, __LINE__)
#define IB_MEM_ZALLOC(N)	                memset(IB_MEM_ALLOC(N), 0, (N))

/// \brief Use this macro instead of the corresponding function! Macro for memory heap creation. */
#define IB_MEM_HEAP_CREATE(N)               mem_heap_create_func((N), MEM_HEAP_DYNAMIC, __FILE__, __LINE__)

/// \brief Use this macro instead of the corresponding function! Macro for memory heap creation.
#define IB_MEM_HEAP_CREATE_IN_BUFFER(N)     mem_heap_create_func((N), MEM_HEAP_BUFFER, __FILE__, __LINE__)

/// \brief Use this macro instead of the corresponding function! Macro for memory heap creation.
#define IB_MEM_HEAP_CREATE_IN_BTR_SEARCH(N) mem_heap_create_func((N), MEM_HEAP_BTR_SEARCH | MEM_HEAP_BUFFER, __FILE__, __LINE__)

/// \brief Use this macro instead of the corresponding function! Macro for memory heap freeing.
#define IB_MEM_HEAP_FREE(heap)              mem_heap_free_func((heap), __FILE__, __LINE__)

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Routines 
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/// \brief Initializes the memory system.
/// \param [in] size common pool size in bytes
IB_INTERN void mem_init(ulint size);

/// \brief Closes the memory system.
IB_INTERN void mem_close(void);

/// \brief Creates a memory heap.
/// \details For debugging purposes, takes also the file name and line as arguments. Use the corresponding macros instead of this function.
/// \param [in] n desired start block size, this means that a single user buffer of size n will fit in the block, 0 creates a default size block
/// \param [in] type heap type
/// \param [in] file_name file name where created
/// \param [in] line line where created
/// \return own: memory heap, NULL if did not succeed (only possible for MEM_HEAP_BTR_SEARCH type heaps)
IB_INLINE mem_heap_t* mem_heap_create_func(ulint n, ulint type, const char* file_name, ulint line);

/// \brief Frees the space occupied by a memory heap.
/// \details In the debug version erases the heap memory blocks. Use the corresponding macros instead of this function.
/// \param [in] heap in, own: heap to be freed
/// \param [in] file_name file name where freed
/// \param [in] line line where freed
IB_INLINE void mem_heap_free_func(mem_heap_t* heap, const char* file_name, ulint line);

/// \brief Allocates and zero-fills n bytes of memory from a memory heap.
/// \param [in] heap in: memory heap
/// \param [in] n in: number of bytes; if the heap is allowed to grow into the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
/// \return allocated, zero-filled storage
IB_INLINE void* mem_heap_zalloc(mem_heap_t* heap, ulint n);	

/// \brief Allocates n bytes of memory from a memory heap.
/// \param [in] heap in: memory heap
/// \param [in] n in: number of bytes; if the heap is allowed to grow into the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
/// \return allocated storage, NULL if did not succeed (only possible for MEM_HEAP_BTR_SEARCH type heaps)
IB_INLINE void* mem_heap_alloc(mem_heap_t* heap, ulint n);

/// \brief Frees the space in a memory heap exceeding the pointer given.
/// \details The pointer must have been acquired from mem_heap_get_heap_top. The first memory block of the heap is not freed.
/// \param [in] heap from which to free
/// \param [in] old_top pointer to old top of heap
IB_INLINE void mem_heap_free_heap_top(mem_heap_t* heap, byte* old_top);

/// \brief Empties a memory heap.
/// \details The first memory block of the heap is not freed.
/// \param [in] heap to empty
IB_INLINE void mem_heap_empty(mem_heap_t* heap);

/// \brief Returns a pointer to the topmost element in a memory heap.
/// \details The size of the element must be given.
/// \param [in] heap memory heap
/// \param [in] n size of the topmost element
/// \return pointer to the topmost element
IB_INLINE void* mem_heap_get_top(mem_heap_t* heap, ulint n);

/// \brief Frees the topmost element in a memory heap.
/// \details The size of the element must be given.
/// \param [in] heap memory heap
/// \param [in] n size of the topmost element
IB_INLINE void mem_heap_free_top(mem_heap_t* heap, ulint n);

/// \brief Returns the space in bytes occupied by a memory heap.
/// \param [in] heap heap
/// \return space in bytes occupied by the memory heap
IB_INLINE ulint mem_heap_get_size(mem_heap_t* heap);

/// \brief Allocates a single buffer of memory from the dynamic memory of the C compiler.
/// \details Is like malloc of C. The buffer must be freed with IB_MEM_FREE. NOTE: Use the corresponding macro instead of this function.
/// \param [in] n requested size in bytes
/// \param [out] size allocated size in bytes, or NULL
/// \param [in] file_name file name where created
/// \param [in] line line where created
/// \return own: free storage
IB_INLINE void* mem_alloc_func(ulint n, ulint* size, const char* file_name, ulint line);

/// \brief Frees a single buffer of storage from the dynamic memory of C compiler.
/// \details Similar to free of C. NOTE: Use the corresponding macro instead of this function.
/// \param [in] ptr buffer to be freed
/// \param [in] file_name file name where created
/// \param [in] line line where created
IB_INLINE void mem_free_func(void* ptr, const char* file_name, ulint line);

/// \brief Duplicates a NUL-terminated string.
/// \param [in] str string to be copied
/// \return own: a copy of the string, must be deallocated with IB_MEM_FREE
IB_INLINE char* mem_strdup(const char* str);

/// \brief Makes a NUL-terminated copy of a nonterminated string.
/// \param [in] str string to be copied
/// \param [in] len length of str, in bytes
/// \return own: a copy of the string, must be deallocated with IB_MEM_FREE
IB_INLINE char* mem_strdupl(const char* str, ulint len);

/// \brief Duplicates a NUL-terminated string, allocated from a memory heap.
/// \param [in] heap memory heap where string is allocated
/// \param [in] str string to be copied
/// \return own: a copy of the string
IB_INTERN char* mem_heap_strdup(mem_heap_t* heap, const char* str);

/// \brief Makes a NUL-terminated copy of a nonterminated string, allocated from a memory heap.
/// \param [in] heap memory heap where string is allocated
/// \param [in] str string to be copied
/// \param [in] len length of str, in bytes
/// \return own: a copy of the string
IB_INLINE char* mem_heap_strdupl(mem_heap_t* heap, const char* str, ulint len);

/// \brief Concatenate two strings and return the result, using a memory heap.
/// \param [in] heap memory heap where string is allocated
/// \param [in] s1 string 1
/// \param [in] s2 string 2
/// \return own: the result
IB_INTERN char* mem_heap_strcat(mem_heap_t* heap, const char* s1, const char* s2);

/// \brief Duplicate a block of data, allocated from a memory heap.
/// \param [in] heap memory heap where copy is allocated
/// \param [in] data data to be copied
/// \param [in] len length of data, in bytes
/// \return own: a copy of the data
IB_INTERN void* mem_heap_dup(mem_heap_t* heap, const void* data, ulint len);

/// \brief A simple (s)printf replacement that dynamically allocates the space for the formatted string from the given heap.
/// \details This supports a very limited set of the printf syntax: types 's' and 'u' and length modifier 'l' (which is required for the 'u' type).
/// \param [in] heap memory heap
/// \param [in] format format string
/// \return heap-allocated formatted string
IB_INTERN char* mem_heap_printf(mem_heap_t* heap, const char* format, ...) __attribute__ ((format (printf, 2, 3)));

#ifdef IB_DEBUG
/// \brief Goes through the list of all allocated mem blocks, checks their magic numbers, and reports possible corruption.
/// \param [in] heap heap to verify
IB_INTERN void mem_heap_verify(const mem_heap_t* heap);
#endif

#include "mem_dbg.hpp"

#ifndef IB_DO_NOT_INLINE
  #include "mem_mem.inl"
#endif

