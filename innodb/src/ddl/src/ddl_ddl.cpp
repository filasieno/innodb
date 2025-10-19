/// \file ddl_ddl.cpp
/// \brief Implementation of Innobase DDL operations
/// \details Originally created on 12 Oct 2008 by Oracle Corpn/Innobase Oy
/// \author Fabio N. Filasieno
/// \date 20/10/2025

#include "srv_srv.hpp"
#include "api_misc.hpp"
#include "trx_roll.hpp"
#include "dict_crea.hpp"
#include "dict_boot.hpp"
#include "dict_load.hpp"
#include "log_log.hpp"
#include "lock_lock.hpp"
#include "btr_pcur.hpp"
#include "ddl_ddl.hpp"
#include "pars_pars.hpp"
#include "ut_lst.hpp"

// -----------------------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------------------

static UT_LIST_BASE_NODE_T(ddl_drop_t) ddl_drop_list;
static ibool ddl_drop_list_inited = FALSE;

// -----------------------------------------------------------------------------------------
// constants
// -----------------------------------------------------------------------------------------

/* Magic table names for invoking various monitor threads */
static const char S_innodb_monitor[] = "innodb_monitor";
static const char S_innodb_lock_monitor[] = "innodb_lock_monitor";
static const char S_innodb_tablespace_monitor[] = "innodb_tablespace_monitor";
static const char S_innodb_table_monitor[] = "innodb_table_monitor";
static const char S_innodb_mem_validate[] = "innodb_mem_validate";

// -----------------------------------------------------------------------------------------
// Static helper routine declarations
// -----------------------------------------------------------------------------------------

static ulint       ddl_drop_table_in_background(const char* name);
static ibool       ddl_add_table_to_background_drop_list(const char* name);
static int         ddl_delete_constraint_low(const char* id, trx_t* trx);
static int         ddl_delete_constraint(const char* id, const char* database_name, mem_heap_t* heap, trx_t* trx);
static enum db_err ddl_drop_all_foreign_keys_in_db(const char* name, trx_t* trx);

// -----------------------------------------------------------------------------------------
// routine definitions
// -----------------------------------------------------------------------------------------

/// \brief Drops a table as a background operation.
/// \details On Unix in ALTER TABLE the table handler does not remove the table before all handles to it has been removed. Furthermore, the call to the drop table must be non-blocking. Therefore we do the drop table as a background operation, which is taken care of by the master thread in srv0srv.c.
/// \param [in] name table name
/// \return error code or DB_SUCCESS
static ulint ddl_drop_table_in_background(const char* name)
{
	trx_t* trx = trx_allocate_for_background();
	ibool started = trx_start(trx, ULINT_UNDEFINED);
	ut_a(started);
	/* If the original transaction was dropping a table referenced by
	foreign keys, we must set the following to be able to drop the
	table: */
	trx->check_foreigns = FALSE;
#if 0
	ib_log(state, "InnoDB: Info: Dropping table ");
	ut_print_name(state->stream, trx, TRUE, name);
	ib_log(state, " from background drop list\n");
#endif
	/* Try to drop the table in InnoDB */
	dict_lock_data_dictionary(trx);
	ulint error = ddl_drop_table(name, trx, FALSE);
	trx_commit(trx);
	dict_unlock_data_dictionary(trx);
	log_buffer_flush_to_disk();
	trx_free_for_background(trx);
	return error;
}

/// \brief The master thread in srv0srv.c calls this regularly to drop tables which we must drop in background after queries to them have ended.
/// \details Such lazy dropping of tables is needed in ALTER TABLE on Unix.
/// \return how many tables dropped + remaining tables in list
IB_INTERN ulint ddl_drop_tables_in_background(void)
{
	ulint n_tables_dropped = 0;
loop:
	mutex_enter(&kernel_mutex);
	if (!ddl_drop_list_inited) {
		UT_LIST_INIT(ddl_drop_list);
		ddl_drop_list_inited = TRUE;
	}
	ddl_drop_t* drop = UT_LIST_GET_FIRST(ddl_drop_list);
	ulint n_tables = UT_LIST_GET_LEN(ddl_drop_list);
	mutex_exit(&kernel_mutex);
	if (drop == NULL) {
		// All tables dropped
		return n_tables + n_tables_dropped;
	}
	mutex_enter(&(dict_sys->mutex));
	dict_table_t* table = dict_table_get_low(drop->table_name);
	mutex_exit(&(dict_sys->mutex));
	if (table == NULL) {
		// If for some reason the table has already been dropped through some other mechanism, do not try to drop it
		goto already_dropped;
	}

	if (DB_SUCCESS != ddl_drop_table_in_background(drop->table_name)) {
		// If the DROP fails for some table, we return, and let the main thread retry later
		return n_tables + n_tables_dropped;
	}

	n_tables_dropped++;

already_dropped:
	mutex_enter(&kernel_mutex);
	UT_LIST_REMOVE(ddl_drop_list, ddl_drop_list, drop);
	ut_print_timestamp(state->stream);
	ib_log(state, "  InnoDB: Dropped table ");
	ut_print_name(state->stream, NULL, TRUE, drop->table_name);
	ib_log(state, " in background drop queue.\n");
	mem_free(drop->table_name);
	mem_free(drop);
	mutex_exit(&kernel_mutex);
	goto loop;
}

/// \brief Get the background drop list length.
/// \details NOTE: the caller must own the kernel mutex!
/// \return how many tables in list
IB_INTERN ulint ddl_get_background_drop_list_len_low(void)
{
	ut_ad(mutex_own(&kernel_mutex));

	if (!ddl_drop_list_inited) {
		UT_LIST_INIT(ddl_drop_list);
		ddl_drop_list_inited = TRUE;
	}

	return UT_LIST_GET_LEN(ddl_drop_list);
}

/// \brief If a table is not yet in the drop list, adds the table to the list of tables which the master thread drops in background.
/// \details We need this on Unix because in ALTER TABLE may call drop table even if the table has running queries on it. Also, if there are running foreign key checks on the table, we drop the table lazily.
/// \param [in] name table name
/// \return TRUE if the table was not yet in the drop list, and was added there
static ibool ddl_add_table_to_background_drop_list(const char* name)
{
	mutex_enter(&kernel_mutex);
	if (!ddl_drop_list_inited) {
		UT_LIST_INIT(ddl_drop_list);
		ddl_drop_list_inited = TRUE;
	}
	// Look if the table already is in the drop list
	ddl_drop_t* drop = UT_LIST_GET_FIRST(ddl_drop_list);
	while (drop != NULL) {
		if (strcmp(drop->table_name, name) == 0) {
			// Already in the list
			mutex_exit(&kernel_mutex);
			return FALSE;
		}
		drop = UT_LIST_GET_NEXT(ddl_drop_list, drop);
	}

	drop = mem_alloc(sizeof(ddl_drop_t));
	drop->table_name = mem_strdup(name);
	UT_LIST_ADD_LAST(ddl_drop_list, ddl_drop_list, drop);

	ib_log(state, "InnoDB: Adding table ");
	ut_print_name(state->stream, trx, TRUE, drop->table_name);
	ib_log(state, " to background drop list\n");
	mutex_exit(&kernel_mutex);

	return TRUE;
}

/// \brief Drops a table but does not commit the transaction.
/// \details If the name of the dropped table ends in one of "innodb_monitor", "innodb_lock_monitor", "innodb_tablespace_monitor", "innodb_table_monitor", then this will also stop the printing of monitor output by the master thread.
/// \param [in] name table name
/// \param [in] trx transaction handle
/// \param [in] drop_db TRUE=dropping whole database
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_drop_table(const char* name, trx_t* trx, ibool drop_db)
{
	dict_foreign_t*	foreign;
	dict_table_t*	table;
	ulint		space_id;
	enum db_err	err;
	const char*	table_name;
	ulint		namelen;
	pars_info_t*    info			= NULL;

	ut_a(name != NULL);

	if (srv_created_new_raw) {
		ib_log(state,
		      "InnoDB: A new raw disk partition was initialized:\n"
		      "InnoDB: we do not allow database modifications"
		      " by the user.\n"
		      "InnoDB: Shut down the server and edit your config file "
		      "so that newraw is replaced with raw.\n");

		return DB_ERROR;
	}

	trx->op_info = "dropping table";

	/* The table name is prefixed with the database name and a '/'.
	Certain table names starting with 'innodb_' have their special
	meaning regardless of the database name.  Thus, we need to
	ignore the database name prefix in the comparisons. */
	table_name = strchr(name, '/');
	ut_a(table_name);
	table_name++;
	namelen = strlen(table_name) + 1;

	if (namelen == sizeof S_innodb_monitor
	    && !memcmp(table_name, S_innodb_monitor,
		       sizeof S_innodb_monitor)) {

		/* Table name equals "innodb_monitor":
		stop monitor prints */

		srv_print_innodb_monitor = FALSE;
		srv_print_innodb_lock_monitor = FALSE;
	} else if (namelen == sizeof S_innodb_lock_monitor
		   && !memcmp(table_name, S_innodb_lock_monitor,
			      sizeof S_innodb_lock_monitor)) {
		srv_print_innodb_monitor = FALSE;
		srv_print_innodb_lock_monitor = FALSE;
	} else if (namelen == sizeof S_innodb_tablespace_monitor
		   && !memcmp(table_name, S_innodb_tablespace_monitor,
			      sizeof S_innodb_tablespace_monitor)) {

		srv_print_innodb_tablespace_monitor = FALSE;
	} else if (namelen == sizeof S_innodb_table_monitor
		   && !memcmp(table_name, S_innodb_table_monitor,
			      sizeof S_innodb_table_monitor)) {

		srv_print_innodb_table_monitor = FALSE;
	}

	/* Serialize data dictionary operations with dictionary mutex:
	no deadlocks can occur then in these operations */

	if (trx->dict_operation_lock_mode != RW_X_LATCH) {
		return DB_SCHEMA_NOT_LOCKED;
	}

	ut_ad(mutex_own(&(dict_sys->mutex)));
#ifdef IB_SYNC_DEBUG
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
#endif /* IB_SYNC_DEBUG */

	table = dict_table_get_low(name);

	if (!table) {
		err = DB_TABLE_NOT_FOUND;
		ut_print_timestamp(state->stream);

		ib_log(state,
		      "  InnoDB: Error: table ");
		ut_print_name(state->stream, trx, TRUE, name);
		ib_log(state,
		      " does not exist in the InnoDB internal\n"
		      "InnoDB: data dictionary though the client is"
		      " trying to drop it.\n"
		      "InnoDB: You can look for further help on the\n"
		      "InnoDB: InnoDB website. Check the site for details\n");
		goto func_exit;
	}

	/* Check if the table is referenced by foreign key constraints from
	some other table (not the table itself) */

	foreign = UT_LIST_GET_FIRST(table->referenced_list);

	while (foreign && foreign->foreign_table == table) {
check_next_foreign:
		foreign = UT_LIST_GET_NEXT(referenced_list, foreign);
	}

	if (foreign && trx->check_foreigns
	    && !(drop_db && dict_tables_IB_HAVE_same_db(
			 name, foreign->foreign_table_name))) {
		/* We only allow dropping a referenced table if
		FOREIGN_KEY_CHECKS is set to 0 */

		err = DB_CANNOT_DROP_CONSTRAINT;

		mutex_enter(&dict_foreign_err_mutex);
		ut_print_timestamp(state->stream);

		ib_log(state, "  Cannot drop table ");
		ut_print_name(state->stream, trx, TRUE, name);
		ib_log(state, "\nbecause it is referenced by ");
		ut_print_name(state->stream,
			trx, TRUE, foreign->foreign_table_name);
		ib_log(state, "\n");
		mutex_exit(&dict_foreign_err_mutex);

		goto func_exit;
	}

	if (foreign && trx->check_foreigns) {
		goto check_next_foreign;
	}

	if (table->n_handles_opened > 0) {
		ibool	added;

		added = ddl_add_table_to_background_drop_list(table->name);

		if (added) {
			ut_print_timestamp(state->stream);
			ib_log(state,
				"  InnoDB: Warning: Client is"
				" trying to drop table (%lu) ",
				(ulint) table->id.low);
			ut_print_name(state->stream, trx, TRUE, table->name);
			ib_log(state, "\n"
			      "InnoDB: though there are still"
			      " open handles to it.\n"
			      "InnoDB: Adding the table to the"
			      " background drop queue.\n");

			/* We return DB_SUCCESS though the drop will
			happen lazily later */
			err = DB_SUCCESS;
		} else {
			/* The table is already in the background drop list */
			err = DB_TABLESPACE_DELETED;
		}

		goto func_exit;
	}

	/* TODO: could we replace the counter n_foreign_key_checks_running
	with lock checks on the table? Acquire here an exclusive lock on the
	table, and rewrite lock0lock.c and the lock wait in srv0srv.c so that
	they can cope with the table having been dropped here? Foreign key
	checks take an IS or IX lock on the table. */

	if (table->n_foreign_key_checks_running > 0) {

		const char*	table_name = table->name;
		ibool		added;

		added = ddl_add_table_to_background_drop_list(table_name);

		if (added) {
			ut_print_timestamp(state->stream);
			ib_log(state,
				"  InnoDB: You are trying to drop table ");
			ut_print_name(state->stream, trx, TRUE, table_name);
			ib_log(state, "\n"
			      "InnoDB: though there is a"
			      " foreign key check running on it.\n"
			      "InnoDB: Adding the table to"
			      " the background drop queue.\n");

			/* We return DB_SUCCESS though the drop will
			happen lazily later */

			err = DB_SUCCESS;
		} else {
			/* The table is already in the background drop list */
			err = DB_TABLESPACE_DELETED;
		}

		goto func_exit;
	}

	/* Remove any locks there are on the table or its records */

	lock_remove_all_on_table(table, TRUE);

	trx_set_dict_operation(trx, TRX_DICT_OP_TABLE);
	trx->table_id = table->id;

#if 0
	ib_log(state, "Dropping: %ld\n", (long) table->id.low);
#endif

	/* We use the private SQL parser of Innobase to generate the
	query graphs needed in deleting the dictionary data from system
	tables in Innobase. Deleting a row from SYS_INDEXES table also
	frees the file segments of the B-tree associated with the index. */

	info = pars_info_create();

	pars_info_add_str_literal(info, "table_name", name);

	static const char DROP_TABLE_PROC[] = R"(
	PROCEDURE DROP_TABLE_PROC () IS
		sys_foreign_id CHAR;
		table_id CHAR;
		index_id CHAR;
		foreign_id CHAR;
		found INT;
	BEGIN
		SELECT ID INTO table_id
		FROM SYS_TABLES
		WHERE NAME = :table_name
		LOCK IN SHARE MODE;

		IF (SQL % NOTFOUND) THEN
		RETURN;
		END IF;
		
		found := 1;
		SELECT ID INTO sys_foreign_id
		FROM SYS_TABLES
		WHERE NAME = 'SYS_FOREIGN'
		LOCK IN SHARE MODE;

		IF (SQL % NOTFOUND) THEN
			found := 0;
		END IF;
		
		IF (:table_name = 'SYS_FOREIGN') THEN
			found := 0;
		END IF;
		
		IF (:table_name = 'SYS_FOREIGN_COLS') THEN
		found := 0;
		END IF;

		WHILE found = 1 LOOP
			SELECT ID INTO foreign_id
			FROM SYS_FOREIGN
			WHERE FOR_NAME = :table_name
			AND TO_BINARY(FOR_NAME) = TO_BINARY(:table_name)
			LOCK IN SHARE MODE;

			IF (SQL % NOTFOUND) THEN
				found := 0;
			ELSE
				DELETE FROM SYS_FOREIGN_COLS
				WHERE ID = foreign_id;
				DELETE FROM SYS_FOREIGN
				WHERE ID = foreign_id;
			END IF;
		END LOOP;
		
		found := 1;

		WHILE found = 1 LOOP
			SELECT ID INTO index_id
			FROM SYS_INDEXES
			WHERE TABLE_ID = table_id
			LOCK IN SHARE MODE;
			IF (SQL % NOTFOUND) THEN
				found := 0;
			ELSE
				DELETE FROM SYS_FIELDS
				WHERE INDEX_ID = index_id;
				DELETE FROM SYS_INDEXES
				WHERE ID = index_id 
					AND TABLE_ID = table_id;
			END IF;
		END LOOP;

		DELETE FROM SYS_COLUMNS
		WHERE TABLE_ID = table_id;
		DELETE FROM SYS_TABLES
		WHERE ID = table_id;

	END;
	)";

	err = que_eval_sql(info, DROP_TABLE_PROC, FALSE, trx);
	if (err != DB_SUCCESS) {
		if (err != DB_OUT_OF_FILE_SPACE) {
			ib_log(state, "InnoDB: Error: unexpected err: %d", err);
			UT_ERROR;
		}
		err = DB_MUST_GET_MORE_FILE_SPACE;
		ib_handle_errors(&err, trx, NULL, NULL);
		UT_ERROR;
	} else {
		ibool is_path;
		const char* name_or_path;
		mem_heap_t* heap = mem_heap_create(200);

		// Clone the name, in case it has been allocated from table->heap, which will be freed by dict_table_remove_from_cache(table) below. 
		
		name = mem_heap_strdup(heap, name);
		space_id = table->space;

		if (table->dir_path_of_temp_table != NULL) {
			is_path = TRUE;
			name_or_path = mem_heap_strdup(heap, table->dir_path_of_temp_table);
		} else {
			is_path = FALSE;
			name_or_path = name;
		}

		dict_table_remove_from_cache(table);

		// FIXME: srv_force_recovery should be passed in as an arg
		if (dict_load_table(srv_force_recovery, name) != NULL) {
			ut_print_timestamp(state->stream);
			ib_log(state, "  InnoDB: Error: not able to remove table ");
			ut_print_name(state->stream, trx, TRUE, name);
			ib_log(state, " from the dictionary cache!\n");
			err = DB_ERROR;
		}

		// Do not drop possible .ibd tablespace if something went wrong: we do not want to delete valuable data of the user
		if (err == DB_SUCCESS && space_id > 0) {
			if (!fil_space_for_table_exists_in_mem(space_id, name_or_path, is_path, FALSE, TRUE)) {
				err = DB_SUCCESS;
				ib_log(state,
					"InnoDB: We removed now the InnoDB internal data dictionary entry\n"
					"InnoDB: of table ");
				ut_print_name(state->stream, trx, TRUE, name);
				ib_log(state, ".\n");
			} else if (!fil_delete_tablespace(space_id)) {
				ib_log(state,
					"InnoDB: We removed now the InnoDB internal data dictionary entry\n"
					"InnoDB: of table ");
				ut_print_name(state->stream, trx, TRUE, name);
				ib_log(state, ".\n");
				ut_print_timestamp(state->stream);
				ib_log(state, "  InnoDB: Error: not able to delete tablespace %lu of table ", (ulong) space_id);
				ut_print_name(state->stream, trx, TRUE, name);
				ib_log(state, "!\n");
				err = DB_ERROR;
			}
		}
		mem_heap_free(heap);
	}
func_exit:
	trx->op_info = "";
#ifndef IB_HOTBACKUP
	srv_wake_master_thread();
#endif // !IB_HOTBACKUP
	return (int) err;
}

/* Evaluates to true if str1 equals str2_onstack, used for comparing
the above strings. */
#define STR_EQ(str1, str1_len, str2_onstack) ((str1_len) == sizeof(str2_onstack) && memcmp(str1, str2_onstack, sizeof(str2_onstack)) == 0)

/// \brief Creates a table, if the name of the table ends in one of "innodb_monitor", "innodb_lock_monitor", "innodb_tablespace_monitor", "innodb_table_monitor", then this will also start the printing of monitor output by the master thread.
/// \details If the table name ends in "innodb_mem_validate", InnoDB will try to invoke mem_validate().
/// \param [in] table table definition
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_create_table(dict_table_t* table, trx_t* trx)
{
	tab_node_t*	node;
	mem_heap_t*	heap;
	que_thr_t*	thr;
	const char*	table_name;
	ulint		table_IB_NAME_LEN;
	ulint		err;
	ulint		i;

	ut_ad(trx->client_thread_id == os_thread_get_curr_id());
#ifdef IB_SYNC_DEBUG
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
#endif /* IB_SYNC_DEBUG */
	ut_ad(mutex_own(&(dict_sys->mutex)));
	ut_ad(trx->dict_operation_lock_mode == RW_X_LATCH);

	if (srv_created_new_raw) {
		ib_log(state,
		      "InnoDB: A new raw disk partition was initialized:\n"
		      "InnoDB: we do not allow database modifications"
		      " by the user.\n"
		      "InnoDB: Shut down the database and edit your config "
		      "file so that newraw is replaced with raw.\n");
err_exit:
		dict_mem_table_free(table);

		return DB_ERROR;

	/* The table name is prefixed with the database name and a '/'.
	Certain table names starting with 'innodb_' have their special
	meaning regardless of the database name.  Thus, we need to
	ignore the database name prefix in the comparisons. */
	} else if (strchr(table->name, '/') == NULL) {
		ib_log(state, "  InnoDB: Error: table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state,
			"not prefixed with a database name and '/'\n");
		goto err_exit;
	}

	trx->op_info = "creating table";

	/* Check that no reserved column names are used. */
	for (i = 0; i < dict_table_get_n_user_cols(table); i++) {
		if (dict_col_name_is_reserved(
			    dict_table_get_col_name(table, i))) {

			goto err_exit;
		}
	}

	table_name = strchr(table->name, '/');
	table_name++;
	table_IB_NAME_LEN = strlen(table_name) + 1;

	if (STR_EQ(table_name, table_IB_NAME_LEN, S_innodb_monitor)) {

		/* Table equals "innodb_monitor":
		start monitor prints */

		srv_print_innodb_monitor = TRUE;

		/* The lock timeout monitor thread also takes care
		of InnoDB monitor prints */

		os_event_set(srv_lock_timeout_thread_event);
	} else if (STR_EQ(table_name, table_IB_NAME_LEN,
			  S_innodb_lock_monitor)) {

		srv_print_innodb_monitor = TRUE;
		srv_print_innodb_lock_monitor = TRUE;
		os_event_set(srv_lock_timeout_thread_event);
	} else if (STR_EQ(table_name, table_IB_NAME_LEN,
			  S_innodb_tablespace_monitor)) {

		srv_print_innodb_tablespace_monitor = TRUE;
		os_event_set(srv_lock_timeout_thread_event);
	} else if (STR_EQ(table_name, table_IB_NAME_LEN,
			  S_innodb_table_monitor)) {

		srv_print_innodb_table_monitor = TRUE;
		os_event_set(srv_lock_timeout_thread_event);
	} else if (STR_EQ(table_name, table_IB_NAME_LEN,
			  S_innodb_mem_validate)) {
		/* We define here a debugging feature intended for
		developers */

		ib_log(state,
		      "Validating InnoDB memory:\n"
		      "to use this feature you must compile InnoDB with\n"
		      "IB_MEM_DEBUG defined in univ.i and"
		      " the server must be\n"
		      "quiet because allocation from a mem heap"
		      " is not protected\n"
		      "by any semaphore.\n");
#ifdef IB_MEM_DEBUG
		ut_a(mem_validate());
		ib_log(state, "Memory validated\n");
#else /* IB_MEM_DEBUG */
		ib_log(state,
			"Memory NOT validated (recompile with "
			"IB_MEM_DEBUG)\n");
#endif /* IB_MEM_DEBUG */
	}

	heap = mem_heap_create(512);

	trx_set_dict_operation(trx, TRX_DICT_OP_TABLE);

	node = tab_create_graph_create(table, heap, FALSE);

	thr = pars_complete_graph_for_exec(node, trx, heap);

	ut_a(thr == que_fork_start_command(que_node_get_parent(thr)));
	que_run_threads(thr);

	err = trx->error_state;

	if (IB_UNLIKELY(err != DB_SUCCESS)) {
		trx->error_state = DB_SUCCESS;
	}

	switch (err) {
	case DB_OUT_OF_FILE_SPACE:
		ut_print_timestamp(state->stream);
		ib_log(state, "  InnoDB: Warning: cannot create table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state, " because tablespace full\n");

		if (dict_table_get_low(table->name)) {

			ddl_drop_table(table->name, trx, FALSE);
		}
		break;

	case DB_DUPLICATE_KEY:
		ut_print_timestamp(state->stream);
		ib_log(state, "  InnoDB: Error: table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state,
		      " already exists in InnoDB internal\n"
		      "InnoDB: data dictionary.\n"
		      "InnoDB: You can look for further help on\n"
		      "InnoDB: the InnoDB website\n");

		/* We may also get err == DB_ERROR if the .ibd file for the
		table already exists */

		break;
	}

	que_graph_free((que_t*) que_node_get_parent(thr));

	trx->op_info = "";

	return (int) err;
}

/// \brief Does an index creation operation.
/// \param [in] index index definition
/// \param [in] trx transaction handle
/// \return error number or DB_SUCCESS
IB_INTERN ulint ddl_create_index(ib_dict_index_t* index, trx_t* trx)
{
	ulint		err;
	que_thr_t*	thr;		/* Query thread */
	ind_node_t*	node;		/* Index creation node */
	mem_heap_t*	heap;		/* Memory heap */

#ifdef IB_SYNC_DEBUG
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
#endif /* IB_SYNC_DEBUG */
	ut_ad(mutex_own(&(dict_sys->mutex)));

	/* This heap is destroyed when the query graph is freed. */
	heap = mem_heap_create(512);

	node = ind_create_graph_create(index, heap, FALSE);
	thr = pars_complete_graph_for_exec(node, trx, heap);

	ut_a(thr == que_fork_start_command(que_node_get_parent(thr)));

	que_run_threads(thr);

	err = trx->error_state;

	que_graph_free((que_t*) que_node_get_parent(thr));

	return err;
}

/// \brief Truncates a table
/// \param [in] table table handle
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN enum db_err ddl_truncate_table(dict_table_t* table, trx_t* trx)
{
	dict_foreign_t*	foreign;
	enum db_err	err;
	mem_heap_t*	heap;
	byte*		buf;
	dtuple_t*	tuple;
	dfield_t*	dfield;
	ib_dict_index_t*	sys_index;
	btr_pcur_t	pcur;
	mtr_t		mtr;
	dulint		new_id;
	ulint		recreate_space = 0;
	pars_info_t*	info = NULL;

	/* How do we prevent crashes caused by ongoing operations on
	the table? Old operations could try to access non-existent
	pages.

	1) SQL queries, INSERT, SELECT, ...: we must get an exclusive
	table lock on the table before we can do TRUNCATE TABLE. Ensure
	there are no running queries on the table. This guarantee has
	to be provided by the SQL layer.

	2) Purge and rollback: we assign a new table id for the
	table. Since purge and rollback look for the table based on
	the table id, they see the table as 'dropped' and discard
	their operations.

	3) Insert buffer: TRUNCATE TABLE is analogous to DROP TABLE,
	so we do not have to remove insert buffer records, as the
	insert buffer works at a low level. If a freed page is later
	reallocated, the allocator will remove the ibuf entries for
	it.

	When we truncate *.ibd files by recreating them (analogous to
	DISCARD TABLESPACE), we remove all entries for the table in the
	insert buffer tree.  This is not strictly necessary, because
	in 6) we will assign a new tablespace identifier, but we can
	free up some space in the system tablespace.

	4) Linear readahead and random readahead: we use the same
	method as in 3) to discard ongoing operations. (This is only
	relevant for TRUNCATE TABLE by DISCARD TABLESPACE.)

	5) FOREIGN KEY operations: if
	table->n_foreign_key_checks_running > 0, we do not allow the
	TRUNCATE. We also reserve the data dictionary latch.

	6) Crash recovery: To prevent the application of pre-truncation
	redo log records on the truncated tablespace, we will assign
	a new tablespace identifier to the truncated tablespace. */

	if (srv_created_new_raw) {
		ib_log(state,
		      "InnoDB: A new raw disk partition was initialized:\n"
		      "InnoDB: we do not allow database modifications"
		      " by the user.\n"
		      "InnoDB: Shut down server and edit config file so "
		      "that newraw is replaced with raw.\n");

		return DB_ERROR;
	}

	trx->op_info = "truncating table";

	/* Serialize data dictionary operations with dictionary mutex:
	no deadlocks can occur then in these operations */
	ut_a(trx->dict_operation_lock_mode != 0);

	/* Prevent foreign key checks etc. while we are truncating the
	table */
	ut_ad(mutex_own(&(dict_sys->mutex)));

#ifdef IB_SYNC_DEBUG
	ut_ad(rw_lock_own(&dict_operation_lock, RW_LOCK_EX));
#endif /* IB_SYNC_DEBUG */

	/* Check if the table is referenced by foreign key constraints from
	some other table (not the table itself) */

	foreign = UT_LIST_GET_FIRST(table->referenced_list);

	while (foreign && foreign->foreign_table == table) {
		foreign = UT_LIST_GET_NEXT(referenced_list, foreign);
	}

	if (foreign && trx->check_foreigns) {
		/* We only allow truncating a referenced table if
		FOREIGN_KEY_CHECKS is set to 0 */

		mutex_enter(&dict_foreign_err_mutex);
		ut_print_timestamp(state->stream);

		ib_log(state, "  Cannot truncate table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state, " by DROP+CREATE\n"
		      "InnoDB: because it is referenced by ");
		ut_print_name(state->stream,
			trx, TRUE, foreign->foreign_table_name);
		ib_log(state, "\n");
		mutex_exit(&dict_foreign_err_mutex);

		err = DB_ERROR;
		goto func_exit;
	}

	/* TODO: could we replace the counter n_foreign_key_checks_running
	with lock checks on the table? Acquire here an exclusive lock on the
	table, and rewrite lock0lock.c and the lock wait in srv0srv.c so that
	they can cope with the table having been truncated here? Foreign key
	checks take an IS or IX lock on the table. */

	if (table->n_foreign_key_checks_running > 0) {
		ut_print_timestamp(state->stream);
		ib_log(state, "  InnoDB: Cannot truncate table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state, " by DROP+CREATE\n"
		      "InnoDB: because there is a foreign key check"
		      " running on it.\n");
		err = DB_ERROR;

		goto func_exit;
	}

	/* Remove all locks except the table-level S and X locks. */
	lock_remove_all_on_table(table, FALSE);

	trx->table_id = table->id;

	if (table->space && !table->dir_path_of_temp_table) {
		/* Discard and create the single-table tablespace. */
		ulint	space	= table->space;
		ulint	flags	= fil_space_get_flags(space);

		if (flags != ULINT_UNDEFINED
		    && fil_discard_tablespace(space)) {

			ib_dict_index_t*	index;

			space = 0;

			if (fil_create_new_single_table_tablespace(
				    &space, table->name, FALSE, flags,
				    FIL_IBD_FILE_INITIAL_SIZE) != DB_SUCCESS) {
				ut_print_timestamp(state->stream);
				ib_log(state,
					"  InnoDB: TRUNCATE TABLE %s failed to"
					" create a new tablespace\n",
					table->name);
				table->ibd_file_missing = 1;
				err = DB_ERROR;
				goto func_exit;
			}

			recreate_space = space;

			/* Replace the space_id in the data dictionary cache.
			The persisent data dictionary (SYS_TABLES.SPACE
			and SYS_INDEXES.SPACE) are updated later in this
			function. */
			table->space = space;
			index = dict_table_get_first_index(table);
			do {
				index->space = space;
				index = dict_table_get_next_index(index);
			} while (index);

			mtr_start(&mtr);
			fsp_header_init(space,
					FIL_IBD_FILE_INITIAL_SIZE, &mtr);
			mtr_commit(&mtr);
		}
	}

	/* scan SYS_INDEXES for all indexes of the table */
	heap = mem_heap_create(800);

	tuple = dtuple_create(heap, 1);
	dfield = dtuple_get_nth_field(tuple, 0);

	buf = mem_heap_alloc(heap, 8);
	mach_write_to_8(buf, table->id);

	dfield_set_data(dfield, buf, 8);
	sys_index = dict_table_get_first_index(dict_sys->sys_indexes);
	dict_index_copy_types(tuple, sys_index, 1);

	mtr_start(&mtr);
	btr_pcur_open_on_user_rec(sys_index, tuple, PAGE_CUR_GE,
				  BTR_MODIFY_LEAF, &pcur, &mtr);
	for (;;) {
		rec_t*		rec;
		const byte*	field;
		ulint		len;
		ulint		root_page_no;

		if (!btr_pcur_is_on_user_rec(&pcur)) {
			/* The end of SYS_INDEXES has been reached. */
			break;
		}

		rec = btr_pcur_get_rec(&pcur);

		field = rec_get_nth_field_old(rec, 0, &len);
		ut_ad(len == 8);

		if (memcmp(buf, field, len) != 0) {
			/* End of indexes for the table (TABLE_ID mismatch). */
			break;
		}

		if (rec_get_deleted_flag(rec, FALSE)) {
			/* The index has been dropped. */
			goto next_rec;
		}

		/* This call may commit and restart mtr
		and reposition pcur. */
		root_page_no = dict_truncate_index_tree(table, recreate_space,
							&pcur, &mtr);

		rec = btr_pcur_get_rec(&pcur);

		if (root_page_no != FIL_NULL) {
			page_rec_write_index_page_no(
				rec, DICT_SYS_INDEXES_PAGE_NO_FIELD,
				root_page_no, &mtr);
			/* We will need to commit and restart the
			mini-transaction in order to avoid deadlocks.
			The dict_truncate_index_tree() call has allocated
			a page in this mini-transaction, and the rest of
			this loop could latch another index page. */
			mtr_commit(&mtr);
			mtr_start(&mtr);
			btr_pcur_restore_position(BTR_MODIFY_LEAF,
						  &pcur, &mtr);
		}

next_rec:
		btr_pcur_move_to_next_user_rec(&pcur, &mtr);
	}

	btr_pcur_close(&pcur);
	mtr_commit(&mtr);

	mem_heap_free(heap);

	new_id = dict_hdr_get_new_id(DICT_HDR_TABLE_ID);

	info = pars_info_create();

	pars_info_add_int4_literal(info, "space", (lint) table->space);
	pars_info_add_dulint_literal(info, "old_id", table->id);
	pars_info_add_dulint_literal(info, "new_id", new_id);

	static const char RENUMBER_TABLESPACE_PROC[] = R"(
	PROCEDURE RENUMBER_TABLESPACE_PROC () IS
	BEGIN
		UPDATE SYS_TABLES SET ID = :new_id, SPACE = :space
		  WHERE ID = :old_id;
		UPDATE SYS_COLUMNS SET TABLE_ID = :new_id
		  WHERE TABLE_ID = :old_id;
		UPDATE SYS_INDEXES SET TABLE_ID = :new_id, SPACE = :space
		  WHERE TABLE_ID = :old_id;
		COMMIT WORK;
	END;
	)";

	err = que_eval_sql(info, RENUMBER_TABLESPACE_PROC, FALSE, trx);

	if (err != DB_SUCCESS) {
		trx->error_state = DB_SUCCESS;
		trx_rollback(trx, FALSE, NULL);
		trx->error_state = DB_SUCCESS;
		ut_print_timestamp(state->stream);
		ib_log(state, "  InnoDB: Unable to assign a new identifier to table ");
		ut_print_name(state->stream, trx, TRUE, table->name);
		ib_log(state, "\nInnoDB: after truncating it. Background processes may corrupt the table!\n");
		err = DB_ERROR;
	} else {
		dict_table_change_id_in_cache(table, new_id);
	}

	dict_update_statistics(table);

func_exit:

	trx->op_info = "";
	srv_wake_master_thread();
	return err;
}

/// \brief Drops an index.
/// \param [in] table table instance
/// \param [in] index index to drop
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_drop_index(dict_table_t* table, ib_dict_index_t* index, trx_t* trx)
{
	ulint err = DB_SUCCESS;
	pars_info_t* info = pars_info_create();

	/* We use the private SQL parser of Innobase to generate the
	query graphs needed in deleting the dictionary data from system
	tables in Innobase. Deleting a row from SYS_INDEXES table also
	frees the file segments of the B-tree associated with the index. */

	static const char str1[] =
		"PROCEDURE DROP_INDEX_PROC () IS\n"
		"BEGIN\n"
		/* Rename the index, so that it will be dropped by
		row_merge_drop_temp_indexes() at crash recovery
		if the server crashes before this trx is committed. */
		"UPDATE SYS_INDEXES SET NAME=CONCAT('"
		TEMP_INDEX_PREFIX_STR "', NAME) WHERE ID = :indexid;\n"
		"COMMIT WORK;\n"
		/* Drop the field definitions of the index. */
		"DELETE FROM SYS_FIELDS WHERE INDEX_ID = :indexid;\n"
		/* Drop the index definition and the B-tree. */
		"DELETE FROM SYS_INDEXES WHERE ID = :indexid;\n"
		"END;\n";

	ut_ad(index && table && trx);

	pars_info_add_dulint_literal(info, "indexid", index->id);

	trx_start_if_not_started(trx);
	trx->op_info = "dropping index";

	ut_a(trx->dict_operation_lock_mode == RW_X_LATCH);

	err = que_eval_sql(info, str1, FALSE, trx);

	ut_a(err == DB_SUCCESS);

	/* Replace this index with another equivalent index for all
	foreign key constraints on this table where this index is used */

	dict_table_replace_index_in_foreign_list(table, index);
	dict_index_remove_from_cache(table, index);

	trx->op_info = "";
	return err;
}

/// \brief Delete a single constraint.
/// \param [in] id constraint id
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
static int ddl_delete_constraint_low(const char* id, trx_t* trx)
{
	pars_info_t*	info = pars_info_create();

	pars_info_add_str_literal(info, "id", id);

	static const char DELETE_CONSTRAINT[] = R"(
		PROCEDURE DELETE_CONSTRAINT () IS
		BEGIN
			DELETE FROM SYS_FOREIGN_COLS WHERE ID = :id;
			DELETE FROM SYS_FOREIGN WHERE ID = :id;
		END;
	)";

	return (int) que_eval_sql(info, DELETE_CONSTRAINT, FALSE, trx);
}

/// \brief Delete a single constraint.
/// \param [in] id constraint id
/// \param [in] database_name database name, with the trailing '/'
/// \param [in] heap memory heap
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
static int ddl_delete_constraint(const char* id, const char* database_name, mem_heap_t* heap, trx_t* trx)
{
	ulint err;

	// New format constraints have ids <databasename>/<constraintname>.
	err = ddl_delete_constraint_low(mem_heap_strcat(heap, database_name, id), trx);

	if (err == DB_SUCCESS && !strchr(id, '/')) {
		// Old format < 4.0.18 constraints have constraint ids
		// <number>_<number>. We only try deleting them if the
		// constraint name does not contain a '/' character, otherwise
		// deleting a new format constraint named 'foo/bar' from
		// database 'baz' would remove constraint 'bar' from database
		// 'foo', if it existed.

		err = ddl_delete_constraint_low(id, trx);
	}

	return (int) err;
}

/// \brief Renames a table.
/// \param [in] old_name old table name
/// \param [in] new_name new table name
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_rename_table(const char* old_name, const char* new_name, trx_t* trx)
{
	dict_table_t* table;
	ulint err = DB_ERROR;
	mem_heap_t*	heap = NULL;
	const char**  = NULL;
	ulint n_constraints_to_drop	= 0;
	pars_info_t* info = NULL;

	ut_a(old_name != NULL);
	ut_a(new_name != NULL);
	ut_ad(trx->client_thread_id == os_thread_get_curr_id());

	if (srv_created_new_raw || srv_force_recovery != IB_RECOVERY_DEFAULT) {
		ib_log(state,
		      "InnoDB: A new raw disk partition was initialized or\n"
		      "InnoDB: innodb_force_recovery is on: we do not allow\n"
		      "InnoDB: database modifications by the user. Shut down\n"
		      "InnoDB: the server and ensure that newraw is replaced\n"
		      "InnoDB: with raw, and innodb_force_... is removed.\n");

		goto func_exit;
	}

	trx->op_info = "renaming table";

	table = dict_table_get_low(old_name);

	if (!table) {
		err = DB_TABLE_NOT_FOUND;
		goto func_exit;
	} else if (table->ibd_file_missing) {
		err = DB_TABLE_NOT_FOUND;
		goto func_exit;
	}

	// We use the private SQL parser of Innobase to generate the query graphs needed in updating the dictionary data from system tables.

	info = pars_info_create();

	pars_info_add_str_literal(info, "new_table_name", new_name);
	pars_info_add_str_literal(info, "old_table_name", old_name);

	static const char RENAME_TABLE_PROC[] = R"(
	PROCEDURE RENAME_TABLE () IS
	BEGIN
		UPDATE SYS_TABLES SET NAME = :new_table_name
		  WHERE NAME = :old_table_name;
	END;
	)";

	err = que_eval_sql(info, RENAME_TABLE_PROC, FALSE, trx);

	if (err == DB_SUCCESS) {
		/* Rename all constraints. */

		info = pars_info_create();

		pars_info_add_str_literal(info, "new_table_name", new_name);
		pars_info_add_str_literal(info, "old_table_name", old_name);

		static const char RENAME_CONSTRAINT_IDS[] = R"(

		PROCEDURE RENAME_CONSTRAINT_IDS () IS
			gen_constr_prefix CHAR;
			new_db_name CHAR;
			foreign_id CHAR;
			new_foreign_id CHAR;
			old_db_IB_NAME_LEN INT;
			old_t_IB_NAME_LEN INT;
			new_db_IB_NAME_LEN INT;
			id_len INT;
			found INT;
		BEGIN
			found := 1;
			old_db_IB_NAME_LEN := INSTR(:old_table_name, '/')-1;
			new_db_IB_NAME_LEN := INSTR(:new_table_name, '/')-1;
			new_db_name := SUBSTR(:new_table_name, 0, new_db_IB_NAME_LEN);
			old_t_IB_NAME_LEN := LENGTH(:old_table_name);
			gen_constr_prefix := CONCAT(:old_table_name, '_ibfk_');
		WHILE found = 1 LOOP
			
		    SELECT ID INTO foreign_id
			  FROM SYS_FOREIGN
			  WHERE FOR_NAME = :old_table_name
			  AND TO_BINARY(FOR_NAME) = TO_BINARY(:old_table_name)
			  LOCK IN SHARE MODE;

			IF (SQL % NOTFOUND) THEN
				found := 0;
			ELSE
				UPDATE SYS_FOREIGN
				  SET FOR_NAME = :new_table_name
				  WHERE ID = foreign_id;

				id_len := LENGTH(foreign_id);

				IF (INSTR(foreign_id, '/') > 0) THEN
					IF (INSTR(foreign_id, gen_constr_prefix) > 0) THEN
						new_foreign_id := CONCAT(:new_table_name, SUBSTR(foreign_id, old_t_IB_NAME_LEN, id_len - old_t_IB_NAME_LEN));
					ELSE
						new_foreign_id := CONCAT(new_db_name, SUBSTR(foreign_id, old_db_IB_NAME_LEN, id_len - old_db_IB_NAME_LEN));
					END IF;

					UPDATE SYS_FOREIGN
					  SET ID = new_foreign_id
					  WHERE ID = foreign_id;
					UPDATE SYS_FOREIGN_COLS
					  SET ID = new_foreign_id
					  WHERE ID = foreign_id;
				END IF;
			END IF;
		END LOOP;
		
		UPDATE SYS_FOREIGN SET REF_NAME = :new_table_name
		  WHERE REF_NAME = :old_table_name
		  AND TO_BINARY(REF_NAME) = TO_BINARY(:old_table_name);

		END;

		)";

		err = que_eval_sql(info, RENAME_CONSTRAINT_IDS, FALSE, trx);

	} else if (n_constraints_to_drop > 0) {
		// Drop some constraints of tmp tables
		ulint i;
		char* db_name;
		ulint db_IB_NAME_LEN;
		db_IB_NAME_LEN = dict_get_db_IB_NAME_LEN(old_name) + 1;
		db_name = mem_heap_strdupl(heap, old_name, db_IB_NAME_LEN);

		for (i = 0; i < n_constraints_to_drop; i++) {
			err = ddl_delete_constraint(constraints_to_drop[i], db_name, heap, trx);
			if (err != DB_SUCCESS) {
				break;
			}
		}
	}

	if (err != DB_SUCCESS) {
		if (err == DB_DUPLICATE_KEY) {
			ut_print_timestamp(state->stream);
			ib_log(state,
			      "  InnoDB: Error; possible reasons:\n"
			      "InnoDB: 1) Table rename would cause"
			      " two FOREIGN KEY constraints\n"
			      "InnoDB: to have the same internal name"
			      " in case-insensitive comparison.\n"
			      " trying to rename table.\n"
			      "InnoDB: If table ");
			ut_print_name(state->stream, trx, TRUE, new_name);
			ib_log(state,
			      " is a temporary table, then it can be that\n"
			      "InnoDB: there are still queries running"
			      " on the table, and it will be\n"
			      "InnoDB: dropped automatically when"
			      " the queries end.\n");
		}
		trx->error_state = DB_SUCCESS;
		trx_rollback(trx, FALSE, NULL);
		trx->error_state = DB_SUCCESS;
	} else {
		/* The following call will also rename the .ibd data file if
		the table is stored in a single-table tablespace */

		if (!dict_table_rename_in_cache(table, new_name, TRUE)) {
			trx->error_state = DB_SUCCESS;
			trx_rollback(trx, FALSE, NULL);
			trx->error_state = DB_SUCCESS;
			goto func_exit;
		}

		/* We only want to switch off some of the type checking in
		an ALTER, not in a RENAME. */

		err = dict_load_foreigns(new_name, trx->check_foreigns);

		if (err != DB_SUCCESS) {
			ibool ret;
			ut_print_timestamp(state->stream);
			ib_log(state,"  InnoDB: Error: in RENAME TABLE table ");
			ut_print_name(state->stream, trx, TRUE, new_name);
			ib_log(state, "\nInnoDB: is referenced in foreign key constraints\nInnoDB: which are not compatible with the new table definition.\n");
			ret = dict_table_rename_in_cache(table,old_name, FALSE);
			ut_a(ret);
			trx->error_state = DB_SUCCESS;
			trx_rollback(trx, FALSE, NULL);
			trx->error_state = DB_SUCCESS;
		}
	}

func_exit:

	if (IB_LIKELY_NULL(heap)) {
		mem_heap_free(heap);
	}

	trx->op_info = "";
	return err;
}

/// \brief Renames an index.
/// \param [in] table_name table that owns the index
/// \param [in] old_name old table name
/// \param [in] new_name new table name
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN ulint ddl_rename_index(const char* table_name, const char* old_name, const char* new_name, trx_t* trx)
{
	dict_table_t* table;
	pars_info_t* info = NULL;
	ulint err = DB_ERROR;

	ut_a(old_name != NULL);
	ut_a(old_name != NULL);
	ut_a(table_name != NULL);
	ut_ad(trx->client_thread_id == os_thread_get_curr_id());

	if (srv_created_new_raw || srv_force_recovery != IB_RECOVERY_DEFAULT) {
		ib_log(state,
		      "InnoDB: A new raw disk partition was initialized or\n"
		      "InnoDB: innodb_force_recovery is on: we do not allow\n"
		      "InnoDB: database modifications by the user. Shut down\n"
		      "InnoDB: the server and ensure that newraw is replaced\n"
		      "InnoDB: with raw, and innodb_force_... is removed.\n");

		goto func_exit;
	}

	trx->op_info = "renaming index";
	table = dict_table_get_low(table_name);
	if (!table || table->ibd_file_missing) {
		err = DB_TABLE_NOT_FOUND;
		goto func_exit;
	}

	// We use the private SQL parser of Innobase to generate the query graphs needed in updating the dictionary data from system tables.

	info = pars_info_create();
	pars_info_add_str_literal(info, "table_name", table_name);
	pars_info_add_str_literal(info, "new_index_name", new_name);
	pars_info_add_str_literal(info, "old_index_name", old_name);

	static const char RENAME_INDEX_PROC[] = R"
	(
		PROCEDURE RENAME_TABLE () IS 
			table_id CHAR;
		BEGIN
			SELECT ID INTO table_id FROM SYS_TABLES WHERE NAME = :table_name LOCK IN SHARE MODE;
			IF (SQL % NOTFOUND) THEN 
			RETURN;
			END IF;
			UPDATE SYS_INDEXES SET NAME = :new_index_name 
			WHERE NAME = :old_index_name
				AND table_id = table_id;
		END;
	)";

	err = que_eval_sql(info, RENAME_INDEX_PROC, FALSE, trx);
	if (err == DB_SUCCESS) {
		ib_dict_index_t* index;
		index = dict_table_get_first_index(table);
		do {
			// FIXME: We are leaking memory here, well sort of, since the previous name allocation will not be freed till the index instance is destroyed.
			if (strcasecmp(index->name, old_name) == 0) {
				index->name = mem_heap_strdup(index->heap, new_name);
				break;
			}
			index = dict_table_get_next_index(index);
		} while (index);
	} else {
		trx->error_state = DB_SUCCESS;
		trx_rollback(trx, FALSE, NULL);
	}

func_exit:
	trx->op_info = "";
	return err;
}

/// \brief Drop all foreign keys in a database, see Bug#18942.
/// \param [in] name database name which ends to '/'
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
static enum db_err ddl_drop_all_foreign_keys_in_db(const char* name, trx_t* trx)
{
	ut_a(name[strlen(name) - 1] == '/');
	pars_info_t* pinfo = pars_info_create();
	pars_info_add_str_literal(pinfo, "dbname", name);

	// (removed) TABLE_NOT_IN_THIS_DB macro inlined into raw SQL below */

	static const char DROP_ALL_FOREIGN_KEYS_PROC[] = R"(
	PROCEDURE DROP_ALL_FOREIGN_KEYS_PROC () IS
		foreign_id CHAR;
		for_name CHAR;
		found INT;
	DECLARE CURSOR cur IS
		SELECT ID, FOR_NAME FROM SYS_FOREIGN
		WHERE FOR_NAME >= :dbname 
		LOCK IN SHARE MODE
		ORDER BY FOR_NAME;
	BEGIN
		found := 1;
		OPEN cur;
		WHILE found = 1 LOOP
			FETCH cur INTO foreign_id, for_name;
			IF (SQL % NOTFOUND) THEN
				found := 0;
			ELSIF (SUBSTR(for_name, 0, LENGTH(:dbname)) <> :dbname) THEN
				found := 0;
			ELSIF (1=1) THEN
				DELETE FROM SYS_FOREIGN_COLS WHERE ID = foreign_id;
				DELETE FROM SYS_FOREIGN WHERE ID = foreign_id;
			END IF;
		END LOOP;
		CLOSE cur;
	END;
	)";

	// do not reserve dict mutex, we are already holding it 
	return que_eval_sql(pinfo, DROP_ALL_FOREIGN_KEYS_PROC, FALSE, trx);
}

/// \brief Drops a database.
/// \param [in] name database name which ends in '/'
/// \param [in] trx transaction handle
/// \return error code or DB_SUCCESS
IB_INTERN enum db_err ddl_drop_database(const char* name, trx_t* trx)
{
	char* table_name;
	enum db_err	err = DB_SUCCESS;
	ulint namelen	= ut_strlen(name);

	ut_a(name[namelen - 1] == '/');
	ut_ad(trx->client_thread_id == os_thread_get_curr_id());
	trx->op_info = "dropping database";

loop:
	dict_lock_data_dictionary(trx);
	while ((table_name = dict_get_first_table_name_in_db(name))) {
		dict_table_t* table;
		ut_a(memcmp(table_name, name, namelen) == 0);
		table = dict_table_get_low(table_name);
		ut_a(table);
		// Wait until the user does not have any queries running on the table
		if (table->n_handles_opened > 0) {
			dict_unlock_data_dictionary(trx);
			ut_print_timestamp(state->stream);
			ib_log(state, "  InnoDB: Warning: The client is trying to drop database ");
			ut_print_name(state->stream, trx, TRUE, name);
			ib_log(state, "\nInnoDB: though there are still open handles to table ");
			ut_print_name(state->stream, trx, TRUE, table_name);
			ib_log(state, ".\n");
			os_thread_sleep(1000000);
			mem_free(table_name);
			goto loop;
		}

		err = ddl_drop_table(table_name, trx, TRUE);
		if (err != DB_SUCCESS) {
			ib_log(state, "InnoDB: DROP DATABASE ");
			ut_print_name(state->stream, trx, TRUE, name);
			ib_log(state, " failed with error %lu for table ", (ulint) err);
			ut_print_name(state->stream, trx, TRUE, table_name);
			ib_log(state, "\n");
			mem_free(table_name);
			break;
		}
		mem_free(table_name);
	}

	if (err == DB_SUCCESS) {
		// After dropping all tables try to drop all leftover foreign keys in case orphaned ones exist
		err = ddl_drop_all_foreign_keys_in_db(name, trx);
		if (err != DB_SUCCESS) {
			ib_log(state, "InnoDB: DROP DATABASE ");
			ut_print_name(state->stream, trx, TRUE, name);
			ib_log(state, " failed with error %d while dropping all foreign keys", err);
		}
	}

	dict_unlock_data_dictionary(trx);
	trx->op_info = "";
	return err;
}

/// \brief Drop all partially created indexes.
/// \param [in] recovery recovery level setting
IB_INTERN void ddl_drop_all_temp_indexes(ib_recovery_t recovery)
{
	trx_t* trx;
	btr_pcur_t pcur;
	mtr_t mtr;
	ibool started;

	// Load the table definitions that contain partially defined indexes, so that the data dictionary information can be checked when accessing the tablename.ibd files.
	trx = trx_allocate_for_background();
	started = trx_start(trx, ULINT_UNDEFINED);
	ut_a(started);
	trx->op_info = "dropping partially created indexes";
	dict_lock_data_dictionary(trx);

	mtr_start(&mtr);
	btr_pcur_open_at_index_side(TRUE, dict_table_get_first_index(dict_sys->sys_indexes), BTR_SEARCH_LEAF, &pcur, TRUE, &mtr);

	for (;;) {
		const rec_t* rec;
		ulint len;
		const byte* field;
		dict_table_t* table;
		dulint table_id;
		btr_pcur_move_to_next_user_rec(&pcur, &mtr);
		if (!btr_pcur_is_on_user_rec(&pcur)) {
			break;
		}
		rec = btr_pcur_get_rec(&pcur);
		field = rec_get_nth_field_old(rec, DICT_SYS_INDEXES_NAME_FIELD, &len);
		if (len == IB_SQL_NULL || len == 0
		    || mach_read_from_1(field) != (ulint) TEMP_INDEX_PREFIX) {
			continue;
		}
		// This is a temporary index. 
		field = rec_get_nth_field_old(rec, 0, &len); // TABLE_ID
		if (len != 8) {
			// Corrupted TABLE_ID
			continue;
		}

		table_id = mach_read_from_8(field);
		btr_pcur_store_position(&pcur, &mtr);
		btr_pcur_commit_specify_mtr(&pcur, &mtr);
		table = dict_load_table_on_id(recovery, table_id);
		if (table) {
			ib_dict_index_t* index;
			for (index = dict_table_get_first_index(table); index; index = dict_table_get_next_index(index)) {
				if (*index->name == TEMP_INDEX_PREFIX) {
					ddl_drop_index(table, index, trx);
					trx_commit(trx);
				}
			}
		}
		mtr_start(&mtr);
		btr_pcur_restore_position(BTR_SEARCH_LEAF, &pcur, &mtr);
	}
	btr_pcur_close(&pcur);
	mtr_commit(&mtr);
	dict_unlock_data_dictionary(trx);
	trx_commit(trx);
	trx_free_for_background(trx);
}

/// \brief Drop all temporary tables.
/// \param [in] recovery recovery level setting
IB_INTERN void ddl_drop_all_temp_tables(ib_recovery_t recovery)
{
	trx_t*		trx;
	btr_pcur_t	pcur;
	mtr_t		mtr;
	mem_heap_t*	heap;
	ibool		started;

	trx = trx_allocate_for_background();
	started = trx_start(trx, ULINT_UNDEFINED);
	trx->op_info = "dropping temporary tables";
	dict_lock_data_dictionary(trx);
	heap = mem_heap_create(200);
	mtr_start(&mtr);
	btr_pcur_open_at_index_side( TRUE, dict_table_get_first_index(dict_sys->sys_tables), BTR_SEARCH_LEAF, &pcur, TRUE, &mtr);

	for (;;) {
		const rec_t* rec;
		ulint len;
		const byte* field;
		dict_table_t* table;
		const char* table_name;
		btr_pcur_move_to_next_user_rec(&pcur, &mtr);
		if (!btr_pcur_is_on_user_rec(&pcur)) {
			break;
		}
		rec = btr_pcur_get_rec(&pcur);
		field = rec_get_nth_field_old(rec, 4, &len); //N_COLS
		if (len != 4 || !(mach_read_from_4(field) & 0x80000000UL)) {
			continue;
		}
		// Because this is not a ROW_FORMAT=REDUNDANT table, the is_temp flag is valid. Examine it.
		field = rec_get_nth_field_old(rec, 7, &len); // 7 = MIX_LEN
		if (len != 4
		    || !(mach_read_from_4(field) & DICT_TF2_TEMPORARY)) {
			continue;
		}
		// This is a temporary table.
		field = rec_get_nth_field_old(rec, 0, &len); // NAME
		if (len == IB_SQL_NULL || len == 0) {
			// Corrupted SYS_TABLES.NAME
			continue;
		}
		table_name = mem_heap_strdupl(heap, (const char*) field, len);
		btr_pcur_store_position(&pcur, &mtr);
		btr_pcur_commit_specify_mtr(&pcur, &mtr);
		table = dict_load_table(recovery, table_name);
		if (table) {
			ddl_drop_table(table_name, trx, FALSE);
			trx_commit(trx);
		}
		mtr_start(&mtr);
		btr_pcur_restore_position(BTR_SEARCH_LEAF, &pcur, &mtr);
	}

	btr_pcur_close(&pcur);
	mtr_commit(&mtr);
	mem_heap_free(heap);

	dict_unlock_data_dictionary(trx);

	trx_commit(trx);
	trx_free_for_background(trx);
}

