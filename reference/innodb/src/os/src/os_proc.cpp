/*****************************************************************************

Copyright (c) 1995, 2025, Innobase Oy. All Rights Reserved.

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

/// @file os_proc.cpp
/// \brief The interface to the operating system process control primitives
///
/// Created 9/30/1995 Heikki Tuuri

#include "config.hpp"

#ifndef __WIN__
#include <errno.h>
#include <sys/types.h>
#ifdef IB_HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "os_proc.hpp"
#ifdef IB_DO_NOT_INLINE
#include "os0proc.inl"
#endif

#include "ut_mem.hpp"
#include "ut_byte.hpp"

/* FreeBSD for example has only MAP_ANON, Linux has MAP_ANONYMOUS and
MAP_ANON but MAP_ANON is marked as deprecated */
#if defined(MAP_ANONYMOUS)
#define OS_MAP_ANON	MAP_ANONYMOUS
#elif defined(MAP_ANON)
#define OS_MAP_ANON	MAP_ANON
#endif

IB_INTERN ibool os_use_large_pages;
/* Large page size. This may be a boot-time option on some platforms */
IB_INTERN ulint os_large_page_size;

/// \brief Reset the variables.
IB_INTERN
void os_proc_var_init(void)
{
	os_use_large_pages = 0;
	os_large_page_size = 0;
}

/// \brief Converts the current process id to a number.
/// \details It is not guaranteed that the number is unique. In Linux returns the 'process number' of the current thread. That number is the same as one sees in 'top', for example. In Linux the thread id is not the same as one sees in 'top'.
/// \return Process id as a number.
IB_INTERN
ulint os_proc_get_number(void)
{
#ifdef __WIN__
	return((ulint)GetCurrentProcessId());
#else
	return((ulint)getpid());
#endif
}

/// \brief Allocates large pages memory.
/// \param n Number of bytes.
/// \return Allocated memory.
IB_INTERN
void* os_mem_alloc_large(ulint* n)
{
	void*	ptr;
	ulint	size;
#if defined IB_HAVE_LARGE_PAGES && defined IB_LINUX
	int shmid;
	struct shmid_ds buf;

	if (!os_use_large_pages || !os_large_page_size) {
		goto skip;
	}

	/* Align block size to os_large_page_size */
	ut_ad(ut_is_2pow(os_large_page_size));
	size = ut_2pow_round(*n + (os_large_page_size - 1),
			     os_large_page_size);

	shmid = shmget(IPC_PRIVATE, (size_t)size, SHM_HUGETLB | SHM_R | SHM_W);
	if (shmid < 0) {
		ib_log(state,
			"InnoDB: HugeTLB: Warning: Failed to allocate"
			" %lu bytes. errno %d\n", size, errno);
		ptr = NULL;
	} else {
		ptr = shmat(shmid, NULL, 0);
		if (ptr == (void *)-1) {
			ib_log(state,
				"InnoDB: HugeTLB: Warning: Failed to"
				" attach shared memory segment, errno %d\n",
				errno);
			ptr = NULL;
		}

		/* Remove the shared memory segment so that it will be
		automatically freed after memory is detached or
		process exits */
		shmctl(shmid, IPC_RMID, &buf);
	}

	if (ptr) {
		*n = size;
		os_fast_mutex_lock(&ut_list_mutex);
		ut_total_allocated_memory += size;
		os_fast_mutex_unlock(&ut_list_mutex);
# ifdef IB_SET_MEM_TO_ZERO
		memset(ptr, '\0', size);
# endif
		IB_MEM_ALLOC(ptr, size);
		return(ptr);
	}

	ib_log(state, "InnoDB HugeTLB: Warning: Using conventional"
		" memory pool\n");
skip:
#endif /* IB_HAVE_LARGE_PAGES && IB_LINUX */

#ifdef __WIN__
	SYSTEM_INFO	system_info;
	GetSystemInfo(&system_info);

	/* Align block size to system page size */
	ut_ad(ut_is_2pow(system_info.dwPageSize));
	/* system_info.dwPageSize is only 32-bit. Casting to ulint is required
	on 64-bit Windows. */
	size = *n = ut_2pow_round(*n + (system_info.dwPageSize - 1),
				  (ulint) system_info.dwPageSize);
	ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE,
			   PAGE_READWRITE);
	if (!ptr) {
		ib_log(state, "InnoDB: VirtualAlloc(%lu bytes) failed;"
			" Windows error %lu\n",
			(ulong) size, (ulong) GetLastError());
	} else {
		os_fast_mutex_lock(&ut_list_mutex);
		ut_total_allocated_memory += size;
		os_fast_mutex_unlock(&ut_list_mutex);
		IB_MEM_ALLOC(ptr, size);
	}
#elif defined __NETWARE__ || !defined OS_MAP_ANON
	size = *n;
	ptr = ut_malloc_low(size, TRUE, FALSE);
#else
# ifdef IB_HAVE_GETPAGESIZE
	size = getpagesize();
# else
	size = IB_PAGE_SIZE;
# endif
	/* Align block size to system page size */
	ut_ad(ut_is_2pow(size));
	size = *n = ut_2pow_round(*n + (size - 1), size);
	ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | OS_MAP_ANON, -1, 0);
	if (IB_UNLIKELY(ptr == (void*) -1)) {
		ib_log(state, "InnoDB: mmap(%lu bytes) failed;"
			" errno %lu\n",
			(ulong) size, (ulong) errno);
		ptr = NULL;
	} else {
		os_fast_mutex_lock(&ut_list_mutex);
		ut_total_allocated_memory += size;
		os_fast_mutex_unlock(&ut_list_mutex);
		IB_MEM_ALLOC(ptr, size);
	}
#endif
	return(ptr);
}

/// \brief Frees large pages memory.
/// \param ptr Pointer returned by os_mem_alloc_large().
/// \param size Size returned by os_mem_alloc_large().
IB_INTERN
void os_mem_free_large(void *ptr, ulint size)
{
	os_fast_mutex_lock(&ut_list_mutex);
	ut_a(ut_total_allocated_memory >= size);
	os_fast_mutex_unlock(&ut_list_mutex);

#if defined IB_HAVE_LARGE_PAGES && defined IB_LINUX
	if (os_use_large_pages && os_large_page_size && !shmdt(ptr)) {
		os_fast_mutex_lock(&ut_list_mutex);
		ut_a(ut_total_allocated_memory >= size);
		ut_total_allocated_memory -= size;
		os_fast_mutex_unlock(&ut_list_mutex);
		IB_MEM_FREE(ptr, size);
		return;
	}
#endif /* IB_HAVE_LARGE_PAGES && IB_LINUX */
#ifdef __WIN__
	/* When RELEASE memory, the size parameter must be 0.
	Do not use MEM_RELEASE with MEM_DECOMMIT. */
	if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
		ib_log(state, "InnoDB: VirtualFree(%p, %lu) failed;"
			" Windows error %lu\n",
			ptr, (ulong) size, (ulong) GetLastError());
	} else {
		os_fast_mutex_lock(&ut_list_mutex);
		ut_a(ut_total_allocated_memory >= size);
		ut_total_allocated_memory -= size;
		os_fast_mutex_unlock(&ut_list_mutex);
		IB_MEM_FREE(ptr, size);
	}
#elif defined __NETWARE__ || !defined OS_MAP_ANON
	ut_free(ptr);
#else
	if (munmap(ptr, size)) {
		ib_log(state, "InnoDB: munmap(%p, %lu) failed;"
			" errno %lu\n",
			ptr, (ulong) size, (ulong) errno);
	} else {
		os_fast_mutex_lock(&ut_list_mutex);
		ut_a(ut_total_allocated_memory >= size);
		ut_total_allocated_memory -= size;
		os_fast_mutex_unlock(&ut_list_mutex);
		IB_MEM_FREE(ptr, size);
	}
#endif
}
