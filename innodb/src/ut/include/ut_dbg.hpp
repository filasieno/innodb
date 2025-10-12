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

/// \file ut_dbg.h
/// \brief Debug utilities for Innobase
/// \author Fabio N. Filasieno

#pragma once

#include "defs.hpp"
#include <stdlib.h>
#include "os_thread.hpp"

/// \brief Test if an assertion fails.
/// \param expr assertion expression
/// \return true if the assertion fails, false otherwise
IB_INTERN bool ut_dbg_fail(bool expr)
{
    return IB_UNLIKELY(!expr);
}

#define UT_DBG_PRINT_FUNC	printf("%s\n", __func__)

/* you must #define UT_DBG_ENTER_FUNC_ENABLED to something before
using this macro */
#define UT_DBG_ENTER_FUNC				\
	do {						\
		if (UT_DBG_ENTER_FUNC_ENABLED) {	\
			UT_DBG_PRINT_FUNC;		\
		}					\
	} while (0)


/// \brief Report a failed assertion.
/// \param expr in: the failed assertion
/// \param file in: source file containing the assertion
/// \param line in: line number of the assertion
IB_INTERN void ut_dbg_assertion_failed(const char* expr, const char* file, ulint line);

/// \brief Abort execution if EXPR does not evaluate to nonzero.
/// \param EXPR assertion expression that should hold
#define ut_a(EXPR) do { \
	if (UT_DBG_FAIL(EXPR)) { \
		ut_dbg_assertion_failed(#EXPR, __FILE__, (ulint) __LINE__);	\
		UT_DBG_PANIC; \
	} \
	UT_DBG_STOP; \
} while (0)

/// \brief Abort execution.
#define UT_ERROR do {						\
	ut_dbg_assertion_failed(0, __FILE__, (ulint) __LINE__);	\
	UT_DBG_PANIC;						\
} while (0)

#ifdef IB_DEBUG
	/// \brief Debug assertion. Does nothing unless IB_DEBUG is defined.
	#define ut_ad(EXPR)	ut_a(EXPR)
	/// \brief Debug statement. Does nothing unless IB_DEBUG is defined.
	#define ut_d(EXPR)	do {EXPR;} while (0)
#else
	/// \brief Debug assertion. Does nothing unless IB_DEBUG is defined.
	#define ut_ad(EXPR)
	/// \brief Debug statement. Does nothing unless IB_DEBUG is defined.
	#define ut_d(EXPR)
#endif

/// \brief Silence warnings about an unused variable by doing a null assignment.
///	\param A the unused variable
#define UT_NOT_USED(A)	((void)A)
