/*****************************************************************************

Copyright (c) 2006, 2010, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/*******************************************************************//**
@file include/ut0vec.h
A vector of pointers to data items

Created 4/6/2006 Osku Salerma
************************************************************************/

#pragma once

#include "defs.hpp"
#include "mem_mem.hpp"

/// \brief An automatically resizing vector data type.
/// \details Contains T items.
///
/// - The items are owned by the caller.
/// - All memory allocation is done through a heap owned by the caller, who is responsible for freeing it when done with the vector.
/// - When the vector is resized, the old memory area is left allocated since it uses the same heap as the new memory area, so this is best used for
///   relatively small or short-lived uses.
/// \tparam T The type of the items in the vector.
template <typename T>
struct ib_vector_struct {
	mem_heap_t*	heap;	//!< heap
	T*          data;	//!< data elements
	ulint		used;	//!< number of elements currently used
	ulint		total;	//!< number of elements allocated
};

/// \brief An automatically resizing vector data type.
template <typename T>
using ib_vector_t = ib_vector_struct<T>;

/// \brief Create a new vector with the given initial size.
/// \param [in] heap heap
/// \param [in] size initial size
/// \return vector
template <typename T>
IB_INTERN ib_vector_t<T>* ib_vector_create(mem_heap_t* heap, ulint size);

/// \brief Push a new element to the vector, increasing its size if necessary.
/// \param [in] vec vector
/// \param [in] elem data element
template <typename T>
IB_INTERN void ib_vector_push(ib_vector_t<T>* vec, T* elem);

/// \brief Get the number of elements in the vector.
/// \param [in] vec vector
/// \return number of elements in vector
template <typename T>
IB_INTERN ulint ib_vector_size(const ib_vector_t<T>* vec);

/// \brief Test whether a vector is empty or not.
/// \param [in] vec vector
/// \return TRUE if empty
template <typename T>
IB_INTERN bool ib_vector_is_empty(const ib_vector_t<T>* vec);

/// \brief Get the n'th element.
/// \param [in] vec vector
/// \param [in] n element index to get
/// \return n'th element
template <typename T>
IB_INTERN T* ib_vector_get(ib_vector_t<T>* vec, ulint n);

/// \brief Get the n'th element as a const pointer.
/// \param [in] vec vector
/// \param [in] n element index to get
/// \return n'th element
template <typename T>
IB_INTERN const T* ib_vector_get(const ib_vector_t<T>* vec, ulint n);

/// \brief Set the n'th element and return the previous value.
/// \param [in] vec vector
/// \param [in] n element index to set
/// \param [in] p new value to set
/// \return n'th element
template <typename T>
IB_INLINE T* ib_vector_set(ib_vector_t<T>* vec, ulint n, T* p);

/// Remove the last element from the vector. 
template <typename T>
IB_INLINE T ib_vector_pop(ib_vector_t<T>* vec);

/// \brief Free the underlying heap of the vector. Note that vec is invalid after this call.
/// \param [in] vec vector
/// \return void
template <typename T>
IB_INLINE void ib_vector_free(ib_vector_t<T>* vec);

#ifndef IB_DO_NOT_INLINE
  #include "ut_vec.inl"
#endif
