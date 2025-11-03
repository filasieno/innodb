/*****************************************************************************

Copyright (c) 1995, 2009, Innobase Oy. All Rights Reserved.

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

/**************************************************//**
@file include/sync0arr.h
The wait array used in synchronization primitives

Created 9/5/1995 Heikki Tuuri
*******************************************************/

#ifndef sync0arr_h
#define sync0arr_h

#include "univ.i"
#include "ut_lst.hpp"
#include "ut_mem.hpp"
#include "os_thread.hpp"

/** Synchronization wait array cell */
typedef struct sync_cell_struct		sync_cell_t;
/** Synchronization wait array */
typedef struct sync_array_struct	sync_array_t;

/** Parameters for sync_array_create() @{ */
#define SYNC_ARRAY_OS_MUTEX	1	/*!< protected by os_mutex_t */
#define SYNC_ARRAY_MUTEX	2	/*!< protected by mutex_t */
/* @} */

/// \brief  Creates a synchronization wait array. 
/// \details It is protected by a mutexwhich is automatically reserved when the functions operating on it are called.
/// \param n_cells The number of cells in the array to create
/// \param protection in: either SYNC_ARRAY_OS_MUTEX or SYNC_ARRAY_MUTEX: determines the type of mutex protecting the data structure
/// \return The created wait array
IB_INTERN sync_array_t* sync_array_create(ulint n_cells, ulint protection);

/// \brief Frees the resources in a wait array.
/// \param arr The wait array to free
IB_INTERN void sync_array_free(sync_array_t* arr);	/*!< in, own: sync wait array */

/// \brief Reserves a wait array cell for waiting for an object.
/// \details The event of the cell is reset to nonsignalled state.
/// \param arr The wait array
/// \param object The object to wait for
/// \param type The type of lock request
/// \param file The file where the request was made
/// \param line The line where the request was made
/// \param index The index of the reserved cell
IB_INTERN void sync_array_reserve_cell(sync_array_t* arr, void* object, ulint type, const char* file, ulint line, ulint* index);
	
/// \brief This function should be called when a thread starts to wait on
/// \details In the debug version this function checks
/// \param arr The wait array
/// \param index The index of the reserved cell
IB_INTERN void sync_array_wait_event(sync_array_t* arr, ulint index);

/// \brief Frees the cell. NOTE! sync_array_wait_event frees the cell automatically!
/// \param arr The wait array
/// \param index The index of the reserved cell
IB_INTERN void sync_array_free_cell(sync_array_t* arr, ulint index);

/// \brief Note that one of the wait objects was signalled.
/// \param arr The wait array
IB_INTERN void sync_array_object_signalled(sync_array_t* arr);

/**
If the wakeup algorithm does not work perfectly at semaphore relases,
this function will do the waking (see the comment in mutex_exit). This
function should be called about every 1 second in the server. */
IB_INTERN void sync_arr_wake_threads_if_sema_free(void);

/// \brief Prints warnings of long semaphore waits to stderr.
/// \return TRUE if fatal semaphore wait threshold was exceeded
IB_INTERN ibool sync_array_print_long_waits(void);

/// \brief Validates the integrity of the wait array. Checks
/// \param arr The wait array
IB_INTERN void sync_array_validate(sync_array_t*	arr);

/// \brief Prints info of the wait array.
/// \param state The state
/// \param arr The wait array
IB_INTERN void sync_array_print_info(ib_stream_t state->stream, sync_array_t* arr);


#ifndef IB_DO_NOT_INLINE
  #include "sync_arr.inl"
#endif

#endif
