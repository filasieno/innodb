# LSP Object Model

## Workspace Model

### [ws] Workspace

Root entity representing an LSP workspace (collection of folders and configuration).

```text
[ws] Workspace:
  - ws_id            : UUID <<PK>>                      # Internal database identifier
  - ws_uuid          : UUID <<UNIQUE>>                  # Globally unique workspace identifier
  - ws_name          : String <<UNIQUE>>                # Human-readable workspace name
  - ws_created_at    : Timestamp = CURRENT_TIMESTAMP
  - ws_deleted_at    : Timestamp? = NULL                # Soft-delete timestamp
```

**Field Roles:**

- `ws_id`: Primary key for internal database operations
- `ws_uuid`: External stable identifier; used in UNIX socket protocol
- `ws_name`: Human-readable name for UI and CLI; must be unique across active workspaces
- `ws_created_at`: When workspace was first opened/created
- `ws_deleted_at`: Soft deletion; allows historical queries and undo operations

Constraints:

- UNIQUE: (ws_uuid)
- UNIQUE: (ws_name) WHERE ws_deleted_at IS NULL
- BUSINESS RULE: A workspace contains folders, documents, and configuration

---

### [wsf] WorkspaceFolder

An LSP workspace folder within a workspace.

```text
[wsf] WorkspaceFolder:
  - wsf_id           : UUID <<PK>>                    # Internal database identifier
  - ws_id            : UUID <<FK ws>>                 # Parent workspace
  - wsf_uri          : TEXT <<UNIQUE>>                # LSP URI (file:// scheme) of the folder
  - wsf_name         : String                         # Human-readable folder name (shown in UI)
  - wsf_created_at   : Timestamp = CURRENT_TIMESTAMP
  - wsf_deleted_at   : Timestamp? = NULL              # Soft-delete timestamp
```

**Field Roles:**

- `wsf_id`: Primary key for internal database operations
- `ws_id`: Associates folder with workspace; a workspace can have multiple folders
- `wsf_uri`: Canonical file system location; must be valid file:// URI; globally unique
- `wsf_name`: User-visible name; can differ from folder name on disk; unique within workspace
- `wsf_created_at`: When folder was added to workspace
- `wsf_deleted_at`: Soft deletion; folder removed from workspace but history preserved

Constraints:

- UNIQUE: (wsf_uri) WHERE wsf_deleted_at IS NULL
- UNIQUE: (ws_id, wsf_name) WHERE wsf_deleted_at IS NULL
- INDEX: (ws_id, wsf_deleted_at)
- BUSINESS RULE: wsf_uri must be a valid file:// URI

---

### [wscfg] WorkspaceConfiguration

Configuration settings for a workspace or folder.

```text
[wscfg] WorkspaceConfiguration:
  - wscfg_id         : UUID <<PK>>                    # Internal database identifier
  - ws_id            : UUID <<FK ws>>?                # Workspace (NULL = global configuration)
  - wsf_id           : UUID <<FK wsf>>?               # Folder (NULL = workspace-level)
  - wscfg_section    : String                         # Configuration section (e.g., "editor.tabSize")
  - wscfg_value      : JSONB                          # Configuration value (any JSON type)
  - wscfg_updated_at : Timestamp = CURRENT_TIMESTAMP
```

**Field Roles:**

- `wscfg_id`: Primary key for internal database operations
- `ws_id`: Scopes configuration to workspace; NULL = global (all workspaces)
- `wsf_id`: Scopes configuration to specific folder; NULL = workspace-level
- `wscfg_section`: Dotted configuration key (e.g., "editor.tabSize", "files.exclude")
- `wscfg_value`: The actual configuration value; JSONB supports any JSON type (string, number, object, array)
- `wscfg_updated_at`: When configuration was last changed; enables configuration change tracking

Constraints:

- UNIQUE: (ws_id, wsf_id, wscfg_section)
- CHECK: (ws_id IS NOT NULL) OR (wsf_id IS NULL)  # folder config requires workspace
- BUSINESS RULE: NULL ws_id = global, NULL wsf_id = workspace-level, both set = folder-level

---

## LSP Document Model

### [td] TextDocument

LSP-compliant text document with multiversioned PieceTable implementation.

```text
[td] TextDocument:
  - td_id            : UUID <<PK>>                # Internal database identifier
  - td_uuid          : UUID <<UNIQUE>>            # Globally unique document identifier
  - ws_id            : UUID <<FK ws>>             # Workspace containing this document
  - td_uri           : TEXT <<UNIQUE>>            # LSP DocumentUri (file://, untitled:, etc.)
  - td_language_id   : String                     # Language identifier (e.g., "typescript", "python")
  - td_created_at    : Timestamp = CURRENT_TIMESTAMP
  - td_closed_at     : Timestamp?                 # When document was closed (NULL = currently open)
  - td_current_version : INTEGER = 0              # Latest LSP version number
```

**Field Roles:**

- `td_id`: Primary key for internal database operations
- `td_uuid`: External stable identifier; survives URI changes; used for identity
- `ws_id`: Associates document with workspace; enables workspace-scoped queries
- `td_uri`: LSP-compliant URI (file://, untitled:, etc.); globally unique; used by LSP clients
- `td_language_id`: Determines syntax, features, and applicable propagators (e.g., "python" → Python analyzers)
- `td_created_at`: When document was first opened in this workspace
- `td_closed_at`: NULL = document is open; non-NULL = document was closed; enables filtering active documents
- `td_current_version`: Fast lookup of latest version; increments on every LSP textDocument/didChange

Constraints:

- UNIQUE: (td_uuid)
- UNIQUE: (td_uri)
- INDEX: (ws_id, td_closed_at)
- INDEX: (td_uri)
- BUSINESS RULE: td_uri must be valid LSP DocumentUri
- BUSINESS RULE: Text is stored in immutable buffers; edits create new pieces
- BUSINESS RULE: Each version is a complete snapshot of the piece table state
- BUSINESS RULE: Version increments on every change (LSP textDocument/didChange)

---

### [tdv] TextDocumentVersion

A specific version snapshot of a TextDocument's piece table (LSP version).

```text
[tdv] TextDocumentVersion:
  - tdv_id           : UUID <<PK>>                # Internal database identifier
  - td_id            : UUID <<FK td>>             # Parent TextDocument
  - tdv_version      : INTEGER                    # LSP version number (monotonic)
  - tdv_created_at   : Timestamp = CURRENT_TIMESTAMP
  - tdv_total_length : BIGINT                     # Total character length of document
  - tdv_line_count   : BIGINT                     # Total number of lines
  - tdv_encoding     : Enum (UTF8, UTF16, UTF32) = UTF16  # Position encoding for offsets
```

**Field Roles:**

- `tdv_id`: Primary key for internal database operations
- `td_id`: Links version to document; one document has many versions over time
- `tdv_version`: LSP version number; increments on each change; client and server stay synchronized
- `tdv_created_at`: Wall-clock time when this version was created
- `tdv_total_length`: Cached total character count; optimization for length queries
- `tdv_line_count`: Cached line count; optimization for line-based operations
- `tdv_encoding`: How character offsets are counted (UTF-8/16/32); affects Position calculations; LSP default is UTF-16

Constraints:

- UNIQUE: (td_id, tdv_version)
- INDEX: (td_id, tdv_version DESC)
- CHECK: tdv_encoding IN ('UTF8', 'UTF16', 'UTF32')
- BUSINESS RULE: Version numbers are monotonically increasing
- BUSINESS RULE: LSP default encoding is UTF-16

---

### [tdb] TextDocumentBuffer

Immutable append-only buffers that store the actual text content.

```text
[tdb] TextDocumentBuffer:
  - tdb_id           : UUID <<PK>>                # Internal database identifier
  - td_id            : UUID <<FK td>>             # Parent TextDocument
  - tdb_buffer_type  : Enum (ORIGINAL, ADD)      # ORIGINAL = initial text, ADD = inserted text
  - tdb_content      : TEXT                       # The actual text content
  - tdb_created_at   : Timestamp = CURRENT_TIMESTAMP
  - tdb_hash         : BYTEA                      # SHA256 hash for content-addressable storage
```

**Field Roles:**

- `tdb_id`: Primary key for internal database operations
- `td_id`: Associates buffer with document; one document can have many buffers over time
- `tdb_buffer_type`: ORIGINAL = initial content; ADD = subsequent insertions; enables PieceTable algorithm
- `tdb_content`: The raw text; immutable once created; never modified
- `tdb_created_at`: When buffer was created (when text was first added)
- `tdb_hash`: Content-addressable hash; enables deduplication of identical text across edits/documents

Constraints:

- INDEX: (td_id, tdb_created_at)
- INDEX: (tdb_hash)
- CHECK: tdb_buffer_type IN ('ORIGINAL', 'ADD')
- BUSINESS RULE: Buffers are immutable once created
- BUSINESS RULE: Content-addressable by hash enables deduplication

---

### [tdp] TextDocumentPiece

A piece in the piece table pointing to a range in a buffer.

```text
[tdp] TextDocumentPiece:
  - tdp_id           : UUID <<PK>>                # Internal database identifier
  - tdv_id           : UUID <<FK tdv>>            # Version this piece belongs to
  - tdp_sequence     : INTEGER                    # Order in the piece table (0-indexed)
  - tdb_id           : UUID <<FK tdb>>            # Buffer being referenced
  - tdp_buffer_start : BIGINT                     # Start offset in buffer (inclusive)
  - tdp_buffer_end   : BIGINT                     # End offset in buffer (exclusive)
  - tdp_length       : BIGINT                     # Length of this piece (bytes/chars)
```

**Field Roles:**

- `tdp_id`: Primary key for internal database operations
- `tdv_id`: Links piece to specific version; versions share pieces (structural sharing)
- `tdp_sequence`: Defines document order; pieces concatenated in sequence order form the document
- `tdb_id`: Points to immutable buffer; multiple pieces can reference same buffer
- `tdp_buffer_start`: Offset where this piece starts in the buffer (inclusive)
- `tdp_buffer_end`: Offset where this piece ends in the buffer (exclusive)
- `tdp_length`: Precomputed length; optimization to avoid repeated (end - start) calculations

Constraints:

- UNIQUE: (tdv_id, tdp_sequence)
- INDEX: (tdv_id, tdp_sequence)
- CHECK: tdp_buffer_start >= 0
- CHECK: tdp_buffer_end > tdp_buffer_start
- CHECK: tdp_length = (tdp_buffer_end - tdp_buffer_start)
- BUSINESS RULE: Pieces are ordered by tdp_sequence to form the complete document

---

### [tdli] TextDocumentLineIndex

Line and column index for efficient position lookups in a TextDocument version.

```text
[tdli] TextDocumentLineIndex:
  - tdli_id          : UUID <<PK>>                # Internal database identifier
  - tdv_id           : UUID <<FK tdv>>            # Version this index applies to
  - tdli_line_number : BIGINT                     # 0-indexed line number
  - tdli_char_offset : BIGINT                     # Character offset where this line starts
  - tdli_char_length : BIGINT                     # Length of this line (excluding newline)
  - tdli_column_count: BIGINT                     # Number of columns/characters in line
```

**Field Roles:**

- `tdli_id`: Primary key for internal database operations
- `tdv_id`: Links index to specific version; each version has its own line index
- `tdli_line_number`: Line number (0-indexed per LSP); enables line → offset lookup
- `tdli_char_offset`: Absolute character position where line starts; enables offset → line lookup
- `tdli_char_length`: Line length without newline; optimization for line queries
- `tdli_column_count`: Columns in line; may differ from char_length with multi-byte characters

Constraints:

- UNIQUE: (tdv_id, tdli_line_number)
- INDEX: (tdv_id, tdli_char_offset)
- CHECK: tdli_line_number >= 0
- CHECK: tdli_char_offset >= 0
- BUSINESS RULE: Enables O(log n) line/column → offset conversions

---

## NotebookDocument Model

### [nd] NotebookDocument

LSP-compliant notebook document containing cells of code and markup.

```text
[nd] NotebookDocument:
  - nd_id            : UUID <<PK>>                # Internal database identifier
  - nd_uuid          : UUID <<UNIQUE>>            # Globally unique notebook identifier
  - ws_id            : UUID <<FK ws>>             # Workspace containing this notebook
  - nd_uri           : TEXT <<UNIQUE>>            # LSP URI for the notebook (e.g., file://*.ipynb)
  - nd_notebook_type : String                     # Notebook type (e.g., "jupyter-notebook")
  - nd_created_at    : Timestamp = CURRENT_TIMESTAMP
  - nd_closed_at     : Timestamp?                 # When notebook was closed (NULL = currently open)
  - nd_current_version : INTEGER = 0              # Latest LSP version number
  - nd_metadata      : JSONB?                     # Notebook-level metadata (kernel info, etc.)
```

**Field Roles:**

- `nd_id`: Primary key for internal database operations
- `nd_uuid`: External stable identifier; survives URI changes
- `ws_id`: Associates notebook with workspace; enables workspace-scoped queries
- `nd_uri`: LSP-compliant URI; globally unique; used by LSP clients
- `nd_notebook_type`: Identifies notebook format (Jupyter, etc.); determines cell execution semantics
- `nd_created_at`: When notebook was first opened
- `nd_closed_at`: NULL = notebook is open; non-NULL = closed; enables filtering active notebooks
- `nd_current_version`: Fast lookup of latest version; increments on cell structure or content changes
- `nd_metadata`: Extensibility for notebook-specific data (kernel info, language, display preferences)

Constraints:

- UNIQUE: (nd_uuid)
- UNIQUE: (nd_uri)
- INDEX: (ws_id, nd_closed_at)
- INDEX: (nd_uri)
- BUSINESS RULE: nd_uri must be valid LSP URI
- BUSINESS RULE: Version increments on every change (LSP notebookDocument/didChange)

---

### [ndv] NotebookDocumentVersion

A specific version snapshot of a NotebookDocument.

```text
[ndv] NotebookDocumentVersion:
  - ndv_id           : UUID <<PK>>                # Internal database identifier
  - nd_id            : UUID <<FK nd>>             # Parent NotebookDocument
  - ndv_version      : INTEGER                    # LSP version number (monotonic)
  - ndv_created_at   : Timestamp = CURRENT_TIMESTAMP
  - ndv_cell_count   : INTEGER                    # Number of cells in this version
  - ndv_metadata     : JSONB?                     # Version-specific metadata
```

**Field Roles:**

- `ndv_id`: Primary key for internal database operations
- `nd_id`: Links version to notebook; one notebook has many versions
- `ndv_version`: LSP version number; increments on cell structure changes; client/server synchronization
- `ndv_created_at`: Wall-clock time when version was created
- `ndv_cell_count`: Cached count of cells; optimization for notebook size queries
- `ndv_metadata`: Version-specific metadata that changed

Constraints:

- UNIQUE: (nd_id, ndv_version)
- INDEX: (nd_id, ndv_version DESC)
- BUSINESS RULE: Version numbers are monotonically increasing

---

### [ndc] NotebookDocumentCell

A cell within a notebook document version.

```text
[ndc] NotebookDocumentCell:
  - ndc_id           : UUID <<PK>>                # Internal database identifier
  - ndv_id           : UUID <<FK ndv>>            # Version this cell belongs to
  - ndc_index        : INTEGER                    # 0-indexed position in notebook
  - ndc_kind         : Enum (MARKUP, CODE)        # Cell type (display vs executable)
  - td_id            : UUID <<FK td>>             # TextDocument containing cell content
  - ndc_metadata     : JSONB?                     # Cell metadata (tags, collapsed state, etc.)
  - ndc_execution_order : INTEGER?                # Execution sequence number (CODE cells only)
  - ndc_execution_success : Boolean?              # Whether execution succeeded (CODE cells only)
```

**Field Roles:**

- `ndc_id`: Primary key for internal database operations
- `ndv_id`: Links cell to notebook version; cells belong to specific version snapshot
- `ndc_index`: Position in notebook; defines display order; 0-indexed
- `ndc_kind`: MARKUP = markdown/display; CODE = executable; determines cell behavior
- `td_id`: Each cell's content is a full TextDocument; enables reuse of text editing infrastructure
- `ndc_metadata`: Extensibility for cell-specific data (tags, UI state)
- `ndc_execution_order`: For CODE cells, tracks execution sequence; NULL for MARKUP or unexecuted
- `ndc_execution_success`: For CODE cells, whether last execution succeeded; NULL if not executed

Constraints:

- UNIQUE: (ndv_id, ndc_index)
- INDEX: (ndv_id, ndc_index)
- INDEX: (td_id)
- CHECK: ndc_kind IN ('MARKUP', 'CODE')
- CHECK: ndc_index >= 0
- BUSINESS RULE: Each cell has its own TextDocument for content
- BUSINESS RULE: MARKUP cells are for display, CODE cells are executable
- BUSINESS RULE: Execution fields only meaningful for CODE cells

---

## WorkspaceEdit Model

### [we] WorkspaceEdit

LSP WorkspaceEdit representing a set of changes to apply to the workspace.

```text
[we] WorkspaceEdit:
  - we_id            : UUID <<PK>>                # Internal database identifier
  - cvwe_id          : UUID <<FK cvwe>> <<UNIQUE>>  # Parent CellValueWorkspaceEdit (1:1)
  - we_label         : String?                      # Optional human-readable label
  - we_created_at    : Timestamp = CURRENT_TIMESTAMP
```

**Field Roles:**

- `we_id`: Primary key for internal database operations
- `cvwe_id`: Links to CellValue variant; enables WorkspaceEdit to be stored in cells
- `we_label`: Human-readable description (e.g., "Rename variable foo"); aids UI and debugging
- `we_created_at`: When edit was computed/created

Constraints:

- UNIQUE: (cvwe_id)
- BUSINESS RULE: Contains ordered list of file operations and text edits

---

### [wete] WorkspaceEditTextEdit

Text edits within a WorkspaceEdit.

```text
[wete] WorkspaceEditTextEdit:
  - wete_id          : UUID <<PK>>                # Internal database identifier
  - we_id            : UUID <<FK we>>             # Parent WorkspaceEdit
  - td_id            : UUID <<FK td>>             # Document to edit
  - wete_sequence    : INTEGER                    # Global order of application
  - wete_range_start_line : INTEGER               # Start line of edit (0-indexed)
  - wete_range_start_char : INTEGER               # Start character (0-indexed)
  - wete_range_end_line   : INTEGER               # End line (0-indexed)
  - wete_range_end_char   : INTEGER               # End character (0-indexed)
  - wete_new_text    : TEXT                       # Text to insert/replace (empty = delete)
  - wete_annotation_id : String?                  # Optional change annotation reference
```

**Field Roles:**

- `wete_id`: Primary key for internal database operations
- `we_id`: Groups edits into a WorkspaceEdit; multiple edits can be part of one logical change
- `td_id`: Target document; which file to edit
- `wete_sequence`: Global ordering across all operations; determines application order; lower = earlier
- `wete_range_start_line`, `wete_range_start_char`: Start position of text to replace
- `wete_range_end_line`, `wete_range_end_char`: End position of text to replace; same as start = insertion point
- `wete_new_text`: Replacement text; empty string = deletion
- `wete_annotation_id`: Links to annotation explaining why this edit exists

Constraints:

- INDEX: (we_id, wete_sequence)
- INDEX: (td_id)
- CHECK: wete_range_start_line >= 0
- CHECK: wete_range_end_line >= wete_range_start_line
- BUSINESS RULE: Edits applied in sequence order
- BUSINESS RULE: Edits must not overlap within same document

---

### [wefo] WorkspaceEditFileOperation

File operations (create/rename/delete) within a WorkspaceEdit.

```text
[wefo] WorkspaceEditFileOperation:
  - wefo_id          : UUID <<PK>>                # Internal database identifier
  - we_id            : UUID <<FK we>>             # Parent WorkspaceEdit
  - wefo_sequence    : INTEGER                    # Global order (shared with text edits)
  - wefo_kind        : Enum (CREATE, RENAME, DELETE)
  - wefo_uri         : TEXT                       # Target URI (for CREATE/DELETE)
  - wefo_old_uri     : TEXT?                      # Source URI (for RENAME only)
  - wefo_new_uri     : TEXT?                      # Destination URI (for RENAME only)
  - wefo_options     : JSONB?                     # Operation options (overwrite, ignoreIfExists, etc.)
  - wefo_annotation_id : String?                  # Optional change annotation reference
```

**Field Roles:**

- `wefo_id`: Primary key for internal database operations
- `we_id`: Groups operations into a WorkspaceEdit
- `wefo_sequence`: Global ordering; file ops and text edits interleave; e.g., create file then edit it
- `wefo_kind`: Operation type; determines which URI fields are used
- `wefo_uri`: Target for CREATE (new file) or DELETE (file to remove)
- `wefo_old_uri`: For RENAME, the current file location
- `wefo_new_uri`: For RENAME, the destination location
- `wefo_options`: LSP options (overwrite, ignoreIfExists, recursive); JSONB for flexibility
- `wefo_annotation_id`: Links to annotation explaining why this operation exists

Constraints:

- INDEX: (we_id, wefo_sequence)
- CHECK: wefo_kind IN ('CREATE', 'RENAME', 'DELETE')
- CHECK: (wefo_kind = 'RENAME' AND wefo_old_uri IS NOT NULL AND wefo_new_uri IS NOT NULL)
        OR (wefo_kind <> 'RENAME' AND wefo_old_uri IS NULL AND wefo_new_uri IS NULL)
- BUSINESS RULE: CREATE/DELETE use wefo_uri, RENAME uses wefo_old_uri → wefo_new_uri
- BUSINESS RULE: Operations and text edits share global sequence for ordering

---

### [weca] WorkspaceEditChangeAnnotation

Change annotations describing why edits were made.

```text
[weca] WorkspaceEditChangeAnnotation:
  - weca_id          : UUID <<PK>>                # Internal database identifier
  - we_id            : UUID <<FK we>>             # Parent WorkspaceEdit
  - weca_annotation_id : String                   # Annotation identifier (referenced by edits/ops)
  - weca_label       : String                     # Human-readable label (shown in UI)
  - weca_needs_confirmation : Boolean = false     # Whether user must approve this change
  - weca_description : TEXT?                      # Optional detailed explanation
```

**Field Roles:**

- `weca_id`: Primary key for internal database operations
- `we_id`: Associates annotation with WorkspaceEdit
- `weca_annotation_id`: String key used by edits/operations to reference this annotation
- `weca_label`: Short description shown in UI (e.g., "Update imports")
- `weca_needs_confirmation`: Flags potentially dangerous changes requiring user approval
- `weca_description`: Detailed explanation of why change is needed

Constraints:

- UNIQUE: (we_id, weca_annotation_id)
- BUSINESS RULE: Annotations explain rationale for changes
- BUSINESS RULE: Can mark changes as needing user confirmation

---

## FileWatcher Model

### [fswatcher] FileSystemWatcher

Watches file system paths for changes (LSP DidChangeWatchedFiles).

```text
[fswatcher] FileSystemWatcher:
  - fswatcher_id         : UUID <<PK>>                    # Internal database identifier
  - ws_id                : UUID <<FK ws>>                 # Workspace this watcher belongs to
  - fswatcher_glob       : TEXT                           # Glob pattern to watch (e.g., "**/*.{ts,js}")
  - fswatcher_base_uri   : TEXT?                          # Base URI for relative patterns (LSP 3.17+)
  - fswatcher_kind       : INTEGER = 7                    # Bitmask: CREATE(1) | CHANGE(2) | DELETE(4)
  - fswatcher_created_at : Timestamp = CURRENT_TIMESTAMP
  - fswatcher_deleted_at : Timestamp? = NULL              # Soft-delete timestamp
```

**Field Roles:**

- `fswatcher_id`: Primary key for internal database operations
- `ws_id`: Scopes watcher to workspace; enables workspace-specific file monitoring
- `fswatcher_glob`: Pattern matching files to watch; supports *, ?, **, {}, [] glob syntax
- `fswatcher_base_uri`: Makes glob relative to this URI; NULL = absolute pattern; enables folder-scoped watching
- `fswatcher_kind`: Bit flags for event types: 1=CREATE, 2=CHANGE, 4=DELETE, 7=ALL; filter irrelevant events
- `fswatcher_created_at`: When watcher was registered
- `fswatcher_deleted_at`: When watcher was unregistered; enables historical queries

Constraints:

- INDEX: (ws_id, fswatcher_deleted_at)
- CHECK: fswatcher_kind >= 1 AND fswatcher_kind <= 7
- BUSINESS RULE: fswatcher_kind is bit flags: 1=CREATE, 2=CHANGE, 4=DELETE
- BUSINESS RULE: Glob patterns support *, ?, **, {}, [] syntax
- BUSINESS RULE: fswatcher_base_uri enables relative patterns (LSP 3.17+)

---
