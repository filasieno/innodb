// Copyright (c) 1996, 2009, Innobase Oy. All Rights Reserved.
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

/// \file dict_load.hpp
/// \brief Loads to the memory cache database object definitions from dictionary tables
/// \details Originally created by Heikki Tuuri on 4/24/1996
/// \author Fabio N. Filasieno
/// \date 22/10/2025

#pragma once

#include "defs.hpp"
#include "dict_types.hpp"
#include "ut_byte.hpp"
#include "mem_mem.hpp"
#include "srv_srv.hpp"

/// \brief In a crash recovery we already have all the tablespace objects created
/// \param [in] in_crash_recovery are we doing a crash recovery
/// \details This function compares the space id information in the InnoDB data dictionary to what we already read with fil_load_single_table_tablespaces(). In a normal startup, we create the tablespace objects for every table in InnoDB's data dictionary, if the corresponding .ibd file exists. We also scan the biggest space id, and store it to fil_system.
IB_INTERN void dict_check_tablespaces_and_store_max_id(ibool in_crash_recovery);

/// \brief Finds the first table name in the given database
/// \param [in] name database name which ends to '/'
/// \return own: table name, NULL if does not exist; the caller must free the memory in the string!
IB_INTERN char* dict_get_first_table_name_in_db(const char* name);

/// \brief Loads a table definition and also all its index definitions, and also the cluster definition if the table is a member in a cluster
/// \param [in] recovery recovery flag
/// \param [in] name table name in the databasename/tablename format
/// \return table, NULL if does not exist; if the table is stored in an .ibd file, but the file does not exist, then we set the ibd_file_missing flag TRUE in the table object we return
/// \details Also loads all foreign key constraints where the foreign key is in the table or where a foreign key references columns in this table.
IB_INTERN dict_table_t* dict_load_table(ib_recovery_t recovery, const char* name);

/// \brief Loads a table object based on the table id
/// \param [in] recovery recovery flag
/// \param [in] table_id table id
/// \return table; NULL if table does not exist
IB_INTERN dict_table_t* dict_load_table_on_id(ib_recovery_t recovery, dulint table_id);

/// \brief This function is called when the database is booted
/// \param [in] table system table
/// \details Loads system table index definitions except for the clustered index which is added to the dictionary cache at booting before calling this function.
IB_INTERN void dict_load_sys_table(dict_table_t* table);

/// \brief Loads foreign key constraints where the table is either the foreign key holder or where the table is referenced by a foreign key
/// \param [in] table_name table name
/// \param [in] check_charsets TRUE=check charsets compatibility
/// \return DB_SUCCESS or error code
/// \details Adds these constraints to the data dictionary. Note that we know that the dictionary cache already contains all constraints where the other relevant table is already in the dictionary cache.
IB_INTERN ulint dict_load_foreigns(const char* table_name, ibool check_charsets);

/// \brief Prints to the standard output information on all tables found in the data dictionary system table.
IB_INTERN void dict_print(void);

