# Integration plan

## InnoDB modules to process

- ut
- mach
- mem
- os
- sync
- fut
- data
- rem
- page
- fsp
- fil
- log
- mtr
- buf
- ibuf
- btr
- read
- dict
- lock
- trx
- row
- pars
- que
- srv
- arch
- fts
- gis
- ddl
- clone
- lob
- ha
- handler
- api
- usr

Includes:

- include (headers are analyzed in the context of each module)

Notes:

- Each module will get a table (title, description, concurrency, new concurrency, migration plan) and a list of all global variables found in the module.
- Where multiple concurrency strategies exist in a module, multiple rows will be added describing each strategy.
- Each module section will also include ~20 lines describing the module's features and responsibilities.
- When studying a module, also study the relative headers in the `include/` folder (public API) and capture relevant APIs, constants, and concurrency implications.

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

- None found; prefer per-handle/per-thread state and capability queries.

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
