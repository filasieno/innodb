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
// Place, Suite 330, Boston, MA 02111-1307 USA

/// @file ut/ut0dbg.c
/// \brief Debug utilities for Innobase.
///
/// Created 1/30/1994 Heikki Tuuri

#include "ut_dbg.hpp"

#include "univ.inl"

#if defined(__GNUC__) && (__GNUC__ > 2)
#else
/// \brief This is used to eliminate compiler warnings
IB_INTERN ulint ut_dbg_zero = 0;
#endif

#if defined(IB_SYNC_DEBUG) || !defined(UT_DBG_USE_ABORT)
/// \brief If this is set to TRUE by ut_dbg_assertion_failed(), all threads
/// will stop at the next ut_a() or ut_ad().
IB_INTERN ibool ut_dbg_stop_threads = FALSE;
#endif
#ifdef __NETWARE__
/// \brief Flag for ignoring further assertion failures.  This is set to TRUE
/// when on NetWare there happens an InnoDB assertion failure or other
/// fatal error condition that requires an immediate shutdown.
IB_INTERN ibool panic_shutdown = FALSE;
#elif !defined(UT_DBG_USE_ABORT)
/// \brief A null pointer that will be dereferenced to trigger a memory trap
IB_INTERN ulint *ut_dbg_null_ptr = NULL;
#endif


/// \brief Report a failed assertion.
/// \param expr in: the failed assertion (optional)
/// \param file in: source file containing the assertion
/// \param line in: line number of the assertion
IB_INTERN void ut_dbg_assertion_failed(const char *expr, const char *file, ulint line)
{
	ut_print_timestamp(state->stream);

#ifdef IB_HOTBACKUP
	ib_log(state, "  InnoDB: Assertion failure in file %s line %lu\n", file, line);
#else  /* IB_HOTBACKUP */
	ib_log(state, "  InnoDB: Assertion failure in thread %lu in file %s line %lu\n", os_thread_pf(os_thread_get_curr_id()), file, line);
#endif /* IB_HOTBACKUP */
	if (expr) {
		ib_log(state, "InnoDB: Failing assertion: %s\n", expr);
	}

	state->log(
		state->stream,
		"InnoDB: We intentionally generate a memory trap.\n"
		"InnoDB: Submit a detailed bug report, check the InnoDB website for details\n"
		"InnoDB: If you get repeated assertion failures or crashes, even\n"
		"InnoDB: immediately after the server startup, there may be\n"
		"InnoDB: corruption in the InnoDB tablespace. Please refer to\n"
		"InnoDB: the InnoDB website for details\n"
		"InnoDB: about forcing recovery.\n"
	);

#if defined(IB_SYNC_DEBUG) || !defined(UT_DBG_USE_ABORT)
	ut_dbg_stop_threads = TRUE;
#endif

}

#ifdef __NETWARE__
	/// \brief Shut down InnoDB after assertion failure.
	IB_INTERN void ut_dbg_panic(void)
	{
		if (!panic_shutdown) {
			panic_shutdown = TRUE;
			innobase_shutdown(IB_SHUTDOWN_NORMAL);
		}
		exit(1);
	}
#else // __NETWARE__
	#if defined(IB_SYNC_DEBUG) || !defined(UT_DBG_USE_ABORT)
	/// \brief Stop a thread after assertion failure.
	/// \param file in: source file name
	/// \param line in: line number
	IB_INTERN
	void ut_dbg_stop_thread(const char *file, ulint line)
	{
	#ifndef IB_HOTBACKUP
		ib_log(state, "InnoDB: Thread %lu stopped in file %s line %lu\n", os_thread_pf(os_thread_get_curr_id()), file, line);
		os_thread_sleep(1000000000);
	#endif /* !IB_HOTBACKUP */
	}
	#endif
#endif // __NETWARE__

#ifdef IB_COMPILE_TEST_FUNCS

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef IB_HAVE_UNISTD_H
  #include <unistd.h>
#endif

#ifndef timersub

	#define timersub(a, b, r)                           \
		do {                                            \
			(r)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
			(r)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
			if ((r)->tv_usec < 0) {                     \
				(r)->tv_sec--;                          \
				(r)->tv_usec += 1000000;                \
			}                                           \
		} while (0)

#endif // timersub

/// \brief Resets a speedo (records the current time in it).
/// \param speedo out: speedo
IB_INTERN void speedo_reset(speedo_t *speedo)
{
	gettimeofday(&speedo->tv, NULL);
	getrusage(RUSAGE_SELF, &speedo->ru);
}

/// \brief Shows the time elapsed and usage statistics since the last reset of a speedo.
/// \param [in] speedo speedo
IB_INTERN void speedo_show(const speedo_t *speedo)
{
	struct rusage ru_now;
	struct timeval tv_now;
	struct timeval tv_diff;

	getrusage(RUSAGE_SELF, &ru_now);
	gettimeofday(&tv_now, NULL);

#define PRINT_TIMEVAL(prefix, tvp) ib_log(state, "%s% 5ld.%06ld sec\n", prefix, (tvp)->tv_sec, (tvp)->tv_usec)

	timersub(&tv_now, &speedo->tv, &tv_diff);
	PRINT_TIMEVAL("real", &tv_diff);
	timersub(&ru_now.ru_utime, &speedo->ru.ru_utime, &tv_diff);
	PRINT_TIMEVAL("user", &tv_diff);
	timersub(&ru_now.ru_stime, &speedo->ru.ru_stime, &tv_diff);
	PRINT_TIMEVAL("sys ", &tv_diff);
}

#endif /* IB_COMPILE_TEST_FUNCS */
