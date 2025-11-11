# LSP 3.17 Feature Set

**All ~79 LSP methods** implemented as coroutines. Organized by implementation priority.

---

## Phase 1: Infrastructure & Integration (Critical)

**Goal:** Get `.cps` and `.cpsn` files working in editor with full document/workspace/window integration.

---

### **A. Lifecycle & Initialization**

#### `lsp/lifecycle`

- [ ] `initialize`: Server initialization, capability negotiation
- [ ] `initialized`: Notification after initialization complete
- [ ] `shutdown`: Graceful shutdown request
- [ ] `exit`: Server exit notification  
- [ ] `$/setTrace`: Set trace level (off/messages/verbose)
- [ ] `$/logTrace`: Log trace messages
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** Required for server to start

---

### **B. Document Synchronization**

#### `lsp/docsync`

- [ ] `textDocument/didOpen`: Document opened
- [ ] `textDocument/didChange`: Document content changed (full/incremental)
- [ ] `textDocument/willSave`: Before document save
- [ ] `textDocument/willSaveWaitUntil`: Save with server modifications
- [ ] `textDocument/didSave`: Document saved
- [ ] `textDocument/didClose`: Document closed
- [ ] Store in database, MVCC version control
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** Core document synchronization for `.cps` files

#### `lsp/notebook`

- [ ] `notebookDocument/didOpen`: Notebook opened
- [ ] `notebookDocument/didChange`: Notebook cells changed
- [ ] `notebookDocument/didSave`: Notebook saved
- [ ] `notebookDocument/didClose`: Notebook closed
- [ ] Cell management, execution state, dependency tracking
- [ ] Rich output rendering (tables, graphs, interactive visualizations)
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** Literate programming is CPSC's primary development methodology - `.cpsn` files must work

---

### **C. Workspace Integration**

#### `lsp/workspace`

- [ ] `workspace/didChangeConfiguration`: Configuration changed
- [ ] `workspace/didChangeWatchedFiles`: File system changes detected
- [ ] `workspace/didCreateFiles`: Files created
- [ ] `workspace/didRenameFiles`: Files renamed
- [ ] `workspace/didDeleteFiles`: Files deleted
- [ ] `workspace/willCreateFiles`: Before file creation (can modify/reject)
- [ ] `workspace/willRenameFiles`: Before file rename (can modify/reject)
- [ ] `workspace/willDeleteFiles`: Before file deletion (can reject)
- [ ] `workspace/applyEdit`: Apply workspace edit atomically
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** Essential for multi-file projects and workspace management

---

### **D. Window Integration**

#### `lsp/window`

- [ ] `window/showMessage`: Show message to user (info/warning/error)
- [ ] `window/showMessageRequest`: Show message with action buttons
- [ ] `window/showDocument`: Show/reveal document in editor
- [ ] `window/logMessage`: Log message to client
- [ ] `window/workDoneProgress/create`: Create progress indicator
- [ ] `window/workDoneProgress/cancel`: User cancelled operation
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** User feedback and progress reporting for long operations

---

### **E. Progress & Telemetry**

#### `lsp/progress`

- [ ] `$/progress`: Generic progress notifications
- [ ] Long-running operations feedback (parsing, indexing, etc.)
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** Essential for user experience during compilation

#### `lsp/telemetry`

- [ ] `telemetry/event`: Send telemetry data
- [ ] Performance metrics, usage statistics
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** Monitor server health and performance

---

### **F. Execute Command**

#### `lsp/executecommand`

- [ ] `workspace/executeCommand`: Execute custom server commands
- [ ] Server-side command execution framework
- **Priority:** 🔴 Critical (Phase 1)
- **Rationale:** Infrastructure for custom operations (compile, test, debug)

---

**Phase 1 Deliverable:**

- `.cps` files open, edit, save, close
- `.cpsn` notebooks work with cell management
- Workspace operations functional
- User feedback via window/progress
- Plain text editing works perfectly
- **No language intelligence yet** (just infrastructure)

---

## Phase 2: First Language Feature - Semantic Tokens

**Goal:** Syntax highlighting using Tree-sitter parser

---

### **G. Semantic Highlighting**

#### `lsp/semantic`

- [ ] `textDocument/semanticTokens/full`: Full semantic tokens
- [ ] `textDocument/semanticTokens/full/delta`: Delta/incremental updates
- [ ] `textDocument/semanticTokens/range`: Semantic tokens for range
- [ ] Tree-sitter integration for CPS language
- [ ] Token classification (keywords, types, variables, functions, etc.)
- [ ] Cache results per document version in database
- **Priority:** 🔴 Critical (Phase 2)
- **Rationale:** First visible language feature - proper syntax highlighting

---

**Phase 2 Deliverable:**

- Beautiful syntax highlighting in `.cps` files
- Semantic highlighting in notebook cells
- Validates Tree-sitter grammar works
- Foundation for other language features

---

## Phase 3: Basic Language Intelligence

**Goal:** Essential IDE features for development

---

### **H. Diagnostics**

#### `lsp/diagnostics`

- [ ] `textDocument/publishDiagnostics`: Push diagnostics (server→client)
- [ ] `textDocument/diagnostic`: Pull diagnostics (client→server)  
- [ ] `workspace/diagnostic`: Workspace-wide diagnostics
- [ ] Parser errors, type errors, linter warnings, hints
- [ ] Categorized by severity, with related information
- **Priority:** 🔴 Critical (Phase 3)
- **Rationale:** Show errors as you type

#### `lsp/hover`

- [ ] `textDocument/hover`: Hover information  
- [ ] Type signatures, documentation, examples
- [ ] Markdown formatting support
- **Priority:** 🟠 High (Phase 3)
- **Rationale:** Quick type information

#### `lsp/completion`

- [ ] `textDocument/completion`: Code completion
- [ ] `completionItem/resolve`: Resolve additional completion details
- [ ] Context-aware suggestions, snippets, imports
- **Priority:** 🟠 High (Phase 3)
- **Rationale:** Basic productivity feature

#### `lsp/signature`

- [ ] `textDocument/signatureHelp`: Function signature help
- [ ] Active parameter highlighting, overload support
- **Priority:** 🟠 High (Phase 3)
- **Rationale:** Help while typing function calls

---

## Phase 4: Navigation Features

**Goal:** Navigate codebase efficiently

---

### **I. Go To & Find**

#### `lsp/definition`

- [ ] `textDocument/definition`: Go to definition
- [ ] `textDocument/declaration`: Go to declaration
- [ ] `textDocument/typeDefinition`: Go to type definition
- [ ] `textDocument/implementation`: Go to implementation
- **Priority:** 🟠 High (Phase 4)

#### `lsp/references`

- [ ] `textDocument/references`: Find all references
- [ ] Database-indexed for efficient cross-file search
- **Priority:** 🟠 High (Phase 4)

#### `lsp/symbols`

- [ ] `textDocument/documentSymbol`: Document outline/symbol tree
- [ ] `workspace/symbol`: Workspace-wide symbol search
- [ ] Hierarchical representation, fuzzy search
- **Priority:** 🟠 High (Phase 4)

#### `lsp/highlight`

- [ ] `textDocument/documentHighlight`: Highlight symbol occurrences
- [ ] Read/write access differentiation
- **Priority:** 🟡 Medium (Phase 4)

---

## Phase 5: Advanced Navigation

**Goal:** Deep code understanding

---

### **J. Hierarchies**

#### `lsp/callhierarchy`

- [ ] `textDocument/prepareCallHierarchy`: Prepare call hierarchy
- [ ] `callHierarchy/incomingCalls`: Find incoming call sites
- [ ] `callHierarchy/outgoingCalls`: Find outgoing calls
- **Priority:** 🟡 Medium (Phase 5)

#### `lsp/typehierarchy`

- [ ] `textDocument/prepareTypeHierarchy`: Prepare type hierarchy
- [ ] `typeHierarchy/supertypes`: Find base types/interfaces
- [ ] `typeHierarchy/subtypes`: Find derived types/implementations
- **Priority:** 🟡 Medium (Phase 5)

---

## Phase 6: Code Modification

**Goal:** Refactoring and formatting

---

### **K. Formatting & Refactoring**

#### `lsp/formatting`

- [ ] `textDocument/formatting`: Format entire document
- [ ] `textDocument/rangeFormatting`: Format selection
- [ ] `textDocument/onTypeFormatting`: Format on keystroke (e.g., after `;` or `}`)
- **Priority:** 🟡 Medium (Phase 6)

#### `lsp/rename`

- [ ] `textDocument/rename`: Rename symbol across workspace
- [ ] `textDocument/prepareRename`: Validate rename is possible
- [ ] Transactional (all-or-nothing), cross-file
- **Priority:** 🟡 Medium (Phase 6)

##### `lsp/codeaction`

- [ ] `textDocument/codeAction`: Code actions (quick fixes, refactorings)
- [ ] `codeAction/resolve`: Resolve action details/edits on demand
- [ ] Organize imports, fix errors, extract function, generate code
- **Priority:** 🟡 Medium (Phase 6)

---

## Phase 7: Enhanced Editor Features

**Goal:** Polish and productivity enhancements

---

### **L. Editor Enhancements**

#### `lsp/inlayhint`

- [ ] `textDocument/inlayHint`: Inline hints (parameter names, types)
- [ ] `inlayHint/resolve`: Resolve hint details on demand
- **Priority:** 🟡 Medium (Phase 7)

#### `lsp/foldingrange`

- [ ] `textDocument/foldingRange`: Code folding regions
- [ ] Function/class/comment/region/import folding
- **Priority:** 🟡 Medium (Phase 7)

#### `lsp/selectionrange`

- [ ] `textDocument/selectionRange`: Smart selection expansion
- [ ] Expand selection to encompassing syntax nodes
- **Priority:** 🟡 Medium (Phase 7)

#### `lsp/linkediting`

- [ ] `textDocument/linkedEditingRange`: Linked editing ranges
- [ ] Simultaneous edit of related symbols (e.g., HTML open/close tags)
- **Priority:** 🟡 Medium (Phase 7)

#### `lsp/codelens`

- [ ] `textDocument/codeLens`: Code lens (inline actions/info)
- [ ] `codeLens/resolve`: Resolve lens details on demand
- [ ] "Run test", "N references", "Debug", "Benchmark"
- **Priority:** 🟡 Medium (Phase 7)

---

## Phase 8: Optional/Specialized Features

**Goal:** Nice-to-have features for specific use cases

---

### **M. Specialized Features**

#### `lsp/documentlink`

- [ ] `textDocument/documentLink`: Clickable links in code
- [ ] `documentLink/resolve`: Resolve link targets
- [ ] Import statements, URLs, file references
- **Priority:** 🟢 Low (Phase 8)

#### `lsp/color`

- [ ] `textDocument/documentColor`: Color information extraction
- [ ] `textDocument/colorPresentation`: Color picker UI
- [ ] For CSS colors, configuration files, etc.
- **Priority:** 🟢 Low (Phase 8)

#### `lsp/moniker`

- [ ] `textDocument/moniker`: Global symbol identifiers
- [ ] Cross-project, cross-repository symbol tracking
- [ ] For language servers federation
- **Priority:** 🟢 Low (Phase 8)

#### `lsp/inlinevalue`

- [ ] `textDocument/inlineValue`: Inline values during debugging
- [ ] Show variable values inline while debugging
- **Priority:** 🟢 Low (Phase 8)

---

## Implementation Summary

### Phase Breakdown

| Phase        | Focus               | Features                                                           | Timeline    |
|--------------|---------------------|--------------------------------------------------------------------|-------------|
| **Phase 1**  | Infrastructure      | Lifecycle, Docs, Notebooks, Workspace, Window, Progress, Telemetry, Commands     | Weeks 1-4   |
| **Phase 2**  | Syntax Highlighting | Semantic Tokens + Tree-sitter                                      | Weeks 5-6   |
| **Phase 3**  | Basic Intelligence  | Diagnostics, Hover, Completion, Signature                          | Weeks 7-10  |
| **Phase 4**  | Navigation          | Definition, References, Symbols, Highlight                         | Weeks 11-14 |
| **Phase 5**  | Advanced Nav        | Call Hierarchy, Type Hierarchy                                     | Weeks 15-16 |
| **Phase 6**  | Modification        | Formatting, Rename, Code Actions                                   | Weeks 17-20 |
| **Phase 7**  | Enhancements        | Inlay Hints, Folding, Selection, Lens                              | Weeks 21-24 |
| **Phase 8**  | Optional            | Links, Colors, Moniker, Inline Values                              | Weeks 25+   |

### Feature Count by Priority

| Priority               | Count        | Description                                 |
|------------------------|-------------|----------------------------------------------|
| 🔴 Critical Phase 1    | ~35 methods | All infrastructure and integration           |
| 🔴 Critical Phase 2    | 3 methods   | Semantic tokens (first language feature)     |
| 🔴 Critical Phase 3    | 3 methods   | Diagnostics                                  |
| 🟠 High Phase 3-4      | ~20 methods | Basic intelligence and navigation            |
| 🟡 Medium Phase 5-7    | ~15 methods | Advanced features and refactoring            |
| 🟢 Low Phase 8         | ~6 methods  | Specialized/optional features                |
| **TOTAL**              | **~82 LSP methods** | Complete LSP 3.17 support            |

### Key Strategy

1. **Phase 1 First:** Complete infrastructure allows `.cps` and `.cpsn` files to work as plain text
2. **Tree-sitter Second:** Validates parser and enables beautiful syntax highlighting
3. **Incremental Intelligence:** Add language features based on user needs
4. **Database Throughout:** All features leverage persistent database storage

**Implementation:** All features as C++ coroutines (Phases 1-7), eventual migration to CPSC (Phase 8+).
