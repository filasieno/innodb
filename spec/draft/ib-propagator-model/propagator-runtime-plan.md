# Propagator Agentic Platform - Runtime Model

## Overview

This document defines the **runtime execution model** for the Propagator Agentic Platform. While the core data model (see `agentic-propagator-plan.md`) defines the structural elements (cells, propagators, documents, workspaces), the runtime model captures:

- **Execution records**: What happened when propagators ran
- **Notification queues**: Which propagators need to activate
- **Merge operations**: How partial information was combined
- **Scheduler state**: Current execution status
- **Provenance tracking**: Why cells have their current values

These runtime objects are **ephemeral** compared to the core model—they represent the dynamic execution trace rather than the static network structure.

---

## Runtime Data Model

### [pnot] PropagatorNotification

Tracks which propagators need to be activated due to cell changes.

```text
[pnot] PropagatorNotification:
  - pnot_pid         : UUID <<PK>>                # Physical ID: Internal database identifier
  - prop_pid         : UUID <<FK prop>>           # Propagator Physical ID to notify
  - cell_pid         : UUID <<FK cell>>           # Cell Physical ID that changed
  - cv_pid           : UUID <<FK cv>>             # Specific value Physical ID that triggered
  - pnot_created_at  : Timestamp = CURRENT_TIMESTAMP
  - pnot_status      : Enum (PENDING, PROCESSING, COMPLETED, FAILED)
```

**Field Roles:**

- `pnot_pid`: Physical ID - Primary key for internal database operations
- `prop_pid`: Which propagator needs to run (Physical ID); points to registered listener
- `cell_pid`: Which cell changed (Physical ID); enables filtering notifications by cell
- `cv_pid`: Exact value version (Physical ID) that triggered notification; enables precise provenance tracking
- `pnot_created_at`: When notification was created; enables FIFO queue processing
- `pnot_status`: Notification lifecycle; scheduler polls PENDING, marks PROCESSING, then COMPLETED/FAILED

Constraints:

- INDEX: (prop_pid, pnot_status, pnot_created_at)
- INDEX: (pnot_status, pnot_created_at)
- FK: prop_pid → [prop].prop_pid
- FK: cell_pid → [cell].cell_pid
- FK: cv_pid → [cv].cv_pid
- CHECK: pnot_status IN ('PENDING', 'PROCESSING', 'COMPLETED', 'FAILED')
- BUSINESS RULE: Created when a cell value changes and propagator is registered as listener
- BUSINESS RULE: Propagator processes notification to compute new outputs

---

### [pexec] PropagatorExecution

Records an execution of a propagator, including inputs and outputs.

```text
[pexec] PropagatorExecution:
  - pexec_pid        : UUID <<PK>>                # Physical ID: Internal database identifier
  - pexec_id         : UUID <<UNIQUE>>            # Logical ID: External execution identifier
  - prop_pid         : UUID <<FK prop>>           # Propagator Physical ID that executed
  - pexec_started_at : Timestamp = CURRENT_TIMESTAMP
  - pexec_completed_at : Timestamp?               # When execution finished (NULL = still running)
  - pexec_status     : Enum (RUNNING, COMPLETED, FAILED, SKIPPED)
  - pexec_error      : TEXT?                      # Error message if status = FAILED
```

**Field Roles:**

- `pexec_pid`: Physical ID - Primary key for internal database operations; enables linking inputs/outputs
- `pexec_id`: Logical ID - External identifier for this execution; used in UNIX socket protocol for monitoring/debugging
- `prop_pid`: Which propagator executed (Physical ID); enables per-propagator execution history
- `pexec_started_at`: Execution start time; enables duration calculation and chronological ordering
- `pexec_completed_at`: Execution end time; NULL = still running; duration = completed_at - started_at
- `pexec_status`: Execution outcome; enables filtering successful vs failed runs
- `pexec_error`: For FAILED status, diagnostic information about what went wrong

Constraints:

- UNIQUE: (pexec_id)
- INDEX: (prop_pid, pexec_started_at DESC)
- FK: prop_pid → [prop].prop_pid
- CHECK: pexec_status IN ('RUNNING', 'COMPLETED', 'FAILED', 'SKIPPED')
- BUSINESS RULE: Records provenance of computations
- BUSINESS RULE: Links to input cell values that triggered execution

---

### [pexecin] PropagatorExecutionInput

Captures the input cell values used in a propagator execution.

```text
[pexecin] PropagatorExecutionInput:
  - pexecin_pid      : UUID <<PK>>                # Physical ID: Internal database identifier
  - pexec_pid        : UUID <<FK pexec>>          # Parent execution Physical ID
  - cell_pid         : UUID <<FK cell>>           # Input cell Physical ID that was read
  - cv_pid           : UUID <<FK cv>>             # Specific value Physical ID read
  - pconn_name       : String?                    # Named input port (if applicable)
```

**Field Roles:**

- `pexecin_pid`: Physical ID - Primary key for internal database operations
- `pexec_pid`: Links input to execution (Physical ID); groups all inputs for one execution run
- `cell_pid`: Which cell was read (Physical ID); enables "what inputs did this execution use?"
- `cv_pid`: Exact version read (Physical ID); enables reproducibility - rerun with same inputs = same output
- `pconn_name`: Disambiguates inputs when propagator has named ports (e.g., "file1", "file2")

Constraints:

- INDEX: (pexec_pid, cell_pid)
- FK: pexec_pid → [pexec].pexec_pid
- FK: cell_pid → [cell].cell_pid
- FK: cv_pid → [cv].cv_pid
- BUSINESS RULE: Records which cell values were read during execution

---

### [pexecout] PropagatorExecutionOutput

Captures the output cell values produced by a propagator execution.

```text
[pexecout] PropagatorExecutionOutput:
  - pexecout_pid     : UUID <<PK>>                # Physical ID: Internal database identifier
  - pexec_pid        : UUID <<FK pexec>>          # Parent execution Physical ID
  - cell_pid         : UUID <<FK cell>>           # Output cell Physical ID that was written
  - cv_pid           : UUID <<FK cv>>             # Value Physical ID created/added
  - pconn_name       : String?                    # Named output port (if applicable)
```

**Field Roles:**

- `pexecout_pid`: Physical ID - Primary key for internal database operations
- `pexec_pid`: Links output to execution (Physical ID); groups all outputs for one execution run
- `cell_pid`: Which cell was written (Physical ID); enables "what outputs did this execution produce?"
- `cv_pid`: Exact version created (Physical ID); enables "why does this cell have this value?" provenance queries
- `pconn_name`: Disambiguates outputs when propagator has named ports (e.g., "ast", "errors")

Constraints:

- INDEX: (pexec_pid, cell_pid)
- FK: pexec_pid → [pexec].pexec_pid
- FK: cell_pid → [cell].cell_pid
- FK: cv_pid → [cv].cv_pid
- BUSINESS RULE: Records which cell values were produced during execution

---

### [merge] MergeOperation

Records merge operations when multiple values are combined in a cell.

```text
[merge] MergeOperation:
  - merge_pid           : UUID <<PK>>                # Physical ID: Internal database identifier
  - cell_pid            : UUID <<FK cell>>           # Cell Physical ID where merge occurred
  - merge_cv_pid_1      : UUID <<FK cv>>             # First value Physical ID being merged
  - merge_cv_pid_2      : UUID <<FK cv>>             # Second value Physical ID being merged
  - merge_result_cv_pid : UUID <<FK cv>>             # Resulting merged value Physical ID
  - merge_at            : Timestamp = CURRENT_TIMESTAMP
  - merge_strategy      : Enum (UNIFY, INTERSECT, UNION, CUSTOM)
```

**Field Roles:**

- `merge_pid`: Physical ID - Primary key for internal database operations
- `cell_pid`: Where merge happened (Physical ID); enables per-cell merge history
- `merge_cv_pid_1`: First input to merge (Physical ID); order doesn't matter (commutative)
- `merge_cv_pid_2`: Second input to merge (Physical ID)
- `merge_result_cv_pid`: Output of merge (Physical ID); points to new CellValue (or CONTRADICTION)
- `merge_at`: When merge was computed; audit trail
- `merge_strategy`: How values were combined; UNIFY for ATerm unification, INTERSECT for constraints, etc.

Constraints:

- FK: cell_pid → [cell].cell_pid
- FK: merge_cv_pid_1 → [cv].cv_pid
- FK: merge_cv_pid_2 → [cv].cv_pid
- FK: merge_result_cv_pid → [cv].cv_pid
- CHECK: merge_strategy IN ('UNIFY', 'INTERSECT', 'UNION', 'CUSTOM')
- CHECK: merge_cv_pid_1 <> merge_cv_pid_2
- BUSINESS RULE: Merge must be commutative: merge(A, B) = merge(B, A)
- BUSINESS RULE: Merge must be associative: merge(merge(A, B), C) = merge(A, merge(B, C))
- BUSINESS RULE: Merge must be idempotent: merge(A, A) = A
- BUSINESS RULE: If values conflict → produces CONTRADICTION

---

### [sched] SchedulerState

Tracks the overall state of the propagator network scheduler.

```text
[sched] SchedulerState:
  - sched_pid        : UUID <<PK>>                # Physical ID: Internal database identifier
  - net_pid          : UUID <<FK net>> <<UNIQUE>> # Network Physical ID being scheduled (1:1)
  - sched_status     : Enum (IDLE, RUNNING, PAUSED, ERROR)
  - sched_updated_at : Timestamp = CURRENT_TIMESTAMP
  - sched_active_prop_count : INTEGER = 0         # Number of propagators currently executing
  - sched_pending_count : INTEGER = 0             # Number of pending notifications
```

**Field Roles:**

- `sched_pid`: Physical ID - Primary key for internal database operations
- `net_pid`: One-to-one with network (Physical ID); each network has its own scheduler
- `sched_status`: Scheduler state machine; IDLE = quiescent; RUNNING = actively propagating; PAUSED = suspended; ERROR = fatal error
- `sched_updated_at`: Last state change; enables timeout detection
- `sched_active_prop_count`: How many propagators are running now; enables progress monitoring
- `sched_pending_count`: How many notifications queued; 0 = network is quiescent

Constraints:

- UNIQUE: (net_pid)
- FK: net_pid → [net].net_pid
- CHECK: sched_status IN ('IDLE', 'RUNNING', 'PAUSED', 'ERROR')
- BUSINESS RULE: Scheduler runs propagators in response to cell changes
- BUSINESS RULE: Execution continues until no propagator can make progress (fixpoint)

---

## Execution Semantics

### Propagation Flow

1. **Cell Update**: When content is added to a cell:
   - Create new `CellValue` with incremented version
   - Apply merge operation if cell already has values
   - Create `PropagatorNotification` entries for all registered propagators

2. **Propagator Activation**: When a propagator is notified:
   - Check if all required input cells have sufficient information
   - If ready, create `PropagatorExecution` record
   - Read current cell values (record in `PropagatorExecutionInput`)
   - Execute computation
   - Add results to output cells (record in `PropagatorExecutionOutput`)

3. **Merge Operation**: When multiple values exist in a cell:
   - Apply merge strategy based on value types
   - If compatible: produce unified value
   - If incompatible: produce `CONTRADICTION`
   - Record merge in `MergeOperation`

4. **Network Quiescence**: Scheduler reaches fixpoint when:
   - No pending notifications remain
   - All propagators have processed latest input versions
   - No propagator can produce new information

### Monotonicity Guarantees

- **Cell values**: Only accumulate more specific information (moving down the lattice)
- **Propagators**: Given the same inputs, always produce the same or more specific outputs
- **Merges**: Never lose information; only combine or detect contradictions
- **Versions**: Monotonically increasing; later versions never invalidate earlier ones

### Change Detection for TextDocuments

When a `TextDocument` changes:

1. New `TextDocumentVersion` is created with updated piece table
2. New `CellValueTextDocument` references the new version
3. All propagators connected to that cell receive notifications
4. Propagators can diff versions to determine what changed
5. Incremental processing possible via line/column indices

### ATerm Unification

When two `ATerm` values merge:

- If structurally identical: idempotent (same value)
- If compatible: unify (e.g., variable + concrete value)
- If incompatible: produce `CONTRADICTION`
- Structural sharing via content-addressable storage

---

## Scheduling Strategies

### Breadth-First Scheduling

```text
Process all PENDING notifications before processing newly created ones:
  1. Collect batch of PENDING notifications
  2. Mark them PROCESSING
  3. Execute all propagators in batch (can be parallel)
  4. Mark COMPLETED/FAILED
  5. Repeat until no PENDING notifications remain
  
Properties:
  - Fair: All propagators get chance to run before any runs twice
  - Predictable: Execution order less dependent on processing speed
  - Batch-friendly: Good for parallel execution
```

### Depth-First Scheduling

```text
Follow dependency chains to completion:
  1. Pick a PENDING notification
  2. Mark PROCESSING and execute propagator
  3. If execution produces new values:
     a. New notifications created
     b. Immediately process those (recursive)
  4. Mark COMPLETED/FAILED
  5. Return to step 1 if PENDING notifications remain
  
Properties:
  - Focused: Completes dependency chains before switching
  - Memory-efficient: Fewer live notifications at once
  - Can lead to starvation: Some propagators may wait indefinitely
```

### Priority-Based Scheduling

```text
Propagators have priorities; higher priority runs first:
  1. Order PENDING notifications by propagator priority
  2. Process highest priority first
  3. Within same priority, use FIFO or other tiebreaker
  
Properties:
  - Critical path optimization: Important computations run first
  - Can configure: User-facing updates prioritized over background analysis
  - Risk: Low-priority propagators may starve
```

### Parallel Scheduling

```text
Execute independent propagators concurrently:
  1. Collect batch of PENDING notifications
  2. Build dependency graph based on cell connections
  3. Identify independent propagators (no shared input cells)
  4. Execute independent set in parallel
  5. Wait for batch completion, then repeat
  
Properties:
  - Throughput: Maximizes CPU utilization
  - Complexity: Requires thread-safe merge operations
  - Best for: Networks with high parallelism
```

---

## Provenance Queries

The runtime model enables powerful provenance queries:

### Why does this cell have this value?

```sql
-- Find which execution produced a value
SELECT pexec.pexec_id, pexec.pexec_started_at, prop.prop_name
FROM pexecout
JOIN pexec ON pexecout.pexec_pid = pexec.pexec_pid
JOIN prop ON pexec.prop_pid = prop.prop_pid
WHERE pexecout.cv_pid = <target_cv_pid>;

-- Find what inputs were used
SELECT cell.cell_name, cv.cv_version
FROM pexecin
JOIN pexec ON pexecin.pexec_pid = pexec.pexec_pid
JOIN cell ON pexecin.cell_pid = cell.cell_pid
JOIN cv ON pexecin.cv_pid = cv.cv_pid
WHERE pexec.pexec_pid = <execution_pid>;
```

### What propagators are waiting to run?

```sql
SELECT prop.prop_name, COUNT(*) as pending_count
FROM pnot
JOIN prop ON pnot.prop_pid = prop.prop_pid
WHERE pnot.pnot_status = 'PENDING'
GROUP BY prop.prop_name
ORDER BY pending_count DESC;
```

### Merge history of a value

```sql
-- Find source values that were merged to create a result
WITH RECURSIVE merge_tree AS (
  -- Base case: direct sources
  SELECT cv_source_pid, cv_result_pid, 1 as depth
  FROM cvm
  WHERE cv_result_pid = <target_cv_pid>
  
  UNION ALL
  
  -- Recursive case: sources of sources
  SELECT cvm.cv_source_pid, cvm.cv_result_pid, mt.depth + 1
  FROM cvm
  JOIN merge_tree mt ON cvm.cv_result_pid = mt.cv_source_pid
)
SELECT * FROM merge_tree;
```

### Execution timeline

```sql
SELECT 
  pexec.pexec_id,
  prop.prop_name,
  pexec.pexec_started_at,
  pexec.pexec_completed_at,
  EXTRACT(EPOCH FROM (pexec.pexec_completed_at - pexec.pexec_started_at)) as duration_seconds,
  pexec.pexec_status
FROM pexec
JOIN prop ON pexec.prop_pid = prop.prop_pid
ORDER BY pexec.pexec_started_at DESC
LIMIT 100;
```

---

## Garbage Collection

Runtime objects accumulate over time. Strategies for cleanup:

### Notification Cleanup

```sql
-- Delete completed notifications older than 1 hour
DELETE FROM pnot
WHERE pnot_status IN ('COMPLETED', 'FAILED')
  AND pnot_created_at < NOW() - INTERVAL '1 hour';
```

### Execution History Retention

```sql
-- Keep last 1000 executions per propagator, delete older ones
WITH ranked_executions AS (
  SELECT pexec_pid,
         ROW_NUMBER() OVER (PARTITION BY prop_pid ORDER BY pexec_started_at DESC) as rn
  FROM pexec
)
DELETE FROM pexec
WHERE pexec_pid IN (
  SELECT pexec_pid FROM ranked_executions WHERE rn > 1000
);
```

### Merge Operation Cleanup

```sql
-- Delete merge records older than 7 days
DELETE FROM merge
WHERE merge_at < NOW() - INTERVAL '7 days';
```

**Note**: Deleting execution records doesn't affect the propagator network's correctness—it only removes provenance history. The current cell values and network structure remain intact.

---

## Monitoring and Observability

### Real-Time Metrics

```sql
-- Current scheduler status across all networks
SELECT 
  net.net_name,
  sched.sched_status,
  sched.sched_active_prop_count,
  sched.sched_pending_count,
  sched.sched_updated_at
FROM sched
JOIN net ON sched.net_pid = net.net_pid
WHERE net.net_deleted_at IS NULL;
```

### Propagator Performance

```sql
-- Average execution time per propagator
SELECT 
  prop.prop_name,
  COUNT(*) as execution_count,
  AVG(EXTRACT(EPOCH FROM (pexec.pexec_completed_at - pexec.pexec_started_at))) as avg_duration_seconds,
  SUM(CASE WHEN pexec.pexec_status = 'FAILED' THEN 1 ELSE 0 END) as failure_count
FROM pexec
JOIN prop ON pexec.prop_pid = prop.prop_pid
WHERE pexec.pexec_completed_at IS NOT NULL
  AND pexec.pexec_started_at > NOW() - INTERVAL '1 day'
GROUP BY prop.prop_name
ORDER BY avg_duration_seconds DESC;
```

### Cell Activity

```sql
-- Most frequently updated cells
SELECT 
  cell.cell_name,
  COUNT(DISTINCT cv.cv_pid) as value_count,
  MAX(cv.cv_added_at) as last_updated
FROM cv
JOIN cell ON cv.cell_pid = cell.cell_pid
WHERE cv.cv_added_at > NOW() - INTERVAL '1 hour'
GROUP BY cell.cell_name
ORDER BY value_count DESC
LIMIT 20;
```

---

## Debugging Scenarios

### Propagator Not Firing

**Problem**: Expected a propagator to run, but it didn't.

**Debug Steps**:

1. Check if propagator is enabled: `SELECT prop_enabled FROM prop WHERE prop_id = <id>`
2. Check input cells: Do all required inputs have values?
3. Check notifications: Were notifications created?

   ```sql
   SELECT * FROM pnot WHERE prop_pid = <pid> ORDER BY pnot_created_at DESC LIMIT 10;
   ```

4. Check last execution: Did it fail?

   ```sql
   SELECT * FROM pexec WHERE prop_pid = <pid> ORDER BY pexec_started_at DESC LIMIT 5;
   ```

### Infinite Propagation Loop

**Problem**: Scheduler never reaches quiescence; propagators keep firing.

**Debug Steps**:

1. Check pending count: `SELECT sched_pending_count FROM sched WHERE net_pid = <pid>`
2. Identify which propagators are firing repeatedly:

   ```sql
   SELECT prop.prop_name, COUNT(*) as fire_count
   FROM pexec
   JOIN prop ON pexec.prop_pid = prop.prop_pid
   WHERE pexec.pexec_started_at > NOW() - INTERVAL '1 minute'
   GROUP BY prop.prop_name
   HAVING COUNT(*) > 10
   ORDER BY fire_count DESC;
   ```

3. Check for non-monotonic behavior: Is a propagator producing less specific outputs than inputs?
4. Check for circular dependencies without fixpoint

### Contradiction Debugging

**Problem**: Cell contains CONTRADICTION, need to understand why.

**Debug Steps**:

1. Find the contradiction value:

   ```sql
   SELECT cv.cv_pid, cv.cv_added_at, cv.cv_added_by_prop_pid
   FROM cv
   WHERE cv.cell_pid = <cell_pid> AND cv.cv_value_type = 'CONTRADICTION'
   ORDER BY cv.cv_added_at DESC LIMIT 1;
   ```

2. Find which merge produced it:

   ```sql
   SELECT merge.merge_cv_pid_1, merge.merge_cv_pid_2, merge.merge_strategy
   FROM merge
   WHERE merge.merge_result_cv_pid = <contradiction_cv_pid>;
   ```

3. Examine the conflicting source values
4. Trace back to which propagators produced the conflicting values

---

## Performance Optimization

### Index Strategy

Critical indexes for runtime performance:

```sql
-- Notification processing (hot path)
CREATE INDEX idx_pnot_pending ON pnot(pnot_status, pnot_created_at) 
  WHERE pnot_status = 'PENDING';

-- Execution history queries
CREATE INDEX idx_pexec_prop_time ON pexec(prop_pid, pexec_started_at DESC);

-- Provenance lookups
CREATE INDEX idx_pexecout_cv ON pexecout(cv_pid);
CREATE INDEX idx_pexecin_cv ON pexecin(cv_pid);

-- Merge history
CREATE INDEX idx_merge_result ON merge(merge_result_cv_pid);
```

### Partitioning Strategy

For high-volume systems, partition runtime tables by time:

```sql
-- Partition executions by month
CREATE TABLE pexec (
  ...
) PARTITION BY RANGE (pexec_started_at);

CREATE TABLE pexec_2025_11 PARTITION OF pexec
  FOR VALUES FROM ('2025-11-01') TO ('2025-12-01');
  
-- Drop old partitions to manage disk usage
DROP TABLE pexec_2025_01;
```

### Archival Strategy

Move old runtime data to archive tables:

```sql
-- Archive completed executions older than 30 days
INSERT INTO pexec_archive
SELECT * FROM pexec
WHERE pexec_completed_at < NOW() - INTERVAL '30 days'
  AND pexec_status IN ('COMPLETED', 'FAILED', 'SKIPPED');

DELETE FROM pexec
WHERE pexec_completed_at < NOW() - INTERVAL '30 days'
  AND pexec_status IN ('COMPLETED', 'FAILED', 'SKIPPED');
```

---

## UNIX Domain Socket Protocol

The runtime model is exposed via UNIX domain socket for real-time monitoring and control.

### Runtime Monitoring Messages

**Subscribe to notifications:**

```json
{
  "type": "subscribe_notifications",
  "network_id": "<net_id>",
  "propagator_id": "<prop_id>?"  // Optional: filter by propagator
}
```

**Execution status stream:**

```json
{
  "type": "execution_update",
  "execution_id": "<pexec_id>",
  "propagator_id": "<prop_id>",
  "status": "COMPLETED",
  "started_at": "2025-11-09T12:34:56Z",
  "completed_at": "2025-11-09T12:34:57Z",
  "duration_ms": 1234
}
```

**Scheduler status:**

```json
{
  "type": "scheduler_status",
  "network_id": "<net_id>",
  "status": "RUNNING",
  "active_propagator_count": 3,
  "pending_count": 15,
  "updated_at": "2025-11-09T12:35:00Z"
}
```

### Runtime Control Messages

**Pause/Resume scheduler:**

```json
{
  "type": "control_scheduler",
  "network_id": "<net_id>",
  "action": "PAUSE" | "RESUME" | "STEP"
}
```

**Query execution history:**

```json
{
  "type": "query_executions",
  "propagator_id": "<prop_id>",
  "since": "2025-11-09T12:00:00Z",
  "limit": 100
}
```

---

## Future Runtime Extensions

- **Distributed Execution**: Propagators run on different machines; execution records track node assignment
- **Incremental Checkpointing**: Periodically snapshot scheduler state for crash recovery
- **Execution Replay**: Rerun historical executions with same inputs for debugging
- **Performance Profiling**: Detailed timing breakdown within propagator execution
- **Resource Limits**: CPU/memory quotas per propagator; throttling when limits exceeded
- **Execution DAG Visualization**: Real-time graph of which propagators are firing and why
