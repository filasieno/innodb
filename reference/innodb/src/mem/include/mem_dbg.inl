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

/// \file include/mem_dbg.inl
/// \brief The memory management: the debug code. This is not an independent compilation module but is included in mem0mem.*.
/// \details Created 6/8/1994 Heikki Tuuri

#ifdef IB_MEM_DEBUG


	/// \brief Initializes an allocated memory field in the debug version.
	/// \param [in] buf memory field
	/// \param [in] n how many bytes the user requested
	IB_INTERN void mem_field_init(byte* buf, ulint n);	

	///\brief Erases an allocated memory field in the debug version.
	/// \param [in] buf memory field
	/// \param [in] n how many bytes the user requested
	IB_INTERN void mem_field_erase(byte* buf, ulint n);	

	/// \brief Initializes a buffer to a random combination of hex BA and BE.
	/// \param [in] buf pointer to buffer
	/// \param [in] n length of buffer
	IB_INTERN void mem_init_buf(byte* buf, ulint n);	

	/// \brief Initializes a buffer to a random combination of hex DE and AD.
	/// \param [in] buf pointer to buffer
	/// \param [in] n length of buffer
	IB_INTERN void mem_erase_buf(byte* buf, ulint n);	

	/// \brief Inserts a created memory heap to the hash table of current allocated memory heaps.
	/// \param [in] heap memory heap
	/// \param [in] file_name file name of creation
	/// \param [in] line line where created
	IB_INTERN void mem_hash_insert(mem_heap_t* heap, const char* file_name, ulint line);	


	/// \brief Removes a memory heap (which is going to be freed by the caller) from the list of live memory heaps.
	/// \param [in] heap memory heap
	/// \param [in] file_name file name of freeing
	/// \param [in] line line where freed
	IB_INTERN void mem_hash_remove(mem_heap_t* heap, const char* file_name, ulint line);	


	/// \brief Sets the length of a memory field
	/// \param [in] field memory field
	/// \param [in] len length of memory field
	IB_INTERN void mem_field_header_set_len(byte* field, ulint len);	


	/// \brief Gets the length of a memory field
	/// \param [in] field memory field
	/// \return length of memory field
	IB_INTERN ulint mem_field_header_get_len(byte* field);	


	/// \brief Sets the check of a memory field
	/// \param [in] field memory field
	/// \param [in] check check of memory field
	IB_INTERN void mem_field_header_set_check(byte* field, ulint check);	


	/// \brief Gets the check of a memory field
	/// \param [in] field memory field
	/// \return check of memory field
	IB_INTERN ulint mem_field_header_get_check(byte* field);	


	/// \brief Sets the check of a memory field
	/// \param [in] field memory field
	/// \param [in] check check of memory field
	IB_INTERN void mem_field_trailer_set_check(byte* field, ulint check);	


	/// \brief Gets the check of a memory field
	/// \param [in] field memory field
	/// \return check of memory field
	IB_INTERN ulint mem_field_trailer_get_check(byte* field);	
#endif

/// \brief Initializes a buffer to a random combination of hex DE and AD.
/// \param [in] buf pointer to buffer
/// \param [in] n length of buffer
void mem_init_buf(byte*	buf, ulint n);

/// \brief Used to erase freed memory.
/// \param [in] buf pointer to buffer
/// \param [in] n length of buffer
IB_INTERN void mem_erase_buf(byte* buf, ulint n);

/// \brief Inserts a created memory heap to the hash table of current allocated memory heaps.
/// \param [in] heap memory heap
/// \param [in] file_name file name of creation
/// \param [in] line line where created
IB_INTERN void mem_hash_insert(mem_heap_t* heap, const char* file_name, ulint line);

/// \brief Removes a memory heap (which is going to be freed by the caller)
/// from the list of live memory heaps. Returns the size of the heap
/// in terms of how much memory in bytes was allocated for the user of
/// the heap (not the total space occupied by the heap).
/// Also validates the heap.
/// NOTE: This function does not free the storage occupied by the
/// heap itself, only the node in the list of heaps.
/// \param [in] heap memory heap
/// \param [in] file_name file name of freeing
/// \param [in] line line where freed
IB_INTERN void mem_hash_remove(mem_heap_t* heap, const char* file_name, ulint line);

void  mem_field_header_set_len(byte* field, ulint len);
ulint mem_field_header_get_len(byte* field);
void  mem_field_header_set_check(byte* field, ulint check);
ulint mem_field_header_get_check(byte* field);
void  mem_field_trailer_set_check(byte* field, ulint check);
ulint mem_field_trailer_get_check(byte* field);

#endif // IB_MEM_DEBUG
