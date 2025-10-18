/// \file ddl_ddl.hpp
/// \brief Contains InnoDB DDL operations
/// \details Originally created in 12 Oct 2008 by Oracle Corpn/Innobase Oy
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#pragma once

#include "univ.i"
#include "trx_types.hpp"
#include "dict_types.hpp"

/// \brief Get the background drop list length.
/// \details NOTE: the caller must own the kernel mutex!
/// \return how many tables in list
IB_INTERN ulint ddl_get_background_drop_list_len_low(void);

/// \brief Creates a table, if the name of the table ends in one of "innodb_monitor", "innodb_lock_monitor", "innodb_tablespace_monitor", "innodb_table_monitor", then this will also start the printing of monitor output by the master thread.
/// \details If the table name ends in "innodb_mem_validate", InnoDB will try to invoke mem_validate().
/// \param [in] table table definition
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_create_table(dict_table_t* table, trx_t* trx);

/// \brief Does an index creation operation.
/// \details TODO: currently failure to create an index results in dropping the whole table! This is no problem currently as all indexes must be created at the same time as the table.
/// \param [in] index index definition
/// \param [in] trx transaction handle
/// \return error number or DB_SUCCESS
IB_INTERN ulint ddl_create_index(dict_index_t* index, trx_t* trx);

/// \brief Drops a table but does not commit the transaction.
/// \details If the name of the dropped table ends in one of "innodb_monitor", "innodb_lock_monitor", "innodb_tablespace_monitor", "innodb_table_monitor", then this will also stop the printing of monitor output by the master thread.
/// \param [in] name table name
/// \param [in] trx transaction handle
/// \param [in] drop_db TRUE=dropping whole database
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_drop_table(const char* name, trx_t* trx, ibool drop_db);

/// \brief Drops an index.
/// \param [in] table table instance
/// \param [in] index index to drop
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_drop_index(dict_table_t* table, dict_index_t* index, trx_t* trx);

/// \brief The master thread in srv0srv.c calls this regularly to drop tables which we must drop in background after queries to them have ended.
/// \details Such lazy dropping of tables is needed in ALTER TABLE on Unix.
/// \return how many tables dropped + remaining tables in list
IB_INTERN ulint ddl_drop_tables_in_background(void);

/// \brief Truncates a table
/// \param [in] table table handle
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN enum db_err ddl_truncate_table(dict_table_t* table, trx_t* trx);
/// \brief Renames a table.
/// \param [in] old_name old table name
/// \param [in] new_name new table name
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_rename_table(const char* old_name, const char* new_name, trx_t* trx);

/// \brief Renames an index.
/// \param [in] table_name table that owns the index
/// \param [in] old_name old table name
/// \param [in] new_name new table name
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_rename_index(const char* table_name, const char* old_name, const char* new_name, trx_t* trx);

/// \brief Drops a database.
/// \param [in] name database name which ends in '/'
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN enum db_err ddl_drop_database(const char* name, trx_t* trx);
/// \brief Drop all partially created indexes.
/// \param [in] recovery recovery level setting
IB_INTERN void ddl_drop_all_temp_indexes(ib_recovery_t recovery);
/// \brief Drop all temporary tables.
/// \param [in] recovery recovery level setting
IB_INTERN void ddl_drop_all_temp_tables(ib_recovery_t recovery);
