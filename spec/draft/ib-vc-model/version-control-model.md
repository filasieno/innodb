# Version Control System - Data Model

## Overview

A git-inspired version control system specifically designed for versioning notebooks and their contents. The system operates on immutable, content-addressable data structures with efficient structural sharing.

Key principles:

- **Content-addressable storage**: All content identified by SHA256 hash (like git blobs)
- **Immutable snapshots**: Commits represent immutable points in time
- **Structural sharing**: Unchanged content is never duplicated
- **Notebook-aware**: Understands notebook structure (cells can change independently of notebook structure)
- **Cell-level granularity**: Track changes at individual cell level, not just whole notebooks
- **Merge support**: Three-way merge with conflict detection
- **Branch management**: Lightweight branches and tags as references
- **Efficient diffing**: Content-addressable storage enables fast comparison

---

## Core Architecture

The version control system follows git's four-layer architecture:

1. **Content Layer**: Immutable blobs (content-addressable by hash)
2. **Tree Layer**: Snapshots of directory/notebook structure
3. **Commit Layer**: History nodes with metadata and parent links
4. **Reference Layer**: Named pointers (branches, tags, HEAD)

### Notebook Versioning Strategy

Unlike file-based systems, notebooks have special semantics:

- **Notebook structure** (cell order, cell types, metadata) is versioned separately from **cell content**
- When a cell's content changes, only the cell content blob changes
- The notebook structure (tree) may remain unchanged if only cell content changed
- This enables efficient diffing: "cell 3 content changed, but notebook structure stayed the same"

**Example scenario:**

```text
Initial state:
  Notebook "analysis.ipynb"
    ├─ Cell 0 (markdown): "# Introduction"  [blob hash: abc123]
    ├─ Cell 1 (code):     "import pandas"   [blob hash: def456]
    └─ Cell 2 (code):     "df.head()"       [blob hash: ghi789]

User edits Cell 1 → "import pandas as pd"

New state:
  Notebook "analysis.ipynb"  [SAME structure tree hash]
    ├─ Cell 0 (markdown): "# Introduction"     [blob: abc123] (unchanged)
    ├─ Cell 1 (code):     "import pandas as pd" [blob: jkl012] (NEW)
    └─ Cell 2 (code):     "df.head()"          [blob: ghi789] (unchanged)

Result: Only one new blob created; tree is updated to point to new blob
```

---

## Fundamental Entities

### Repository: The Version Control Container

**Role**: Root container for all version-controlled data

A **Repository** is a database that stores all version history, branches, and content.

**Properties**:

- Has a working directory (current checkout state)
- Has a HEAD reference (current branch/commit)
- Contains all commits, trees, blobs, and references
- Can be cloned, pushed, pulled (future: distributed)

---

### Blob: Immutable Content Storage

**Role**: Content-addressable storage for file/cell content

A **Blob** stores raw content (text, code, markdown) identified by its SHA256 hash.

**Analogy**: Like a git blob - immutable content identified by hash; enables structural sharing

**Properties**:

- **Immutable**: Once created, never modified
- **Content-addressable**: Hash determines identity
- **Deduplication**: Same content = same blob
- **Type-agnostic**: Can store any UTF-8 text content

---

### Tree: Snapshot of Structure

**Role**: Directory/notebook structure at a point in time

A **Tree** represents the structure of a notebook or directory, containing references to blobs and other trees.

**Analogy**: Like a git tree - captures directory structure; composed of tree entries

**Properties**:

- **Immutable**: Once created, never modified
- **Content-addressable**: Hash computed from entries
- **Structural sharing**: Unchanged subtrees reused across commits
- **Hierarchical**: Trees can contain other trees (nested directories)

---

### Commit: History Node

**Role**: Snapshot in time with metadata and lineage

A **Commit** represents a specific state of the repository with author, timestamp, message, and parent links.

**Analogy**: Like a git commit - points to a root tree and parent commit(s); forms a DAG

**Properties**:

- **Immutable**: Once created, never modified
- **Content-addressable**: Hash computed from content
- **Lineage**: References parent commit(s) - enables history traversal
- **Metadata**: Author, timestamp, commit message
- **Merge commits**: Can have multiple parents (2+ for merge commits)

---

### Reference: Named Pointer

**Role**: Human-readable pointer to commits

A **Reference** is a named pointer to a commit (branches, tags, HEAD).

**Analogy**: Like git refs - branches are mutable pointers; tags are immutable markers

**Properties**:

- **Mutable** (for branches): Can be updated to point to different commits
- **Immutable** (for tags): Fixed pointer to a specific commit
- **Symbolic** (for HEAD): Can point to another reference (current branch)
- **Namespace**: `refs/heads/*` (branches), `refs/tags/*` (tags)

---

## Data Model

### [repo] Repository

The root container for all version-controlled data.

```text
[repo] Repository:
  - repo_pid                : UUID <<PK>>                    # Physical ID: Internal database identifier
  - repo_id                 : UUID <<UNIQUE>>                # Logical ID: External stable identifier
  - repo_name               : String <<UNIQUE>>              # Human-readable name (e.g., "my-project")
  - repo_description        : TEXT?                          # Optional description
  - repo_working_dir        : String                         # Working directory path
  - repo_head_ref_pid       : UUID <<FK ref>>?               # Current HEAD reference (symbolic)
  - repo_created_at         : Timestamp = CURRENT_TIMESTAMP
  - repo_deleted_at         : Timestamp? = NULL              # Soft-delete timestamp
```

**Field Roles:**

- `repo_pid`: Physical ID - Primary key for internal database operations
- `repo_id`: Logical ID - External stable identifier
- `repo_name`: Unique repository name
- `repo_description`: Documentation of repository purpose
- `repo_working_dir`: Working directory path on filesystem
- `repo_head_ref_pid`: Current HEAD reference (Physical ID); determines current checkout
- `repo_created_at`: When repository was initialized
- `repo_deleted_at`: Soft deletion; repository archived with history

Constraints:

- UNIQUE: (repo_id)
- UNIQUE: (repo_name) WHERE repo_deleted_at IS NULL
- FK: repo_head_ref_pid → [ref].ref_pid

---

### [blob] Blob

Immutable content storage identified by SHA256 hash.

```text
[blob] Blob:
  - blob_pid         : UUID <<PK>>                # Physical ID: Internal database identifier
  - blob_hash        : BYTEA <<UNIQUE>>           # SHA256 hash of content (32 bytes)
  - blob_content     : TEXT                       # Actual content (UTF-8 text)
  - blob_size        : BIGINT                     # Content size in bytes
  - blob_created_at  : Timestamp = CURRENT_TIMESTAMP
```

**Field Roles:**

- `blob_pid`: Physical ID - Primary key for internal database operations
- `blob_hash`: Content-addressable identifier; SHA256(blob_content)
- `blob_content`: The actual content stored (cell code, markdown, file content)
- `blob_size`: Size of content in bytes; enables efficient storage queries
- `blob_created_at`: When blob was first added to repository

Constraints:

- UNIQUE: (blob_hash)
- INDEX: (blob_hash)
- BUSINESS RULE: blob_hash = SHA256(blob_content)
- BUSINESS RULE: Blobs are immutable; never updated after creation
- BUSINESS RULE: Same content → same hash → deduplication

---

### [tree] Tree

Represents a directory or notebook structure at a point in time.

```text
[tree] Tree:
  - tree_pid         : UUID <<PK>>                # Physical ID: Internal database identifier
  - tree_hash        : BYTEA <<UNIQUE>>           # SHA256 hash of tree structure (32 bytes)
  - tree_created_at  : Timestamp = CURRENT_TIMESTAMP
```

**Field Roles:**

- `tree_pid`: Physical ID - Primary key for internal database operations
- `tree_hash`: Content-addressable identifier; SHA256(canonical(tree_entries))
- `tree_created_at`: When tree was first created

Constraints:

- UNIQUE: (tree_hash)
- INDEX: (tree_hash)
- BUSINESS RULE: tree_hash = SHA256(sorted canonical representation of all entries)
- BUSINESS RULE: Trees are immutable; never updated after creation
- BUSINESS RULE: Same structure → same hash → structural sharing

**Hash Computation:**

```text
tree_hash = SHA256(
  SORT(entries by name) THEN 
  FOR EACH entry:
    CONCAT(entry_mode, entry_type, entry_name, entry_object_hash)
)
```

---

### [te] TreeEntry

An entry within a tree (file, blob, subtree, or cell reference).

```text
[te] TreeEntry:
  - te_pid               : UUID <<PK>>                # Physical ID: Internal database identifier
  - tree_pid             : UUID <<FK tree>>           # Parent tree Physical ID
  - te_name              : String                     # Entry name (filename, cell_id, etc.)
  - te_type              : Enum (BLOB, TREE, NOTEBOOK_CELL, NOTEBOOK_METADATA)
  - te_mode              : String                     # Unix-style permissions/type (e.g., "100644", "040000")
  - te_object_hash       : BYTEA                      # Hash of referenced object (blob or tree)
  - te_object_pid        : UUID?                      # Physical ID of referenced object (optional, for resolution)
  - te_sort_order        : INTEGER                    # Position within tree (for ordered structures like notebooks)
```

**Field Roles:**

- `te_pid`: Physical ID - Primary key for internal database operations
- `tree_pid`: Parent tree Physical ID; one tree has many entries
- `te_name`: Entry name (e.g., "cell_0", "analysis.ipynb", "README.md")
- `te_type`: Discriminator for entry type
  - `BLOB`: Regular file content
  - `TREE`: Subdirectory
  - `NOTEBOOK_CELL`: Reference to a notebook cell's content blob
  - `NOTEBOOK_METADATA`: Notebook-level metadata blob
- `te_mode`: Unix-style mode (e.g., "100644" for file, "040000" for directory, "100755" for executable)
- `te_object_hash`: Hash of the referenced blob or tree (for content-addressable lookup)
- `te_object_pid`: Cached Physical ID for efficient joins (can be recomputed from hash)
- `te_sort_order`: Position within tree; important for notebooks (cell order matters)

Constraints:

- UNIQUE: (tree_pid, te_name)
- UNIQUE: (tree_pid, te_sort_order) WHERE te_sort_order IS NOT NULL
- INDEX: (tree_pid, te_sort_order)
- INDEX: (te_object_hash)
- FK: tree_pid → [tree].tree_pid
- CHECK: te_type IN ('BLOB', 'TREE', 'NOTEBOOK_CELL', 'NOTEBOOK_METADATA')
- BUSINESS RULE: te_object_hash must reference a valid blob_hash or tree_hash
- BUSINESS RULE: For notebooks, te_sort_order determines cell execution order

**Notebook Structure Example:**

```text
Tree for "analysis.ipynb":
  - TreeEntry: name="metadata",  type=NOTEBOOK_METADATA, object_hash=<blob_hash>, sort_order=0
  - TreeEntry: name="cell_0",    type=NOTEBOOK_CELL,     object_hash=<blob_hash>, sort_order=1
  - TreeEntry: name="cell_1",    type=NOTEBOOK_CELL,     object_hash=<blob_hash>, sort_order=2
  - TreeEntry: name="cell_2",    type=NOTEBOOK_CELL,     object_hash=<blob_hash>, sort_order=3
```

When cell_1 content changes, only its object_hash changes; other entries remain identical.

---

### [commit] Commit

A snapshot in time with metadata and lineage.

```text
[commit] Commit:
  - commit_pid          : UUID <<PK>>                  # Physical ID: Internal database identifier
  - commit_hash         : BYTEA <<UNIQUE>>             # SHA256 hash of commit (32 bytes)
  - commit_tree_pid     : UUID <<FK tree>>             # Root tree Physical ID (snapshot)
  - commit_author       : String                       # Author name
  - commit_email        : String                       # Author email
  - commit_timestamp    : Timestamp                    # When commit was created
  - commit_message      : TEXT                         # Commit message
  - commit_created_at   : Timestamp = CURRENT_TIMESTAMP # Database creation time
```

**Field Roles:**

- `commit_pid`: Physical ID - Primary key for internal database operations
- `commit_hash`: Content-addressable identifier; SHA256(tree_hash + parent_hashes + author + timestamp + message)
- `commit_tree_pid`: Root tree Physical ID; represents state of repository at this commit
- `commit_author`: Who created this commit
- `commit_email`: Author's email
- `commit_timestamp`: When commit was created (user-provided, can be historical)
- `commit_message`: Human-readable description of changes
- `commit_created_at`: Database creation timestamp (system time)

Constraints:

- UNIQUE: (commit_hash)
- INDEX: (commit_hash)
- INDEX: (commit_timestamp DESC)
- FK: commit_tree_pid → [tree].tree_pid
- BUSINESS RULE: commit_hash = SHA256(commit_tree_hash + sorted_parent_hashes + author + email + timestamp + message)
- BUSINESS RULE: Commits are immutable; never updated after creation

**Hash Computation:**

```text
commit_hash = SHA256(
  "tree " + tree_hash + "\n" +
  FOR EACH parent: "parent " + parent_hash + "\n" +
  "author " + author + " <" + email + "> " + timestamp + "\n" +
  "committer " + author + " <" + email + "> " + timestamp + "\n" +
  "\n" +
  message
)
```

---

### [cp] CommitParent

Links commits to their parent commits (forms the commit DAG).

```text
[cp] CommitParent:
  - cp_pid              : UUID <<PK>>                  # Physical ID: Internal database identifier
  - commit_pid          : UUID <<FK commit>>           # Child commit Physical ID
  - parent_commit_pid   : UUID <<FK commit>>           # Parent commit Physical ID
  - cp_parent_index     : INTEGER                      # Parent index (0=first parent, 1=merge parent)
```

**Field Roles:**

- `cp_pid`: Physical ID - Primary key for internal database operations
- `commit_pid`: Child commit Physical ID; this commit has parents
- `parent_commit_pid`: Parent commit Physical ID; predecessor in history
- `cp_parent_index`: Order of parents; 0=primary parent (branch history), 1+=merge parents

Constraints:

- UNIQUE: (commit_pid, parent_commit_pid)
- UNIQUE: (commit_pid, cp_parent_index)
- INDEX: (commit_pid)
- INDEX: (parent_commit_pid)
- FK: commit_pid → [commit].commit_pid
- FK: parent_commit_pid → [commit].commit_pid
- BUSINESS RULE: Most commits have 1 parent (linear history)
- BUSINESS RULE: Merge commits have 2+ parents
- BUSINESS RULE: Initial commit has 0 parents (root of DAG)

---

### [ref] Reference

Named pointer to commits (branches, tags, HEAD).

```text
[ref] Reference:
  - ref_pid             : UUID <<PK>>                  # Physical ID: Internal database identifier
  - repo_pid            : UUID <<FK repo>>             # Repository Physical ID
  - ref_name            : String                       # Reference name (e.g., "refs/heads/main", "refs/tags/v1.0")
  - ref_type            : Enum (BRANCH, TAG, SYMBOLIC)
  - ref_target_type     : Enum (COMMIT, REFERENCE)     # What this ref points to
  - ref_target_commit_pid : UUID <<FK commit>>?        # Target commit Physical ID (for direct refs)
  - ref_target_ref_pid    : UUID <<FK ref>>?           # Target reference Physical ID (for symbolic refs like HEAD)
  - ref_created_at      : Timestamp = CURRENT_TIMESTAMP
  - ref_updated_at      : Timestamp = CURRENT_TIMESTAMP
  - ref_deleted_at      : Timestamp? = NULL            # Soft-delete for deleted branches
```

**Field Roles:**

- `ref_pid`: Physical ID - Primary key for internal database operations
- `repo_pid`: Repository Physical ID; refs are scoped to repository
- `ref_name`: Full reference name following git convention (e.g., "refs/heads/main", "refs/tags/v1.0", "HEAD")
- `ref_type`: 
  - `BRANCH`: Mutable pointer to commit (can be updated)
  - `TAG`: Immutable pointer to commit (fixed)
  - `SYMBOLIC`: Points to another reference (e.g., HEAD → refs/heads/main)
- `ref_target_type`: What this reference points to (COMMIT or REFERENCE)
- `ref_target_commit_pid`: If target_type=COMMIT, this is the commit Physical ID
- `ref_target_ref_pid`: If target_type=REFERENCE, this is the reference Physical ID (symbolic)
- `ref_created_at`: When reference was created
- `ref_updated_at`: Last time reference was updated (for branches)
- `ref_deleted_at`: Soft deletion for deleted branches

Constraints:

- UNIQUE: (repo_pid, ref_name) WHERE ref_deleted_at IS NULL
- INDEX: (repo_pid, ref_type)
- INDEX: (ref_target_commit_pid)
- FK: repo_pid → [repo].repo_pid
- FK: ref_target_commit_pid → [commit].commit_pid
- FK: ref_target_ref_pid → [ref].ref_pid
- CHECK: ref_type IN ('BRANCH', 'TAG', 'SYMBOLIC')
- CHECK: ref_target_type IN ('COMMIT', 'REFERENCE')
- CHECK: (ref_target_type = 'COMMIT' AND ref_target_commit_pid IS NOT NULL) OR
         (ref_target_type = 'REFERENCE' AND ref_target_ref_pid IS NOT NULL)
- BUSINESS RULE: Tags cannot be updated (ref_updated_at = ref_created_at for tags)
- BUSINESS RULE: Branches can be updated (ref_updated_at changes)
- BUSINESS RULE: HEAD is typically symbolic, pointing to current branch

**Reference Namespace:**

```text
refs/heads/*    - Branches (local)
refs/tags/*     - Tags
refs/remotes/*  - Remote-tracking branches (future)
HEAD            - Symbolic reference to current branch
```

---

### [rh] ReferenceHistory

Audit trail of reference changes (reflog).

```text
[rh] ReferenceHistory:
  - rh_pid                 : UUID <<PK>>                  # Physical ID: Internal database identifier
  - ref_pid                : UUID <<FK ref>>              # Reference Physical ID
  - rh_from_commit_pid     : UUID <<FK commit>>?          # Previous commit (NULL for creation)
  - rh_to_commit_pid       : UUID <<FK commit>>           # New commit
  - rh_action              : Enum (CREATE, UPDATE, DELETE, RESET, MERGE, REBASE, CHECKOUT)
  - rh_actor               : String                       # Who performed the action
  - rh_message             : TEXT?                        # Optional message describing action
  - rh_timestamp           : Timestamp = CURRENT_TIMESTAMP
```

**Field Roles:**

- `rh_pid`: Physical ID - Primary key for internal database operations
- `ref_pid`: Reference Physical ID; which reference was modified
- `rh_from_commit_pid`: Previous commit (Physical ID); NULL for creation
- `rh_to_commit_pid`: New commit (Physical ID); where reference now points
- `rh_action`: Type of action performed
- `rh_actor`: Who performed the action (user name)
- `rh_message`: Optional description of action (e.g., "commit: Initial commit")
- `rh_timestamp`: When action occurred

Constraints:

- INDEX: (ref_pid, rh_timestamp DESC)
- INDEX: (rh_timestamp DESC)
- FK: ref_pid → [ref].ref_pid
- FK: rh_from_commit_pid → [commit].commit_pid
- FK: rh_to_commit_pid → [commit].commit_pid
- CHECK: rh_action IN ('CREATE', 'UPDATE', 'DELETE', 'RESET', 'MERGE', 'REBASE', 'CHECKOUT')
- BUSINESS RULE: Enables "reflog" functionality - undo/recovery of lost commits
- BUSINESS RULE: Never deleted (append-only audit log)

---

## Operations

### Commit Operation

Creating a new commit:

1. **Stage changes**: Identify modified files/cells
2. **Create blobs**: For each modified file/cell, create blob (if not exists)
3. **Build tree**: Create tree entries referencing blobs
4. **Create tree**: Compute tree hash, insert tree
5. **Create commit**: Reference tree, parent(s), metadata
6. **Update HEAD**: Move current branch reference to new commit
7. **Record reflog**: Add entry to ReferenceHistory

**Notebook-specific optimization:**

- When only cell content changes, reuse existing tree structure
- Only create new tree if cell order/types change
- Most commits: 1-3 new blobs, 1 new tree, 1 new commit

---

### Diff Operation

Comparing two commits:

1. **Resolve commits**: Get commit1 and commit2 by hash/ref
2. **Get trees**: Load root trees for both commits
3. **Compare trees**: Recursively diff tree entries
4. **Identify changes**: Report added/modified/deleted entries
5. **Compute line-level diff**: For modified blobs, compute textual diff

**Notebook-specific diff:**

```text
Input: commit1, commit2
Output: NotebookDiff

For each notebook:
  - Structure changes: Added/removed/reordered cells
  - Content changes: Per-cell diffs (line-level)
  - Metadata changes: Notebook settings changes

Example output:
  analysis.ipynb:
    Structure: Cell 2 moved to position 1
    Cell 0: No changes
    Cell 1: Modified (lines 2-5)
    Cell 2: No changes
```

---

### Merge Operation

Three-way merge:

1. **Find merge base**: Common ancestor of two commits (LCA in DAG)
2. **Diff base → ours**: Changes in our branch
3. **Diff base → theirs**: Changes in their branch
4. **Compute merge**:
   - Non-conflicting: Apply both changes
   - Conflicting: Mark conflict regions
5. **Create merge commit**: Two parents (ours + theirs)

**Conflict Detection:**

```text
Conflict occurs when:
  - Same blob modified in both branches (content conflict)
  - Same cell modified in both branches (notebook conflict)
  - Cell reordered in one branch, modified in another (structure conflict)

Conflict resolution:
  - User must manually resolve
  - Create conflict markers in working directory
  - User edits, then commits merge
```

---

### Checkout Operation

Switch to different commit/branch:

1. **Resolve target**: Get commit by branch/tag/hash
2. **Get tree**: Load root tree for target commit
3. **Update working directory**: Write tree contents to filesystem
4. **Update HEAD**: Point to new commit/branch
5. **Record reflog**: Add checkout entry

**Notebook checkout:**

```text
For each notebook in tree:
  1. Load tree entries (cells)
  2. Resolve blob hashes to content
  3. Reconstruct notebook JSON structure
  4. Write to working directory
```

---

## Notebook-Specific Features

### Cell-Level History

Query history of a specific cell:

```sql
-- Find all commits that modified cell_1 in analysis.ipynb
WITH commit_tree AS (
  SELECT c.commit_pid, c.commit_hash, c.commit_timestamp, c.commit_message,
         te.te_object_hash AS cell_blob_hash
  FROM commit c
  JOIN tree t ON c.commit_tree_pid = t.tree_pid
  JOIN tree_entry te_notebook ON t.tree_pid = te_notebook.tree_pid 
    AND te_notebook.te_name = 'analysis.ipynb'
  JOIN tree t_notebook ON te_notebook.te_object_hash = t_notebook.tree_hash
  JOIN tree_entry te ON t_notebook.tree_pid = te.tree_pid 
    AND te.te_name = 'cell_1'
)
SELECT commit_hash, commit_timestamp, commit_message, cell_blob_hash
FROM commit_tree
ORDER BY commit_timestamp DESC;
```

---

### Blame: Who Changed Each Cell

Find who last modified each cell:

```sql
-- For each cell in current commit, find last author who modified it
WITH RECURSIVE history AS (
  -- Start from HEAD commit
  SELECT c.commit_pid, c.commit_hash, c.commit_author, 
         te.te_name AS cell_name, te.te_object_hash AS cell_hash
  FROM ref r
  JOIN commit c ON r.ref_target_commit_pid = c.commit_pid
  JOIN tree t ON c.commit_tree_pid = t.tree_pid
  JOIN tree_entry te_nb ON t.tree_pid = te_nb.tree_pid 
    AND te_nb.te_name = 'analysis.ipynb'
  JOIN tree t_nb ON te_nb.te_object_hash = t_nb.tree_hash
  JOIN tree_entry te ON t_nb.tree_pid = te.tree_pid
  WHERE r.ref_name = 'HEAD'
  
  UNION ALL
  
  -- Walk back through history
  SELECT c.commit_pid, c.commit_hash, c.commit_author,
         te.te_name, te.te_object_hash
  FROM history h
  JOIN commit_parent cp ON h.commit_pid = cp.commit_pid AND cp.cp_parent_index = 0
  JOIN commit c ON cp.parent_commit_pid = c.commit_pid
  JOIN tree t ON c.commit_tree_pid = t.tree_pid
  JOIN tree_entry te_nb ON t.tree_pid = te_nb.tree_pid 
    AND te_nb.te_name = 'analysis.ipynb'
  JOIN tree t_nb ON te_nb.te_object_hash = t_nb.tree_hash
  JOIN tree_entry te ON t_nb.tree_pid = te.tree_pid AND te.te_name = h.cell_name
  WHERE te.te_object_hash != h.cell_hash -- Cell content changed
)
SELECT DISTINCT ON (cell_name) 
  cell_name, commit_hash, commit_author
FROM history
ORDER BY cell_name, commit_pid DESC;
```

---

### Interactive Rebase (Cell Reordering)

Reorder cells without losing history:

```text
Operation: Reorder cells in notebook
  1. Get current tree for notebook
  2. Modify te_sort_order for affected cells
  3. Create new tree with updated sort orders
  4. Create commit with reorder message
  5. Blob hashes remain unchanged (content preserved)

Result: 
  - Cell content unchanged (same blob hashes)
  - Cell order changed (new tree)
  - Commit message: "Reorder cells: move cell_2 before cell_1"
```

---

## Storage Optimization

### Deduplication

Content-addressable storage eliminates duplicates:

```text
Scenario: 100 notebooks, each has identical cell "import pandas"

Storage:
  - 1 blob: "import pandas" (hash: abc123)
  - 100 tree entries: All reference abc123
  
Savings: 99 blob copies eliminated
```

---

### Incremental Storage

Only store what changed:

```text
Edit sequence:
  Commit 1: Create notebook with 10 cells → 10 blobs, 1 tree, 1 commit
  Commit 2: Edit cell 3 → 1 new blob, 1 new tree, 1 new commit (reuses 9 blobs)
  Commit 3: Edit cell 3 again → 1 new blob, 1 new tree, 1 new commit (reuses 9 blobs)
  
Total: 12 blobs, 3 trees, 3 commits
Git-style efficiency without explicit delta compression
```

---

### Garbage Collection

Prune unreferenced objects:

```sql
-- Find blobs not referenced by any reachable tree
WITH reachable_commits AS (
  -- All commits reachable from any reference
  SELECT DISTINCT commit_pid
  FROM ref r
  JOIN commit c ON r.ref_target_commit_pid = c.commit_pid
  -- (Include recursive parent traversal)
),
reachable_trees AS (
  SELECT DISTINCT tree_pid
  FROM reachable_commits rc
  JOIN commit c ON rc.commit_pid = c.commit_pid
  JOIN tree t ON c.commit_tree_pid = t.tree_pid
  -- (Include recursive tree traversal)
),
reachable_blobs AS (
  SELECT DISTINCT b.blob_pid
  FROM reachable_trees rt
  JOIN tree_entry te ON rt.tree_pid = te.tree_pid
  JOIN blob b ON te.te_object_hash = b.blob_hash
)
DELETE FROM blob
WHERE blob_pid NOT IN (SELECT blob_pid FROM reachable_blobs);
```

---

## Future Extensions

- **Delta Compression**: Store blobs as deltas for even better compression
- **Pack Files**: Bundle related objects for efficient transfer
- **Distributed Sync**: Push/pull between repositories
- **Signed Commits**: Cryptographic verification of authorship
- **Hooks**: Pre-commit, post-commit, pre-push hooks
- **Submodules**: Nested repository references
- **Worktrees**: Multiple working directories for same repository
- **Sparse Checkout**: Partial working directory (only some notebooks)
- **LFS Support**: Large file storage for binary outputs (plots, models)

---

## Implementation Notes

### Hash Computation

All hashes are SHA256 (32 bytes):

```python
def compute_blob_hash(content: str) -> bytes:
    return hashlib.sha256(content.encode('utf-8')).digest()

def compute_tree_hash(entries: List[TreeEntry]) -> bytes:
    # Sort entries by name for canonical ordering
    sorted_entries = sorted(entries, key=lambda e: e.name)
    
    # Build canonical representation
    parts = []
    for entry in sorted_entries:
        parts.append(f"{entry.mode} {entry.type} {entry.name}\0")
        parts.append(entry.object_hash)
    
    canonical = ''.join(parts)
    return hashlib.sha256(canonical.encode('utf-8')).digest()

def compute_commit_hash(commit: Commit) -> bytes:
    parts = [
        f"tree {commit.tree_hash.hex()}\n",
    ]
    for parent in sorted(commit.parent_hashes):
        parts.append(f"parent {parent.hex()}\n")
    parts.extend([
        f"author {commit.author} <{commit.email}> {commit.timestamp.timestamp()}\n",
        f"committer {commit.author} <{commit.email}> {commit.timestamp.timestamp()}\n",
        "\n",
        commit.message
    ])
    
    canonical = ''.join(parts)
    return hashlib.sha256(canonical.encode('utf-8')).digest()
```

---

### Notebook Serialization

Notebooks stored as trees with special structure:

```text
Notebook Tree Structure:
  entry[0]: name="metadata",  type=NOTEBOOK_METADATA, sort_order=0
    → Blob containing: {"kernelspec": {...}, "language_info": {...}, ...}
  
  entry[1]: name="cell_0_metadata", type=NOTEBOOK_CELL_METADATA, sort_order=1
    → Blob containing: {"cell_type": "markdown", "id": "...", "metadata": {...}}
  
  entry[2]: name="cell_0_source", type=NOTEBOOK_CELL, sort_order=2
    → Blob containing: "# Introduction\n\nThis notebook..."
  
  entry[3]: name="cell_1_metadata", type=NOTEBOOK_CELL_METADATA, sort_order=3
    → Blob containing: {"cell_type": "code", "execution_count": 1, ...}
  
  entry[4]: name="cell_1_source", type=NOTEBOOK_CELL, sort_order=4
    → Blob containing: "import pandas as pd\nimport numpy as np"
  
  ... (repeating pattern for each cell)
```

**Benefits:**

- Cell metadata changes independently from cell source
- Granular diff: "execution_count changed" vs "source code changed"
- Efficient storage: Unchanged cells share blobs across commits

---

### Naming Convention

Following the propagator model notation:

- **Physical ID (`_pid`)**: Internal database primary key
- **Logical ID (`_id`)** (where needed): External stable identifier
- **Content Hash (`_hash`)**: Content-addressable identifier (SHA256)

For content-addressable entities (blob, tree, commit):
- No logical ID needed (hash serves as global identifier)
- Physical ID for internal DB operations
- Hash for content-addressable lookups

For references and repositories:
- Both PID (internal) and ID (external) for flexibility

---

## Related Documentation

This document defines the **core version control data model**. See also:

- **`propagator-object-model-plan.md`**: Propagator system that could use this VCS
- **`model-notation.md`**: Naming conventions (Physical ID vs Logical ID)
- **`agentic-propagator-runtime-plan.md`**: Runtime model (if integrating with propagator system)

---

## Summary

This version control system provides:

✅ **Git-compatible architecture**: Blobs, trees, commits, references  
✅ **Notebook-aware versioning**: Cell-level granularity  
✅ **Content-addressable storage**: Automatic deduplication  
✅ **Structural sharing**: Efficient storage across versions  
✅ **Full history**: Complete audit trail with reflog  
✅ **Merge support**: Three-way merge with conflict detection  
✅ **Branch management**: Lightweight branches and tags  
✅ **Query-friendly schema**: SQL queries for history, blame, diff  

The model is specifically designed for notebooks where:
- Cell content changes frequently
- Notebook structure (cell order) changes less frequently
- Efficient diffing at cell level is crucial
- Collaboration requires merge support
- History and provenance are important

