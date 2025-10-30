// Copyright (c) 2025 Fabio N. Filasieno. All Rights Reserved.
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

/// \file ut_vec.inl
/// \brief A vector of pointers to data items
/// \details Initial draft
/// \date 10/10/2025
/// \author Fabio N Filasieno

template <typename T>
IB_INTERN ib_vector_t<T>* ib_vector_create(mem_heap_t* heap, ulint size)
{
	ut_a(size > 0);

	ib_vector_t<T>* vec = static_cast<ib_vector_t<T>*>(mem_heap_alloc(heap, sizeof(*vec)));

	vec->heap = heap;
	vec->data = static_cast<T**>(mem_heap_alloc(heap, sizeof(T*) * size));
	vec->used = 0;
	vec->total = size;

	return vec;
}

template <typename T>
IB_INTERN void ib_vector_push(ib_vector_t<T>* vec, T* elem)
{
	if (vec->used >= vec->total) {
		ulint new_total = vec->total * 2;
		T** new_data = static_cast<T**>(mem_heap_alloc(vec->heap, sizeof(T*) * new_total));

		memcpy(new_data, vec->data, sizeof(T*) * vec->total);

		vec->data = new_data;
		vec->total = new_total;
	}

	vec->data[vec->used] = elem;
	vec->used++;
}

template <typename T>
IB_INLINE ulint ib_vector_size(const ib_vector_t<T>* vec)
{
	return vec->used;
}

template <typename T> 
IB_INLINE T* ib_vector_get(ib_vector_t<T>* vec, ulint n)
{
	ut_a(n < ib_vector_size(vec));
	return(vec->data[n]);
}

template <typename T> 
IB_INLINE const T* ib_vector_get(const ib_vector_t<T>* vec, ulint n)
{
	ut_a(n < ib_vector_size(vec));
	return vec->data[n];
}

template <typename T> 
IB_INLINE T* ib_vector_set(ib_vector_t<T>* vec, ulint n, T* p)
{
	ut_a(n < ib_vector_size(vec));
	T* prev = vec->data[n];
	vec->data[n] = p;
	return prev;
}

template <typename T> 
IB_INLINE T* ib_vector_pop(ib_vector_t<T>*	vec)
{
	ut_a(vec->used > 0);
	--vec->used;
	T* elem = vec->data[vec->used];
	ut_d(vec->data[vec->used] = NULL);
	IB_MEM_INVALID(&vec->data[vec->used], sizeof(*vec->data));
	return elem;
}

template <typename T>
IB_INLINE void ib_vector_free(ib_vector_t<T>* vec)
{
	IB_MEM_HEAP_FREE(vec->heap);
}

template <typename T>
IB_INLINE bool ib_vector_is_empty(const ib_vector_t<T>* vec)
{
	return ib_vector_size(vec) == 0 ;
}
