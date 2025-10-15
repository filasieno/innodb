// Copyright (c) 1994, 2009, Innobase Oy. All Rights Reserved.
// Copyright (c) 2009, Sun Microsystems, Inc.
//
// Portions of this file contain modifications contributed and copyrighted by
// Sun Microsystems, Inc. Those modifications are gratefully acknowledged and
// are described briefly in the InnoDB documentation. The contributions by
// Sun Microsystems are incorporated with their permission, and subject to the
// conditions contained in the file COPYING.Sun_Microsystems.
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

/// \file ut_ut.c
/// \brief Various utilities for Innobase.
///
/// Created 5/11/1994 Heikki Tuuri

#include "ut_ut.hpp"

#ifdef IB_DO_NOT_INLINE
  #include "ut_ut.inl"
#endif

#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifndef IB_HOTBACKUP
  #include "api_ucode.hpp"
  #include "trx_trx.hpp"
#endif // IB_HOTBACKUP

static ibool ut_always_false = FALSE;

#ifdef __WIN__

/**
NOTE: The Windows epoch starts from 1601/01/01 whereas the Unix
epoch starts from 1970/1/1. For selection of constant see:
http://support.microsoft.com/kb/167296/ */
#define WIN_TO_UNIX_DELTA_USEC ((ib_int64_t)11644473600000000ULL)

IB_INTERN static int ut_gettimeofday(struct timeval *tv, void *tz)
{
	FILETIME ft;
	ib_int64_t tm;
	if (!tv) {
		errno = EINVAL;
		return (-1);
	}
	GetSystemTimeAsFileTime(&ft);
	tm = (ib_int64_t)ft.dwHighDateTime << 32;
	tm |= ft.dwLowDateTime;
	ut_a(tm >= 0); // If tm wraps over to negative, the quotient / 10 does not work
	tm /= 10; // Convert from 100 nsec periods to usec
	// If we don't convert to the Unix epoch the value for struct timeval::tv_sec will overflow
	tm -= WIN_TO_UNIX_DELTA_USEC;
	tv->tv_sec = (long)(tm / 1000000L);
	tv->tv_usec = (long)(tm % 1000000L);
	return 0;
}
#else
	#include <sys/time.h>
	// An alias for gettimeofday(2).  On Microsoft Windows, we have to reimplement this function.
	#define ut_gettimeofday gettimeofday
#endif

IB_INTERN ulint ut_get_high32(ulint a) 
{
	ib_int64_t i = (ib_int64_t)a;
	i = i >> 32;
	return (ulint)i;
}

IB_INTERN ib_time_t ut_time(void)
{
	return time(NULL);
}

#ifndef IB_HOTBACKUP

/// \brief Returns system time.
/// \details Upon successful completion, the value 0 is returned; otherwise the value -1 is returned and the global variable errno is set to indicate the error.
/// \param sec out: seconds since the Epoch
/// \param ms out: microseconds since the Epoch+*sec
/// \return 0 on success, -1 otherwise
IB_INTERN int ut_usectime(ulint *sec, ulint *ms)
{
	struct timeval tv;
	int ret;
	for (int i = 0; i < 10; i++) {
		ret = ut_gettimeofday(&tv, NULL);
		if (ret == -1) {
			int errno_gettimeofday = errno;
			ut_print_timestamp(state->stream);
			ib_log(state, "  InnoDB: gettimeofday(): %s\n", strerror(errno_gettimeofday));
			os_thread_sleep(100000); // 0.1 sec
			errno = errno_gettimeofday;
		} else {
			break;
		}
	}

	if (ret != -1) {
		*sec = (ulint)tv.tv_sec;
		*ms = (ulint)tv.tv_usec;
	}

	return ret;
}

IB_INTERN ib_uint64_t ut_time_us(ib_uint64_t *tloc) 
{
	struct timeval tv;
	ut_gettimeofday(&tv, NULL);
	ib_uint64_t us = (ib_uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
	if (tloc != NULL) {
		*tloc = us;
	}
	return us;
}

IB_INTERN ulint ut_time_ms(void)
{
	struct timeval tv;
	ut_gettimeofday(&tv, NULL);
	return (ulint)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#endif // !IB_HOTBACKUP

IB_INTERN double ut_difftime(ib_time_t time2, ib_time_t time1)
{
	return difftime(time2, time1);
}

IB_INTERN void ut_print_timestamp(ib_stream_t state->stream)
{
#ifdef __WIN__
	SYSTEMTIME cal_tm;
	GetLocalTime(&cal_tm);
	ib_log(state, "%02d%02d%02d %2d:%02d:%02d", (int)cal_tm.wYear % 100, (int)cal_tm.wMonth, (int)cal_tm.wDay, (int)cal_tm.wHour, (int)cal_tm.wMinute, (int)cal_tm.wSecond);
#else
	struct tm cal_tm;
	struct tm *cal_tm_ptr;
	time_t tm;
	time(&tm);
#ifdef IB_HAVE_LOCALTIME_R
	localtime_r(&tm, &cal_tm);
	cal_tm_ptr = &cal_tm;
#else
	cal_tm_ptr = localtime(&tm);
#endif
	ib_log(state, "%02d%02d%02d %2d:%02d:%02d", cal_tm_ptr->tm_year % 100, cal_tm_ptr->tm_mon + 1, cal_tm_ptr->tm_mday, cal_tm_ptr->tm_hour, cal_tm_ptr->tm_min, cal_tm_ptr->tm_sec);
#endif
}


IB_INTERN void ut_sprintf_timestamp(char *buf)
{
#ifdef __WIN__
	SYSTEMTIME cal_tm;
	GetLocalTime(&cal_tm);
	sprintf(buf, "%02d%02d%02d %2d:%02d:%02d", (int)cal_tm.wYear % 100, (int)cal_tm.wMonth, (int)cal_tm.wDay, (int)cal_tm.wHour, (int)cal_tm.wMinute, (int)cal_tm.wSecond);
#else
	struct tm cal_tm;
	struct tm *cal_tm_ptr;
	time_t tm;
	time(&tm);
#ifdef IB_HAVE_LOCALTIME_R
	localtime_r(&tm, &cal_tm);
	cal_tm_ptr = &cal_tm;
#else
	cal_tm_ptr = localtime(&tm);
#endif
	sprintf(buf, "%02d%02d%02d %2d:%02d:%02d", cal_tm_ptr->tm_year % 100, cal_tm_ptr->tm_mon + 1, cal_tm_ptr->tm_mday, cal_tm_ptr->tm_hour, cal_tm_ptr->tm_min, cal_tm_ptr->tm_sec);
#endif
}

#ifdef IB_HOTBACKUP
IB_INTERN void ut_sprintf_timestamp_without_extra_chars(char *buf)
{
#ifdef __WIN__
	SYSTEMTIME cal_tm;
	GetLocalTime(&cal_tm);
	sprintf(buf, "%02d%02d%02d_%2d_%02d_%02d", (int)cal_tm.wYear % 100, (int)cal_tm.wMonth, (int)cal_tm.wDay, (int)cal_tm.wHour, (int)cal_tm.wMinute, (int)cal_tm.wSecond);
#else
	struct tm cal_tm;
	struct tm *cal_tm_ptr;
	time_t tm;
	time(&tm);
#ifdef IB_HAVE_LOCALTIME_R
	localtime_r(&tm, &cal_tm);
	cal_tm_ptr = &cal_tm;
#else
	cal_tm_ptr = localtime(&tm);
#endif
	sprintf(buf, "%02d%02d%02d_%2d_%02d_%02d", cal_tm_ptr->tm_year % 100, cal_tm_ptr->tm_mon + 1, cal_tm_ptr->tm_mday, cal_tm_ptr->tm_hour, cal_tm_ptr->tm_min, cal_tm_ptr->tm_sec);
#endif
}

/// \brief Returns current year, month, day.
/// \param year out: current year
/// \param month out: month
/// \param day out: day
IB_INTERN void ut_get_year_month_day(ulint *year, ulint *month, ulint *day)
{
#ifdef __WIN__
	SYSTEMTIME cal_tm;
	GetLocalTime(&cal_tm);
	*year = (ulint)cal_tm.wYear;
	*month = (ulint)cal_tm.wMonth;
	*day = (ulint)cal_tm.wDay;
#else
	struct tm cal_tm;
	struct tm *cal_tm_ptr;
	time_t tm;
	time(&tm);
#ifdef IB_HAVE_LOCALTIME_R
	localtime_r(&tm, &cal_tm);
	cal_tm_ptr = &cal_tm;
#else
	cal_tm_ptr = localtime(&tm);
#endif
	*year = (ulint)cal_tm_ptr->tm_year + 1900;
	*month = (ulint)cal_tm_ptr->tm_mon + 1;
	*day = (ulint)cal_tm_ptr->tm_mday;
#endif
}
#endif /* IB_HOTBACKUP */

#ifndef IB_HOTBACKUP

/// \brief Runs an idle loop on CPU. The argument gives the desired delay in microseconds on 100 MHz Pentium + Visual C++.
/// \param delay in: delay in microseconds on 100 MHz Pentium
/// \return dummy value
IB_INTERN ulint ut_delay(ulint delay)
{
	ulint i, j;
	j = 0;
	for (i = 0; i < delay * 50; i++) {
		j += i;
		UT_RELAX_CPU();
	}
	if (ut_always_false) {
		ut_always_false = (ibool)j;
	}
	return j;
}
#endif // !IB_HOTBACKUP

/// \brief Prints the contents of a memory buffer in hex and ascii.
/// \param state->stream in: stream where to print
/// \param buf in: memory buffer
/// \param len in: length of the buffer
IB_INTERN void ut_print_buf(ib_stream_t state->stream, const void *buf, ulint len)
{
	const byte *data;
	ulint i;
	IB_MEM_ASSERT_RW(buf, len);
	ib_log(state, " len %lu; hex ", len);
	for (data = (const byte *)buf, i = 0; i < len; i++) {
		ib_log(state, "%02lx", (ulong)*data++);
	}
	ib_log(state, "; asc ");
	data = (const byte *)buf;
	for (i = 0; i < len; i++) {
		int c = (int)*data++;
		ib_log(state, "%c", isprint(c) ? c : ' ');
	}
	ib_log(state, ";");
}

/// \brief Calculates fast the number rounded up to the nearest power of 2.
/// \param n in: number
/// \return first power of 2 which is >= n
IB_INTERN ulint ut_2_power_up(ulint n)
{
	ut_ad(n > 0);
	ulint res = 1;	
	while (res < n) {
		res = res * 2;
	}
	return res;
}

/// \brief Outputs a NUL-terminated file name, quoted with apostrophes.
/// \param state->stream in: output stream
/// \param name in: name to print
IB_INTERN void ut_print_filename(ib_stream_t state->stream, const char *name)
{
	ib_log(state, "'");
	for (;;) {
		int c = *name++;
		switch (c) {
			case 0:
				goto done;
			case '\'':
				ib_log(state, "%c", c);
				/* fall through */
			default:
				ib_log(state, "%c", c);
		}
	}
done:
	ib_log(state, "'");
}


#ifndef IB_HOTBACKUP
/// \brief Outputs a fixed-length string, quoted as an SQL identifier.
/// \details If the string contains a slash '/', the string will be
/// output as two identifiers separated by a period (.),
/// as in SQL database_name.identifier.
/// \param state->stream in: output stream
/// \param trx in: transaction
/// \param table_id in: TRUE=print a table name, FALSE=print other identifier
/// \param name in: name to print
IB_INTERN void ut_print_name(ib_stream_t state->stream, trx_t *trx, ibool table_id, const char *name)
{
	ut_print_namel(state->stream, name, strlen(name));
}

/// \brief Outputs a fixed-length string, quoted as an SQL identifier.
/// \details If the string contains a slash '/', the string will be
/// output as two identifiers separated by a period (.),
/// as in SQL database_name.identifier.
/// \param state->stream in: output stream
/// \param name in: name to print
/// \param namelen in: length of name
IB_INTERN void ut_print_namel(ib_stream_t state->stream, const char *name, ulint namelen)
{
	// 2 * IB_NAME_LEN for database and table name, and some slack for the extra prefix and quotes
	char buf[3 * IB_NAME_LEN];
	int len = ut_snprintf(buf, sizeof(buf), "%.*s", (int)namelen, name);
	ut_a(len >= (int)namelen);
	ib_log(state, "%.*s", len, buf);
}
#endif // !IB_HOTBACKUP

#ifdef __WIN__
#include <stdarg.h>

/// \brief A substitute for snprintf(3), formatted output conversion into a limited buffer.
/// \param str in: string
/// \param size in: size
/// \param fmt in: format
/// \param ... in: format values
/// \return number of characters that would have been printed if the size were unlimited, not including the terminating '\0'. */
IB_INTERN int ut_snprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list ap1;
	va_list ap2;
	va_start(ap1, fmt);
	va_start(ap2, fmt);
	int res = _vscprintf(fmt, ap1);
	ut_a(res != -1);
	if (size > 0) {
		_vsnprintf(str, size, fmt, ap2);
		if ((size_t)res >= size) {
			str[size - 1] = '\0';
		}
	}

	va_end(ap1);
	va_end(ap2);
	return res;
}
#endif // __WIN__
