# RFC 001: Literate Programming Format for XInnoDB

**Status**: Draft  
**Author**: XInnoDB Team  
**Created**: 2025-11-11  
**Updated**: 2025-11-11  

---

## Abstract

This RFC defines a literate programming format that combines human-readable documentation with executable code in a single source file. The format supports both plain-text editing (without specialized tools) and enhanced IDE features through LSP (Language Server Protocol) integration. Files can be authored in Markdown (`.lit.md`) or Typst (`.typ`) with identical chunk semantics, enabling beautiful PDF documentation generation while maintaining git-friendly plain-text sources.

**Key Design Goals:**

1. **Dual-mode editing**: Works in any text editor, enhanced by LSP when available
2. **Standard format compatibility**: Valid Markdown or Typst documents
3. **Named, composable chunks**: Code organized into reusable, referenced fragments
4. **Additive composition**: Chunks can be extended without modification
5. **Namespace isolation**: Multiple files can define same-named chunks without conflicts
6. **Tangle and weave**: Extract executable code or generate documentation
7. **Version control friendly**: Clean diffs, merge-friendly
8. **Beautiful output**: Typst generates publication-quality PDFs

---

## 1. Motivation

### 1.1 Problems with Traditional Approaches

**Separate Code and Documentation:**
- Documentation drifts from implementation
- No enforcement of consistency
- Difficult to understand code context
- Examples become outdated

**Traditional Literate Programming (WEB, noweb):**
- Requires specialized tools to view/edit
- Poor IDE support (no syntax highlighting, completion)
- Not version-control friendly
- Complex toolchains

**Notebooks (Jupyter, etc.):**
- JSON-based format (merge conflicts, poor diffs)
- Limited modularity (hard to reuse chunks)
- Execution-centric, not composition-centric
- Platform lock-in

### 1.2 Our Approach

Combine the best of all approaches:

```
┌─────────────────────────────────────────┐
│  Plain Text Source (.lit.md or .typ)   │
│  • Readable without tools               │
│  • Version control friendly             │
│  • Standard format (Markdown/Typst)     │
└────────────┬────────────────────────────┘
             │
    ┌────────┴────────┐
    │                 │
    ▼                 ▼
┌─────────┐      ┌──────────┐
│ TANGLE  │      │  WEAVE   │
│ Extract │      │ Generate │
│  Code   │      │   Docs   │
└────┬────┘      └────┬─────┘
     │                │
     ▼                ▼
┌─────────┐      ┌──────────┐
│ .py/.rs │      │ .pdf/    │
│ .cpp    │      │ .html    │
└─────────┘      └──────────┘

      + LSP Support (optional)
      • Syntax highlighting in chunks
      • Go-to-definition for chunk refs
      • Diagnostics for undefined chunks
      • Completion for chunk names
```

---

## 2. Format Specification

### 2.1 Chunk Syntax

Literate programming revolves around **named code chunks** that can reference other chunks.

**Chunk Definition (≡):**
```
⟨ chunk name ⟩ ≡
content...
```

**Chunk Extension (additive, +):**
```
⟨ chunk name ⟩+
additional content...
```

**Chunk Reference:**
```
⟨ chunk name ⟩
```

The angle brackets `⟨` (U+27E8) and `⟩` (U+27E9) are chosen because they:
- Are visually distinctive
- Rarely appear in code or prose
- Are easy to type with compose keys or IDE snippets
- Are valid in comments in most languages

### 2.2 Markdown Format (`.lit.md`)

Chunks are embedded in standard Markdown code fences:

```markdown
# Document Title

Regular Markdown prose goes here. This explains what the code does.

You can use all standard Markdown features: **bold**, *italic*, links,
images, tables, etc.

## Code Section

Here's a named chunk that defines a module:

```python ⟨ main module ⟩
#!/usr/bin/env python3
"""⟨ module docstring ⟩"""

⟨ imports ⟩
⟨ main function ⟩
⟨ entry point ⟩
```

The imports we need:

```python ⟨ imports ⟩
import sys
import logging
from typing import List, Optional
```

Later we realize we need more imports:

```python ⟨ imports ⟩+
from datetime import datetime
from pathlib import Path
```

---

## Main Function

The main function coordinates everything:

```python ⟨ main function ⟩
def main(args: List[str]) -> int:
    """Main entry point."""
    ⟨ setup logging ⟩
    ⟨ parse arguments ⟩
    ⟨ execute command ⟩
    return 0
```
```

**Key Properties:**
- Valid Markdown (renders on GitHub, GitLab, etc.)
- Syntax highlighting in code blocks
- Standard tooling works (linters, formatters on tangled code)
- Human-readable without processing

### 2.3 Typst Format (`.typ`)

Chunks are embedded in Typst code blocks:

```typst
= The Fibonacci Sequence
_A literate programming example_

The Fibonacci sequence is where each number is the sum of the two 
preceding ones. Mathematically: $F(n) = F(n-1) + F(n-2)$

== Implementation

```python ⟨ fibonacci module ⟩
"""Fibonacci implementation"""
⟨ imports ⟩
⟨ fibonacci function ⟩
```

We use type hints for clarity:

```python ⟨ imports ⟩
from typing import List
from functools import lru_cache
```

The main algorithm:

```python ⟨ fibonacci function ⟩
def fibonacci(n: int) -> List[int]:
    """Generate first n Fibonacci numbers."""
    if n <= 0:
        return []
    if n == 1:
        return [0]
    
    ⟨ build sequence ⟩
    return sequence
```

#pagebreak()

== Performance Analysis

#table(
  columns: 3,
  [*Method*], [*Time*], [*Space*],
  [Iterative], [$O(n)$], [$O(n)$],
  [Recursive], [$O(2^n)$], [$O(n)$],
)
```

**Key Properties:**
- Publication-quality PDF output
- Mathematical typesetting
- Professional diagrams and layouts
- All Typst features available (tables, figures, cross-references)

### 2.4 Metadata Block (Optional)

Both formats support a metadata block for namespace and configuration:

```markdown
---lp-meta
title: Web Server Implementation
language: python
namespace: webserver
author: XInnoDB Team
version: 1.0.0
---
```

**Standard Fields:**
- `title`: Document title
- `language`: Default language for chunks
- `namespace`: Namespace for all chunks in this file
- `author`: Author(s)
- `version`: Version number
- `license`: Software license

---

## 3. Chunk Composition Rules

### 3.1 Definition and Extension

**Definition (≡):** First occurrence of a chunk name

```python ⟨ config ⟩
HOST = "localhost"
PORT = 8080
```

**Extension (+):** Subsequent occurrences append content

```python ⟨ config ⟩+
TIMEOUT = 30
MAX_CONNECTIONS = 100
```

**Expansion:** When tangled, produces:

```python
HOST = "localhost"
PORT = 8080
TIMEOUT = 30
MAX_CONNECTIONS = 100
```

**Rules:**
1. A chunk can be defined (≡) exactly once per namespace
2. A chunk can be extended (+) any number of times
3. Extensions are appended in document order
4. Cannot extend (⟨ chunk ⟩+) before defining (⟨ chunk ⟩≡)
5. References (⟨ chunk ⟩) can appear before or after definition

### 3.2 Chunk References

References are expanded recursively with indentation preservation:

```python ⟨ server class ⟩
class Server:
    def __init__(self):
        ⟨ initialize fields ⟩
    
    def run(self):
        ⟨ main server loop ⟩
```

If `⟨ initialize fields ⟩` contains:
```python
self.host = HOST
self.port = PORT
```

Then tangling produces:
```python
class Server:
    def __init__(self):
        self.host = HOST
        self.port = PORT
    
    def run(self):
        ⟨ main server loop ⟩
```

**Indentation Rule:** Referenced chunk content is indented to match the reference location.

### 3.3 Root Chunks

A **root chunk** is one that is never referenced by other chunks. These are the entry points for tangling.

Common conventions:
- `⟨ * ⟩`: The complete file
- `⟨ main ⟩`: The main entry point
- `⟨ module-name ⟩`: The module itself

Example:
```python ⟨ * ⟩
#!/usr/bin/env python3
⟨ imports ⟩
⟨ classes ⟩
⟨ main ⟩
```

To tangle: `literate tangle myfile.lit.md --chunk "*" -o output.py`

---

## 4. Namespace System

### 4.1 Motivation

Multiple files may define chunks with the same name (e.g., `imports`, `tests`). Namespaces prevent conflicts.

### 4.2 File-Level Namespace

Set in metadata:

```markdown
---lp-meta
namespace: webserver.middleware
---

⟨ logging middleware ⟩≡
def log_requests(req, res):
    ...
```

**Qualified name:** `webserver.middleware::logging middleware`

### 4.3 Explicit Namespace

Override in chunk name:

```python ⟨ utils::string helpers ⟩
def capitalize(s):
    return s.upper()
```

**Qualified name:** `utils::string helpers` (ignores file namespace)

### 4.4 Hierarchical Scoping

Use dots for hierarchy:

```python ⟨ server ⟩
class Server:
    ⟨ server.init ⟩
    ⟨ server.methods ⟩
```

```python ⟨ server.init ⟩
def __init__(self):
    ...
```

```python ⟨ server.methods ⟩
def run(self):
    ...
```

**Qualified names:**
- `server`
- `server.init`
- `server.methods`

Hierarchy is nominal (no automatic prefix resolution).

### 4.5 Cross-File References

Reference chunks from other files using full qualified names:

```python
# File: server.lit.md (namespace: webserver)
⟨ main ⟩
from middleware import ⟨ auth::authenticate ⟩
```

```python
# File: auth.lit.md (namespace: auth)
⟨ authenticate ⟩
def authenticate(token):
    ...
```

### 4.6 Namespace Resolution Algorithm

When resolving `⟨ name ⟩`:

1. Check if `name` contains `::` → use as-is (explicit namespace)
2. Otherwise, try `current-namespace::name`
3. If not found, try `name` (global namespace)
4. If not found, ERROR: undefined chunk

---

## 5. Operations

### 5.1 Tangle (Code Extraction)

**Purpose:** Extract executable code from literate source

**Algorithm:**
1. Parse document into chunks
2. Select root chunk (specified by user or `*`)
3. Recursively expand all references
4. Preserve indentation
5. Detect cycles (error if found)
6. Write to output file

**Example:**
```bash
literate tangle server.lit.md --chunk "*" -o server.py
```

**Options:**
- `--chunk NAME`: Specify root chunk (default: `*`)
- `--output FILE`: Output file
- `--namespace NS`: Override file namespace
- `--check`: Verify without writing

### 5.2 Weave (Documentation Generation)

**Purpose:** Generate beautiful documentation with expanded code

**Algorithm:**
1. Parse document into text and code cells
2. For each code chunk:
   - Optionally expand references
   - Show chunk name and mode (≡ or +)
   - Optionally show what it references
3. For text cells, pass through unchanged
4. Generate output (PDF via Typst, HTML, etc.)

**Example:**
```bash
literate weave server.typ -o server.pdf --expand-chunks
```

**Options:**
- `--expand-chunks`: Show expanded code (all refs resolved)
- `--show-chunk-names`: Display chunk names in output
- `--show-references`: List what each chunk uses
- `--format FORMAT`: Output format (pdf, html, markdown)
- `--toc`: Include table of contents
- `--index`: Include chunk index

### 5.3 Check (Validation)

**Purpose:** Verify document integrity

**Checks:**
- All references are defined
- No circular dependencies
- No duplicate definitions (same chunk, same namespace, ≡ used twice)
- Extensions come after definition
- Namespace syntax is valid

**Example:**
```bash
literate check *.lit.md
```

**Exit codes:**
- 0: All checks passed
- 1: Errors found (undefined refs, cycles, etc.)
- 2: Warnings (unused chunks, etc.)

### 5.4 List (Inventory)

**Purpose:** Show all chunks in a document

**Example:**
```bash
literate list server.lit.md
```

**Output:**
```
Root chunks (entry points):
  ⟨ * ⟩ (line 15)

Defined chunks:
  ⟨ imports ⟩ (line 45, extended at lines 78, 103)
  ⟨ config ⟩ (line 52, extended at line 89)
  ⟨ server class ⟩ (line 112)
  ...

Referenced chunks:
  imports: referenced by ⟨ * ⟩
  config: referenced by ⟨ server class ⟩
  ...

Unreferenced chunks (may be dead code):
  ⟨ experimental feature ⟩ (line 423)
```

### 5.5 Graph (Dependency Visualization)

**Purpose:** Show chunk dependencies

**Example:**
```bash
literate graph server.lit.md -o deps.dot
dot -Tpng deps.dot -o deps.png
```

**Output:** GraphViz DOT format showing chunk references

---

## 6. LSP Integration

### 6.1 Dual-Mode Architecture

The LSP server operates in two modes depending on client capabilities:

**Notebook Mode** (if client supports LSP notebooks):
- Exposes document as `NotebookDocument`
- Each chunk becomes a `NotebookCell`
- Text becomes markup cells, code chunks become code cells
- Full notebook features: cell execution, outputs, etc.

**Text Mode** (fallback):
- Exposes document as single `TextDocument`
- Provides virtual documents for each chunk
- Language features scoped to code chunks

### 6.2 Language Features

#### 6.2.1 Document Symbols

Show document structure:
```
server.lit.md
├─ # Introduction (heading)
├─ ⟨ * ⟩≡ (root chunk, python)
├─ # Configuration (heading)
├─ ⟨ imports ⟩≡ (chunk, python)
├─ ⟨ imports ⟩+ (additive, python)
├─ ⟨ config ⟩≡ (chunk, python)
└─ ...
```

#### 6.2.2 Go to Definition

Navigate from reference to definition:
- Click on `⟨ imports ⟩` → jump to `⟨ imports ⟩≡`
- If multiple definitions (via +), show all locations

#### 6.2.3 Find References

Find all uses of a chunk:
- Cursor on `⟨ imports ⟩≡` → shows all `⟨ imports ⟩` references

#### 6.2.4 Hover

Show chunk information on hover:
```
⟨ imports ⟩

Defined: line 45 (definition), line 78 (additive)
Referenced by:
  - ⟨ * ⟩ (line 15)
  - ⟨ server class ⟩ (line 112)

Contains:
  import sys
  import logging
  from typing import List
  ...
```

#### 6.2.5 Completion

Complete chunk names when typing `⟨`:
- Type `⟨` → show all defined chunks
- Type `⟨ imp` → filter to chunks starting with "imp"
- Insert `⟩` automatically

#### 6.2.6 Diagnostics

Real-time error detection:
- **Error:** Undefined chunk reference
- **Error:** Circular dependency detected
- **Error:** Duplicate definition (≡ used twice)
- **Warning:** Chunk defined but never used
- **Warning:** Extension (+) before definition (≡)
- **Info:** Consider breaking large chunk into smaller ones

#### 6.2.7 Code Actions

Quick fixes and refactorings:
- **Tangle chunk**: Extract to file
- **Inline chunk**: Replace reference with definition
- **Extract chunk**: Create new chunk from selection
- **Rename chunk**: Rename with all references
- **Show expanded**: Preview with all refs resolved
- **Show graph**: Visualize dependencies

#### 6.2.8 Folding Ranges

Fold/unfold chunks:
- Fold chunk definition: show only `⟨ name ⟩≡ ...`
- Fold text sections by Markdown headings

#### 6.2.9 Semantic Tokens

Syntax highlighting:
- Chunk names: special color/style
- `⟨`, `⟩`, `≡`, `+`: operators
- Code inside chunks: language-specific highlighting
- Markdown: standard Markdown highlighting

#### 6.2.10 Inlay Hints

Show additional information inline:
- At chunk reference: "defined at line X"
- At chunk definition: "referenced N times"
- At root chunk: "entry point for tangling"

### 6.3 Server Capabilities

```typescript
{
  textDocumentSync: {
    openClose: true,
    change: 2, // Incremental
    willSave: true,
    save: { includeText: true }
  },
  notebookDocumentSync: {
    notebookSelector: [{
      notebook: { notebookType: "literate-program" },
      cells: [{ language: "*" }]
    }]
  },
  completionProvider: {
    triggerCharacters: ["⟨", " "]
  },
  hoverProvider: true,
  definitionProvider: true,
  referencesProvider: true,
  documentSymbolProvider: true,
  documentHighlightProvider: true,
  codeActionProvider: {
    codeActionKinds: [
      "quickfix",
      "refactor.extract",
      "refactor.inline",
      "refactor.rewrite",
      "source"
    ]
  },
  renameProvider: {
    prepareProvider: true
  },
  foldingRangeProvider: true,
  semanticTokensProvider: {
    legend: {
      tokenTypes: ["macro", "operator", "parameter"],
      tokenModifiers: ["definition", "reference"]
    },
    full: true
  },
  inlayHintProvider: true,
  diagnosticProvider: {
    interFileDependencies: true,
    workspaceDiagnostics: true
  },
  executeCommandProvider: {
    commands: [
      "literate.tangle",
      "literate.weave",
      "literate.check",
      "literate.graph"
    ]
  }
}
```

---

## 7. File Organization

### 7.1 Single-File Programs

Simple programs fit in one file:

```
fibonacci.lit.md
  - Contains all code
  - Root chunk: ⟨ * ⟩
  - Tangle to fibonacci.py
```

### 7.2 Multi-File Projects

Larger projects split across files:

```
project/
├─ docs/
│  ├─ architecture.lit.md    (namespace: arch)
│  ├─ api.lit.md             (namespace: api)
│  └─ examples.lit.md        (namespace: examples)
├─ src/
│  ├─ server.lit.md          (namespace: server)
│  ├─ middleware.lit.md      (namespace: middleware)
│  └─ routing.lit.md         (namespace: routing)
└─ literate.toml             (project configuration)
```

### 7.3 Project Configuration

`literate.toml`:

```toml
[project]
name = "webserver"
version = "1.0.0"
default_language = "python"

[build]
# Tangle all src/*.lit.md files
tangle = [
  { source = "src/server.lit.md", chunk = "*", output = "build/server.py" },
  { source = "src/middleware.lit.md", chunk = "*", output = "build/middleware.py" },
  { source = "src/routing.lit.md", chunk = "*", output = "build/routing.py" },
]

# Generate documentation
weave = [
  { sources = ["docs/*.lit.md", "src/*.lit.md"], output = "docs/manual.pdf", format = "pdf" },
]

[lsp]
# LSP server configuration
enable_notebook_mode = true
diagnostics_on_save = true
```

---

## 8. Tooling

### 8.1 Command-Line Interface

```bash
# Core operations
literate tangle FILE --chunk CHUNK -o OUTPUT
literate weave FILE -o OUTPUT [OPTIONS]
literate check FILE...
literate list FILE
literate graph FILE -o OUTPUT

# Project operations
literate build              # Run all tangle/weave from literate.toml
literate watch              # Watch and auto-rebuild

# LSP server
literate lsp --stdio        # stdio transport
literate lsp --socket PATH  # UNIX domain socket
literate lsp --tcp PORT     # TCP socket
```

### 8.2 Editor Integration

#### VS Code Extension

```json
{
  "name": "literate-programming",
  "displayName": "Literate Programming",
  "description": "Support for .lit.md and .typ literate documents",
  "version": "1.0.0",
  "engines": { "vscode": "^1.80.0" },
  "categories": ["Programming Languages", "Notebooks"],
  "activationEvents": [
    "onLanguage:literate-markdown",
    "onLanguage:typst",
    "onNotebook:literate-program"
  ],
  "contributes": {
    "languages": [
      {
        "id": "literate-markdown",
        "extensions": [".lit.md", ".lp.md"],
        "aliases": ["Literate Markdown"],
        "configuration": "./language-configuration.json"
      }
    ],
    "notebookRenderer": [{
      "id": "literate-program-renderer",
      "displayName": "Literate Program",
      "mimeTypes": ["x-application/literate-program"]
    }],
    "commands": [
      {
        "command": "literate.tangle",
        "title": "Tangle: Extract Code"
      },
      {
        "command": "literate.weave",
        "title": "Weave: Generate Documentation"
      },
      {
        "command": "literate.check",
        "title": "Check: Validate Document"
      }
    ],
    "keybindings": [
      {
        "command": "literate.tangle",
        "key": "ctrl+alt+t",
        "when": "editorLangId == literate-markdown"
      }
    ]
  }
}
```

#### Neovim Plugin

```lua
-- literate.nvim
local M = {}

M.setup = function(opts)
  opts = opts or {}
  
  -- Register LSP client
  vim.lsp.start({
    name = "literate-lsp",
    cmd = {"literate", "lsp", "--stdio"},
    root_dir = vim.fs.dirname(vim.fs.find({'literate.toml'}, { upward = true })[1]),
    filetypes = {"literate-markdown", "typst"},
  })
  
  -- Add commands
  vim.api.nvim_create_user_command("LiterateTangle", function(args)
    vim.cmd("!literate tangle % --chunk * -o " .. args.args)
  end, { nargs = 1 })
  
  vim.api.nvim_create_user_command("LiterateWeave", function()
    vim.cmd("!literate weave % -o %.pdf")
  end, {})
end

return M
```

### 8.3 Build Integration

#### Make

```makefile
# Makefile
.SUFFIXES: .lit.md .py .pdf

%.py: %.lit.md
	literate tangle $< --chunk "*" -o $@

%.pdf: %.lit.md
	literate weave $< -o $@

all: server.py server.pdf

clean:
	rm -f *.py *.pdf

.PHONY: all clean
```

#### CMake

```cmake
# FindLiterate.cmake
find_program(LITERATE_EXECUTABLE literate)

function(literate_tangle SOURCE CHUNK OUTPUT)
  add_custom_command(
    OUTPUT ${OUTPUT}
    COMMAND ${LITERATE_EXECUTABLE} tangle ${SOURCE} --chunk ${CHUNK} -o ${OUTPUT}
    DEPENDS ${SOURCE}
    COMMENT "Tangling ${SOURCE} -> ${OUTPUT}"
  )
endfunction()

# Usage in CMakeLists.txt
find_package(Literate REQUIRED)

literate_tangle(
  ${CMAKE_CURRENT_SOURCE_DIR}/server.lit.md
  "*"
  ${CMAKE_CURRENT_BINARY_DIR}/server.py
)

add_custom_target(literate_sources ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/server.py
)
```

---

## 9. Examples

### 9.1 Complete Example: Web Server

See appendix A for a complete, working example demonstrating:
- Multiple files with namespaces
- Chunk composition and extension
- Cross-file references
- Documentation generation
- Executable extraction

### 9.2 Use Cases

**Operating System Kernel:**
```
kernel/
├─ memory.lit.md      # Memory management
├─ scheduler.lit.md   # Process scheduling
├─ syscalls.lit.md    # System calls
└─ drivers/
   ├─ disk.lit.md     # Disk driver
   └─ network.lit.md  # Network driver
```

**Academic Paper with Code:**
```
paper.typ
  - Introduction
  - ⟨ algorithm implementation ⟩
  - Performance analysis
  - ⟨ benchmarks ⟩
  - Conclusion

Compile to paper.pdf (with code)
Tangle to algorithm.py (executable)
```

**Tutorial/Book:**
```
book/
├─ chapter01-intro.lit.md
├─ chapter02-basics.lit.md
├─ chapter03-advanced.lit.md
└─ examples/
   ├─ example01.lit.md
   └─ example02.lit.md

Weave all → book.pdf
Tangle examples → working code
```

---

## 10. Implementation Plan

### Phase 1: Core Parser (Week 1-2)
- [ ] Markdown parser with chunk extraction
- [ ] Typst parser with chunk extraction
- [ ] Namespace resolution
- [ ] Chunk graph construction
- [ ] Cycle detection

### Phase 2: Basic Operations (Week 3-4)
- [ ] Tangle implementation
- [ ] Weave implementation (basic)
- [ ] Check/validate
- [ ] List chunks
- [ ] CLI interface

### Phase 3: LSP Server - Text Mode (Week 5-6)
- [ ] LSP server scaffold
- [ ] Document symbols
- [ ] Go to definition
- [ ] Find references
- [ ] Hover
- [ ] Diagnostics

### Phase 4: LSP Server - Advanced (Week 7-8)
- [ ] Completion
- [ ] Code actions
- [ ] Rename
- [ ] Semantic tokens
- [ ] Inlay hints
- [ ] Folding ranges

### Phase 5: Notebook Mode (Week 9-10)
- [ ] NotebookDocument conversion
- [ ] Cell synchronization
- [ ] Execution support (via commands)
- [ ] Output handling

### Phase 6: Tooling (Week 11-12)
- [ ] VS Code extension
- [ ] Project configuration (literate.toml)
- [ ] Build system integration
- [ ] Documentation generator
- [ ] Website/examples

### Phase 7: Advanced Features (Week 13+)
- [ ] Incremental parsing
- [ ] Caching/memoization
- [ ] Multi-file analysis
- [ ] Refactoring tools
- [ ] Template system

---

## 11. Open Questions

### 11.1 Chunk Naming Conventions

**Question:** Should we enforce naming conventions?

**Options:**
1. Free-form (any string)
2. Identifier-like (`[a-zA-Z0-9_-]+`)
3. Path-like (`module/submodule/chunk`)

**Recommendation:** Free-form with warnings for problematic names (very long, special characters that may break tools).

### 11.2 Execution Model

**Question:** Should chunks be executable in-place (like Jupyter)?

**Options:**
1. No execution (tangle + run externally)
2. Execute via command (like Code Lens)
3. Full notebook execution model

**Recommendation:** Start with (1), add (2) later. Full notebook execution (3) is out of scope.

### 11.3 Parameterized Chunks

**Question:** Should chunks accept parameters?

```python
⟨ sort function | Type: T, Comparator: cmp ⟩
def sort(items: List[T], compare: cmp) -> List[T]:
    ...
```

**Recommendation:** Not in v1. This adds significant complexity. Use language features (generics, templates) instead.

### 11.4 Transclusion vs. Expansion

**Question:** Should we support transclusion (include file contents)?

```markdown
⟨ config.py ⟩ ← file("config.py")
```

**Recommendation:** Not in v1. Use chunk references across files instead.

---

## 12. Alternatives Considered

### 12.1 Using JSON-based Notebook Format

**Rejected because:**
- Poor version control (binary-ish JSON)
- Not human-readable without tools
- Platform-specific (Jupyter, VS Code, etc.)

### 12.2 Using Custom File Format

**Rejected because:**
- Requires specialized viewers
- Not compatible with existing tools
- Higher adoption barrier

### 12.3 Using Org-mode

**Considered:** Emacs org-mode has literate programming support

**Rejected because:**
- Emacs-centric (limits adoption)
- Complex syntax (learning curve)
- We want Markdown/Typst compatibility

### 12.4 Using Literate Haskell (.lhs)

**Considered:** Haskell's literate programming style

**Rejected because:**
- Language-specific
- Limited tool support
- Bird tracks (>) are visually noisy

---

## 13. Security Considerations

### 13.1 Code Injection

**Risk:** Malicious chunks could execute arbitrary code during tangle/weave

**Mitigation:**
- Tangle/weave are pure transformations (no execution)
- If execution is added later, require explicit opt-in
- Sandbox execution environments

### 13.2 Path Traversal

**Risk:** Output paths could escape intended directory

**Mitigation:**
- Validate all file paths
- Restrict output to current directory or explicitly allowed paths
- Require `--allow-write` flag for absolute paths

### 13.3 Resource Exhaustion

**Risk:** Circular references could cause infinite loops

**Mitigation:**
- Detect cycles before expansion
- Limit recursion depth
- Timeout for long operations

### 13.4 LSP Server Security

**Risk:** LSP server has file system access

**Mitigation:**
- Run with user permissions (no privilege escalation)
- Restrict file access to workspace
- Audit all file operations

---

## 14. Performance Considerations

### 14.1 Large Documents

**Challenge:** Documents with 10,000+ lines

**Strategy:**
- Incremental parsing (only re-parse changed sections)
- Lazy expansion (don't expand chunks until needed)
- Cache chunk graph between operations

### 14.2 Many Files

**Challenge:** Projects with 100+ literate files

**Strategy:**
- Parallel parsing (process files concurrently)
- Index chunks across files (SQLite database)
- Only reprocess changed files

### 14.3 LSP Responsiveness

**Challenge:** Sub-100ms response time for language features

**Strategy:**
- Async processing (don't block on I/O)
- Incremental updates (TextDocumentSync: Incremental)
- Cache AST and chunk graph
- Background indexing

---

## 15. Compatibility

### 15.1 Markdown Compatibility

**Goal:** Documents render correctly on GitHub, GitLab, etc.

**Strategy:**
- Use standard Markdown syntax only
- Chunk syntax (⟨ ⟩) appears in code blocks (no special rendering needed)
- Test against CommonMark, GFM (GitHub Flavored Markdown)

### 15.2 Typst Compatibility

**Goal:** Documents compile with standard Typst

**Strategy:**
- Chunk syntax is valid Typst (in code blocks)
- Don't require special imports (unless using custom templates)
- Test against official Typst compiler

### 15.3 Language Agnostic

**Goal:** Support any programming language

**Strategy:**
- Chunk semantics are language-independent
- Syntax highlighting via language identifiers in code fences
- Language-specific features delegated to embedded LSP servers

---

## 16. Testing Strategy

### 16.1 Unit Tests

**Parser:**
- Parse valid documents
- Handle malformed input gracefully
- Preserve whitespace and indentation

**Operations:**
- Tangle: correct expansion, indentation, cycle detection
- Weave: correct output generation
- Check: catch all error conditions

### 16.2 Integration Tests

**Multi-file projects:**
- Cross-file references resolve correctly
- Namespace isolation works
- Build system integration

**LSP:**
- All language features work
- Notebook mode conversion
- Diagnostics are accurate

### 16.3 End-to-End Tests

**Real projects:**
- Port existing literate programs
- Write new projects from scratch
- Measure usability and ergonomics

### 16.4 Performance Tests

**Benchmarks:**
- Parse 10,000 line document: < 100ms
- Tangle 1,000 chunks: < 1s
- LSP response time: < 100ms (p95)

---

## 17. Documentation Plan

### 17.1 User Documentation

- **Getting Started Guide**: Installation, first document, tangle/weave
- **Tutorial**: Step-by-step creation of a literate program
- **Reference Manual**: Complete syntax, all operations, LSP features
- **Examples**: Real-world projects demonstrating best practices

### 17.2 Developer Documentation

- **Architecture Overview**: Parser, operations, LSP server
- **API Reference**: Public APIs for embedding/extending
- **Contributing Guide**: How to contribute, code style, testing

### 17.3 Video Tutorials

- Introduction to literate programming
- Creating your first .lit.md document
- LSP features walkthrough
- Multi-file project organization

---

## 18. Success Metrics

### 18.1 Adoption

- Number of projects using the format
- GitHub repositories with .lit.md files
- VS Code extension installs

### 18.2 Quality

- Bug reports / active users
- User satisfaction surveys
- Documentation quality feedback

### 18.3 Performance

- LSP response time < 100ms (p95)
- Tangle/weave time linear in document size
- Memory usage < 100MB for typical projects

---

## 19. Future Work

### 19.1 Advanced Chunk Features

- **Chunk parameters:** `⟨ sort | Type: T ⟩`
- **Conditional chunks:** `⟨ debug code | if DEBUG ⟩`
- **Chunk variants:** Multiple implementations of same interface

### 19.2 Collaboration Features

- **Chunk comments:** Annotate chunks with discussions
- **Version history:** Track chunk evolution over time
- **Diff visualization:** Show chunk changes

### 19.3 Interactive Features

- **REPL integration:** Execute chunks interactively
- **Debugger support:** Step through tangled code, map back to chunks
- **Profiler integration:** Performance data at chunk granularity

### 19.4 Code Quality

- **Linting:** Run linters on tangled code, map errors back to chunks
- **Testing:** Test individual chunks or combinations
- **Coverage:** Show which chunks are tested

---

## 20. Conclusion

This RFC defines a literate programming format that balances:

- **Simplicity:** Plain text, standard formats, no special tools required
- **Power:** Named chunks, composition, namespaces, cross-file references
- **Usability:** LSP integration, beautiful documentation, excellent tooling
- **Compatibility:** Works with existing Markdown/Typst ecosystem

The format enables true literate programming: code that reads like an essay, with executable implementation and publication-quality documentation generated from the same source.

**Next steps:**
1. Review and gather feedback
2. Implement Phase 1 (parser)
3. Create proof-of-concept examples
4. Iterate based on real-world usage

---

## Appendix A: Complete Example

See `examples/webserver.lit.md` for a complete, working example including:

- Multi-file structure with namespaces
- Chunk composition and extension patterns
- Cross-file references
- Documentation with Typst
- Build system integration
- LSP features demonstration

---

## Appendix B: Grammar

### B.1 Markdown Format

```ebnf
document       = metadata? content*
metadata       = "---lp-meta" meta-field* "---"
meta-field     = key ":" value "\n"
content        = text-block | code-chunk
text-block     = markdown-text
code-chunk     = "```" language? chunk-header "\n" chunk-body "```"
chunk-header   = "⟨" identifier "⟩" mode?
mode           = "≡" | "+"
chunk-body     = (text-line | chunk-ref)*
chunk-ref      = "⟨" identifier "⟩"
identifier     = [^⟩]+
language       = [a-z]+
```

### B.2 Typst Format

```ebnf
document       = content*
content        = typst-element | code-chunk
typst-element  = heading | paragraph | math | block | etc.
code-chunk     = "```" language? chunk-header "\n" chunk-body "```"
(same as Markdown)
```

### B.3 Namespace Syntax

```ebnf
qualified-name = namespace? "::" identifier
namespace      = identifier ("." identifier)*
identifier     = [a-zA-Z0-9_-]+
```

---

## Appendix C: LSP Message Examples

### C.1 Document Symbols Request

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "textDocument/documentSymbol",
  "params": {
    "textDocument": {
      "uri": "file:///path/to/server.lit.md"
    }
  }
}
```

**Response:**

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": [
    {
      "name": "# Introduction",
      "kind": 2,
      "range": { "start": { "line": 0, "character": 0 }, "end": { "line": 5, "character": 0 } }
    },
    {
      "name": "⟨ * ⟩≡",
      "kind": 12,
      "detail": "python",
      "range": { "start": { "line": 10, "character": 0 }, "end": { "line": 15, "character": 3 } }
    }
  ]
}
```

### C.2 Go to Definition

```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "method": "textDocument/definition",
  "params": {
    "textDocument": { "uri": "file:///path/to/server.lit.md" },
    "position": { "line": 25, "character": 10 }
  }
}
```

**Response:**

```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "uri": "file:///path/to/server.lit.md",
    "range": {
      "start": { "line": 45, "character": 0 },
      "end": { "line": 52, "character": 3 }
    }
  }
}
```

---

## Appendix D: Error Messages

### D.1 Undefined Chunk

```
Error: Undefined chunk reference
  ┌─ server.lit.md:45:5
  │
45│     ⟨ database connection ⟩
  │     ^^^^^^^^^^^^^^^^^^^^^^^^ chunk not defined
  │
  = help: Define this chunk using ⟨ database connection ⟩≡
```

### D.2 Circular Dependency

```
Error: Circular dependency detected
  ┌─ server.lit.md:30:5
  │
30│     ⟨ init database ⟩
  │     ^^^^^^^^^^^^^^^^^ 
  │
  = note: dependency chain:
    ⟨ init database ⟩
      → ⟨ create tables ⟩
      → ⟨ init schema ⟩
      → ⟨ init database ⟩ (circular)
```

### D.3 Duplicate Definition

```
Error: Duplicate chunk definition
   ┌─ server.lit.md:45:1
   │
45 │ ⟨ config ⟩≡
   │ ^^^^^^^^^^^ chunk already defined here
   │
   ┌─ server.lit.md:78:1
   │
78 │ ⟨ config ⟩≡
   │ ^^^^^^^^^^^ duplicate definition
   │
   = help: Use ⟨ config ⟩+ to extend the chunk instead
```

---

## Appendix E: Comparison with Other Systems

| Feature | This RFC | noweb | Jupyter | Org-mode | Literate Haskell |
|---------|----------|-------|---------|----------|------------------|
| Plain text source | ✅ | ✅ | ❌ (JSON) | ✅ | ✅ |
| Standard format | ✅ (Markdown/Typst) | ❌ | ❌ | ❌ | ❌ |
| LSP support | ✅ | ❌ | ⚠️ | ⚠️ | ⚠️ |
| Beautiful docs | ✅ (Typst PDF) | ⚠️ (LaTeX) | ❌ | ⚠️ | ❌ |
| Named chunks | ✅ | ✅ | ❌ | ✅ | ❌ |
| Additive chunks | ✅ | ✅ | ❌ | ✅ | ❌ |
| Namespaces | ✅ | ❌ | ❌ | ❌ | ❌ |
| Multi-file | ✅ | ⚠️ | ❌ | ⚠️ | ❌ |
| Execution | ⚠️ (future) | ❌ | ✅ | ✅ | ❌ |
| Version control | ✅ | ✅ | ❌ | ✅ | ✅ |
| Language agnostic | ✅ | ✅ | ⚠️ | ✅ | ❌ (Haskell) |

---

## References

1. **Knuth, D. E.** (1984). *Literate Programming*. The Computer Journal, 27(2), 97-111.
2. **Ramsey, N.** (1994). *Literate Programming Simplified*. IEEE Software, 11(5), 97-105.
3. **Schulte, E., & Davison, D.** (2011). *Active Documents with Org-Mode*. Computing in Science & Engineering, 13(3), 66-73.
4. **LSP Specification** (2023). Language Server Protocol. https://microsoft.github.io/language-server-protocol/
5. **Typst Documentation** (2024). https://typst.app/docs/

---

**End of RFC 001**


