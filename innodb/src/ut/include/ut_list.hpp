/*****************************************************************************

Copyright (c) 2006, 2009, Innobase Oy. All Rights Reserved.

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
@file include/ut0list.h
A double-linked list

Created 4/26/2006 Osku Salerma
************************************************************************/

/*******************************************************************//**
A double-linked list. This differs from the one in ut0lst.h in that in this
one, each list node contains a pointer to the data, whereas the one in
ut0lst.h uses a strategy where the list pointers are embedded in the data
items themselves.

Use this one when you need to store arbitrary data in the list where you
can't embed the list pointers in the data, if a data item needs to be
stored in multiple lists, etc.

Note about the memory management: ib_list_t is a fixed-size struct whose
allocation/deallocation is done through ib_list_create/ib_list_free, but the
memory for the list nodes is allocated through a user-given memory heap,
which can either be the same for all nodes or vary per node. Most users will
probably want to create a memory heap to store the item-specific data, and
pass in this same heap to the list node creation functions, thus
automatically freeing the list node when the item's heap is freed.

************************************************************************/

#ifndef IB_LIST_H
#define IB_LIST_H

#include "mem_mem.hpp"

typedef struct ib_list_struct ib_list_t;
typedef struct ib_list_node_struct ib_list_node_t;
typedef struct ib_list_helper_struct ib_list_helper_t;

/// \brief Create a new list using mem_alloc. Lists created with this function must be freed with ib_list_free.
/// \return list
IB_INTERN ib_list_t* ib_list_create(void);

/// \brief Free a list.
/// \param [in] list list
IB_INTERN void ib_list_free(ib_list_t* list);	/*!< in: list */

/// \brief Add the data to the end of the list.
/// \param [in] list list
/// \param [in] data data
/// \param [in] heap memory heap to use
/// \return new list node
IB_INTERN ib_list_node_t* ib_list_add_last(ib_list_t* list, void* data, mem_heap_t* heap);	

/// \brief Add the data after the indicated node.
/// \param [in] list list
/// \param [in] prev_node node preceding new node (can be NULL)
/// \param [in] data data
/// \param [in] heap memory heap to use
/// \return new list node
IB_INTERN ib_list_node_t* ib_list_add_after(ib_list_t* list, ib_list_node_t* prev_node, void* data, mem_heap_t* heap);		


/// \brief Remove the node from the list.
/// \param [in] list list
/// \param [in] node node to remove
/// \return new list node
IB_INTERN void ib_list_remove(ib_list_t* list, ib_list_node_t* node);


/// \brief Get the first node in the list.
/// \param [in] list list
/// \return first node, or NULL
IB_INLINE ib_list_node_t* ib_list_get_first(ib_list_t* list);



/// \brief Get the last node in the list.
/// \param [in] list list
/// \return last node, or NULL
IB_INLINE ib_list_node_t* ib_list_get_last(ib_list_t* list);

/// \brief List.
struct ib_list_struct {
	ib_list_node_t* first;
	ib_list_node_t* last;
	ibool			is_heap_list;
};

/// \brief A list node.
/// \param [in] prev previous node
/// \param [in] next next node
/// \param [in] data user data
struct ib_list_node_struct {
	ib_list_node_t* prev;
	ib_list_node_t* next;
	void*			data;
};

/// \brief Quite often, the only additional piece of data you need is the per-item
/// memory heap, so we have this generic struct available to use in those cases.
struct ib_list_helper_struct {
	mem_heap_t* heap;
	void*		data;
};

#ifndef IB_DO_NOT_INLINE
  #include "ut0list.inl"
#endif

#endif
