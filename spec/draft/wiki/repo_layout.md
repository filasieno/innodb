xinnodb/
├── src/
│   ├── cpscmain/                      ### LSP server entry point
│   │   └── main.cpp                   # Event loop, Cap'n Proto server
│   │
│   ├── xinnodbmain/                   # Database server entry point
│   │   └── main.cpp                   # Database server process
│   │
│   ├── sys/                           ### System runtime (language-agnostic)
│   │   ├── defs/                      # ✅ Common definitions, macros
│   │   ├── alloc/                     # ✅ Memory allocators (arena, general)
│   │   ├── task/                      # ✅ CPS coroutines (tail calls, ghccc)
│   │   ├── ut/                        # ✅ Utilities (assert, intrusive lists)
│   │   └── io/                        # ✅ I/O operations (files, sockets)
│   │
│   ├── lspsys/                        ### LSP infrastructure (language-agnostic)
│   │   ├── msgbus/                    # Message bus (lock-free MPMC queue)
│   │   ├── worker/                    # Worker pool (M:N coroutine scheduling)
│   │   ├── lspcheck/                  # Protocol validation (Cap'n Proto schemas)
│   │   └── transport/                 # Zero-copy transport (shared memory)
│   │
│   ├── lsp/                           ### LSP features (CPSC language-specific)
│   │   ├── lifecycle/                 # initialize, shutdown, exit, setTrace
│   │   ├── docsync/                   # textDocument/did                          {Open,Change,Save,Close}
│   │   ├── notebook/                  # notebookDocument/*                        (literate programming)
│   │   ├── workspace/                 # workspace/*                               (config, files, edit)
│   │   ├── window/                    # window/                                   {showMessage,logMessage}
│   │   ├── progress/                  # $/progress, workDoneProgress/*
│   │   ├── telemetry/                 # telemetry/event                           (performance monitoring)
│   │   ├── executecommand/            # workspace/executeCommand                  (custom commands)
│   │   ├── semantic/                  # textDocument/semanticTokens/*             (highlighting)
│   │   ├── diagnostics/               # textDocument/publishDiagnostics           (errors)
│   │   ├── hover/                     # textDocument/hover                        (type info)
│   │   ├── completion/                # textDocument/completion                   (code complete)
│   │   ├── signature/                 # textDocument/signatureHelp                (params)
│   │   ├── definition/                # textDocument/                             {definition,declaration}
│   │   ├── references/                # textDocument/references                   (find usages)
│   │   ├── symbols/                   # textDocument/documentSymbol               (outline)
│   │   ├── callhierarchy/             # callHierarchy/*                           (call graph)
│   │   ├── typehierarchy/             # typeHierarchy/*                           (type graph)
│   │   ├── highlight/                 # textDocument/documentHighlight
│   │   ├── inlayhint/                 # textDocument/inlayHint                    (inline hints)
│   │   ├── formatting/                # textDocument/formatting                   (code format)
│   │   ├── rename/                    # textDocument/rename                       (refactor)
│   │   ├── linkediting/               # textDocument/linkedEditingRange
│   │   ├── foldingrange/              # textDocument/foldingRange                 (code folding)
│   │   ├── selectionrange/            # textDocument/selectionRange               (smart select)
│   │   ├── documentlink/              # textDocument/documentLink                 (clickable links)
│   │   ├── color/                     # textDocument/documentColor                (color picker)
│   │   ├── codeaction/                # textDocument/codeAction                   (quick fixes)
│   │   ├── codelens/                  # textDocument/codeLens                     (inline actions)
│   │   ├── moniker/                   # textDocument/moniker                      (global IDs)
│   │   └── inlinevalue/               # textDocument/inlineValue                  (debug values)
│   │
│   ├── cps/                           # CPS compiler modules
│   │   ├── types/                     # Type system definitions
│   │   ├── parser/                    # Tree-sitter integration, AST
│   │   ├── tysys/                     # Type checker, type inference
│   │   ├── codegen/                   # LLVM IR generation (ghccc)
│   │   └── incremental/               # PIE incremental computation
│   │
│   └── db/                            # Database modules (self-hosted)
│       ├── aries/                     # ARIES write-ahead logging
│       ├── mvcc/                      # MVCC transaction manager
│       ├── btree/                     # B-tree indexes
│       ├── buffer/                    # Buffer pool manager
│       ├── query/                     # Query executor (vectorized)
│       └── tablespace/                # Coroutine frame storage
│
├── tools/
│   └── tree-sitter-cps/               # ✅ CPS grammar for tree-sitter
│
└── tests/
    ├── unit/                          # Unit tests per module
    ├── integration/                   # Cross-module integration tests
    └── benchmarks/                    # Performance benchmarks (TPC-C, etc.)
```

