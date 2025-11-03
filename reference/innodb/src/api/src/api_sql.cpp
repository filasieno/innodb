
// Copyright (c) 2025 Fabio N. Filasieno. All Rights Reserved.
// Copyright (c) 2010, 2024 Stewart Smith. All Rights Reserved.
// Copyright (c) 2008, 2010 Oracle. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

/// @file api_sql.cpp
/// \brief HailDB API SQL implementation

#include "config.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef IB_HAVE_UNISTD_H
  #include <unistd.h>
#endif

#include "dict_dict.hpp"
#include "pars_pars.hpp"
#include "que_que.hpp"
#include "trx_roll.hpp"
#include "api_api.hpp"

static int api_sql_enter_func_enabled = 0;
#define UT_DBG_ENTER_FUNC_ENABLED api_sql_enter_func_enabled

/**
Function to parse ib_exec_sql() and ib_exec_ddl_sql() args.
@return	own: info struct */
static pars_info_t* ib_exec_vsql(
	int		n_args,		/*!< in: no. of args */
	va_list		ap)		/*!< in: arg list */
{
	pars_info_t* info = pars_info_create();
	for (int i = 0; i < n_args; ++i) {
		ib_col_type_t type = va_arg(ap, ib_col_type_t);
		switch (type) {
		case IB_CHAR:
		case IB_VARCHAR: {
			const char* n = va_arg(ap, const char *);
			const char* v = va_arg(ap, const char *);
			char prefix = *n;
			ut_a(prefix == ':' || prefix == '$');
			++n;
			if (prefix == '$') {
				pars_info_add_id(info, n, v);
			} else {
				pars_info_add_str_literal(info, n, v);
			}
			break;
		}
		case IB_INT: {
			byte* p;	// dest buffer
			ulint l = va_arg(ap, ib_ulint_t);
			ulint s = va_arg(ap, ib_ulint_t);
			ulint n = va_arg(ap, const char *);
			prtype = s ? 0 : DATA_UNSIGNED;
			p = mem_heap_alloc(info->heap, l);
			switch (l) {
			case 1: {
				byte v = va_arg(ap, int);
				mach_write_int_type(p, (byte*)&v, l, s);
				break;
			}
			case 2: {
				ib_uint16_t v = va_arg(ap, int);
				mach_write_int_type(p, (byte*)&v, l, s);
				break;
			}
			case 4: {
				ib_uint32_t v = va_arg(ap, ib_uint32_t);
				break;
			}
			case 8: {
				ib_uint64_t v = va_arg(ap, ib_uint64_t);
				mach_write_int_type(p, (byte*)&v, l, s);
				break;
			}
			default:
				UT_ERROR;
			}
			pars_info_add_literal(info, n, p, l, DATA_INT, prtype);
			break;
		}
		case IB_SYS: {
			const char* n = va_arg(ap, const char *);
			pars_user_func_cb_t f = va_arg(ap, pars_user_func_cb_t);
			void* a = va_arg(ap, void*);
			pars_info_add_function(info, n, f, a);
			break;
		}
		default:
			// FIXME: Do the other types too
			UT_ERROR;
		}
	}

	return info;
}

/**
Execute arbitrary SQL using InnoDB's internal parser. The statement
is executed in a new transaction. Table name parameters must be prefixed
with a '$' symbol and variables with ':'
@return	DB_SUCCESS or error code */
/*!< in: sql to execute */
/*!< in: no. of args */
ib_err_t ib_exec_sql(const char* sql, ib_ulint_t n_args, ...)
{
	UT_DBG_ENTER_FUNC;

	va_list ap;
	trx_t* trx;
	ib_err_t err;
	pars_info_t* info;

	va_start(ap, n_args);
	info = ib_exec_vsql(n_args, ap);
	va_end(ap);
	// We use the private SQL parser of Innobase to generate the query graphs needed to execute the SQL statement.
	trx = trx_allocate_for_client(NULL);
	err = trx_start(trx, ULINT_UNDEFINED);
	ut_a(err == DB_SUCCESS);
	trx->op_info = "exec client sql";
	dict_mutex_enter();
	// Note that we've already acquired the dictionary mutex.
	err = que_eval_sql(info, sql, FALSE, trx);
	ut_a(err == DB_SUCCESS);
	dict_mutex_exit();
	if (err != DB_SUCCESS) {
		trx_rollback(trx, FALSE, NULL);
	} else {
		trx_commit(trx);
	}
	trx->op_info = "";
	trx_free_for_client(trx);
	return err;
}

/**
Execute arbitrary SQL using InnoDB's internal parser. The statement
is executed in a background transaction. It will lock the data
dictionary lock for the duration of the query.
@return	DB_SUCCESS or error code */
/*!< in: sql to execute */
/*!< in: no. of args */
ib_err_t ib_exec_ddl_sql(const char*	sql, ib_ulint_t	n_args, ...)
{
	UT_DBG_ENTER_FUNC;

	va_list ap;
	trx_t* trx;
	ib_err_t err;
	pars_info_t* info;
	int started;
	va_start(ap, n_args);
	info = ib_exec_vsql(n_args, ap);
	va_end(ap);
	// We use the private SQL parser of Innobase to generate the query graphs needed to execute the SQL statement.
	trx = trx_allocate_for_background();
	started = trx_start(trx, ULINT_UNDEFINED);
	ut_a(started);
	trx->op_info = "exec client ddl sql";
	err = ib_schema_lock_exclusive((ib_trx_t) trx);
	ut_a(err == DB_SUCCESS);
	// Note that we've already acquired the dictionary mutex by setting reserve_dict_mutex to FALSE.
	err = que_eval_sql(info, sql, FALSE, trx);
	ut_a(err == DB_SUCCESS);
	ib_schema_unlock((ib_trx_t) trx);
	if (err != DB_SUCCESS) {
		trx_rollback(trx, FALSE, NULL);
	} else {
		trx_commit(trx);
	}
	trx->op_info = "";
	trx_free_for_background(trx);
	return err;
}

