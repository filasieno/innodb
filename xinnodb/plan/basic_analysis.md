# Integration plan

## InnoDB modules to process

- ✅ ut
- ✅ mach
- ✅ mem
- ✅ os
- ✅ sync
- ✅ fut
- ✅ data
- ✅ mtr
- ✅ page
- fil
- fsp
- log
- buf
- ibuf
- btr
- read
- rem
- dict
- lock
- trx
- row
- pars
- que
- srv
- usr
- ddl
- dyn
- eval
- ha
- hash
- thr
- api

Includes:

- include (headers are analyzed in the context of each module)

Notes:

- Each module will get a table (title, description, concurrency, new concurrency, migration plan) and a list of all global variables found in the module.
- Where multiple concurrency strategies exist in a module, multiple rows will be added describing each strategy.
- Each module section will also include ~20 lines describing the module's features and responsibilities.
- When studying a module, also study the relative headers in the `include/` folder (public API) and capture relevant APIs, constants, and concurrency implications.

### Global runtime state (`state->...`)

- **Purpose**: Centralize runtime globals (logger, streams, OS counters, memory counters) under a single `innodb_state` instance.
- **Definition**: See `include/sdk/state_types.hpp` (`innodb_state { log, stream, os, ut_total_allocated_memory, ut_list_mutex }`).
- **Observed uses**:
  - `state->stream`, `ib_logger` used across logging and diagnostics.
  - `state->os.{thread_count,event_count,mutex_count,fast_mutex_count}` used in `os_thread`, `os_sync`, `srv_start`.
  - Planned: `state->ut_total_allocated_memory`, `state->ut_list_mutex` for UT memory accounting.
- **Current issues**:
  - Some files contain invalid definitions like `IB_INTERN ulint state->os.thread_count = 0;` which must be removed in favor of fields initialized in `innodb_state`.
  - Several OS/UT globals are still module‐scope (`IB_INTERN`) and should be folded into `state` or made thread‑local/atomic where appropriate.
- **Migration plan**:
  - Introduce a single owning `innodb_state* g_state` (or pass `innodb_state&` through APIs) and eliminate bare `IB_INTERN` globals where feasible.
  - Replace direct global accesses with `state->...` fields (logger, stream, OS counters, UT memory counters).
  - Ensure init/teardown paths call per‑subsystem var‑init functions that write into `state` (e.g., `os_*_var_init`, `ut_mem_var_init`).
  - Make counters atomic or sharded if written from multiple threads; retain mutexes only where necessary.
  - Provide a small adapter so existing `ib_log(state, ...)` sites remain unchanged while the backend uses `state->log/state->stream`.

Concurrency types

- lockless: a data structure that can be made lock-free or used without locks via atomics/RCU/partitioning.
- mutex: related to shared data and must be protected by locks/latches.
- pure: pure transformations (serialization, parsing, formatting, math) with no shared-state concurrency.
- orchestration: higher-level coordinator algorithms using multiple lower-layer functions/components.

### Module: ut (Utility) — @innodb layer

Features and responsibilities

- Core byte/word helpers (e.g., `dulint` operations) and endian-safe byte utils (`ut_byte.hpp/.cpp`).
- Debug helpers, assertions, and test scaffolding (`ut_dbg.hpp/.cpp`, `ut_test_dbg.hpp`).
- Intrusive list primitives and adapters (`ut_list.hpp/.inl`, `ut_lst.hpp`, `ut_list.cpp`).
- Memory helpers and light allocation utilities (`ut_mem.hpp/.inl`, `ut_mem.cpp`).
- Red‑black tree helpers (`ut_rbt.hpp`, `ut_rbt.cpp`) and small containers (`ut_vec.hpp/.inl`).
- Random utilities (`ut_rnd.hpp/.inl`, `ut_rnd.cpp`) providing simple PRNG for backoffs/utilities.
- Utility core (`ut_ut.hpp/.inl`, `ut_ut.cpp`) including logging hook typedefs.
- Sorting helpers (`ut_sort.hpp`) and lightweight algorithms used by containers.
- Minimal surface; sits at the bottom of @innodb’s DAG with no higher-layer dependencies.
- Primitives reused by `mem/`, `sync/`, `buf/`, `btr/` without pulling heavier modules.
- Portable and unit-test friendly (see `src/ut/test/`).
- No server instrumentation here; pure utilities and small data structures only.
- Header‑inline for hot paths; cpp files provide global definitions and heavier helpers.
- Avoid OS calls except a fast mutex used where necessary (`ut_list_mutex`).
- Candidates for lockless refactors (atomics/RCU/partitioning) without changing APIs.
- Random utils currently use a module‑local counter; can be made thread‑local safely.
- Debug flags are globals for convenience; can be `std::atomic` to avoid races.
- Memory counters are global; shard or make atomic for scalability.
- Goal: keep UT small, deterministic, and non‑blocking.

| title                        | description                                         | concurrency   | new concurrency | migration plan                                                             |
|------------------------------|-----------------------------------------------------|---------------|-----------------|----------------------------------------------------------------------------|
| Byte/word helpers            | `dulint` ops, byte utilities                        | pure          | pure            | Keep; stable API                                                           |
| Debug helpers                | Assertions, test flags, panic controls              | pure          | pure            | Keep; make debug flags atomic if read concurrently                         |
| Intrusive list helpers       | Intrusive list primitives                           | mutex         | lockless        | Guard at call sites now; move to sharded lockless/MPSC where applicable    |
| Memory helpers               | Small alloc helpers, counters                       | orchestration | lockless        | Make counters atomic/sharded; keep orchestration role for policies         |
| Red‑black tree helpers       | RB‑tree utilities                                   | mutex         | lockless        | Replace with concurrent ART/lock-free hash for read-heavy workloads        |
| Vector/sort helpers          | Small containers and sort utilities                 | pure          | pure            | Keep; header‑inline for hot paths                                          |
| Random utilities             | Simple PRNG, global counter                         | mutex         | lockless        | Convert to thread_local/atomic; seed per coroutine/task if available       |
| Utility core                 | Generic helpers (`ut_ut`)                           | pure          | pure            | Keep; wire `ib_logger` to top‑level logging if needed                      |

Global variables in module `ut`

| name                      | type            | file                   | concurrency | notes                                                                      |
|---------------------------|-----------------|------------------------|-------------|----------------------------------------------------------------------------|
| ut_dulint_zero            | const dulint    | ut/src/ut_byte.cpp     | pure        | Zero constant                                                              |
| ut_dulint_max             | const dulint    | ut/src/ut_byte.cpp     | pure        | Max constant                                                               |
| ut_total_allocated_memory | ulint           | ut/src/ut_mem.cpp      | mutex       | Memory bytes counter; make atomic/sharded                                  |
| ut_list_mutex             | os_fast_mutex_t | ut/src/ut_mem.cpp      | mutex       | Protects list ops; candidate to remove with caller‑side guards             |
| ut_dbg_zero               | ulint           | ut/src/ut_dbg.cpp      | pure        | Debug/test value                                                           |
| ut_dbg_stop_threads       | ibool           | ut/src/ut_dbg.cpp      | pure        | Debug flag; make atomic if read concurrently                               |
| panic_shutdown            | ibool           | ut/src/ut_dbg.cpp      | pure        | Global panic flag; make atomic                                             |
| ut_dbg_null_ptr           | ulint*          | ut/src/ut_dbg.cpp      | pure        | Debug null pointer                                                         |
| ut_rnd_ulint_counter      | ulint           | ut/src/ut_rnd.cpp      | mutex       | PRNG state; convert to thread_local or atomic                              |
| ib_logger                 | ib_logger_t     | ut/include/ut_ut.hpp   | orchestration| Logger hook (extern); wire to top‑level logging                           |

Concurrency strategies present in `ut`

- lockless: preferred direction for PRNG and counters if safe (thread_local/atomic/sharded).
- mutex: required for shared data structures when used across threads (lists, RB‑trees) at call sites.
- pure: formatting/math/helpers/constants with no shared mutable state.
- orchestration: integration hooks (e.g., logging) coordinating lower-level helpers.

### Module: mach (Machine format) — @innodb layer

Features and responsibilities

- Big‑endian storage encoding/decoding for fixed‑width integers: 1, 2, 3, 4, 6, 7, 8 bytes (`mach_write_to_N`, `mach_read_from_N`).
- Variable‑length compressed integer codecs for `ulint` (1..5 bytes) and 64‑bit/dulint variants (`mach_write_compressed`, `mach_read_compressed`, `mach_u64_*`, `mach_dulint_*`).
- Parse helpers that consume from a `[ptr, end_ptr)` buffer and either advance the pointer or return NULL on truncation (`mach_parse_compressed`, `mach_dulint_parse_compressed`).
- Host↔storage integral conversions with byte‑order swap utilities and little‑endian helpers (`mach_read_int_type`, `mach_write_int_type`, `mach_*_little_endian`).
- Floating‑point serialization in little‑endian format for on‑disk representation (`mach_double_read/write`, `mach_float_read/write`).
- Page field helpers that read/write 1/2/4‑byte values using an MLOG width selector (`mach_read_ulint`, `mach_write_ulint`).
- Pure, allocation‑free, I/O‑free algorithms designed for hot paths; majority are `inline` in `mach_data.inl`.
- Defensive checks and invariants via `ut_ad` assertions; no side effects beyond target memory writes.
- Narrow, reusable surface used by `page/`, `log/`, `mtr/`, `buf/`, and higher layers for stable on‑disk format access.
- Public headers provide C/C++ friendly APIs; C++ wrappers add attributes (`[[nodiscard]]`, `const`) for safety and inlining.

Reviewed headers (include/)

- `reference/innodb/src/mach/include/mach_data.hpp`: public API declarations for codecs and fixed‑width helpers.
- `reference/innodb/src/mach/include/mach_data.inl`: inline implementations for hot‑path functions.
- `reference/innodb/include/sdk/mach_types.hpp`: placeholder for public types (currently minimal/empty).
- Reference originals for parity: `reference/innobase/include/mach0data.h`, `reference/innobase/include/mach0data.ic`.

| title                             | description                                                | concurrency | new concurrency | migration plan |
|-----------------------------------|------------------------------------------------------------|-------------|-----------------|----------------|
| Fixed‑width int codecs            | Read/write 1/2/3/4/6/7/8‑byte big‑endian integers          | pure        | pure            | Keep header‑inline; add exhaustive tests; mark `constexpr` where possible |
| Compressed `ulint` codec          | 1..5 byte variable‑length encoding for 32‑bit values       | pure        | pure            | Keep; add boundary tests (0, 0x7F, 0x80 … 0xF0 path)                      |
| Compressed 64‑bit/dulint codecs   | Compact encoding for 64‑bit and `dulint` values            | pure        | pure            | Keep; unify naming conventions; cross‑verify with legacy outputs          |
| Floating‑point serialization      | Little‑endian read/write for `double`/`float`              | pure        | pure            | Prefer `memcpy`‑style to avoid aliasing/UB; tests across endianness       |
| Host↔storage int conversions      | Endian swap and sign handling helpers                      | pure        | pure            | Keep; consider C++20 `std::endian` where available (feature‑gated)        |
| Page field helpers (MLOG‑width)   | Read/write 1/2/4 bytes selected by `mlog_id_t`             | pure        | pure            | Keep; validate selector coverage; assert invalid widths                    |
| Parse helpers with bounds checks  | Safe pointer‑advancing decoders with truncation detection  | pure        | pure            | Keep; consider error enum return in new APIs while preserving legacy      |

Global variables in module `mach`

- None found; module is fully stateless.

Concurrency strategies present in `mach`

- pure: all functions are deterministic transforms with no shared mutable state.

Notes for migration and quality

- Ensure duplicate declarations in headers are deduplicated; keep a single declaration per symbol.
- Align signatures between legacy C headers and C++ API; prefer fixed‑width types (`uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`).
- Add golden tests against the legacy `mach0data` to guarantee bit‑exact parity.
- Validate `mach_read_int_type`/`mach_write_int_type` parameter lists and behavior; cover both signed/unsigned cases.

### Module: mem (Memory management) — @innodb layer

Features and responsibilities

- Memory heap subsystem: stack-like heaps composed of blocks; fast bump-pointer allocations; free-top and empty operations.
- Two allocation domains: dynamic (C `malloc` backing) and buffer-pool backed; optional `MEM_HEAP_BTR_SEARCH` for B-tree search fast-paths.
- Public macros for call-site capture of file/line and to encourage fast paths: `IB_MEM_ALLOC/ZALLOC/FREE`, `IB_MEM_HEAP_CREATE/*`, etc.
- API surface: heap lifecycle (`mem_heap_create_func`, `mem_heap_free_func`, `mem_heap_empty`), allocation (`mem_heap_alloc/zalloc`, `mem_heap_get_top`, `mem_heap_free_top`), and single-buffer helpers (`mem_alloc_func`, `mem_free_func`).
- String/data helpers: `mem_strdup`, `mem_strdupl`, `mem_heap_strdup`, `mem_heap_strdupl`, `mem_heap_strcat`, `mem_heap_dup`.
- Debug facilities (guard regions, poisoning, validation, prints) gated by `IB_MEM_DEBUG`/`IB_DEBUG`; accounting via `mem_current_allocated_memory` and a heap hash protected by a mutex.
- Tight integration with page/block layout: uses `ut_list` for block lists and page size constants for buffer-backed blocks; aligns to `IB_MEM_ALIGNMENT`.
- Inline-heavy hot paths in `mem_mem.inl`; out-of-line implementations in `mem_mem.cpp` for block create/free, parsing, and printf helpers.
- No direct OS calls on hot path except `malloc`/buffer-pool entry points; designed for deterministic, low-overhead allocation.
- Heaps are typically short-lived and local to operations (parser, row build, page operations), enabling amortized constant-time frees.

Reviewed headers (include/)

- `reference/innodb/src/mem/include/mem_mem.hpp`: public API for heaps, single-buffer alloc, and helpers; macro layer.
- `reference/innodb/src/mem/include/mem_mem.inl`: inline implementations for hot-path allocation and heap operations.
- `reference/innodb/src/mem/include/mem_dbg.hpp` and `mem_dbg.inl`: debug accounting, validation, and guard-field logic.
- `reference/innodb/include/sdk/mem_types.hpp`: public struct layout for `mem_block_info_struct` and related types.
- Reference originals for parity: `reference/innobase/include/mem0mem.h`, `mem0mem.ic`.

| title                                 | description                                                        | concurrency | new concurrency | migration plan |
|---------------------------------------|--------------------------------------------------------------------|-------------|-----------------|----------------|
| Heap allocator (dynamic)              | Heap backed by `malloc`                                            | lockless    | lockless        | Encourage per-task/thread heaps; document no cross-thread use without guards |
| Heap allocator (buffer)               | Heap backed by buffer pool pages                                   | mutex       | mutex           | Keep; rely on buffer-pool latches; validate max alloc thresholds and invariants |
| B-tree search heap flag               | `MEM_HEAP_BTR_SEARCH` allows NULL returns to avoid stalls          | lockless    | lockless        | Preserve behavior; add explicit error handling at call sites                  |
| Single-buffer alloc/free              | `mem_alloc_func`/`mem_free_func` with file/line capture            | lockless    | lockless        | Keep; add RAII wrappers; prefer typed helpers for safety                      |
| Heap top operations                   | `get_top`, `free_top`, `empty`                                     | lockless    | lockless        | Keep; add assertions/tests for boundary and alignment                         |
| Debug accounting and validation       | Guard regions, poison/erase, heap hash with mutex                  | mutex       | mutex           | Keep behind `IB_MEM_DEBUG`; ensure mutex name/typo fixes; improve test cover  |

Global variables in module `mem`

| name                         | type       | file                                         | concurrency | notes |
|------------------------------|------------|----------------------------------------------|-------------|-------|
| mem_current_allocated_memory | ulint      | mem/include/mem_dbg.hpp                      | mutex       | Debug-only counter; protected by `mem_hash_mutex` |
| mem_hash_mutex               | mutex_t    | mem/include/mem_dbg.hpp (debug, non-hotbackup)| mutex       | Protects heap hash and debug globals |
| MEM_HEAP_DYNAMIC             | ulint      | mem/include/mem_mem.hpp                      | pure        | Consider `constexpr`/`inline constexpr` to avoid ODR globals |
| MEM_HEAP_BUFFER              | ulint      | mem/include/mem_mem.hpp                      | pure        | Same as above |
| MEM_HEAP_BTR_SEARCH          | ulint      | mem/include/mem_mem.hpp                      | pure        | Same as above |
| MEM_BLOCK_MAGIC_N            | ulint      | mem/include/mem_mem.hpp                      | pure        | Same as above |
| MEM_FREED_BLOCK_MAGIC_N      | ulint      | mem/include/mem_mem.hpp                      | pure        | Same as above |
| MEM_BLOCK_HEADER_SIZE        | ulint      | mem/include/mem_mem.hpp                      | pure        | Header-aligned size; candidate for `constexpr` |
| MEM_BLOCK_START_SIZE         | ulint      | mem/include/mem_mem.hpp                      | pure        | Candidate for `constexpr` |
| MEM_BLOCK_STANDARD_SIZE      | ulint      | mem/include/mem_mem.hpp                      | pure        | Depends on `IB_PAGE_SIZE`; feature-gate constexpr |
| MEM_MAX_ALLOC_IN_BUF         | ulint      | mem/include/mem_mem.hpp                      | pure        | Derived from `IB_PAGE_SIZE`; candidate for `constexpr` |

Concurrency strategies present in `mem`

- lockless: per-heap usage in a single thread/task; no internal locking; fast bump-pointer path.
- mutex: debug-only global accounting and heap-hash protections; buffer-backed heaps rely on buffer-pool latches.

Notes for migration and quality

- Convert `constinit ulint` module constants to `inline constexpr` where ABI allows; reduces globals and enables compile-time evaluation.
- Clarify thread-safety: heaps are not thread-safe unless externally synchronized; document ownership patterns in call sites.
- Provide RAII wrappers (e.g., `ScopedHeap`) for safer creation/disposal; add span-like views for heap-backed buffers.
- Expand tests: guard regions, poison checks, boundary cases (`MEM_MAX_ALLOC_IN_BUF`), and parity vs. legacy `mem0mem`.
- Fix minor header typos (e.g., `mutex_tmem_hash_mutex` → `mutex_t mem_hash_mutex`) and ensure consistent naming.

### Module: os (OS abstraction) — @innodb layer

Features and responsibilities

- File system primitives: open/close, create/delete, truncate, stat, directory create/list helpers.
- Posix-style pread/pwrite and vectored I/O wrappers; safe retries on EINTR/EAGAIN.
- Direct I/O handling (O_DIRECT) with alignment helpers for page/log sizes.
- fsync/fdatasync, range flushes, fallocate/punch hole; sync policy hooks.
- File path utilities: absolute/relative resolution, tmp file generation, permissions/modes.
- IO scheduling hints: fadvise/madvise style wrappers where available (read-ahead, no-reuse).
- Asynchronous I/O facade: submission, completion polling, batching; integrates with IO threads.
- Thread creation/join, naming, priorities, CPU affinity pinning where supported.
- High-resolution time, sleep/yield, monotonic clock, coarse clock; conversion helpers.
- Random seed and entropy source bridge for PRNG initialisation.
- Error handling: errno to unified status mapping; rich diagnostics and retry policies.
- Platform feature detection and capability flags (O_DIRECT support, io_uring, fallocate modes).
- Minimal locking; delegates higher-level latching to `sync/` and coordination to `srv/`.
- Hot-path operations avoid allocations; careful buffer alignment and size checks.

Reviewed headers (include/)

- `reference/innodb/src/os/include/os_file.hpp`: file open/close, read/write, fsync, direct I/O helpers.
- `reference/innodb/src/os/include/os_aio.hpp`: async I/O submission/completion façade.
- `reference/innodb/src/os/include/os_thread.hpp`: thread create/join, affinity, naming.
- `reference/innodb/src/os/include/os_time.hpp`: clocks, sleep, time conversions.
- `reference/innodb/include/sdk/os_types.hpp`: public types for file handles, flags, statuses.
- Reference originals for parity: `reference/innobase/include/os0file.h`, `os0proc.h`, `os0thread.h`.

| title                             | description                                                     | concurrency   | new concurrency | migration plan |
|-----------------------------------|-----------------------------------------------------------------|---------------|-----------------|----------------|
| File open/close & metadata        | Create/open, stat, permissions, truncate                        | mutex         | lockless        | Keep per-handle state; avoid globals; shard any caches if introduced |
| Synchronous I/O (pread/pwrite)    | Posix read/write wrappers with EINTR/EAGAIN handling            | lockless      | lockless        | Keep; validate short I/O handling; add exhaustive error-path tests |
| Direct I/O alignment              | O_DIRECT enable/disable, alignment helpers                      | pure          | pure            | Keep; centralize page/log alignment constants; assert invariants |
| Flush/sync controls               | fsync/fdatasync, range flush, fallocate                         | lockless      | lockless        | Keep; guard platform variants; expose policy hooks to `log/` and `fil/` |
| Path utilities                    | Join/split, tmp names, dir create/list                          | pure          | pure            | Keep; avoid globals; prefer `std::filesystem` where available |
| Async I/O façade                  | Submit, poll/await completions, batching                        | orchestration | lockless        | Migrate to io_uring (feature-gated); fallback to thread-pool on non-Linux |
| Thread lifecycle & affinity       | Create/join, set name, pin to CPUs                              | orchestration | orchestration   | Keep; isolate policy in `srv/`; wrap platform-specific calls |
| Time and sleep                    | Monotonic/real clocks, nanosleep, yield                         | pure          | pure            | Keep; provide steady-clock wrappers and conversion helpers |
| Error/status mapping              | errno → status codes, retry helpers                             | pure          | pure            | Keep; unify with project-wide status/expected type |

Global variables in module `os`

- Key module globals (to migrate into `state` or atomics as feasible):

| name                              | type         | file                        | notes |
|-----------------------------------|--------------|-----------------------------|-------|
| os_innodb_umask                   | ulint        | os/src/os_file.cpp          | file create mask; platform-specific defaults |
| os_do_not_call_flush_at_each_write| ibool        | os/src/os_file.cpp          | gated by IB_DO_FLUSH; write policy hint |
| os_file_seek_mutexes              | os_mutex_t[] | os/src/os_file.cpp          | protect seek+i/o on platforms without p{read,write} |
| os_aio_use_native_aio             | ibool        | os/src/os_file.cpp          | enable native AIO if available |
| os_aio_print_debug                | ibool        | os/src/os_file.cpp          | debug flag |
| os_n_file_reads/writes/fsyncs     | ulint        | os/src/os_file.cpp          | I/O counters; also *_old, bytes_since_printout, last_printout |
| os_file_n_pending_p{read,write}s  | ulint        | os/src/os_file.cpp          | pending counts |
| os_n_pending_{reads,writes}       | ulint        | os/src/os_file.cpp          | pending counts |
| os_has_said_disk_full             | ibool        | os/src/os_file.cpp          | ENOSPC diagnostic guard |
| os_use_large_pages                | ibool        | os/src/os_proc.cpp          | large pages toggle |
| os_large_page_size                | ulint        | os/src/os_proc.cpp          | large page size |

- Migration: prefer moving counters/flags under `state->os` and removing raw `IB_INTERN` globals; keep true file‑scope statics private.

Concurrency strategies present in `os`

- lockless: I/O calls on independent file descriptors; alignment checks; error mapping.
- mutex: transient protection for any internal maps/caches if introduced; avoid global locks.
- pure: path utilities, time conversions, constants, capability checks (cached atomics if needed).
- orchestration: async I/O submission/completion, thread lifecycle, policy hooks.

Notes for migration and quality

- Provide a thin shim to an async runtime (e.g., `libak`), enabling io_uring on Linux and clean fallbacks.
- Ensure strict handling of short reads/writes and partial completions; add load/stress tests.
- Centralize O_DIRECT and alignment policy; validate against `IB_PAGE_SIZE` and log block sizes.
- Feature-gate advanced APIs (io_uring, fallocate punch hole) and probe at startup with cached atomics.
- Unify error/status codes with the rest of @innodb; avoid leaking raw errno beyond `os/`.
- Add exhaustive tests for failure injection (ENOSPC, EIO, EINTR, EAGAIN) and recovery behavior.

### Module: sync (Synchronization primitives) — @innodb layer

Features and responsibilities

- Core mutex implementation with spin-wait, OS event handoff, and waiter tracking (`sync_sync.*`).
- Read–write latch implementation with atomic `lock_word` state machine and wait events (`sync_rw.*`).
- Primary wait array and helpers to reserve/wait/signal latch objects (`sync_arr.*`).
- Latching order bookkeeping for debug builds (`IB_SYNC_DEBUG`), including per-thread level arrays and validation.
- Integration with OS primitives (`os_event`, fast mutex) and runtime config (`state->srv.n_spin_wait_rounds`).
- Diagnostics: performance counters for spin rounds, OS waits, and exit counts; debug print utilities.
- Global lists for created mutexes and rw-locks; optional deadlock detection hooks.

Reviewed headers (include/)

- `reference/innodb/src/sync/include/sync_sync.hpp`: mutex API, globals, initialization and diagnostics.
- `reference/innodb/src/sync/include/sync_rw.hpp`: rw-lock API, debug globals and perf counters.
- `reference/innodb/src/sync/include/sync_arr.hpp`: primary wait array API.
- `reference/innodb/src/sync/include/sync_sync.inl`, `sync_rw.inl`: inline hot paths.

| title                         | description                                                     | concurrency   | new concurrency | migration plan |
|-------------------------------|-----------------------------------------------------------------|---------------|-----------------|----------------|
| Mutex (spin + event)          | TAS + spin, waiter bit, OS event wakeups                        | orchestration | lockless/event  | Keep; ensure atomics/fences correct; tune spin rounds via `state` |
| RW-lock (atomic lock_word)    | Atomic state machine, reader/writer events                      | orchestration | lockless/event  | Keep; verify memory ordering; keep recursive x-lock invariants    |
| Primary wait array            | Cell reservation, wait, signal                                  | orchestration | orchestration   | Keep; move pointer into `state`                                    |
| Debug latching order          | Per-thread level arrays, validation                             | debug         | debug           | Keep under `IB_SYNC_DEBUG`; isolate counts under `state`          |
| Perf counters                 | Spin rounds, waits, exits                                       | pure          | pure            | Prefer atomics; expose via status API                              |

Global variables in module `sync`

| name                         | type                        | file                      | notes |
|------------------------------|-----------------------------|---------------------------|-------|
| mutex_exit_count             | ib_int64_t                  | sync/src/sync_sync.cpp    | perf counter; declared extern in header |
| sync_primary_wait_array      | sync_array_t*               | sync/src/sync_sync.cpp    | primary wait array pointer |
| sync_initialized             | ibool                       | sync/src/sync_sync.cpp    | init flag |
| sync_thread_level_arrays     | sync_thread_t*              | sync/src/sync_sync.cpp    | IB_SYNC_DEBUG only |
| sync_order_checks_on         | ibool                       | sync/src/sync_sync.cpp    | IB_SYNC_DEBUG only |
| mutex_list                   | ut_list_base_node_t         | sync/src/sync_sync.cpp    | global list of DB mutexes |
| mutex_list_mutex             | mutex_t                     | sync/src/sync_sync.cpp    | protects `mutex_list` |
| rw_lock_list                 | rw_lock_list_t              | sync/src/sync_rw.cpp      | global list of rw-locks |
| rw_lock_list_mutex           | mutex_t                     | sync/src/sync_rw.cpp      | protects `rw_lock_list` |
| rw_lock_debug_mutex/event    | mutex_t / os_event_t / ibool| sync/src/sync_rw.cpp      | debug deadlock helpers |
| rw_*_spin/os/exit counters   | ib_int64_t                  | sync/src/sync_rw.cpp      | perf counters (s/x variants) |

Private file-scope statics (examples)

- `mutex_spin_round_count`, `mutex_spin_wait_count`, `mutex_os_wait_count` (sync_sync.cpp).
- `timed_mutexes` (IB_DEBUG) and `sync_thread_mutex` (IB_SYNC_DEBUG) (sync_sync.cpp).

Concurrency strategies present in `sync`

- orchestration: core mutex/rw-lock algorithms and wait array coordination.
- debug: latching order arrays and validation.
- pure: counters/printing; can be atomic and read-only from other modules.

Notes for migration and quality

- Move exported globals into `state` where practical: `sync_primary_wait_array`, `mutex_exit_count`, perf counters.
- Keep true module-private metrics as `static` and surface via accessors; prefer atomics for counters.
- Ensure all externs in headers have exactly one definition; avoid `IB_INTERN state->...` style invalid declarations.
- Audit fences and memory ordering in rw-lock paths for correctness on weak memory models.

### Module: fut (File-based utilities and lists) — @innodb layer

Features and responsibilities

- File-based list primitives (`fut_lst.*`) operating on on-disk structures embedded in pages; manipulates `flst_base_node_t` and `flst_node_t` via mini-transactions.
- Functions to add/remove/insert/cut/truncate file-based linked lists while updating page contents and `mlog` for redo logging.
- Helpers to safely traverse lists using `mtr` to avoid buffer deadlocks (commit secondary mtrs during long traversals).
- Pointer acquisition into frames via `fut_get_ptr` with proper latching based on `fil_addr_t` and `RW_*_LATCH` and zip size.
- Strict invariants around `prev/next/first/last/len` fields in base/node structures; null checks with `fil_addr_null` and `fil_addr_is_null`.
- Integrates with buffer pool (`buf_ptr_get_fsp_addr`, `page_align`), file space helpers (`fil_space_get_zip_size`), and page accessors.

Reviewed headers (include/)

- `reference/innodb/src/fut/include/fut_lst.hpp`: public API for file-based list operations and validation/print.
- `reference/innodb/src/fut/include/fut_fut.hpp`: inline pointer helper `fut_get_ptr` for resolving file addresses to frame pointers under latch and mtr.
- Inlines: `fut_lst.inl`, `fut_fut.inl` for hot-path operations.

| title                             | description                                                        | concurrency   | new concurrency | migration plan |
|-----------------------------------|--------------------------------------------------------------------|---------------|-----------------|----------------|
| File-based list operations        | Add/insert/remove/cut/truncate on-disk lists via mtr               | orchestration | lockless/event  | Keep; assert invariants; ensure redo logging via `mlog_write_*`    |
| List validation and printing      | Validate forward/backward traversal; print metadata                | pure          | pure            | Keep; gated by debug; avoid long holds by secondary mtrs            |
| Frame pointer resolution          | `fut_get_ptr` to resolve `fil_addr_t` into frame pointer           | orchestration | lockless        | Keep; centralize latch mode, zip size resolution                    |

Global variables in module `fut`

- None found; module exposes only functions; all state is in-page and via other modules.

Private file-scope statics (examples)

- `flst_add_to_empty(...)` helper in `fut_lst.cpp` (non-exported).

Concurrency strategies present in `fut`

- orchestration: operations orchestrate buffer latches/MTRs, redo logging, and page updates.
- pure: validation and logging/printing helpers.

Notes for migration and quality

- Maintain strict use of `mtr_memo_contains_page(..., MTR_MEMO_PAGE_X_FIX)` assertions on all touching pages.
- When traversing long lists, use a secondary `mtr` and commit per-step to avoid buffer pool deadlocks.
- Consolidate pointer resolution via `fut_get_ptr` to unify page latch and zip-size decisions.
- Validate `len` updates and base `first/last` adjustments on all edge cases (empty, single-element, boundaries).

### Module: data (Data fields and types) — @innodb layer

Features and responsibilities

- Data field (`dfield_t`) and tuple (`dtuple_t`) lifecycle: create, set, type-check, validate, print.
- Tuple comparison with collation-aware semantics using a client compare context (`dtuple_coll_cmp`).
- Big record handling: convert excessive in-row payload to externally stored columns and reconstruct
  (`dtuple_convert_big_rec`, `dtuple_convert_back_big_rec`).
- Data type (`dtype_t`) utilities: string/binary classification, precise-type composition with charset/collation
  (`dtype_form_prtype`), multibyte prefix sizing (`dtype_get_at_most_n_mbchars`), validation, and printing.
- Integration: `rem_*` record accessors, `page_*` layout, `btr_*` index metadata, `dict_*` schemas, `mtr`/`mlog` for durability,
  and `ucode` charset tables for multibyte logic.
- Debug scaffolding: `data_error` sentinel for uninitialized fields; `data_dummy` compiler aide in validation.

Reviewed headers (include/)

- `reference/innodb/src/data/include/data_data.hpp/.inl`: dfield/dtuple APIs, validation, print, big-rec conversion.
- `reference/innodb/src/data/include/data_type.hpp/.inl`: dtype APIs, charset/collation helpers, classification and validation.

| title                                 | description                                                       | concurrency   | new concurrency | migration plan |
|---------------------------------------|-------------------------------------------------------------------|---------------|-----------------|----------------|
| dfield/dtuple core                    | Create/validate/print tuples and fields                           | pure          | pure            | Keep; reinforce invariants and assertions                           |
| Tuple collation compare               | Compare tuples per collation and index order                      | pure          | pure            | Keep; pass compare ctx explicitly; fuzz edge cases                   |
| Big record conversion                 | Spill/restore overflow to external columns                        | orchestration | orchestration   | Keep; ensure redo/undo correctness; test partial conversions         |
| dtype classification/forming          | String/binary checks, compose precise type                        | pure          | pure            | Keep; encode flags clearly; unit test combinations                   |
| Multibyte prefix sizing               | Compute byte length for n chars given mb min/max                  | pure          | pure            | Keep; delegate to charset API; test boundary values                  |

Global variables in module `data`

| name                             | type   | file                         | notes |
|----------------------------------|--------|------------------------------|-------|
| data_error (debug)               | byte   | data/src/data_data.cpp       | Debug-only sentinel for uninitialized fields; declared extern in `data_data.inl` |
| data_dummy (debug, !valgrind)    | ulint  | data/src/data_data.cpp       | Debug-only helper used in validation paths |
| data_client_default_charset_coll | ulint  | data/src/data_type.cpp       | Default charset-collation for legacy (<4.1.2) artifacts; declared extern in `data_type.hpp` |

Private file-scope statics (examples)

- `dfield_check_typed_no_assert(const dfield_t* field)` (data_data.cpp) helper.

Concurrency strategies present in `data`

- pure: struct manipulation, comparisons, printing; no shared mutable state.
- orchestration: big record conversion drives page updates under `mtr` with redo logging.

Notes for migration and quality

- Keep debug globals behind `#ifdef IB_DEBUG`; ensure exactly one definition for each extern.
- Consider moving `data_client_default_charset_coll` into `state` if multi-engine instances are supported; otherwise document as process-global.
- Harden `dtuple_coll_cmp` for mixed collations and partial-field comparisons; fuzz with random inputs and boundary lengths.
- Ensure big-rec conversion updates `n_ext` consistently and preserves index ordering; cover convert/revert symmetry and error paths.

### Module: mtr (Mini-transactions) — @innodb layer

Features and responsibilities

- Mini-transaction (mtr) handle and buffer: memo stack for latches/objects and in-memory redo log buffer.
- Start/commit/savepoint/rollback-to-savepoint lifecycle, including releasing memo entries and enforcing order.
- Page read helpers under mtr (`mtr_read_ulint`, `mtr_read_dulint`) ensuring correct latch state and logging mode.
- Logging API for per-mtr redo: initial record writers, typed n-byte writes, strings, and index metadata logging; parsing routines for recovery side (`mlog_parse_*`).
- Strong invariants: memo contains entries for any page/object being modified; logging mode governs emitted redo.
- Integration: `sync_rw` (latches), `buf_*` (buffer frames), `page_*`, `dict_*` (index metadata), `dyn_array` for log/memo storage.

Reviewed headers (include/)

- `reference/innodb/src/mtr/include/mtr_mtr.hpp/.inl`: mtr struct, constants, lifecycle, memo ops, page read helpers.
- `reference/innodb/src/mtr/include/mtr_log.hpp/.inl`: redo logging APIs and parsing helpers.
- `reference/innodb/src/mtr/include/mtr_types.hpp`: mtr-related types and flags.

| title                         | description                                                           | concurrency   | new concurrency | migration plan |
|-------------------------------|-----------------------------------------------------------------------|---------------|-----------------|----------------|
| MTR lifecycle                 | start/commit/savepoint/rollback, memo management                      | orchestration | orchestration   | Keep; assert memo invariants; clarify savepoint usage and error paths |
| Page reads under mtr          | `mtr_read_ulint/dulint` ensure latch and logging mode correctness     | pure          | pure            | Keep; validate type sizes; add boundary tests for endianness/width    |
| Redo logging (mlog write)     | initial record, n-bytes, string, index logging                        | orchestration | orchestration   | Keep; ensure sizes match parser; document buffer sizing                |
| Redo parsing (recovery)       | parse initial/n-bytes/string/index log records                        | orchestration | orchestration   | Keep; harden validation; fuzz malformed inputs                         |

Global variables in module `mtr`

- None found; APIs are function-based; constants are `constinit` in headers.

Private file-scope statics

- None observed in `mtr_mtr.cpp` / `mtr_log.cpp` for globals; only function definitions.

Concurrency strategies present in `mtr`

- orchestration: coordinates latches, memo, and redo logging atomically at the mtr scope.
- pure: small read helpers and log buffer composition operations.

Notes for migration and quality

- Keep constants as `inline constexpr` where ABI permits; maintain legacy numeric values for on-disk compatibility.
- Ensure `mlog_write_*` sizes align with `mlog_parse_*` expectations; adjust `mlog_open` size guidance when formats change.
- Strengthen memo containment checks (`mtr_memo_contains_page`) in debug builds; add coverage for mis-ordered releases.
- Document logging modes (`MTR_LOG_ALL`, `MTR_LOG_NONE`, `MTR_LOG_SHORT_INSERTS`) and audit their usage across modules.

### Module: page (Page layout, cursors, compression) — @innodb layer

Features and responsibilities

- Core page operations (`page_page.*`): page create/parse, directory slot split/balance, record list move/copy/delete, validation and printing.
- Page cursor (`page_cur.*`): search/position, random open, insert (zip and non-zip variants), delete; parse of redo for insert/delete/copy.
- Compressed pages (`page_zip.*`): compress/decompress, write specific fields (trx_id/roll_ptr, node/blob pointers), header operations, validate, reorganize, copy between pages; redo parse/write helpers; stats and debug controls.
- Integration with `dict_index_t`, `mtr` redo logging, `buf_block_t` frames, `mlog_*` APIs, and `rec`/`rem` record access.
- Invariants: directory counts and slots consistent with record list; middle record computations; heap no ownership; for zip, directory slot masks and header fields must match; validation helpers enforce consistency.

Reviewed headers (include/)

- `reference/innodb/src/page/include/page_page.hpp/.inl`: page layout, directory, record list operations, validation/printing.
- `reference/innodb/src/page/include/page_cur.hpp/.inl`: cursor search/position, insert/delete, redo parse helpers.
- `reference/innodb/src/page/include/page_zip.hpp/.inl`: compressed page ops, validate, parse, write helpers; allocator hooks; stats.
- `reference/innodb/src/page/include/page_types.hpp`: public types and exported stats declarations.

| title                           | description                                                        | concurrency   | new concurrency | migration plan |
|---------------------------------|--------------------------------------------------------------------|---------------|-----------------|----------------|
| Page create/dir/record ops      | Create, split/balance dir slots, move/copy/delete record lists     | orchestration | orchestration   | Keep; assert dir invariants; comprehensive validation paths         |
| Cursor search/insert/delete     | Positioning, insert (zip/nonzip), delete, redo parse               | orchestration | orchestration   | Keep; ensure redo emits/parsers match; test edge cases              |
| Page compression                | Compress/decompress, header ops, pointer writes, reorganize        | orchestration | orchestration   | Keep; maintain format; fuzz validate/parser against malformed data  |
| Page printing/validation        | Record/dir/page printing and validation helpers                    | pure          | pure            | Keep; extensive assertions; debug-only heavy prints                 |

Global variables in module `page`

| name                         | type                              | file                       | notes |
|------------------------------|-----------------------------------|----------------------------|-------|
| page_zip_stat                | page_zip_stat_t[PAGE_ZIP_NUM…-1]  | page/src/page_zip.cpp      | Compression stats table; declared extern in `page_types.hpp` |
| page_zip_compress_dbg        | ibool                             | page/src/page_zip.cpp      | Debug flag (under PAGE_ZIP_COMPRESS_DBG) |
| page_zip_compress_log        | unsigned                          | page/src/page_zip.cpp      | Debug log sequence/generator (under PAGE_ZIP_COMPRESS_DBG) |
| page_zip_validate_header_only| ibool                             | page/src/page_zip.cpp      | Compare headers only during validation |
| page_zip_clear_rec_disable   | ibool                             | page/src/page_zip.cpp      | Debug: disable clearing rec during zip ops (IB_ZIP_DEBUG) |

Private file-scope statics (examples)

- `page_cur_short_succ` (under IB_SEARCH_PERF_STAT) in `page_cur.cpp`.
- LCG local statics in `page_cur.cpp` for random operations.
- `page_zip_compress_write_log(...)` helper in `page_zip.cpp`.

Concurrency strategies present in `page`

- orchestration: page modifications under `mtr`, coordinating redo and buffer frames; cursor operations manage latching.
- pure: validation and printing helpers; some parsing functions read-only.

Notes for migration and quality

- Keep exported stats declared in headers with single definition in source; consider moving runtime debug flags under `state` if needed across instances.
- Ensure all `page_*_parse_*` functions have matching `mlog_*` write paths; fuzz parse with truncated/malformed buffers.
- Strengthen zip validation paths; preserve header/data invariants; compare header-only mode behind a clearly documented flag.
- Expand tests for dir slot split/balance boundary cases, heap no extremes, and compressed/uncompressed parity.
