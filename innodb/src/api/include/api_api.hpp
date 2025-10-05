
// Copyright (c) 2025 Fabio N. Filasieno. All Rights Reserved.
// Copyright (c) 2008, 2025 Innobase Oy. All Rights Reserved.
// Copyright (c) 2008, 2009 Oracle. All Rights Reserved.
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

/// \file  api_api.hpp
/// \brief InnoDB API header file

#pragma once

// TODO: Review inclues
#include "db_err.hpp"
#include "haildb.hpp"
#include <stdarg.h>

/// \brief Execute arbitrary SQL using InnoDB's internal parser. 
/// \details The statement is executed in a new transaction. Table name parameters must be prefixed
/// with a '$' symbol and variables with ':'
/// \param sql to execute
/// \param n_args no. of args
/// \return	DB_SUCCESS or error code 
/// \internal
ib_err_t ib_exec_sql(const char *sql, ib_ulint_t n_args, ...);

/// \brief Execute arbitrary SQL using InnoDB's internal parser. 
// The statement is executed in a background transaction. It will lock the data
/// dictionary lock for the duration of the query.
/// \return	DB_SUCCESS or error code 
/// \param sql sql to execute
/// \param n_args no of args
ib_err_t ib_exec_ddl_sql(const char *sql, ib_ulint_t n_args, ...);

/// \brief Initialize the config system.
/// \return	DB_SUCCESS or error code
ib_err_t ib_cfg_init(void);

/// \brief Shutdown the config system.
/// \return	DB_SUCCESS or error code
ib_err_t ib_cfg_shutdown(void);

extern int srv_panic_status;

#define IB_CHECK_PANIC() do { if (srv_panic_status) return srv_panic_status; } while (0)
