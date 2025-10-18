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

/// \file ha_ha.hpp
/// \brief The hash table with external chains
/// \details Originally created by Heikki Tuuri in 8/18/1994
/// \author Fabio N. Filasieno
/// \date 18/10/2025

#pragma once

#include "univ.i"

#include "hash_hash.hpp"
#include "page_types.hpp"
#include "buf_types.hpp"

/// \brief Looks for an element in a hash table.
/// \return pointer to the data of the first hash table node in chain having the fold number, NULL if not found
/// \param [in] table hash table
/// \param [in] fold folded value of the searched data
IB_INLINE void* ha_search_and_get_data(hash_table_t* table, ulint fold);
/// \brief Looks for an element when we know the pointer to the data and updates the pointer to data if found.
/// \param [in,out] table hash table
/// \param [in] fold folded value of the searched data
/// \param [in] data pointer to the data
/// \param [in] new_block block containing new_data (only in debug builds)
/// \param [in] new_data new pointer to the data
IB_INTERN void ha_search_and_update_if_found_func(hash_table_t* table, ulint fold, void* data,
#if defined IB_AHI_DEBUG || defined IB_DEBUG
	buf_block_t* new_block,
#endif /* IB_AHI_DEBUG || IB_DEBUG */
	void* new_data);

#if defined IB_AHI_DEBUG || defined IB_DEBUG
/** Looks for an element when we know the pointer to the data and
updates the pointer to data if found.
@param table		in/out: hash table
@param fold		in: folded value of the searched data
@param data		in: pointer to the data
@param new_block	in: block containing new_data
@param new_data		in: new pointer to the data */
# define ha_search_and_update_if_found(table,fold,data,new_block,new_data) \
	ha_search_and_update_if_found_func(table,fold,data,new_block,new_data)
#else /* IB_AHI_DEBUG || IB_DEBUG */
/** Looks for an element when we know the pointer to the data and
updates the pointer to data if found.
@param table		in/out: hash table
@param fold		in: folded value of the searched data
@param data		in: pointer to the data
@param new_block	ignored: block containing new_data
@param new_data		in: new pointer to the data */
# define ha_search_and_update_if_found(table,fold,data,new_block,new_data) \
	ha_search_and_update_if_found_func(table,fold,data,new_data)
#endif /* IB_AHI_DEBUG || IB_DEBUG */
/// \brief Creates a hash table with at least n array cells. The actual number of cells is chosen to be a prime number slightly bigger than n.
/// \return own: created table
/// \param [in] n number of array cells
/// \param [in] mutex_level level of the mutexes in the latching order: this is used in the debug version
/// \param [in] n_mutexes number of mutexes to protect the hash table: must be a power of 2, or 0
IB_INTERN hash_table_t* ha_create_func(ulint n,
#ifdef IB_SYNC_DEBUG
	ulint mutex_level,
#endif /* IB_SYNC_DEBUG */
	ulint n_mutexes);
#ifdef IB_SYNC_DEBUG
/** Creates a hash table.
@return		own: created table
@param n_c	in: number of array cells.  The actual number of cells is
chosen to be a slightly bigger prime number.
@param level	in: level of the mutexes in the latching order
@param n_m	in: number of mutexes to protect the hash table;
		must be a power of 2, or 0 */
# define ha_create(n_c,n_m,level) ha_create_func(n_c,level,n_m)
#else /* IB_SYNC_DEBUG */
/** Creates a hash table.
@return		own: created table
@param n_c	in: number of array cells.  The actual number of cells is
chosen to be a slightly bigger prime number.
@param level	in: level of the mutexes in the latching order
@param n_m	in: number of mutexes to protect the hash table;
		must be a power of 2, or 0 */
# define ha_create(n_c,n_m,level) ha_create_func(n_c,n_m)
#endif /* IB_SYNC_DEBUG */

/// \brief Empties a hash table and frees the memory heaps.
/// \param [in,own] table hash table
IB_INTERN void ha_clear(hash_table_t* table);

/// \brief Inserts an entry into a hash table. If an entry with the same fold number is found, its node is updated to point to the new data, and no new node is inserted.
/// \return TRUE if succeed, FALSE if no more memory could be allocated
/// \param [in] table hash table
/// \param [in] fold folded value of data; if a node with the same fold value already exists, it is updated to point to the same data, and no new node is created!
/// \param [in] block buffer block containing the data (only in debug builds)
/// \param [in] data data, must not be NULL
IB_INTERN ibool ha_insert_for_fold_func(hash_table_t* table, ulint fold,
#if defined IB_AHI_DEBUG || defined IB_DEBUG
	buf_block_t* block,
#endif /* IB_AHI_DEBUG || IB_DEBUG */
	void* data);

#if defined IB_AHI_DEBUG || defined IB_DEBUG
/**
Inserts an entry into a hash table. If an entry with the same fold number
is found, its node is updated to point to the new data, and no new node
is inserted.
@return	TRUE if succeed, FALSE if no more memory could be allocated
@param t	in: hash table
@param f	in: folded value of data
@param b	in: buffer block containing the data
@param d	in: data, must not be NULL */
# define ha_insert_for_fold(t,f,b,d) ha_insert_for_fold_func(t,f,b,d)
#else /* IB_AHI_DEBUG || IB_DEBUG */
/**
Inserts an entry into a hash table. If an entry with the same fold number
is found, its node is updated to point to the new data, and no new node
is inserted.
@return	TRUE if succeed, FALSE if no more memory could be allocated
@param t	in: hash table
@param f	in: folded value of data
@param b	ignored: buffer block containing the data
@param d	in: data, must not be NULL */
# define ha_insert_for_fold(t,f,b,d) ha_insert_for_fold_func(t,f,d)
#endif /* IB_AHI_DEBUG || IB_DEBUG */

/// \brief Looks for an element when we know the pointer to the data and deletes it from the hash table if found.
/// \return TRUE if found
/// \param [in] table hash table
/// \param [in] fold folded value of the searched data
/// \param [in] data pointer to the data
IB_INLINE ibool ha_search_and_delete_if_found(hash_table_t* table, ulint fold, void* data);
#ifndef IB_HOTBACKUP
/// \brief Removes from the chain determined by fold all nodes whose data pointer points to the page given.
/// \param [in] table hash table
/// \param [in] fold fold value
/// \param [in] page buffer page
IB_INTERN void ha_remove_all_nodes_to_page(hash_table_t* table, ulint fold, const page_t* page);
/// \brief Validates a given range of the cells in hash table.
/// \return TRUE if ok
/// \param [in] table hash table
/// \param [in] start_index start index
/// \param [in] end_index end index
IB_INTERN ibool ha_validate(hash_table_t* table, ulint start_index, ulint end_index);
/// \brief Prints info of a hash table.
/// \param [in] state->stream stream where to print
/// \param [in] table hash table
IB_INTERN void ha_print_info(ib_stream_t state->stream, hash_table_t* table);
#endif /* !IB_HOTBACKUP */

/** The hash table external chain node */
typedef struct ha_node_struct ha_node_t;

/** The hash table external chain node */
struct ha_node_struct {
	ha_node_t*	next;	/*!< next chain node or NULL if none */
#if defined IB_AHI_DEBUG || defined IB_DEBUG
	buf_block_t*	block;	/*!< buffer block containing the data, or NULL */
#endif /* IB_AHI_DEBUG || IB_DEBUG */
	void*		data;	/*!< pointer to the data */
	ulint		fold;	/*!< fold value for the data */
};

#ifndef IB_HOTBACKUP
/** Assert that the current thread is holding the mutex protecting a
hash bucket corresponding to a fold value.
@param table	in: hash table
@param fold	in: fold value */
# define ASSERT_HASH_MUTEX_OWN(table, fold)				\
	ut_ad(!(table)->mutexes || mutex_own(hash_get_mutex(table, fold)))
#else /* !IB_HOTBACKUP */
/** Assert that the current thread is holding the mutex protecting a
hash bucket corresponding to a fold value.
@param table	in: hash table
@param fold	in: fold value */
# define ASSERT_HASH_MUTEX_OWN(table, fold) ((void) 0)
#endif /* !IB_HOTBACKUP */

#ifndef IB_DO_NOT_INLINE
#include "ha0ha.inl"
#endif
