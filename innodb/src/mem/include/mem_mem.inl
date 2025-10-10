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

/// \file mem_mem.ic
/// The memory management

// Created 6/8/1994 Heikki Tuuri

#include "mem_dbg.inl"

/// \brief Creates a memory heap block where data can be allocated.
/// \param [in] heap in: memory heap or NULL if first block should be created
/// \param [in] n in: number of bytes needed for user data
/// \param [in] type in: type of heap: MEM_HEAP_DYNAMIC or MEM_HEAP_BUFFER
/// \param [in] file_name in: file name where created
/// \param [in] line in: line where created
/// \return own: memory heap block, NULL if did not succeed (only possible for MEM_HEAP_BTR_SEARCH type heaps)
IB_INTERN mem_block_t* mem_heap_create_block(mem_heap_t* heap, ulint n, ulint type, const char* file_name, ulint line);

/// \brief Frees a block from a memory heap.
/// \param [in] heap in: heap
/// \param [in] block in: block to free
IB_INTERN void mem_heap_block_free(mem_heap_t* heap, mem_block_t* block);

#ifndef IB_HOTBACKUP

	/// \brief Frees the free_block field from a memory heap.
	/// \param [in] heap in: heap
	IB_INTERN void mem_heap_free_block_free(mem_heap_t* heap);

#endif // !IB_HOTBACKUP

/// \brief Adds a new block to a memory heap.
/// \param [in] heap in: memory heap
/// \param [in] n in: number of bytes user needs
/// \return created block, NULL if did not succeed (only possible for MEM_HEAP_BTR_SEARCH type heaps)
IB_INTERN mem_block_t* mem_heap_add_block(mem_heap_t* heap, ulint n);

IB_INLINE void mem_block_set_len(mem_block_t* block, ulint len)
{
	ut_ad(len > 0);
	block->len = len;
}

IB_INLINE ulint mem_block_get_len(mem_block_t* block)
{
	return(block->len);
}

IB_INLINE void mem_block_set_type(mem_block_t* block, ulint type)
{
	ut_ad((type == MEM_HEAP_DYNAMIC) || (type == MEM_HEAP_BUFFER) || (type == MEM_HEAP_BUFFER + MEM_HEAP_BTR_SEARCH));
	block->type = type;
}

IB_INLINE ulint mem_block_get_type(mem_block_t* block)
{
	return(block->type);
}

IB_INLINE void mem_block_set_free(mem_block_t* block, ulint free_block)
{
	ut_ad(free_block > 0);
	ut_ad(free_block <= mem_block_get_len(block));
	block->free = free_block;
}

IB_INLINE ulint mem_block_get_free(mem_block_t* block)
{
	return(block->free);
}

IB_INLINE void mem_block_set_start(mem_block_t* block, ulint start)
{
	ut_ad(start > 0);
	block->start = start;
}

IB_INLINE ulint mem_block_get_start(mem_block_t* block)
{
	return(block->start);
}

/// \brief Allocates and zero-fills n bytes of memory from a memory heap.
/// \param [in] heap in: memory heap
/// \param [in] n in: number of bytes; if the heap is allowed to grow into the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
/// \return allocated, zero-filled storage
IB_INLINE void* mem_heap_zalloc(mem_heap_t* heap, ulint n)	
{
	ut_ad(heap);
	ut_ad(!(heap->type & MEM_HEAP_BTR_SEARCH));
	return(memset(mem_heap_alloc(heap, n), 0, n));
}

/// \brief Allocates n bytes of memory from a memory heap.
/// \param [in] heap in: memory heap
/// \param [in] n in: number of bytes; if the heap is allowed to grow into the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
/// \return allocated storage, NULL if did not succeed (only possible for MEM_HEAP_BTR_SEARCH type heaps)
IB_INLINE void* mem_heap_alloc(mem_heap_t* heap, ulint n);
{
	ut_ad(mem_heap_check(heap));
	mem_block_t* block = UT_LIST_GET_LAST(heap->base);
	ut_ad(!(block->type & MEM_HEAP_BUFFER) || (n <= MEM_MAX_ALLOC_IN_BUF));
	// Check if there is enough space in block. If not, create a new block to the heap 
	if (mem_block_get_len(block) < mem_block_get_free(block) + MEM_SPACE_NEEDED(n)) {
		block = mem_heap_add_block(heap, n);
		if (block == NULL) {
			return NULL;
		}
	}
	ulint free_sz = mem_block_get_free(block);
	void* buf = (byte*)block + free_sz;
	mem_block_set_free(block, free_sz + MEM_SPACE_NEEDED(n));

#ifdef IB_MEM_DEBUG
	IB_MEM_ALLOC(buf, n + MEM_FIELD_HEADER_SIZE + MEM_FIELD_TRAILER_SIZE);
	// In the debug version write debugging info to the field
	mem_field_init((byte*)buf, n);
	// Advance buf to point at the storage which will be given to the caller
	buf = (byte*)buf + MEM_FIELD_HEADER_SIZE;
#endif

#ifdef IB_SET_MEM_TO_ZERO
	IB_MEM_ALLOC(buf, n);
	memset(buf, '\0', n);
#endif
	IB_MEM_ALLOC(buf, n);

	return buf;
}


/// \brief Frees the space in a memory heap exceeding the pointer given. The
/// pointer must have been acquired from mem_heap_get_heap_top. The first
/// memory block of the heap is not freed. 
/// \param [in] heap in: heap from which to free
/// \param [in] old_top in: pointer to old top of heap
IB_INLINE void mem_heap_free_heap_top(mem_heap_t* heap, byte* old_top)
{
#ifdef IB_MEM_DEBUG
	ibool		error;
	ulint		total_size;
	ulint		size;
#endif

	ut_ad(mem_heap_check(heap));
#ifdef IB_MEM_DEBUG
	// Validate the heap and get its total allocated size 
	mem_heap_validate_or_print(heap, NULL, FALSE, &error, &total_size, NULL, NULL);
	ut_a(!error);
	// Get the size below top pointer 
	mem_heap_validate_or_print(heap, old_top, FALSE, &error, &size, NULL, NULL);
	ut_a(!error);
#endif

	mem_block_t* block = UT_LIST_GET_LAST(heap->base);
	while (block != NULL) {
		if (((byte*)block + mem_block_get_free(block) >= old_top) && ((byte*)block <= old_top)) {
			break;
		}
		// Store prev_block value before freeing the current block (the current block will be erased in freeing)
		mem_block_t* prev_block = UT_LIST_GET_PREV(list, block);
		mem_heap_block_free(heap, block);
		block = prev_block;
	}

	ut_ad(block);
	// Set the free field of block
	mem_block_set_free(block, old_top - (byte*)block);

#ifdef IB_MEM_DEBUG
	ut_ad(mem_block_get_start(block) <= mem_block_get_free(block));
	// In the debug version erase block from top up 
	mem_erase_buf(old_top, (byte*)block + block->len - old_top);
	// Update allocated memory count 
	mutex_enter(&mem_hash_mutex);
	mem_current_allocated_memory -= (total_size - size);
	mutex_exit(&mem_hash_mutex);
#else // IB_MEM_DEBUG 
	IB_MEM_ASSERT_W(old_top, (byte*)block + block->len - old_top);
#endif // IB_MEM_DEBUG 
	IB_MEM_ALLOC(old_top, (byte*)block + block->len - old_top);
	// If free == start, we may free the block if it is not the first one 
	if ((heap != block) && (mem_block_get_free(block) == mem_block_get_start(block))) {
		mem_heap_block_free(heap, block);
	}
}

/// \brief Empties a memory heap. The first memory block of the heap is not freed.
/// \param [in] heap heap to empty
IB_INLINE void mem_heap_empty(mem_heap_t*	heap)
{
	mem_heap_free_heap_top(heap, (byte*)heap + mem_block_get_start(heap));
#ifndef IB_HOTBACKUP
	if (heap->free_block) {
		mem_heap_free_block_free(heap);
	}
#endif // !IB_HOTBACKUP 
}

/// \brief Returns a pointer to the topmost element in a memory heap. The size of the element must be given.
/// \param [in] heap memory heap
/// \param [in] n size of the topmost element
/// \return pointer to the topmost element
IB_INLINE void* mem_heap_get_top( mem_heap_t*	heap, ulint n)
{
	ut_ad(mem_heap_check(heap));
	mem_block_t* block = UT_LIST_GET_LAST(heap->base);
	void* buf = (byte*)block + mem_block_get_free(block) - MEM_SPACE_NEEDED(n);
#ifdef IB_MEM_DEBUG
	ut_ad(mem_block_get_start(block) <=(ulint)((byte*)buf - (byte*)block));
	// In the debug version, advance buf to point at the storage which was given to the caller in the allocation
	buf = (byte*)buf + MEM_FIELD_HEADER_SIZE;
	// Check that the field lengths agree
	ut_ad(n == (ulint)mem_field_header_get_len(buf));
#endif
	return buf;
}

/// \brief Frees the topmost element in a memory heap. The size of the element must be given.
/// \param [in] heap memory heap
/// \param [in] n size of the topmost element
/// \return pointer to the topmost element
IB_INLINE void mem_heap_free_top( mem_heap_t*	heap, ulint n)
{
	ut_ad(mem_heap_check(heap));
	mem_block_t* block = UT_LIST_GET_LAST(heap->base);
	// Subtract the free field of block
	mem_block_set_free(block, mem_block_get_free(block) - MEM_SPACE_NEEDED(n));
	IB_MEM_ASSERT_W((byte*) block + mem_block_get_free(block), n);
#ifdef IB_MEM_DEBUG
	ut_ad(mem_block_get_start(block) <= mem_block_get_free(block));
	// In the debug version check the consistency, and erase field
	mem_field_erase((byte*)block + mem_block_get_free(block), n);
#endif
	// If free == start, we may free the block if it is not the first one
	if ((heap != block) && (mem_block_get_free(block) == mem_block_get_start(block))) {
		mem_heap_block_free(heap, block);
	} else {
		// Avoid a bogus IB_MEM_ASSERT_W() warning in a subsequent invocation of mem_heap_free_top().
		// Originally, this was IB_MEM_FREE(), to catch writes to freed memory. 
		IB_MEM_ALLOC((byte*) block + mem_block_get_free(block), n);
	}
}

/// \brief Use the corresponding macros instead of this function. 
/// Creates a memory heap. For debugging purposes, takes also the file name and line as argument.
/// \param [in] n in: desired start block size, this means that a single user buffer of size n will fit in the block, 0 creates a default size block
/// \param [in] type in: heap type
/// \param [in] file_name in: file name where created
/// \param [in] line in: line where created
/// \return own: memory heap, NULL if did not succeed (only possible for MEM_HEAP_BTR_SEARCH type heaps)
IB_INLINE mem_heap_t* mem_heap_create_func(ulint n, ulint type, const char* file_name, ulint line)		
{
	if (!n) {
		n = MEM_BLOCK_START_SIZE;
	}
	mem_block_t* block = mem_heap_create_block(NULL, n, type, file_name, line);
	if (block == NULL) {
		return(NULL);
	}
	UT_LIST_INIT(block->base);
	// Add the created block itself as the first block in the list
	UT_LIST_ADD_FIRST(list, block->base, block);

#ifdef IB_MEM_DEBUG
	mem_hash_insert(block, file_name, line);
#endif
	return block;
}

/// \brief Use the corresponding macro instead of this function.
/// \note Use the corresponding macro instead of this function. 
/// Frees the space occupied by a memory heap. In the debug version erases the heap memory blocks. 
/// \param [in] heap in, own: heap to be freed
/// \param [in] file_name file name where freed
/// \param [in] line line where freed
IB_INLINE void mem_heap_free_func(mem_heap_t*	heap, const char* file_name __attribute__((unused)), ulint line  __attribute__((unused)))
{
	ut_ad(mem_heap_check(heap));
	mem_block_t* block = UT_LIST_GET_LAST(heap->base);
#ifdef IB_MEM_DEBUG
	// In the debug version remove the heap from the hash table of heaps and check its consistency 
	mem_hash_remove(heap, file_name, line);
#endif

#ifndef IB_HOTBACKUP
	if (heap->free_block) {
		mem_heap_free_block_free(heap);
	}
#endif // !IB_HOTBACKUP

	while (block != NULL) { // Store the contents of info before freeing current block (it is erased in freeing) 
		mem_block_t* prev_block = UT_LIST_GET_PREV(list, block);
		mem_heap_block_free(heap, block);
		block = prev_block;
	}
}

/// \brief Use the corresponding macro instead of this function.
/// \details Allocates a single buffer of memory from the dynamic memory of the C compiler. Is like malloc of C. 
/// The buffer must be freed with mem_free.
/// \param [in] n desired number of bytes
/// \param [out] size allocated size in bytes, or NULL
/// \param [in] file_name file name where created
/// \param [in] line line where created
/// \return	own: free storage
IB_INLINE void* mem_alloc_func(ulint n, ulint* size, const char*	file_name, ulint line)	
{
	mem_heap_t* heap = mem_heap_create_func(n, MEM_HEAP_DYNAMIC, file_name, line);

	// Note that as we created the first block in the heap big enough
	// for the buffer requested by the caller, the buffer will be in the
	// first block and thus we can calculate the pointer to the heap from
	// the pointer to the buffer when we free the memory buffer. 

	if (IB_LIKELY_NULL(size)) {
		// Adjust the allocation to the actual size of the memory block.
		ulint m = mem_block_get_len(heap) - mem_block_get_free(heap);
#ifdef IB_MEM_DEBUG
		m -= MEM_FIELD_HEADER_SIZE + MEM_FIELD_TRAILER_SIZE;
#endif // IB_MEM_DEBUG 
		ut_ad(m >= n);
		*size = n = m;
	}
	void* buf = mem_heap_alloc(heap, n);
	ut_a((byte*)heap == (byte*)buf - MEM_BLOCK_HEADER_SIZE - MEM_FIELD_HEADER_SIZE);
	return(buf);
}

/***************************************************************//**
NOTE: Use the corresponding macro instead of this function. Frees a single
buffer of storage from the dynamic memory of the C compiler. Similar to the
free of C. */
IB_INLINE
void
mem_free_func(
/*==========*/
	void*		ptr,		/*!< in, own: buffer to be freed */
	const char*	file_name,	/*!< in: file name where created */
	ulint		line)		/*!< in: line where created */
{
	mem_heap_t*   heap;
	heap = (mem_heap_t*)((byte*)ptr - MEM_BLOCK_HEADER_SIZE - MEM_FIELD_HEADER_SIZE);
	mem_heap_free_func(heap, file_name, line);
}

/// Returns the space in bytes occupied by a memory heap.
/// \param [in] heap the memory heap
/// \return	the space in bytes occupied by a memory heap
IB_INLINE ulint mem_heap_get_size(mem_heap_t*	heap)
{
	ut_ad(mem_heap_check(heap));
	ulint size = heap->total_size;
#ifndef IB_HOTBACKUP
	if (heap->free_block) {
		size += IB_PAGE_SIZE;
	}
#endif /* !IB_HOTBACKUP */

	return size;
}

/// Duplicates a NUL-terminated string.
/// \param str in: string to be copied
/// \return	a copy of the string, must be deallocated with mem_free
IB_INLINE char* mem_strdup(const char* str)
{
	ulint len = strlen(str) + 1;
	return (char*) memcpy(mem_alloc(len), str, len);
}

/// Makes a NUL-terminated copy of a nonterminated string
/// \param str in: string to be copied
/// \param len in: length of str, in bytes
/// \return	a copy of the string, must be deallocated with mem_free
IB_INLINE char* mem_strdupl(const char* str, ulint len)	
{
	char* s = (char*) mem_alloc(len + 1);
	s[len] = 0;
	return (char*) memcpy(s, str, len) ;
}

/// Makes a NUL-terminated copy of a nonterminated string, allocated from a memory heap.
/// \param heap in: memory heap where string is allocated
/// \param str in: string to be copied
/// \param len in: length of str, in bytes
/// \return	a copy of the string
IB_INLINE char* mem_heap_strdupl(mem_heap_t* heap, const char* str, ulint len)
{
	char* s = (char*) mem_heap_alloc(heap, len + 1);
	s[len] = 0;
	return (char*) memcpy(s, str, len);
}
