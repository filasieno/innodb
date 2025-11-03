# XInnoDB Architecture Document

## Overview

XInnoDB implements a distributed, coroutine-based database engine architecture optimized for high-core workstations. The system combines shared global state coordination with process isolation, using modern C++20 coroutines for efficient concurrency management.

## Core Architecture

### System Components

```
Client Processes [PCA] → [SGA] ← Worker Processes [PWA]
                       ↑
                Coordinator Process
```

#### Shared Global Area (SGA)
- **Purpose**: Centralized coordination and shared state management
- **Contents**:
  - Buffer pool for cached data pages
  - Lock manager for distributed locking
  - Transaction coordinator state
  - Work queues for client requests
  - Shared metadata and system catalogs
- **Implementation**: Configurable (shared memory, database-backed, memory-mapped files)
- **Access**: All processes have read/write access with appropriate synchronization

#### Client Processes
- **Role**: Handle user connections, protocol parsing, query planning
- **Private Client Area (PCA)**:
  - Session state and connection context
  - Prepared statement cache
  - Result set buffers
  - Client-specific temporary storage
- **Coroutine Scheduler**: Manages client-side async operations
- **Scaling**: Multiple clients can connect simultaneously

#### Worker Processes
- **Role**: Execute database operations (queries, transactions, maintenance)
- **Characteristics**:
  - One worker per CPU core for optimal hardware utilization
  - Symmetrical code across all workers
  - NUMA-aware placement for memory locality
- **Private Worker Area (PWA)**:
  - Thread-local caches and scratch space
  - Worker-specific statistics and metrics
  - Private memory pools for operation execution
- **Coroutine Scheduler**: Manages worker-side task execution

#### Coordinator Process (Optional)
- **Role**: System orchestration and monitoring
- **Responsibilities**:
  - Load balancing across workers
  - Failure detection and recovery
  - SGA lifecycle management
  - System health monitoring

## Runtime Architecture

### Modular Component Design

XInnoDB's runtime is decomposed into independent, testable modules with clean dependencies:

```
base/ ← alloc/ ← runtime/ (integration)
    ↑         ↑
task/ ← scheduler/ ← sync/
    ↑
   io/
```

#### Task Module (`ak/task/`)

**Purpose**: Core coroutine and task abstraction
**Responsibilities**:
- C++20 coroutine promise types
- Task lifecycle management (creation, destruction)
- Task state enumeration and utilities
- Memory management for coroutine frames
**Dependencies**: `base/` (types, assertions)
**Used by**: `scheduler/`, `io/`, `runtime/`

#### Scheduler Module (`ak/scheduler/`)

**Purpose**: Task scheduling interface and implementations
**Responsibilities**:
- `IScheduler` interface for pluggable scheduling
- `CooperativeScheduler` implementation
- Task queuing and dispatching
- Scheduler coordination primitives
**Dependencies**: `task/`, `base/`
**Used by**: `sync/`, `io/`, `runtime/`

#### IO Module (`ak/io/`)

**Purpose**: Asynchronous IO operations
**Responsibilities**:
- io_uring-based file and network operations
- IO operation awaitables and completion handling
- Buffer management for IO operations
- Platform-specific IO optimizations
**Dependencies**: `task/`, `base/`
**Used by**: `runtime/`

#### Sync Module (`ak/sync/`)

**Purpose**: Synchronization primitives
**Responsibilities**:
- Event-based coordination between tasks
- Cross-task synchronization operations
- Awaitable synchronization types
- Integration with scheduler for task suspension/resumption
**Dependencies**: `scheduler/`, `task/`, `base/`
**Used by**: Application code

#### Runtime Integration (`ak/runtime/`)

**Purpose**: Thin integration layer
**Responsibilities**:
- `Runtime` coordinator class
- Global state management (when needed)
- Component wiring and initialization
- Public API entry points
**Dependencies**: `task/`, `scheduler/`, `io/`, `alloc/`, `base/`
**Used by**: Application entry point

## Development Strategy

### Phase 1: Deterministic Single-Threaded Development

#### Execution Model

- All components (SGA, PCA, PWA) execute in single address space
- Single-threaded coroutine scheduler for deterministic execution
- Coroutines switch at well-defined yield points only
- All memory areas accessible without IPC overhead

#### Testing Framework

- **Deterministic Simulation**: Record and replay execution traces
- **Reproducible Bugs**: Identical execution paths for debugging
- **Operation Logging**: Complete audit trail of all state changes
- **Transaction Isolation**: One coroutine per transaction for clean rollback

#### Benefits

- **Race Condition Detection**: Single-threaded execution exposes concurrency bugs early
- **Debugging**: Deterministic behavior enables reliable reproduction
- **Performance Baseline**: Establish performance characteristics before distribution
- **API Validation**: Test component interfaces without IPC complexity

### Phase 2: Multi-Process Migration

#### SGA Implementation Options

1. **Shared Memory** (Primary): Highest performance, direct memory access
2. **Message Passing**: Simpler implementation, network-transparent
3. **IPC Mechanisms**: Pipes, sockets, domain sockets based on performance measurements

#### Process Communication

- **Work Submission**: Clients post requests to SGA work queues
- **Result Delivery**: Workers signal completion via SGA events
- **Coordination**: SGA-managed message passing between processes
- **Synchronization**: Cross-process events and condition variables

#### Gradual Transition

- Start with logical separation in single process
- Migrate to shared memory segments
- Add process boundaries incrementally
- Maintain deterministic testing throughout

### Phase 3: Production Optimization

#### NUMA-Aware Design

- Worker processes pinned to NUMA nodes
- SGA memory allocated with NUMA affinity
- Memory access patterns optimized for locality

#### Performance Tuning

- Benchmark different SGA implementations
- Optimize message passing overhead
- Tune coroutine scheduling parameters
- Profile and eliminate bottlenecks

## Migration from libak Runtime

### Current State Analysis

The `libak/runtime` module is a monolithic component with global state dependencies:

- `global_kernel_state` used throughout
- Tight coupling between task management, IO, and scheduling
- Difficult to test components independently
- Global state prevents multi-process deployment

### Migration Phases

#### Phase 1A: Task Module Extraction

```text
Source: runtime_api.hpp, runtime_promise.cpp, runtime_task.cpp
Target: xinnodb/src/task/
Changes:
- Extract AkPromise, AkTask, AkCoroutineHandle types
- Remove global_kernel_state dependencies
- Create pure task abstraction layer
```

#### Phase 1B: Scheduler Module Creation

```text
Source: runtime_task_ops.cpp, runtime_boot.cpp
Target: xinnodb/src/sched/
Changes:
- Create IScheduler interface
- Extract CooperativeScheduler implementation
- Remove direct kernel state access
- Implement dependency injection
```

#### Phase 1C: IO Module Adaptation

```text
Source: runtime_io_prep.cpp
Target: xinnodb/src/io/ (existing)
Changes:
- Update to use task module abstractions
- Remove global state dependencies
- Adapt for injected scheduler
```

#### Phase 1D: Runtime Integration

```text
Source: runtime_kernel.cpp, runtime_boot.cpp
Target: xinnodb/src/rt/
Changes:
- Create Runtime coordinator class
- Extract kernel state management
- Wire components together
- Provide clean public API
```

#### Phase 1E: Sync Module Update

```text
Source: sync/ directory
Target: xinnodb/src/sync/
Changes:
- Update to use scheduler injection
- Remove global kernel dependencies
- Adapt for modular architecture
```

### Build System Integration

#### CMake Component Declaration

```cmake
# Add new components in dependency order
xinnodb_component(task  DEPS defs)
xinnodb_component(sched DEPS task defs)
xinnodb_component(rt    DEPS task sched io alloc)
xinnodb_component(sync  DEPS sched task)
xinnodb_component(api   DEPS task sched rt sync io alloc)
```

#### Module Structure

Each component follows standard layout:

```text
xinnodb/src/<component>/
├── include/          # Public headers
├── src/             # Implementation files
└── test/            # Unit tests
```

## Risk Mitigation

### Development Phase Risks

- **Concurrency Bugs**: Addressed by deterministic single-threaded development
- **API Design Issues**: Caught early through modular testing
- **Integration Problems**: Resolved through incremental migration

### Multi-Process Risks

- **IPC Complexity**: Mitigated by starting with shared memory approach
- **Synchronization Issues**: Comprehensive testing with deterministic replay
- **Performance Degradation**: Performance measurements guide implementation choices

### Operational Risks

- **Process Failures**: Process isolation prevents cascading failures
- **Resource Contention**: NUMA-aware design and load balancing
- **Debugging Difficulty**: Deterministic replay capabilities for multi-process issues

## Testing Strategy

### Unit Testing

- Each module tested independently with mocks
- Deterministic execution for reliable test results
- Comprehensive coverage of component interfaces

### Integration Testing

- Component interaction validation
- Performance regression testing
- Multi-process coordination testing

### System Testing

- End-to-end transaction processing
- Load testing with multiple clients
- Failure scenario simulation
- NUMA performance validation

## Performance Considerations

### Opportunistic Implementation Strategy

- **Measure First**: Benchmark different approaches (shared memory, IPC, etc.)
- **Choose Fastest**: Select implementation based on performance data
- **Optimize Iteratively**: Profile and tune based on real workloads

### Key Performance Metrics

- **Latency**: Transaction response time
- **Throughput**: Transactions per second
- **CPU Utilization**: Core efficiency across workers
- **Memory Bandwidth**: SGA access patterns
- **IPC Overhead**: Cross-process communication costs

## Future Extensions

### Additional Scheduler Implementations

- **PreemptiveScheduler**: Time-sliced scheduling for CPU-bound tasks
- **PriorityScheduler**: Quality-of-service based task ordering
- **WorkStealingScheduler**: Dynamic load balancing across workers

### Alternative SGA Backends

- **Database-Backed SGA**: Persistence and crash recovery
- **Distributed SGA**: Multi-machine coordination
- **Hybrid SGA**: Memory + disk with intelligent caching

### Advanced Features

- **Query Parallelization**: Distribute query execution across workers
- **Materialized Views**: Shared cached results in SGA
- **Advanced Indexing**: SGA-managed index structures
- **Replication**: Multi-instance coordination through SGA

## Conclusion

The XInnoDB architecture combines proven database patterns with modern async programming techniques. 
The deterministic development approach ensures reliability while the modular runtime design enables flexibility and testability. The SGA-based coordination model provides efficient shared state management without sacrificing the benefits of process isolation.

This architecture is designed to scale from single-core development systems to high-end multi-socket workstations, providing excellent performance while maintaining code clarity and maintainability.
