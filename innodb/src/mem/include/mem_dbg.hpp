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


/// \file include/mem_dbg.h
/// \brief The memory management: the debug code. This is not a compilation module, but is included in mem_mem.cpp

// Created 6/9/1994 Heikki Tuuri
// In the debug version each allocated field is surrounded with check fields whose sizes are given below

extern ulint mem_current_allocated_memory;

#ifdef IB_MEM_DEBUG

	# ifndef IB_HOTBACKUP
		/// \brief The mutex which protects in the debug version the hash table
		/// containing the list of live memory heaps, and also the global
		/// variables in mem0dbg.c.
		extern mutex_tmem_hash_mutex;
	# endif // IB_HOTBACKUP

	#define MEM_FIELD_HEADER_SIZE	ut_calc_align(2 * sizeof(ulint), IB_MEM_ALIGNMENT)
	#define MEM_FIELD_TRAILER_SIZE	sizeof(ulint)
	#define MEM_SPACE_NEEDED(N)     ut_calc_align((N) + MEM_FIELD_HEADER_SIZE + MEM_FIELD_TRAILER_SIZE, IB_MEM_ALIGNMENT)

#else

  #define MEM_FIELD_HEADER_SIZE	0
  
  /// \brief Space needed when allocating for a user a field of length N.
  /// \details The space is allocated only in multiples of IB_MEM_ALIGNMENT. 
  /// In the debug version there are also check fields at the both ends of the field.
  #define MEM_SPACE_NEEDED(N)      ut_calc_align((N), IB_MEM_ALIGNMENT)
#endif

#if defined IB_MEM_DEBUG || defined IB_DEBUG

	/// \brief Checks a memory heap for consistency and prints the contents if requested.
	/// \param [in] heap memory heap
	/// \param [in] top calculate and validate only until this top pointer in the heap is reached, if this pointer is NULL, ignored
	/// \param [in] print if TRUE, prints the contents of the heap; works only in the debug version
	/// \param [out] error TRUE if error
	/// \param [out] us_size allocated memory (for the user) in the heap, if a NULL pointer is passed as this argument, it is ignored; in the non-debug version this is always -1
	/// \param [out] ph_size physical size of the heap, if a NULL pointer is passed as this argument, it is ignored
	/// \param [out] n_blocks number of blocks in the heap, if a NULL pointer is passed as this argument, it is ignored
	IB_INTERN void mem_heap_validate_or_print(mem_heap_t* heap, byte* top, ibool print, ibool* error, ulint* us_size, ulint* ph_size, ulint* n_blocks);


	/// \brief Validates the contents of a memory heap.
	/// \param [in] heap memory heap
	/// \return TRUE if ok
	IB_INTERN ibool mem_heap_validate(mem_heap_t* heap);

#endif // IB_MEM_DEBUG || IB_DEBUG

#ifdef IB_DEBUG

	/// \brief Checks that an object is a memory heap (or a block of it)
	/// \param [in] heap memory heap
	/// \return TRUE if ok

	IB_INTERN ibool mem_heap_check(mem_heap_t* heap);

#endif // IB_DEBUG

/// \brief Validates the dynamic memory
/// \return TRUE if ok
IB_INTERN ibool mem_validate(void);

