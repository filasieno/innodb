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
	void*           wait_object;      //!< pointer to the object the thread is waiting for; if NULL the cell is free for use
	ib_mutex_t*        old_wait_mutex;	  //!< the latest wait mutex in cell
	rw_lock_t*      old_wait_rw_lock; //!< the latest wait rw-lock in cell
	ulint           request_type;     //!< lock type requested on the object
	const char*     file;             //!< in debug version file where requested
	ulint           line;		      //!< in debug version line where requested
	os_thread_id_t	thread;		      //!< thread id of this waiting thread
	ibool           waiting;          //!< TRUE if the thread has already called sync_array_event_wait on this cell
    /// \brief We capture the signal_count of the wait_object when we reset the event.
    /// \details This value is then passed on to os_event_wait and we wait only if the event has not been signalled in the period between the reset and wait call.
	ib_int64_t      signal_count;	  
	time_t          reservation_time; //!< time when the thread reserved the wait cell
};

/// \brief Synchronization array 
/// \note: It is allowed for a thread to wait for an event allocated for the array without owning the protecting mutex (depending on the case: OS or database mutex), but all changes (set or reset) to the state of the event must be made while owning the mutex.
struct sync_array_struct {
	ulint        n_reserved;   //!< number of currently reservedcells in the wait array
	ulint        n_cells;      //!< number of cells in the wait array
	sync_cell_t* array;        //!< pointer to wait array
	ulint        protection;   //!< this flag tells which mutex protects the data
	ib_mutex_t   mutex;		   //!< possible database mutex protecting this data structure
	os_mutex_t   os_mutex;	   //!< Possible operating system mutex protecting the data structure. As this data structure is used in constructing the database mutex, to prevent infinite recursion in implementation, we fall back to an OS mutex.
	ulint        sg_count;	   //!< count of how many times an object has been signalled 
	ulint        res_count;	   //!< count of cell reservations since creation of the array 
};

/// \brief Mutexes or rw-locks held by a thread
typedef struct sync_thread_struct {
	os_thread_id_t id;	    //!< OS thread id
	sync_level_t*  levels;	//!< level array for this thread; if this is NULL this slot is unused
} sync_thread_t;

/// \brief An acquired mutex or rw-lock and its level in the latching order
typedef struct sync_level_struct{
	void*	latch;	//!< pointer to a mutex or an rw-lock; NULL means that the slot is empty
	ulint	level;	//!< level of the latch in the latching order
} sync_level_t;

/// \brief InnoDB mutex
/// \todo move debug info out of band into another data structure
typedef struct mutex_struct {

	static constinit ulint MAGIC_N = 979585;

	os_event_t              event;         //!< Used by sync_arr.c for the wait queue
	volatile lock_word_t    lock_word;     //!< lock_word is the target of the atomic test-and-set instruction when atomic operations are enabled
    ulong                   count_os_wait; //!< count of os_wait
	ulint                   waiters;       //!< This ulint is set to 1 if there are (or may be) threads waiting in the global wait array for this mutex to be released. Otherwise, this is 0. 
	UT_LIST_NODE_T(ib_mutex_t)	list;          //!< All allocated mutexes are put into a list.	Pointers to the next and prev. 
	const char*	            cfile_name;    //!< File name where mutex created 

#if !defined(IB_HAVE_ATOMIC_BUILTINS)
	os_fast_mutex_t         os_fast_mutex; //!< We use this OS mutex in place of lock_word when atomic operations are not enabled
#endif // !IB_HAVE_ATOMIC_BUILTINS 

#ifdef IB_SYNC_DEBUG
	const char*	file_name;     //!< File where the mutex was locked */
	ulint line;		           //!< Line where the mutex was locked */
	ulint level;		       //!< Level in the global latching order */
#endif // IB_SYNC_DEBUG

#ifdef IB_DEBUG
	os_thread_id_t thread_id;         //!< The thread id of the thread which locked the mutex. */
	ulint          magic_n;           //!< Equal to MUTEX_MAGIC_N
	ulong          count_using;       //!< count of times mutex used */
	ulong          count_spin_loop;   //!< count of spin loops */
	ulong          count_spin_rounds; //!< count of spin rounds */
	ulong          count_os_yield;    //!< count of os_wait */
	ib_uint64_t    lspent_time;       //!< mutex os_wait timer msec */
	ib_uint64_t    lmax_spent_time;   //!< mutex os_wait timer msec */
	const char*    cmutex_name;       //!< mutex name */
	ulint          mutex_type;        //!< 0=usual mutex, 1=rw_lock mutex */
#endif // IB_DEBUG
} ib_mutex_t;

typedef struct mutex_stats_struct {
    ulint spin_round_count;
    ulint spin_wait_count;
    ulint os_wait_count;
    ulint exit_count;
} ib_mutex_stats_t;

/// \brief Synchronization system state
struct ib_sync_state {
    bool          initialized;
    bool          order_checks_on;
    sync_array_t  primary_wait_array;
    ib_mutex_t    thread_mutex;
    sync_thread_t thread_level_arrays;
    mutex_stats_t mutex_stats;
};


// IB_INTERN void sync_var_init(innodb_state* state)
// {
// 	mutex_spin_round_count  = 0;
// 	mutex_spin_wait_count = 0;
// 	mutex_os_wait_count = 0;
// 	mutex_exit_count = 0;
// 	sync_primary_wait_array = NULL;
// 	sync_initialized = FALSE;
// #ifdef IB_SYNC_DEBUG
// 	sync_thread_level_arrays = NULL;
// 	memset(&sync_thread_mutex, 0x0, sizeof(sync_thread_mutex));
// #endif /* IB_SYNC_DEBUG */
// 	memset(&mutex_list, 0x0, sizeof(mutex_list));
// 	memset(&mutex_list_mutex, 0x0, sizeof(mutex_list_mutex));
// #ifdef IB_SYNC_DEBUG
// 	sync_order_checks_on    = FALSE;
// #endif /* IB_SYNC_DEBUG */
// }





/* NOTE! The structure appears here only for the compiler to know its size.
Do not use its fields directly! */

/** The structure used in the spin lock implementation of a read-write
lock. Several threads may have a shared lock simultaneously in this
lock, but only one writer may have an exclusive lock, in which case no
shared locks are allowed. To prevent starving of a writer blocked by
readers, a writer may queue for x-lock by decrementing lock_word: no
new readers will be let in while the thread waits for readers to
exit. */
struct rw_lock_struct {
	volatile lint	lock_word; /*!< Holds the state of the lock. */
	volatile ulint	waiters;/*!< 1: there are waiters */
	volatile ibool	recursive;/*!< Default value FALSE which means the lock
				is non-recursive. The value is typically set
				to TRUE making normal rw_locks recursive. In
				case of asynchronous IO, when a non-zero
				value of 'pass' is passed then we keep the
				lock non-recursive.
				This flag also tells us about the state of
				writer_thread field. If this flag is set
				then writer_thread MUST contain the thread
				id of the current x-holder or wait-x thread.
				This flag must be reset in x_unlock
				functions before incrementing the lock_word */
	volatile os_thread_id_t	writer_thread;
				/*!< Thread id of writer thread. Is only
				guaranteed to have sane and non-stale
				value iff recursive flag is set. */
	os_event_t	event;	/*!< Used by sync0arr.c for thread queueing */
	os_event_t	wait_ex_event;
				/*!< Event for next-writer to wait on. A thread
				must decrement lock_word before waiting. */
#ifndef INNODB_RW_LOCKS_USE_ATOMICS
	ib_mutex_t	mutex;		/*!< The mutex protecting rw_lock_struct */
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */

	UT_LIST_NODE_T(rw_lock_t) list;
				/*!< All allocated rw locks are put into a
				list */
#ifdef IB_SYNC_DEBUG
	UT_LIST_BASE_NODE_T(rw_lock_debug_t) debug_list;
				/*!< In the debug version: pointer to the debug
				info list of the lock */
	ulint	level;		/*!< Level in the global latching order. */
#endif /* IB_SYNC_DEBUG */
	ulint count_os_wait;	/*!< Count of os_waits. May not be accurate */
	const char*	cfile_name;/*!< File name where lock created */
		/* last s-lock file/line is not guaranteed to be correct */
	const char*	last_s_file_name;/*!< File name where last s-locked */
	const char*	last_x_file_name;/*!< File name where last x-locked */
	ibool		writer_is_wait_ex;
				/*!< This is TRUE if the writer field is
				RW_LOCK_WAIT_EX; this field is located far
				from the memory update hotspot fields which
				are at the start of this struct, thus we can
				peek this field without causing much memory
				bus traffic */
	unsigned	cline:14;	/*!< Line where created */
	unsigned	last_s_line:14;	/*!< Line number where last time s-locked */
	unsigned	last_x_line:14;	/*!< Line number where last time x-locked */
	ulint	magic_n;	/*!< RW_LOCK_MAGIC_N */
};

/** Value of rw_lock_struct::magic_n */
#define	RW_LOCK_MAGIC_N	22643

#ifdef IB_SYNC_DEBUG
/** The structure for storing debug info of an rw-lock */
struct	rw_lock_debug_struct {
	os_thread_id_t thread_id;             //!< The thread id of the thread which locked the rw-lock */
	ulint          pass;		          //!< Pass value given in the lock operation */
	ulint          lock_type;	          //!< Type of the lock: RW_LOCK_EX, RW_LOCK_SHARED, RW_LOCK_WAIT_EX */
	const char*    file_name;             //!< File name where the lock was obtained */
	ulint          line;		          //!< Line where the rw-lock was locked */
	UT_LIST_NODE_T(rw_lock_debug_t) list; //!< Debug structs are linked in a two-way list */
};
#endif // IB_SYNC_DEBUG