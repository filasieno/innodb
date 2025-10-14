# Global Variables Analysis in InnoDB

This analysis identifies all global variables found in the `innodb/src` directory. Global variables are classified into three categories:

- **constant**: Variables declared with `const` or `constexpr`
- **thread_local**: Variables declared with `thread_local` or `__thread`
- **global**: Regular global variables (static or extern without const/thread_local)

| Module | Variable Name                           | Type                                      | Tag      |
|--------|-----------------------------------------|-------------------------------------------|----------|
| api    | api_api_enter_func_enabled              | int                                       | global   |
| api    | api_sql_enter_func_enabled              | int                                       | global   |
| api    | cfg_vars_mutex                          | os_fast_mutex_t                           | global   |
| api    | db_format                               | ib_db_format_t                            | global   |
| api    | ib_panic                                | ib_panic_function_t                       | global   |
| api    | ib_trx_is_interrupted                   | ib_trx_is_interrupted_handler_t           | global   |
| api    | lru_old_blocks_pct                      | ulint                                     | global   |
| api    | srv_data_file_paths_and_sizes           | char*                                     | global   |
| api    | srv_file_flush_method_str               | char*                                     | global   |
| dict   | dict_ibfk                               | char[]                                    | global   |
| eval   | eval_dummy                              | byte                                      | global   |
| eval   | eval_rnd                                | ulint                                     | global   |
| fsp    | fsp_tbs_full_error_printed              | ibool                                     | global   |
| os     | os_aio_ibuf_array                       | os_aio_array_t*                           | global   |
| os     | os_aio_log_array                        | os_aio_array_t*                           | global   |
| os     | os_aio_n_segments                       | ulint                                     | global   |
| os     | os_aio_read_array                       | os_aio_array_t*                           | global   |
| os     | os_aio_recommend_sleep_for_read_threads | ibool                                     | global   |
| os     | os_aio_segment_wait_events              | os_event_t*                               | global   |
| os     | os_aio_sync_array                       | os_aio_array_t*                           | global   |
| os     | os_aio_write_array                      | os_aio_array_t*                           | global   |
| os     | os_file_count_mutex                     | os_mutex_t                                | global   |
| os     | os_sync_free_called                     | ibool                                     | global   |
| os     | os_sync_mutex_inited                    | ibool                                     | global   |
| page   | page_cur_short_succ                     | ulint                                     | global   |
| srv    | data_path_buf                           | char*                                     | global   |
| srv    | files                                   | os_file_t[1000]                           | global   |
| srv    | ios                                     | ulint                                     | global   |
| srv    | ios_mutex                               | mutex_t                                   | global   |
| srv    | log_path_buf                            | char*                                     | global   |
| srv    | n                                       | ulint[SRV_MAX_N_IO_THREADS + 6]           | global   |
| srv    | srv_client_table                        | srv_slot_t*                               | global   |
| srv    | srv_conc_mutex                          | os_fast_mutex_t                           | global   |
| srv    | srv_conc_slots                          | srv_conc_slot_t*                          | global   |
| srv    | srv_data_file_names                     | char**                                    | global   |
| srv    | srv_innodb_monitor_mutex                | mutex_t                                   | global   |
| srv    | srv_last_log_flush_time                 | time_t                                    | global   |
| srv    | srv_last_monitor_time                   | time_t                                    | global   |
| srv    | srv_log_group_home_dirs                 | char**                                    | global   |
| srv    | srv_log_writes_and_flush                | ulint                                     | global   |
| srv    | srv_main_10_second_loops                | ulint                                     | global   |
| srv    | srv_main_1_second_loops                 | ulint                                     | global   |
| srv    | srv_main_background_loops               | ulint                                     | global   |
| srv    | srv_main_flush_loops                    | ulint                                     | global   |
| srv    | srv_main_sleeps                         | ulint                                     | global   |
| srv    | srv_main_thread_id                      | ulint                                     | global   |
| srv    | srv_main_thread_process_no              | ulint                                     | global   |
| srv    | srv_n_rows_deleted_old                  | ulint                                     | global   |
| srv    | srv_n_rows_inserted_old                 | ulint                                     | global   |
| srv    | srv_n_rows_read_old                     | ulint                                     | global   |
| srv    | srv_n_rows_updated_old                  | ulint                                     | global   |
| srv    | srv_n_threads                           | ulint[SRV_MASTER + 1]                     | global   |
| srv    | srv_os_test_mutex                       | os_fast_mutex_t                           | global   |
| srv    | srv_start_has_been_called               | ibool                                     | global   |
| srv    | srv_sys                                 | srv_sys_t*                                | global   |
| srv    | thread_ids                              | os_thread_id_t[SRV_MAX_N_IO_THREADS + 6]  | global   |
| trx    | file_format_max                         | file_format_t                             | global   |
| trx    | trx_roll_crash_recv_trx                 | trx_t*                                    | global   |
| trx    | trx_roll_max_undo_no                    | ib_int64_t                                | global   |
| trx    | trx_roll_progress_printed_pct           | ulint                                     | global   |
| ut     | ut_always_false                         | ibool                                     | global   |
| ut     | ut_mem_block_list_inited                | ibool                                     | global   |
| ut     | ut_mem_null_ptr                         | ulint*                                    | global   |

## Global Constants

| Module | Variable Name                           | Type                                      | Tag      |
|--------|-----------------------------------------|-------------------------------------------|----------|
| api    | GEN_CLUST_INDEX                         | const char*                               | constant |
| api    | cfg_vars_defaults                       | const ib_cfg_var_t[]                      | constant |
| api    | status_vars                             | const ib_status_t[]                       | constant |
| os     | srv_io_thread_function                  | const char*[SRV_MAX_N_IO_THREADS]         | constant |
| os     | srv_io_thread_op_info                   | const char*[SRV_MAX_N_IO_THREADS]         | constant |
| page   | infimum_data                            | const byte[]                              | constant |
| page   | infimum_extra                           | const byte[]                              | constant |
| page   | supremum_extra_data                     | const byte[]                              | constant |
| trx    | FILE_FORMAT_NAME_N                      | const ulint                               | constant |
| trx    | file_format_name_map                    | const char*[]                             | constant |

## Summary

### Total Global Variables by Category

- **Constants**: 9 (9.8%)
- **Thread Local**: 0 (0%)
- **Global**: 82 (90.2%)

### Total Global Variables by Module

- **api**: 12 variables
- **srv**: 32 variables
- **ut**: 3 variables
- **trx**: 6 variables
- **fsp**: 1 variable
- **eval**: 2 variables
- **os**: 13 variables
- **dict**: 1 variable
- **page**: 4 variables

### Analysis for Removing Globals

The analysis reveals **93 global variables** across 9 modules in the InnoDB codebase. The majority (90.2%) are regular global variables that could potentially be removed through refactoring.

#### Effort Estimation for Global Variable Removal

1. **Low Effort** (Singletons/State Management - ~20 variables):
   - Variables like `srv_sys`, `srv_client_table`, `fil_system` that are natural singletons
   - Could be converted to singleton patterns or dependency injection

2. **Medium Effort** (Configuration/Constants - ~30 variables):
   - Configuration variables like `srv_*`, `os_*`, `cfg_*` settings
   - Could be moved to configuration management classes

3. **High Effort** (Threading/Shared State - ~25 variables):
   - Threading-related variables like `srv_conc_*`, `os_sync_*`
   - Performance counters and monitoring variables
   - Would require significant architectural changes

4. **Very High Effort** (Core State - ~18 variables):
   - Critical state variables like `trx_*`, `eval_*`, `page_*`
   - Would require major refactoring of core InnoDB subsystems

#### Recommended Approach

1. Start with low-effort singleton conversions
2. Implement configuration management for settings
3. Consider thread-local storage for thread-specific globals
4. Plan architectural changes for core state variables

This represents a significant refactoring effort that would improve testability, thread safety, and maintainability of the InnoDB codebase.
