// Copyright (c) 2006, 2009, Innobase Oy. All Rights Reserved.
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

/// @file ut/ut0vec.c
/// \brief A vector of pointers to data items
///
/// Created 4/6/2006 Osku Salerma

#include "ut_vec.hpp"
#ifdef UNIV_NONINL
#include "ut_vec.inl"
#endif
#include <cstring>

/// \brief Create a new vector with the given initial size.
/// \param heap in: heap
/// \param size in: initial size
/// \return vector
IB_INTERN ib_vector_t *ib_vector_create(mem_heap_t *heap, ulint size)
{
	ut_a(size > 0);

	ib_vector_t *vec = mem_heap_alloc(heap, sizeof(*vec));

	vec->heap = heap;
	vec->data = mem_heap_alloc(heap, sizeof(void *) * size);
	vec->used = 0;
	vec->total = size;

	return (vec);
}

/// \brief Push a new element to the vector, increasing its size if necessary.
/// \param vec in: vector
/// \param elem in: data element
IB_INTERN void ib_vector_push(ib_vector_t *vec, void *elem)
{
	if (vec->used >= vec->total) {
		void **new_data;
		ulint new_total = vec->total * 2;

		new_data = mem_heap_alloc(vec->heap, sizeof(void *) * new_total);
		memcpy(new_data, vec->data, sizeof(void *) * vec->total);

		vec->data = new_data;
		vec->total = new_total;
	}

	vec->data[vec->used] = elem;
	vec->used++;
}
