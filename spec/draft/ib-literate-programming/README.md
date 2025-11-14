# InnoDB Literate Programming System

A literate programming format and toolchain designed for modern software development. Write code that reads like documentation, maintain executable programs and beautiful PDFs from a single source.

---

## Overview

Literate programming inverts the traditional relationship between code and documentation. Instead of:

```
Code with comments → Documentation (separate)
```

We have:

```
Documentation with code → Executable program + Beautiful docs
```

**Key Innovation:** Named, composable code chunks using `⟨ chunk name ⟩` syntax embedded in standard Markdown or Typst documents.

---

## Documents in This Directory

### Core Documents

1. **[RFC-001-literate-programming-format.md](RFC-001-literate-programming-format.md)**
   - Complete specification of the format
   - Rationale and design decisions
   - LSP integration details
   - Implementation plan

2. **[QUICKSTART.md](QUICKSTART.md)**
   - 5-minute introduction
   - First literate program
   - Common patterns
   - Troubleshooting

3. **[example-webserver.lit.md](example-webserver.lit.md)**
   - Complete working example
   - HTTP server with middleware and routing
   - Demonstrates all major features
   - ~500 lines of documented code

---

## Quick Example

A minimal literate program (`hello.lit.md`):

````markdown
# Hello, World!

This program greets the world.

```python ⟨ * ⟩
#!/usr/bin/env python3
⟨ main function ⟩

if __name__ == '__main__':
    main()
```

The greeting function:

```python ⟨ main function ⟩
def main():
    print("Hello, World!")
```
````

**Extract code:**
```bash
literate tangle hello.lit.md --chunk "*" -o hello.py
python hello.py
```

**Generate docs:**
```bash
literate weave hello.lit.md -o hello.pdf
```

---

## Core Concepts

### 1. Chunks

Named code fragments:

````markdown
```python ⟨ chunk name ⟩
code here
```
````

### 2. Chunk References

Compose larger chunks from smaller ones:

````markdown
```python ⟨ module ⟩
⟨ imports ⟩
⟨ classes ⟩
⟨ main ⟩
```
````

### 3. Additive Composition

Extend chunks without modifying original:

````markdown
```python ⟨ config ⟩
HOST = "localhost"
```

Later we add more:

```python ⟨ config ⟩+
PORT = 8080
DEBUG = True
```
````

### 4. Namespaces

Avoid conflicts in multi-file projects:

````markdown
---lp-meta
namespace: webserver
---

```python ⟨ middleware ⟩
# Becomes: webserver::middleware
```
````

---

## Format Variants

### Markdown (`.lit.md`)

- ✅ Renders on GitHub, GitLab
- ✅ Standard Markdown tools work
- ✅ Syntax highlighting in code blocks
- ✅ Git-friendly diffs

### Typst (`.typ`)

- ✅ Beautiful PDF output
- ✅ Mathematical typesetting ($F(n) = F(n-1) + F(n-2)$)
- ✅ Professional layouts
- ✅ Cross-references, citations

**Both formats use identical chunk syntax!**

---

## Operations

### Tangle (Extract Code)

Extract executable code from literate source:

```bash
literate tangle program.lit.md --chunk "*" -o program.py
```

### Weave (Generate Documentation)

Generate beautiful documentation:

```bash
literate weave program.typ -o documentation.pdf
```

### Check (Validate)

Verify document integrity:

```bash
literate check program.lit.md
```

Detects:
- Undefined chunk references
- Circular dependencies
- Duplicate definitions

### List (Inventory)

Show all chunks:

```bash
literate list program.lit.md
```

### Graph (Visualize)

Generate dependency graph:

```bash
literate graph program.lit.md -o deps.dot
dot -Tpng deps.dot -o dependencies.png
```

---

## IDE Integration

### LSP Features

The literate programming LSP server provides:

- **Document Symbols**: Navigate chunk structure
- **Go to Definition**: Jump from reference to definition
- **Find References**: Find all uses of a chunk
- **Hover**: Show chunk information and preview
- **Completion**: Auto-complete chunk names
- **Diagnostics**: Real-time error detection
- **Code Actions**: Tangle, inline, extract, rename
- **Semantic Tokens**: Syntax highlighting

### Editor Support

- **VS Code**: Full notebook and text mode support
- **Neovim**: LSP client integration
- **Any Editor**: Works as plain Markdown/Typst

---

## Project Organization

### Single File

Simple programs in one file:

```
fibonacci.lit.md  → tangle → fibonacci.py
                  → weave  → fibonacci.pdf
```

### Multi-File Project

Complex projects split across files:

```
project/
├─ literate.toml           # Project configuration
├─ docs/
│  ├─ architecture.lit.md  # Architecture documentation
│  └─ api.lit.md           # API reference
└─ src/
   ├─ server.lit.md        # Server implementation
   ├─ middleware.lit.md    # Middleware layer
   └─ routing.lit.md       # Routing system
```

With `literate.toml`:

```toml
[project]
name = "webserver"
version = "1.0.0"

[build]
tangle = [
  { source = "src/server.lit.md", chunk = "*", output = "build/server.py" },
]
weave = [
  { sources = ["docs/*.lit.md", "src/*.lit.md"], output = "docs/manual.pdf" },
]
```

Build with:
```bash
literate build
```

---

## Advantages Over Alternatives

| Feature | Literate (This) | Jupyter | noweb | Org-mode | Comments |
|---------|-----------------|---------|-------|----------|----------|
| **Plain text source** | ✅ | ❌ (JSON) | ✅ | ✅ | ✅ |
| **Standard format** | ✅ (Markdown/Typst) | ❌ | ❌ | ❌ | ✅ (but scattered) |
| **Version control** | ✅ Clean diffs | ❌ Messy | ✅ | ✅ | ✅ |
| **LSP support** | ✅ Full | ⚠️ Limited | ❌ | ⚠️ Limited | ✅ |
| **Named chunks** | ✅ | ❌ | ✅ | ✅ | ❌ |
| **Composable** | ✅ | ❌ | ✅ | ✅ | ❌ |
| **Beautiful docs** | ✅ (Typst PDF) | ⚠️ | ⚠️ (LaTeX) | ⚠️ | ❌ |
| **Readable without tools** | ✅ | ❌ | ⚠️ | ⚠️ | ✅ |
| **Language agnostic** | ✅ | ⚠️ | ✅ | ✅ | ✅ |

**Legend:**
- ✅ Excellent support
- ⚠️ Partial/limited support
- ❌ No support or poor support

---

## Use Cases

### 1. Academic Papers with Code

Write paper in Typst with embedded, executable code:

```
paper.typ → PDF (paper with code)
         → .py  (runnable implementation)
```

### 2. Tutorial/Book Writing

```
book/
├─ chapter01.lit.md  (Introduction)
├─ chapter02.lit.md  (Basics with examples)
├─ chapter03.lit.md  (Advanced topics)
└─ examples/
   ├─ example01.lit.md  (Working code)
   └─ example02.lit.md  (Working code)
```

### 3. Open Source Projects

```
project/
├─ README.lit.md        (Overview with quickstart code)
├─ CONTRIBUTING.lit.md  (Guide with setup scripts)
├─ docs/
│  └─ architecture.lit.md
└─ src/
   ├─ core.lit.md
   └─ plugins/
      ├─ plugin1.lit.md
      └─ plugin2.lit.md
```

### 4. Systems Programming

```
kernel/
├─ memory.lit.md    (Memory management with docs)
├─ scheduler.lit.md (Scheduling algorithms explained)
├─ syscalls.lit.md  (System call interface)
└─ drivers/
   ├─ disk.lit.md
   └─ network.lit.md
```

---

## Philosophy

### Code is Communication

Programs are written once but read many times. Optimize for human understanding, not just machine execution.

### Documentation is First-Class

Documentation and code have equal importance. They should evolve together, not separately.

### Compose, Don't Concatenate

Build complex systems from simple, named pieces that can be understood and tested independently.

### Beauty Matters

Code should be elegant. Documentation should be beautiful. Tools should make both easy.

---

## Implementation Status

### Phase 1: Core Parser ⚠️ In Progress
- [ ] Markdown parser with chunk extraction
- [ ] Typst parser with chunk extraction
- [ ] Namespace resolution
- [ ] Chunk graph construction
- [ ] Cycle detection

### Phase 2: Basic Operations 📋 Planned
- [ ] Tangle implementation
- [ ] Weave implementation
- [ ] Check/validate
- [ ] CLI interface

### Phase 3: LSP Server 📋 Planned
- [ ] Document symbols
- [ ] Go to definition
- [ ] Find references
- [ ] Hover
- [ ] Diagnostics
- [ ] Completion
- [ ] Code actions

### Phase 4: Tooling 📋 Planned
- [ ] VS Code extension
- [ ] Build system integration
- [ ] Documentation generator

---

## Getting Started

1. **Read**: Start with [QUICKSTART.md](QUICKSTART.md) for a 5-minute intro

2. **Study**: Look at [example-webserver.lit.md](example-webserver.lit.md) to see all features

3. **Reference**: Read [RFC-001](RFC-001-literate-programming-format.md) for complete details

4. **Try**: Convert a small program to literate style

5. **Share**: Show others what you've created!

---

## Resources

### Documentation
- [RFC-001: Format Specification](RFC-001-literate-programming-format.md)
- [Quick Start Guide](QUICKSTART.md)
- [Complete Example: Web Server](example-webserver.lit.md)

### Tools (Coming Soon)
- `literate` CLI tool
- LSP server
- VS Code extension
- Neovim plugin

### Examples (Coming Soon)
- Algorithm implementations
- Data structures
- Web applications
- System utilities

---

## Contributing

This is part of the XInnoDB project. Contributions welcome!

See `../../CONTRIBUTING.md` for details.

---

## Related Work

### Historical
- **WEB** (Knuth, 1984): Original literate programming system for TeX
- **noweb** (Ramsey, 1989): Language-agnostic literate programming
- **FunnelWeb** (Williams, 1992): Industrial-strength literate programming

### Modern
- **Jupyter**: Interactive notebooks (execution-focused)
- **Org-mode**: Emacs-based literate programming
- **Quarto**: Multi-language notebooks and documents

### Our Innovation
- Standard format (Markdown/Typst) compatibility
- Modern LSP integration
- Namespace system for multi-file projects
- Dual-mode editing (with/without LSP)

---

## License

Part of XInnoDB. See `../../LICENCE.md` for details.

---

## Contact

- **Project**: XInnoDB (github.com/xinnodb)
- **Discussions**: GitHub Discussions
- **Issues**: GitHub Issues

---

## Acknowledgments

Inspired by:
- Donald Knuth's vision of literate programming
- Norman Ramsey's practical approach (noweb)
- Modern notebook systems (Jupyter, Observable)
- LSP's standardization of editor features
- Typst's beautiful, simple typesetting

Built for the XInnoDB project and the wider programming community.

---

**Start writing beautiful, understandable code today!** 🚀


