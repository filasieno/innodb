// Copyright (c) 1995, 2025, Innobase Oy. All Rights Reserved.
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA

// @file include/os_proc.hpp
// The interface to the operating system process control primitives
// Created 9/30/1995 Heikki Tuuri

#ifndef os0proc_h
#define os0proc_h

#include "univ.inl"

/* Required for HugeTLB support. */
#ifdef IB_LINUX
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

typedef void*			  os_process_t;
typedef unsigned long int os_process_id_t;

extern ibool os_use_large_pages;
extern ulint os_large_page_size; // Large page size. This may be a boot-time option on some platforms

/// \brief Converts the current process id to a number. It is not guaranteed that the
/// number is unique. In Linux returns the 'process number' of the current
/// thread. That number is the same as one sees in 'top', for example. In Linux
/// the thread id is not the same as one sees in 'top'.
/// \return process id as a number
IB_INTERN ulint os_proc_get_number(void);

/// \brief Allocates large pages memory.
/// \return allocated memory
/// \param n in/out: number of bytes
IB_INTERN void* os_mem_alloc_large(ulint* n);

/// \brief Frees large pages memory.
/// \param ptr pointer returned by os_mem_alloc_large()
/// \param size size returned by os_mem_alloc_large()
IB_INTERN void os_mem_free_large(void* ptr, ulint size);

/// \brief Reset the variables.
IB_INTERN void os_proc_var_init(void);

#ifndef IB_DO_NOT_INLINE
#include "os_proc.inl"
#endif

#endif
