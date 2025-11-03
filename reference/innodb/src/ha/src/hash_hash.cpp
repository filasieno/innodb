// Copyright (c) 1997, 2009, Innobase Oy. All Rights Reserved.
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

/// \file hash_hash.cpp
/// \brief The simple hash table utility
/// \details Originally created by Heikki Tuuri in 5/20/1997
/// \author Fabio N. Filasieno
/// \date 18/10/2025

#include "hash_hash.hpp"

#ifdef IB_DO_NOT_INLINE
	#include "hash0hash.inl"
#endif

#include "mem_mem.hpp"

#ifndef IB_HOTBACKUP
/// \brief Reserves the mutex for a fold value in a hash table.
/// \param [in] table hash table
/// \param [in] fold fold
IB_INTERN void hash_mutex_enter(hash_table_t* table, ulint fold)
{
	mutex_enter(hash_get_mutex(table, fold));
}

/// \brief Releases the mutex for a fold value in a hash table.
/// \param [in] table hash table
/// \param [in] fold fold
IB_INTERN void hash_mutex_exit(hash_table_t* table, ulint fold)
{
	mutex_exit(hash_get_mutex(table, fold));
}

/// \brief Reserves all the mutexes of a hash table, in an ascending order.
/// \param [in] table hash table
IB_INTERN void hash_mutex_enter_all(hash_table_t* table)
{
    for (ulint i = 0; i < table->n_mutexes; i++) {
        mutex_enter(table->mutexes + i);
    }
}

/// \brief Releases all the mutexes of a hash table.
/// \param [in] table hash table
IB_INTERN void hash_mutex_exit_all(hash_table_t* table)
{
    for (ulint i = 0; i < table->n_mutexes; i++) {
        mutex_exit(table->mutexes + i);
    }
}
#endif /* !IB_HOTBACKUP */

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Creates a hash table with >= n array cells.
/// \details The actual number of cells is chosen to be a prime number slightly bigger than n.
/// \param [in] n number of array cells
/// \return own: created table
IB_INTERN hash_table_t* hash_create(ulint n)
{
    ulint prime = ut_find_prime(n);
    hash_table_t* table = IB_MEM_ALLOC(sizeof(hash_table_t));
    hash_cell_t* array = ut_malloc(sizeof(hash_cell_t) * prime);

    table->array = array;
    table->n_cells = prime;
#ifndef IB_HOTBACKUP
# if defined IB_AHI_DEBUG || defined IB_DEBUG
    table->adaptive = FALSE;
# endif /* IB_AHI_DEBUG || IB_DEBUG */
    table->n_mutexes = 0;
    table->mutexes = NULL;
    table->heaps = NULL;
#endif /* !IB_HOTBACKUP */
    table->heap = NULL;
    ut_d(table->magic_n = HASH_TABLE_MAGIC_N);

    hash_table_clear(table);
    return table;
}

/// \brief Frees a hash table.
/// \param [in,own] table hash table
IB_INTERN void hash_table_free(hash_table_t* table)
{
    ut_ad(table);
    ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
#ifndef IB_HOTBACKUP
    ut_a(table->mutexes == NULL);
#endif /* !IB_HOTBACKUP */

    ut_free(table->array);
    IB_MEM_FREE(table);
}

#ifndef IB_HOTBACKUP
/// \brief Creates a mutex array to protect a hash table.
/// \param [in] table hash table
#ifdef IB_SYNC_DEBUG
/// \param [in] sync_level latching order level of the mutexes: used in the debug version
#endif /* IB_SYNC_DEBUG */
/// \param [in] n_mutexes number of mutexes, must be a power of 2
IB_INTERN void hash_create_mutexes_func(hash_table_t* table,
#ifdef IB_SYNC_DEBUG
    ulint sync_level,
#endif /* IB_SYNC_DEBUG */
    ulint n_mutexes)
{
    ut_ad(table);
    ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
    ut_a(n_mutexes > 0);
    ut_a(ut_is_2pow(n_mutexes));

    table->mutexes = IB_MEM_ALLOC(n_mutexes * sizeof(mutex_t));

    for (ulint i = 0; i < n_mutexes; i++) {
        mutex_create(table->mutexes + i, sync_level);
    }

    table->n_mutexes = n_mutexes;
}

/// \brief Frees a mutex array created with hash_create_mutexes_func().
/// \param [in,own] table hash table
IB_INTERN void hash_free_mutexes_func(hash_table_t* table)
{
    for (ulint i = 0; i < table->n_mutexes; i++) {
        mutex_free(&table->mutexes[i]);
#ifdef IB_DEBUG
        memset(&table->mutexes[i], 0x0, sizeof(table->mutexes[i]));
#endif
    }

    IB_MEM_FREE(table->mutexes);
}
#endif /* !IB_HOTBACKUP */
