# Propagator Agentic Platform - Data Model

## Overview

A propagator-based computational platform where autonomous propagators communicate through shared cells. The system operates on immutable, multiversioned data structures: **ATerms** (annotated terms), **TextDocuments**, and **NotebookDocuments** (implemented as PieceTables with column/line indices).

The platform provides a fully-featured LSP (Language Server Protocol) compatible environment accessed via **UNIX Domain Sockets**.

Key principles:

- **Cells** accumulate partial information monotonically
- **Propagators** continuously examine cells and add computed information
- **Merge operations** are commutative, associative, and idempotent
- **Propagator behavior** is monotonic with respect to the merge lattice
- All data is **immutable and multiversioned**
- **LSP-compliant** workspace and document synchronization
- **URI-addressable** resources with UUID-based identity

---

## Fundamental Entities

The propagator model consists of three fundamental entities that work together:

### Network: The Container

**Role**: Organizational boundary and execution context

A **Network** is a named collection of cells and propagators that form a cohesive computational unit. It serves as:

- **Namespace**: Groups related cells and propagators together (e.g., "typescript_analyzer" network)
- **Execution boundary**: Each network has its own scheduler; networks can run independently or in parallel
- **Isolation unit**: Networks can be started, paused, or stopped independently
- **Composition unit**: Networks can be nested or composed (compound propagators contain sub-networks)
- **Deployment unit**: A network can be deployed as a service, accessed via UNIX socket

**Analogy**: Think of a network as a **circuit board** - it contains components (propagators) and wiring (cells), and can be treated as a black box with input/output ports.

**Key Properties**:

- Has explicit INPUT/OUTPUT cells that form its external interface
- Has INTERNAL cells that are hidden implementation details
- Can be instantiated multiple times with different input data
- Lifecycle: created → running → quiescent → paused/stopped → deleted

---

### Cell: The Communication Channel

**Role**: Shared memory location for information accumulation

A **Cell** is a communication channel where information accumulates over time. It serves as:

- **Information accumulator**: Stores partial information that becomes more specific monotonically
- **Synchronization point**: Multiple propagators can read from and write to the same cell
- **Decoupling mechanism**: Propagators don't call each other directly; they communicate through cells
- **Version history**: Maintains all values ever added (temporal database)
- **Notification hub**: When a cell's content changes, all connected propagators are notified

**Analogy**: Think of a cell as a **whiteboard** - multiple people (propagators) can read from it and write to it. New information is added (not erased), and everyone watching is notified when something new appears.

**Key Properties**:

- Content is **append-only**: values are added, never removed
- **Merge-based**: When multiple values are added, they merge according to their type
- **Observable**: Propagators register as listeners and get notified of changes
- **Named or anonymous**: Can have human-readable names for debugging, or be anonymous intermediates
- Can contain different types: ATerms, TextDocuments, NotebookDocuments, Workspaces, Diagnostics, etc.

---

### Value: The Information Unit

**Role**: Versioned piece of information in a cell

A **Value** (CellValue) represents a specific piece of information at a point in time. It serves as:

- **Immutable snapshot**: Once created, never modified
- **Typed container**: Can hold ATerm, TextDocument, NotebookDocument, Workspace, Diagnostic, etc.
- **Provenance carrier**: Records who added it (which propagator or user) and when
- **Merge participant**: Can be merged with other values to produce refined information
- **Version marker**: Has monotonically increasing version number within its cell

**Analogy**: Think of a value as a **sticky note on the whiteboard** - it's a specific piece of information posted at a specific time. The whiteboard (cell) can accumulate many sticky notes (values) over time.

**Key Properties**:

- **Versioned**: Each value has a version number; supports time-travel queries
- **Immutable**: Content never changes after creation
- **Mergeable**: Two values can combine to produce a more specific value (or CONTRADICTION)
- **Provenance**: Records which propagator computed it and which values were merged to create it
- **Typed**: Discriminator (`cv_value_type`) determines which variant table holds the actual data

---

### How They Work Together

```text
Network: "Code Analysis Pipeline"
  ├─ Cells: [Source], [AST], [Types], [Diagnostics]
  │   └─ Each cell accumulates Values over time:
  │      ├─ [Source] has Value v1 (TextDocument version 0)
  │      ├─ [Source] has Value v2 (TextDocument version 1) ← edited
  │      ├─ [AST] has Value v1 (ATerm from parsing v1)
  │      └─ [AST] has Value v2 (ATerm from parsing v2)
  │
  └─ Propagators: [Parser], [TypeChecker], [Linter]
      ├─ Parser: Reads [Source] → Writes [AST]
      ├─ TypeChecker: Reads [AST] → Writes [Types]
      └─ Linter: Reads [AST] → Writes [Diagnostics]
```

**Information Flow:**

1. **User action**: Adds new TextDocument value to [Source] cell
2. **Cell notification**: [Source] cell notifies all connected propagators
3. **Parser activation**: Parser propagator fires (sees new source value)
4. **Parser execution**: Reads TextDocument v2, parses it, produces ATerm
5. **Parser output**: Adds new ATerm value to [AST] cell
6. **Cascade**: [AST] cell notifies TypeChecker and Linter
7. **Parallel execution**: TypeChecker and Linter run concurrently
8. **Results**: New values added to [Types] and [Diagnostics] cells
9. **Quiescence**: No more pending notifications; network is stable

**Key Insight**:

- **Networks** provide **structure** (what components exist)
- **Cells** provide **communication** (how information flows)
- **Values** provide **content** (what information is known)

Together they enable:

- **Modularity**: Add new propagators without modifying existing ones
- **Incrementality**: Only recompute what changed
- **Multi-strategy**: Multiple propagators can contribute to same cell
- **Debugging**: Full provenance from inputs to outputs
- **Time-travel**: Access any historical state

---

## Data Model

### [cell] Cell

A cell accumulates partial information and notifies registered propagators when content changes.

```text
[cell] Cell:
  - cell_pid        : UUID <<PK>>                       # Internal database identifier
  - cell_id         : UUID <<UNIQUE>>                   # Globally unique identity (for distributed systems)
  - cell_name       : String?                           # Optional human-readable name (e.g., "source_file", "ast_output")
  - cell_created_at : Timestamp = CURRENT_TIMESTAMP     # When the cell was created
  - cell_deleted_at : Timestamp? = NULL                 # Soft-delete timestamp (NULL = active)
```

**Field Roles:**

- `cell_pid`: Physical ID - Primary key for internal database operations and foreign key references
- `cell_id`: Logical ID - External stable identifier; survives database migrations; used in propagator network serialization and UNIX socket protocol
- `cell_name`: Human-readable label for debugging and visualization; optional because cells can be anonymous intermediates
- `cell_created_at`: Audit trail; helps understand cell lifecycle
- `cell_deleted_at`: Enables soft deletion without breaking referential integrity; allows historical queries

Constraints:

- UNIQUE: (cell_id)
- UNIQUE: (cell_name) WHERE cell_name IS NOT NULL AND cell_deleted_at IS NULL
- BUSINESS RULE: A cell can have multiple versions of content accumulated over time
- BUSINESS RULE: Content merging must be commutative, associative, and idempotent

---

### [cv] CellValue

Represents a versioned value in a cell. Multiple values can coexist, representing partial information that gets merged.

```text
[cv] CellValue:
  - cv_pid                : UUID <<PK>>                        # Physical ID: Internal database identifier
  - cv_id                 : UUID <<UNIQUE>>                    # Logical ID: External stable identifier
  - cell_pid              : UUID <<FK cell>>                   # The cell this value belongs to
  - cv_value_type         : Enum (ATERM, TEXT_DOCUMENT, NOTEBOOK_DOCUMENT, WORKSPACE, WORKSPACE_EDIT, FILE_EVENT, DIAGNOSTIC, NOTHING, CONTRADICTION)
  - cv_version            : BIGSERIAL                          # Monotonically increasing version within cell
  - cv_added_at           : Timestamp = CURRENT_TIMESTAMP      # When this value was added to the cell
  - cv_added_by_prop_pid  : UUID <<FK prop>>?                  # Which propagator computed this (NULL = user-provided)
```

**Field Roles:**

- `cv_pid`: Physical ID - Primary key for internal database operations and foreign key references
- `cv_id`: Logical ID - External stable identifier; used in UNIX socket protocol to reference specific value versions
- `cell_pid`: Links value to its containing cell (via Physical ID); one cell can have many values over time
- `cv_value_type`: Discriminator for single-table inheritance; determines which variant table has details
- `cv_version`: Enables ordering values chronologically; supports time-travel queries; monotonic within cell
- `cv_added_at`: Wall-clock timestamp for audit trail; supplements version ordering
- `cv_added_by_prop_pid`: Provenance tracking; enables "why does this cell have this value?" queries; NULL means user directly added it

Constraints:

- UNIQUE: (cv_id)
- INDEX: (cell_pid, cv_version DESC)
- CHECK: cv_value_type IN ('ATERM', 'TEXT_DOCUMENT', 'NOTEBOOK_DOCUMENT', 'WORKSPACE', 'WORKSPACE_EDIT', 'FILE_EVENT', 'DIAGNOSTIC', 'NOTHING', 'CONTRADICTION')
- BUSINESS RULE: Exactly one row must exist in the appropriate value variant table based on cv_value_type
- BUSINESS RULE: cv_version must be monotonically increasing per cell_pid

---

### [cvm] CellValueMerge

Tracks merge provenance: which values were merged to create a new value.

```text
[cvm] CellValueMerge:
  - cvm_pid          : UUID <<PK>>                # Physical ID: Internal database identifier
  - cv_result_pid    : UUID <<FK cv>>             # The merged result value
  - cv_source_pid    : UUID <<FK cv>>             # One of the source values that was merged
  - cvm_created_at   : Timestamp = CURRENT_TIMESTAMP
```

**Field Roles:**

- `cvm_pid`: Physical ID - Primary key for internal database operations
- `cv_result_pid`: The CellValue (Physical ID) that was created by merging; one result can have multiple sources
- `cv_source_pid`: One of the input CellValues (Physical ID) that contributed to the merge; enables tracing merge history
- `cvm_created_at`: When this merge relationship was recorded; audit trail

Constraints:

- UNIQUE: (cv_result_pid, cv_source_pid)
- INDEX: (cv_result_pid)
- INDEX: (cv_source_pid)
- FK: cv_result_pid → [cv].cv_pid
- FK: cv_source_pid → [cv].cv_pid
- BUSINESS RULE: A merged value typically has 2+ source values (one row per source)
- BUSINESS RULE: Enables reconstructing merge tree: "this value came from merging X, Y, and Z"
- BUSINESS RULE: Non-merged values (directly added) have zero rows in this table

---

### [cvat] CellValueATerm (variant of CellValue)

Stores ATerm values - annotated terms for tree-structured data.

```text
[cvat] CellValueATerm:
  - cvat_pid        : UUID <<PK>>                # Physical ID: Internal database identifier
  - cv_pid          : UUID <<FK cv>> <<UNIQUE>>  # Parent CellValue Physical ID (1:1 relationship)
  - cvat_aterm      : JSONB                      # ATerm structure stored as JSON
  - cvat_hash       : BYTEA                      # Content-addressable SHA256 hash for deduplication
```

**Field Roles:**

- `cvat_pid`: Physical ID - Primary key for this variant record
- `cv_pid`: Links to parent CellValue (via Physical ID); enforces 1:1 relationship with base table
- `cvat_aterm`: The actual ATerm data; JSONB enables indexing and querying substructure
- `cvat_hash`: Enables deduplication (same ATerm structure reuses storage); supports structural sharing across versions

Constraints:

- UNIQUE: (cv_pid)
- INDEX: (cvat_hash)
- FK: cv_pid → [cv].cv_pid
- BUSINESS RULE: cvat_aterm must be a valid ATerm structure
- BUSINESS RULE: cvat_hash = SHA256(canonical(cvat_aterm))
- BUSINESS RULE: Content-addressable storage enables structural sharing

**ATerm Examples:**

- Integer:     `{"type": "int", "value": 42}`
- String:      `{"type": "string", "value": "hello"}`
- Constructor: `{"type": "appl", "constructor": "Add", "args": [<term1>, <term2>]}`
- List:        `{"type": "list", "elements": [<term1>, <term2>, <term3>]}`
- Tuple:       `{"type": "tuple", "elements": [<term1>, <term2>]}`
- Annotated:   `{"type": "annotated", "term": <term>, "annotations": {...}}`

---

### [prop] Propagator

An autonomous computational unit that watches input cells and produces outputs.

```text
[prop] Propagator:
  - prop_pid         : UUID <<PK>>                # Physical ID: Internal database identifier
  - prop_id          : UUID <<UNIQUE>>            # Logical ID: Globally unique propagator identifier
  - prop_type        : Enum (PRIMITIVE, COMPOUND, CONDITIONAL, CONSTRAINT)
  - prop_name        : String                     # Indefifier
  
  - prom_description : String?
  - prop_created_at  : Timestamp = CURRENT_TIMESTAMP
  - prop_deleted_at  : Timestamp? = NULL          # Soft-delete timestamp
  - prop_enabled     : Boolean = true             # Can be toggled off without deleting
```

**Field Roles:**

- `prop_pid`: Physical ID - Primary key for internal database operations and foreign key references
- `prop_id`: Logical ID - External stable identifier; used in UNIX socket protocol; survives migrations
- `prop_type`: Discriminator for single-table inheritance; determines which variant table has implementation details
- `prop_name`: Human-readable label for debugging and network visualization; optional for anonymous propagators
- `prop_created_at`: When propagator was added to network
- `prop_deleted_at`: Soft deletion; propagator removed but execution history preserved
- `prop_enabled`: Runtime toggle; disable temporarily without losing configuration; useful for debugging

Constraints:

- UNIQUE: (prop_id)
- CHECK: prop_type IN ('PRIMITIVE', 'COMPOUND', 'CONDITIONAL', 'CONSTRAINT')
- BUSINESS RULE: Exactly one variant row must exist based on prop_type
- BUSINESS RULE: Propagator behavior must be monotonic with respect to cell value lattice

---

### [propp] PrimitivePropagator (variant of Propagator)

A basic computational propagator with a defined function implementation.

```text
[propp] PrimitivePropagator:
  - propp_pid            : UUID <<PK>>                  # Physical ID: Internal database identifier
  - prop_pid             : UUID <<FK prop>> <<UNIQUE>>  # Parent Propagator Physical ID (1:1 relationship)
  - propp_function_name  : String                       # Function identifier (e.g., "parse", "lint", "diff")
  - propp_implementation : TEXT                         # Code or reference to implementation
  - propp_language       : Enum (BUILTIN, JAVASCRIPT, PYTHON, SQL); SCRIPT/AI/CONST
```

**Field Roles:**

- `propp_pid`: Physical ID - Primary key for this variant record
- `prop_pid`: Links to parent Propagator (via Physical ID); enforces 1:1 relationship
- `propp_function_name`: Identifies the computation (e.g., "parse_typescript", "compute_diagnostics")
- `propp_implementation`: The actual code to execute; can be code string or reference to module/function
- `propp_language`: Runtime environment; determines how to execute the implementation

Constraints:

- UNIQUE: (prop_pid)
- FK: prop_pid → [prop].prop_pid
- CHECK: propp_language IN ('BUILTIN', 'JAVASCRIPT', 'PYTHON', 'SQL')
- BUSINESS RULE: Implementation must respect monotonicity
- BUSINESS RULE: Must handle partial information gracefully

---

### [propc] CompoundPropagator (variant of Propagator)

A propagator composed of a sub-network of other propagators.

```text
[propc] CompoundPropagator:
  - propc_pid         : UUID <<PK>>                  # Physical ID: Internal database identifier
  - prop_pid          : UUID <<FK prop>> <<UNIQUE>>  # Parent Propagator Physical ID (1:1 relationship)
```

**Field Roles:**

- `propc_pid`: Physical ID - Primary key for this variant record
- `prop_pid`: Links to parent Propagator (via Physical ID); enforces 1:1 relationship

Constraints:

- UNIQUE: (prop_pid)
- FK: prop_pid → [prop].prop_pid
- BUSINESS RULE: Has internal cells and propagators forming a sub-network
- BUSINESS RULE: Input/output cells are exposed through PropagatorPort connections

---

### [propco] ConditionalPropagator (variant of Propagator)

A propagator that activates based on a condition cell.

```text
[propco] ConditionalPropagator:
  - propco_pid                : UUID <<PK>>                 # Physical ID: Internal database identifier
  - prop_pid                  : UUID <<FK prop>> <<UNIQUE>> # Parent Propagator Physical ID (1:1 relationship)
  - propco_condition_cell_pid : UUID <<FK cell>>            # Cell containing the boolean condition
  - propco_true_prop_pid      : UUID <<FK prop>>?           # Propagator to activate if condition is true
  - propco_false_prop_pid     : UUID <<FK prop>>?           # Propagator to activate if condition is false
```

**Field Roles:**

- `propco_pid`                : Physical ID - Primary key for this variant record
- `prop_pid`                  : Links to parent Propagator (via Physical ID); enforces 1:1 relationship
- `propco_condition_cell_pid` : Watches this cell (Physical ID) for boolean value; triggers conditional activation
- `propco_true_prop_pid`      : Delegate propagator (Physical ID) for true branch; NULL = no action on true
- `propco_false_prop_pid`     : Delegate propagator (Physical ID) for false branch; NULL = no action on false

Constraints:

- UNIQUE: (prop_pid)
- FK: prop_pid → [prop].prop_pid
- FK: propco_condition_cell_pid → [cell].cell_pid
- FK: propco_true_prop_pid → [prop].prop_pid
- FK: propco_false_prop_pid → [prop].prop_pid
- BUSINESS RULE: Condition cell value determines which sub-propagator activates

---

### [propcs] ConstraintPropagator (variant of Propagator)

A bidirectional propagator that maintains a constraint relationship between cells.

```text
[propcs] ConstraintPropagator:
  - propcs_pid             : UUID <<PK>>                         # Physical ID: Internal database identifier
  - prop_pid               : UUID <<FK prop>> <<UNIQUE>>         # Parent Propagator Physical ID (1:1 relationship)
  - propcs_constraint_type : Enum (EQUALITY, INEQUALITY, CUSTOM)
  - propcs_expression      : TEXT                                # Constraint expression or description
```

**Field Roles:**

- `propcs_pid`: Physical ID - Primary key for this variant record
- `prop_pid`: Links to parent Propagator (via Physical ID); enforces 1:1 relationship
- `propcs_constraint_type`: Category of constraint; determines bidirectional propagation semantics and which algorithm to use
  - `EQUALITY`: Cells must have identical values (e.g., `A = B`)
  - `INEQUALITY`: Cells must satisfy ordering constraint (e.g., `A < B`, `A ≤ B`)
  - `CUSTOM`: User-defined constraint with custom projection functions
- `propcs_expression`: Formal constraint definition; format depends on constraint_type

**Semantics:**

Unlike unidirectional propagators (which compute outputs from inputs), constraint propagators enforce **relationships** between cells.
They can propagate information in **any direction** to maintain consistency.

For a constraint like `C = (F - 32) × 5/9` (Celsius/Fahrenheit conversion):

- If `F` is known → compute and add value to `C`
- If `C` is known → compute and add value to `F`
- If both are known → verify consistency; add CONTRADICTION if inconsistent
- If neither known → no action (waiting for information)

**Constraint Enforcement Algorithm:**

```text
ON cell_change(any connected cell):
  1. Collect current values from all connected cells
  2. Determine which cells have sufficient information
  3. Attempt to infer missing cell values from known ones using constraint
  4. For each inferable cell:
     a. Compute new value based on constraint projection
     b. Add computed value to target cell
     c. Trigger merge if cell already has values
  5. If all cells have values:
     a. Verify constraint is satisfied
     b. If violated → add CONTRADICTION to all participating cells
     c. If satisfied → no action (idempotent)
```

**Bidirectional Propagation:**

The constraint propagator maintains multiple **projection functions**:

- For `C = (F - 32) × 5/9`, we have:
  - `project_C(F) = (F - 32) × 5/9`  # Compute C from F
  - `project_F(C) = C × 9/5 + 32`    # Compute F from C

When a cell changes:

1. Identify which projection can be applied (which unknowns can be computed)
2. Compute missing values using appropriate projection
3. Add results to appropriate cells (triggering further propagation)

**Constraint Type Algorithms:**

**1. EQUALITY Constraint** (`A = B`):

```text
Algorithm:
  ON change to any cell in {A, B, ...}:
    1. Get value from changed cell
    2. For each other cell in constraint:
       a. If cell has no value → add the value
       b. If cell has different value → CONTRADICTION
       c. If cell has same value → idempotent (no-op)
    
Merge behavior:
  - All cells must converge to same value
  - Different values → CONTRADICTION
  
Example uses:
  - Linking multiple references to same entity
  - Synchronizing duplicate data structures
  - Identity constraints
```

**2. INEQUALITY Constraint** (`min ≤ value ≤ max`):

```text
Algorithm:
  ON change to any bound cell:
    1. Collect all bound values (min, max, known values)
    2. Compute feasible range: [max(all_mins), min(all_maxs)]
    3. If range is empty → CONTRADICTION
    4. For each constrained cell:
       a. If cell has interval → intersect with feasible range
       b. If cell has point value → verify it's in range
       c. Add refined bound to cell
    
Merge behavior:
  - Narrow intervals (intersection)
  - CONTRADICTION if bounds become infeasible
  
Example uses:
  - Numeric range constraints
  - Ordering relationships (A < B < C)
  - Interval arithmetic
```

**3. CUSTOM Constraint** (arbitrary algebraic relationship):

```text
Algorithm:
  ON change to any cell:
    1. Identify knowns and unknowns
    2. For each unknown cell:
       a. Check if projection function exists for this configuration
       b. If exists → compute value and add to unknown cell
       c. If multiple projections exist → apply all (values will merge)
    3. If all cells known → verify constraint satisfied
    
Projection function selection:

  - Constraint: area = width × height
  - Knowns: {area, width} → Can compute height = area / width
  - Knowns: {area, height} → Can compute width = area / height  
  - Knowns: {width, height} → Can compute area = width × height
  - Knowns: {area} → Cannot compute (underdetermined)
  
Merge behavior:

  - Multiple projections may produce different values → merge
  - If projections conflict → CONTRADICTION
  
Example uses:

  - Algebraic equations (Ohm's law: V = I × R)
  - Geometric constraints (Pythagorean theorem)
  - Unit conversions
  - Dependency relationships

```

**Monotonicity Guarantee:**

All projection functions must preserve monotonicity:

- More specific input → More specific (or same) output
- Never: Specific input → Less specific output

For intervals:

- `[10, 20]` (input) → `[50, 100]` (output) ✓ Valid
- `[10, 20]` (input) → `[-∞, +∞]` (output) ✗ Violates monotonicity

**Example: Temperature Constraint Network**

```text
Cells: [Celsius], [Fahrenheit]
Constraint: propcs_expression = "C = (F - 32) * 5/9"

Scenario 1: User adds F=212
  → Constraint fires: project_C(212) = (212 - 32) × 5/9 = 100
  → Adds value 100 to [Celsius] cell

Scenario 2: User adds C=0 (to clean slate)
  → Constraint fires: project_F(0) = 0 × 9/5 + 32 = 32
  → Adds value 32 to [Fahrenheit] cell

Scenario 3: User adds F=212 then adds C=50
  → First addition computes C=100, adds to [Celsius]
  → Second addition attempts to add C=50
  → Merge: 100 vs 50 → CONTRADICTION (conflicting information)

Scenario 4: User adds interval F=[32, 212]
  → Constraint computes C = [(32-32)×5/9, (212-32)×5/9] = [0, 100]
  → Adds interval [0, 100] to [Celsius]
  → Constraint is bidirectional: narrowing C narrows F automatically
```

Constraints:

- UNIQUE: (prop_pid)
- FK: prop_pid → [prop].prop_pid
- CHECK: propcs_constraint_type IN ('EQUALITY', 'INEQUALITY', 'CUSTOM')
- BUSINESS RULE: Can propagate information in any direction to satisfy constraint
- BUSINESS RULE: Must maintain consistency across all connected cells
- BUSINESS RULE: Projection functions must be monotonic with respect to information lattice
- BUSINESS RULE: Must detect and signal constraint violations via CONTRADICTION
- BUSINESS RULE: Constraint propagation is symmetric: all cells are peers (no input/output distinction)

---

### [pconn] PropagatorConnection

Connects propagators to cells as inputs or outputs.

```text
[pconn] PropagatorConnection:
  - pconn_pid        : UUID <<PK>>                # Physical ID: Internal database identifier
  - prop_pid         : UUID <<FK prop>>           # Propagator Physical ID being connected
  - cell_pid         : UUID <<FK cell>>           # Cell Physical ID being connected
  - pconn_role       : Enum (INPUT, OUTPUT)       # Whether propagator reads or writes this cell
  - pconn_name       : String?                    # Named port (e.g., "source_code", "diagnostics")
  - pconn_required   : Boolean = true             # Is this input required for activation?
  - pconn_created_at : Timestamp = CURRENT_TIMESTAMP
```

**Field Roles:**

- `pconn_pid`: Physical ID - Primary key for internal database operations
- `prop_pid`: Which propagator is connected (Physical ID); one propagator has many connections
- `cell_pid`: Which cell is connected (Physical ID); one cell can have many connections (multiple readers/writers)
- `pconn_role`: INPUT = propagator reads from cell; OUTPUT = propagator writes to cell
- `pconn_name`: Named port for multi-input/output propagators; enables distinguishing "operand1" vs "operand2"
- `pconn_required`: For INPUT, whether propagator can run without this input; enables optional inputs
- `pconn_created_at`: When connection was established

Constraints:

- UNIQUE: (prop_pid, cell_pid, pconn_role)
- UNIQUE: (prop_pid, pconn_name, pconn_role) WHERE pconn_name IS NOT NULL
- CHECK: pconn_role IN ('INPUT', 'OUTPUT')
- INDEX: (prop_pid, pconn_role)
- INDEX: (cell_pid, pconn_role)
- FK: prop_pid → [prop].prop_pid
- FK: cell_pid → [cell].cell_pid
- BUSINESS RULE: A propagator can have multiple input and output cells
- BUSINESS RULE: Multiple propagators can read from the same cell
- BUSINESS RULE: Multiple propagators can write to the same cell (values merge)

---

## Runtime Execution Model

**Note**: Runtime execution objects (notifications, executions, merge records, scheduler state) are defined in **`agentic-propagator-runtime-plan.md`**.

These include:
- `[pnot]` PropagatorNotification - Activation queue
- `[pexec]` PropagatorExecution - Execution records
- `[pexecin]` PropagatorExecutionInput - Input provenance
- `[pexecout]` PropagatorExecutionOutput - Output provenance
- `[merge]` MergeOperation - Merge operation records
- `[sched]` SchedulerState - Scheduler status

See the runtime plan for execution semantics, scheduling strategies, provenance queries, and debugging guidance.

---

### [net] PropagatorNetwork

A named collection of cells and propagators forming a computational network.

```text
[net] PropagatorNetwork:
  - net_pid          : UUID <<PK>>                # Physical ID: Internal database identifier
  - net_id           : UUID <<UNIQUE>>            # Logical ID: Globally unique network identifier
  - net_name         : String <<UNIQUE>>          # Human-readable network name
  - net_description  : TEXT?                      # Description of network's purpose
  - net_created_at   : Timestamp = CURRENT_TIMESTAMP
  - net_deleted_at   : Timestamp? = NULL          # Soft-delete timestamp
```

**Field Roles:**

- `net_pid`: Physical ID - Primary key for internal database operations and foreign key references
- `net_id`: Logical ID - External stable identifier; used in UNIX socket protocol
- `net_name`: Human-readable name; unique identifier for CLI/UI; e.g., "typescript_analyzer"
- `net_description`: Documentation of what this network computes
- `net_created_at`: When network was created
- `net_deleted_at`: Soft deletion; network can be archived with history

Constraints:

- UNIQUE: (net_id)
- UNIQUE: (net_name) WHERE net_deleted_at IS NULL

---

### [netcell] NetworkCell

Membership of cells in networks, with role designation.

```text
[netcell] NetworkCell:
  - netcell_pid      : UUID <<PK>>                # Physical ID: Internal database identifier
  - net_pid          : UUID <<FK net>>            # Parent network Physical ID
  - cell_pid         : UUID <<FK cell>>           # Cell Physical ID that's part of this network
  - netcell_role     : Enum (INTERNAL, INPUT, OUTPUT, BOTH)
  - netcell_name     : String?                    # Named port within network context
```

**Field Roles:**

- `netcell_pid`: Physical ID - Primary key for internal database operations
- `net_pid`: Which network this cell belongs to (Physical ID); cells can be in multiple networks
- `cell_pid`: Which cell is a member (Physical ID)
- `netcell_role`: INTERNAL = private cell; INPUT = network accepts data here; OUTPUT = network produces data here; BOTH = bidirectional
- `netcell_name`: Name within network's namespace (e.g., "source_file", "ast"); enables referring to cells by name

Constraints:

- UNIQUE: (net_pid, cell_pid)
- UNIQUE: (net_pid, netcell_name) WHERE netcell_name IS NOT NULL
- CHECK: netcell_role IN ('INTERNAL', 'INPUT', 'OUTPUT', 'BOTH')
- FK: net_pid → [net].net_pid
- FK: cell_pid → [cell].cell_pid
- BUSINESS RULE: INPUT/OUTPUT/BOTH cells form the network's external interface

---

### [netprop] NetworkPropagator

Membership of propagators in networks.

```text
[netprop] NetworkPropagator:
  - netprop_pid      : UUID <<PK>>                # Physical ID: Internal database identifier
  - net_pid          : UUID <<FK net>>            # Parent network Physical ID
  - prop_pid         : UUID <<FK prop>>           # Propagator Physical ID that's part of this network
```

**Field Roles:**

- `netprop_pid`: Physical ID - Primary key for internal database operations
- `net_pid`: Which network this propagator belongs to (Physical ID)
- `prop_pid`: Which propagator is a member (Physical ID); propagators can be in multiple networks

Constraints:

- UNIQUE: (net_pid, prop_pid)
- FK: net_pid → [net].net_pid
- FK: prop_pid → [prop].prop_pid
- BUSINESS RULE: All cells connected to network propagators should be in the network

---

## Core Principles

### Monotonicity Guarantees

The propagator model is built on monotonicity—information can only become **more specific**, never less:

- **Cell values**: Only accumulate more specific information (moving down the information lattice)
- **Propagators**: Given the same inputs, always produce the same or more specific outputs (never less specific)
- **Merges**: Never lose information; only combine compatible information or detect contradictions
- **Versions**: Monotonically increasing; later versions never invalidate earlier ones; support time-travel

**Information Lattice:**

```
                    NOTHING (⊤ - no information)
                       ↓
              Partial Information
            (intervals, constraints)
                       ↓
            Specific Values (42, "hello")
                       ↓
              CONTRADICTION (⊥ - conflicting information)
```

Moving down the lattice is allowed (NOTHING → 42 → CONTRADICTION), but never up (42 ↛ NOTHING).

### Change Detection for TextDocuments

When a `TextDocument` changes:

1. New `TextDocumentVersion` is created with updated piece table
2. New `CellValueTextDocument` references the new version
3. All propagators connected to that cell receive notifications (see runtime model)
4. Propagators can diff versions to determine what changed
5. Incremental processing possible via line/column indices

### ATerm Unification

When two `ATerm` values merge:

- If structurally identical: idempotent (same value)
- If compatible: unify (e.g., variable binding + concrete value)
- If incompatible: produce `CONTRADICTION`
- Structural sharing via content-addressable storage (SHA256 hashing)

**Execution details** (propagation flow, scheduling, provenance) are in **`agentic-propagator-runtime-plan.md`**.

---

## Example Use Cases

### 1. Code Analysis Pipeline

```text
[Source File (TextDocument)] 
    → [Parser Propagator] 
        → [AST (ATerm)] 
            → [Type Checker Propagator] 
                → [Type Info (ATerm)]
            → [Linter Propagator] 
                → [Diagnostics (ATerm)]
```

### 2. Bidirectional Constraint

```text
[Celsius Cell] ←→ [Temperature Constraint Propagator] ←→ [Fahrenheit Cell]
```

Either cell can be updated; constraint maintains consistency.

### 3. Multi-Strategy Problem Solving

```text
[Problem (ATerm)]
    → [Strategy A Propagator] → [Solution Cell]
    → [Strategy B Propagator] → [Solution Cell]
    → [Strategy C Propagator] → [Solution Cell]
```
Multiple strategies contribute partial solutions that merge.

### 4. Incremental Document Processing

```text
[Document (TextDocument v1)] → [Analyzer] → [Results v1]
[Document (TextDocument v2)] → [Analyzer] → [Results v2] (incremental)
```
Analyzer detects changed regions and processes incrementally.

---

## Implementation Notes

### Content-Addressable Storage

- Both `ATerm` and `TextDocumentBuffer` use SHA256 hashes
- Enables deduplication and structural sharing
- Immutability allows safe sharing across versions

### PieceTable Implementation

- Original buffer stores initial document content
- Add buffers store inserted text
- Pieces reference ranges in buffers
- Line index enables efficient position calculations
- Versions are cheap (copy-on-write pieces)

### Naming Convention

- **Physical ID (`_pid`)**: Internal database identifier; primary keys and foreign keys
- **Logical ID (`_id`)**: External stable identifier; used in UNIX socket protocol
- See **`propagator-naming-convention.md`** for complete naming patterns and migration guidance

---

## Future Extensions

- **Dependency Tracking**: Track premises for values (TMS-style)
- **Backtracking**: Retract premises and recompute
- **Conditional Values**: Values contingent on assumptions
- **Temporal Logic**: Cells with time-varying values
- **Distributed Propagation**: Networks spanning multiple machines
- **Streaming**: Handle infinite sequences of changes
- **Reactive Queries**: SQL-like queries over propagator networks

---

## Related Documentation

This document defines the **core structural data model**. See also:

- **`agentic-propagator-runtime-plan.md`**: Runtime execution model (notifications, executions, merges, scheduler, provenance tracking, monitoring)
- **`propagator-naming-convention.md`**: Physical ID vs Logical ID naming patterns and migration guidance
- **`agent-task.scheduler-plan.md`**: Task scheduler integration (if combining with agent task system)

### Document Organization

```
Core Model (this document):
  - Cells and CellValues (information containers)
  - Propagators (computational units)
  - Connections (network wiring)
  - Networks (collections of cells and propagators)
  - Workspace, Documents, Notebooks (LSP structures)
  - Monotonicity principles

Runtime Model (agentic-propagator-runtime-plan.md):
  - Notifications (activation queue)
  - Executions (runtime records)
  - Merges (combination operations)
  - Scheduler (execution controller)
  - Provenance queries
  - Monitoring and debugging
  - Performance optimization
```