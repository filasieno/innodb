// MIT License
//
// Copyright (c) 2025 Fabio N. Filasieno
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// \file sync_types.hpp
/// \brief sync module types header file
/// \author Fabio N. Filasieno
/// \date 2025-10-13

#pragma once



/*
The wait array consists of cells each of which has an
an operating system event object created for it. The threads
waiting for a mutex, for example, can reserve a cell
in the array and suspend themselves to wait for the event
to become signaled. When using the wait array, remember to make
sure that some thread holding the synchronization object
will eventually know that there is a waiter in the array and
signal the object, to prevent infinite wait.
Why we chose to implement a wait array? First, to make
mutexes fast, we had to code our own implementation of them,
which only in usually uncommon cases resorts to using
slow operating system primitives. Then we had the choice of
assigning a unique OS event for each mutex, which would
be simpler, or using a global wait array. In some operating systems,
the global wait array solution is more efficient and flexible,
because we can do with a very small number of OS events,
say 200. In NT 3.51, allocating events seems to be a quadratic
algorithm, because 10 000 events are created fast, but
100 000 events takes a couple of minutes to create.

As of 5.0.30 the above mentioned design is changed. Since now
OS can handle millions of wait events efficiently, we no longer
have this concept of each cell of wait array having one event.
Instead, now the event that a thread wants to wait on is embedded
in the wait object (mutex or rw_lock). We still keep the global
wait array for the sake of diagnostics and also to avoid infinite
wait The error_monitor thread scans the global wait array to signal
any waiting threads who have missed the signal. */

/** A cell where an individual thread may wait suspended
until a resource is released. The suspending is implemented
using an operating system event semaphore. */
struct sync_cell_struct {
	void*		wait_object;	  //!< pointer to the object the thread is waiting for; if NULL the cell is free for use
	mutex_t*	old_wait_mutex;	  //!< the latest wait mutex in cell
	rw_lock_t*	old_wait_rw_lock; //!< the latest wait rw-lock in cell
	ulint		request_type;	  //!< lock type requested on the object
	const char*	file;		      //!< in debug version file where requested
	ulint		line;		      //!< in debug version line where requested
	os_thread_id_t	thread;		  //!< thread id of this waiting thread
	ibool		waiting;	      //!< TRUE if the thread has already called sync_array_event_wait on this cell
    /// \brief We capture the signal_count of the wait_object when we reset the event.
    /// \details This value is then passed on to os_event_wait and we wait only if the event has not been signalled in the period between the reset and wait call.
	ib_int64_t	signal_count;	  
	time_t		reservation_time; /*!< time when the thread reserved the wait cell */
};

/// \brief Synchronization array 
/// \note: It is allowed for a thread to wait for an event allocated for the array without owning the protecting mutex (depending on the case: OS or database mutex), but all changes (set or reset) to the state of the event must be made while owning the mutex.
struct sync_array_struct {
	ulint		n_reserved;	//!< number of currently reservedcells in the wait array
	ulint		n_cells;	//!< number of cells in the wait array
	sync_cell_t*	array;	//!< pointer to wait array
	ulint		protection;	//!< this flag tells which mutex protects the data */
	mutex_t		mutex;		//!< possible database mutex protecting this data structure */
	os_mutex_t	os_mutex;	//!< Possible operating system mutex protecting the data structure. As this data structure is used in constructing the database mutex, to prevent infinite recursion in implementation, we fall back to an OS mutex. */
	ulint		sg_count;	//!< count of how many times an object has been signalled */
	ulint		res_count;	//!< count of cell reservations since creation of the array */
};


/// \brief Rename mutex_t to avoid name space collision on some systems
#define mutex_t ib_mutex_t

/// \brief InnoDB mutex
typedef struct mutex_struct mutex_t;