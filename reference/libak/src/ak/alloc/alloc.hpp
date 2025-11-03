#pragma once

#include "ak/base/base.hpp"                // IWYU pragma: keep

#include "ak/alloc/alloc_api.hpp"          // IWYU pragma: keep
#include "ak/alloc/alloc_api_inl.hpp"      // IWYU pragma: keep
#include "ak/alloc/alloc_api_priv.hpp"     // IWYU pragma: keep
#include "ak/alloc/alloc_api_priv_inl.hpp" // IWYU pragma: keep

/// \defgroup alloc Allocator
/// \ingroup components
/// \brief Allocator
///
/// # Allocator Objectives, Strategy, Comparison, and Plan
/// ## Objectives
//// 
/// - Provide an awaitable allocation interface that suspends coroutines when memory is temporarily unavailable and resumes them as soon as memory becomes available.
/// - Operate robustly near sustained high memory pressure with deterministic progress (no allocation failures under designed constraints).
/// - Maintain predictable latency and bounded work on allocation and free paths.
/// - Control external fragmentation with minimal overhead.
/// - Remain a region-backed allocator with zero reliance on OS growth during steady state.
///
/// ## Operating Model and Assumptions
///
/// - Allocation domain: a pre-reserved memory region per process.
/// - Concurrency model: single-threaded per process (no threads), potentially many processes.
/// - Under pressure, allocation requests may wait; the allocator must suspend the caller coroutine and resume on availability.
/// - Defragmentation is opportunistic and bounded.
///
/// ## Current Design (Draft)
///
/// - Small allocations (≤ 2048 bytes): 64 freelist bins at 32-byte granularity with a 64-bit availability mask; O(1) lookup.
/// - Large allocations (> 2048 bytes): best-fit via AVL tree; duplicates grouped via FIFO ring.
/// - Fallback source: contiguous "wild" block split on demand.
/// - Free path: mark FREE and return block to freelist or tree; optional defrag pass.
///
/// ### Strategy to Achieve Objectives
///
/// - Awaitable API
///   - Provide `co_await AllocMem(alloc_table*, size)`; return immediately on success; otherwise suspend and enqueue a waiter.
///   - On `free` or `defrag`, attempt to satisfy pending waiters and resume them.
///
/// - Waiter Queues
///   - Small requests: per-size-class FIFO queues aligned to the 64 small bins; a waiter-mask indicates non-empty queues.
///   - Large requests: an ordered queue keyed by requested size, with FIFO ordering for equal sizes.
///
/// - Wake Policy on Free/Defrag
///   - After any free and bounded coalescing, serve exact-fit small waiters that can be satisfied immediately; otherwise, place the block in freelists.
///   - For large frees, serve the largest pending large waiter first via best-fit; split remainder into appropriate bins.
///   - Limit wake work per call (e.g., up to K resumes or X microseconds).
///
/// - Bounded Coalescing
///   - On free, attempt left/right coalescing up to a small bound per side to reduce fragmentation while capping latency.
///   - Invoke a short defrag slice only when waiter queues are non-empty.
///
/// - Pressure Modes and Watermarks
///   - Enter near-exhaustion mode when `free_mem_size / mem_size` drops below a configurable low watermark.
///   - In pressure mode, prefer splits that directly satisfy queued sizes and avoid creating unusable fragments.
///
/// - Large Request Reserve
///   - Maintain a minimal reserve for large requests by biasing coalescing and limiting oversplitting while large waiters exist.
///
/// - Cancellation and Safety
///   - Support waiter cancellation and removal from queues.
///   - Re-run the fast path before `await_resume` to guard against races.
///
/// - Instrumentation
///   - Track queue lengths per bin, time-to-satisfy distributions, resumes per free, and defrag work budgets.
///
/// ## Comparison With Popular Allocators (Scope-Limited)
///
/// - jemalloc
///   - Region binding: supported via extent hooks and custom arenas.
///   - Backpressure: does not provide await/suspend semantics; returns failure or blocks.
///   - Concurrency: per-thread arenas and caches (not applicable in single-thread per process).
///   - Fragmentation: strong controls and tuning.
///
/// - tcmalloc
///   - Region binding: no first-class extent hooks for fixed region backing.
///   - Backpressure: no await/suspend; returns failure or blocks.
///   - Concurrency: per-thread caches; central transfer.
///
/// - mimalloc
///   - Region binding: supported via managed/arena APIs.
///   - Backpressure: no await/suspend; returns failure or blocks.
///   - Concurrency: per-thread heaps; low-latency small-object paths.
///
/// - ptmalloc (glibc)
///   - Region binding: not first-class for fixed region use.
///   - Backpressure: no await/suspend; returns failure or blocks.
///   - Concurrency: arenas with locking; fewer tuning controls.
///
/// Note: The await/suspend model is not a goal of the compared allocators. The strategy here focuses on wait-queue scheduling and bounded wake work under intentional memory pressure.
///
/// ### Key Performance Indicators
///
/// - Time-to-satisfy for waiting allocations (p50/p90/p99).
/// - Allocation and free latency distributions under pressure and normal load.
/// - External fragmentation relative to live set.
/// - CPU utilization under sustained pressure.
/// - Peak resident set size during churn.
///
/// ### Benchmarking Methodology (Summary)
///
/// - Workloads: steady-state and bursty mixes across small (≤ 2048) and large (> 2048) sizes; power-law distributions; configurable survivorship of objects.
/// - Pressure levels: < 70% (normal), 70–90% (high), 90–98% (near-exhaustion).
/// - Metrics: throughput (ops/s), latency (ns/op and percentiles), time-to-satisfy for blocked requests, fragmentation %, CPU%, cache-miss indicators.
/// - Comparisons: jemalloc, mimalloc, tcmalloc, ptmalloc where region-binding is applicable.
///
/// ### Implementation Plan
///
/// 1. Awaitable Interface
///    - Add `AllocMem` awaiter with fast-path try and wait-path enqueue.
///    - Define waiter node structure (requested size, coroutine handle, intrusive link).
/// 2. Waiter Queues and Masks
///    - Implement per-bin FIFO queues and waiter-mask for small sizes.
///    - Implement ordered large-waiter queue with FIFO for equal sizes.
/// 3. Wake Path and Policies
///    - Add `alloc_table_wake_waiters(at, budget)` invoked from free/defrag.
///    - Implement exact-fit small wake, largest-first large wake, and bounded work.
/// 4. Bounded Coalescing and Pressure Mode
///    - Enable coalescing on free with per-side merge bounds.
///    - Add low-watermark trigger and pressure-aware split policy.
/// 5. Large Reserve Bias
///    - Maintain minimal reserve for large requests while large waiters exist.
/// 6. Cancellation and Safety
///    - Implement waiter cancellation and safe unlinking.
///    - Re-validate availability prior to `await_resume`.
/// 7. Instrumentation and Telemetry
///    - Counters for queue metrics, resumes, defrag budgets; optional tracing.
/// 8. Benchmark Harness and Reports
///    - Implement workloads, automation, and CSV exports; produce baseline and post-change reports.
/// 9. Configuration and Tuning
///    - Expose watermarks, coalescing bounds, wake budgets, and reserve ratios.
///
/// ### Risks and Mitigations
///
/// - Unbounded wake work increasing free latency → enforce strict per-call budgets.
/// - Starvation of large requests under many small waiters → largest-first wake and reserve bias.
/// - Fragmentation under pressure → bounded coalescing and pressure-aware splitting.
/// - Stale waiters due to cancellation → explicit cancellation path with O(1) unlink.
///
/// ### Acceptance Criteria
///
/// - No allocation failures within configured region constraints; requests either return immediately or suspend and later resume with a valid pointer.
/// - Bounded free-path work per call with configurable limits.
/// - Demonstrated time-to-satisfy improvements for waiting allocations at high pressure versus baseline general-purpose allocators.
/// - Fragmentation within defined limits under mixed workloads.
///