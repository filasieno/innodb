# Literate Programming Documentation Index

Navigate the complete literate programming specification and resources.

---

## 📚 Document Hierarchy

### Start Here

1. **[README.md](README.md)** - Overview and introduction
   - What is literate programming?
   - Core concepts
   - Quick example
   - Use cases

2. **[QUICKSTART.md](QUICKSTART.md)** - 5-minute tutorial
   - Your first literate program
   - Essential commands
   - Common patterns
   - Troubleshooting

### Detailed Documentation

3. **[RFC-001-literate-programming-format.md](RFC-001-literate-programming-format.md)** - Complete specification
   - Format specification (Markdown & Typst)
   - Chunk composition rules
   - Namespace system
   - Operations (tangle, weave, check)
   - LSP integration
   - Implementation plan
   - ~15,000 words, comprehensive

### Working Examples

4. **[example-webserver.lit.md](example-webserver.lit.md)** - Complete working example
   - HTTP server implementation
   - Middleware system
   - Routing
   - ~500 lines of documented code
   - Demonstrates all features

---

## 🎯 Reading Paths

### Path 1: Quick Start (15 minutes)
```
README.md (overview)
  ↓
QUICKSTART.md (tutorial)
  ↓
Try creating hello.lit.md yourself
```

**Goal:** Understand basics, create first program

### Path 2: Deep Dive (2 hours)
```
README.md (overview)
  ↓
QUICKSTART.md (tutorial)
  ↓
RFC-001 (specification)
  ↓
example-webserver.lit.md (study example)
```

**Goal:** Full understanding of format and capabilities

### Path 3: Implementation (ongoing)
```
RFC-001 (specification)
  ↓
Study Appendix B (grammar)
  ↓
Study Section 6 (LSP integration)
  ↓
Follow implementation plan (Section 10)
```

**Goal:** Build the toolchain

### Path 4: Use in Project (1-2 days)
```
QUICKSTART.md (learn basics)
  ↓
example-webserver.lit.md (see patterns)
  ↓
RFC-001, Section 7 (file organization)
  ↓
Create your own .lit.md files
```

**Goal:** Adopt in real project

---

## 📖 Document Summaries

### README.md
**Length:** ~400 lines  
**Type:** Overview  
**Audience:** Everyone  

**Contents:**
- What is literate programming
- Quick example
- Core concepts in 5 minutes
- Comparison with alternatives
- Use cases
- Getting started

**When to read:** First thing, for high-level understanding

---

### QUICKSTART.md
**Length:** ~500 lines  
**Type:** Tutorial  
**Audience:** New users  

**Contents:**
- Installation instructions
- Step-by-step first program
- Key concepts explained
- Common patterns with examples
- Command reference
- Tips & tricks
- Troubleshooting

**When to read:** After README, before writing code

---

### RFC-001-literate-programming-format.md
**Length:** ~1,500 lines (~15,000 words)  
**Type:** Specification  
**Audience:** Implementers, advanced users  

**Contents:**
- **Section 1**: Motivation and design goals
- **Section 2**: Format specification (Markdown & Typst)
- **Section 3**: Chunk composition rules
- **Section 4**: Namespace system
- **Section 5**: Operations (tangle, weave, check, list, graph)
- **Section 6**: LSP integration (dual-mode, language features)
- **Section 7**: File organization
- **Section 8**: Tooling (CLI, editors, build systems)
- **Section 9**: Examples
- **Section 10**: Implementation plan
- **Sections 11-20**: Advanced topics, open questions, security, performance
- **Appendices**: Complete example, grammar, LSP messages, error messages, comparisons

**When to read:** 
- Implementing the toolchain
- Need complete specification
- Answering "how does X work?"

---

### example-webserver.lit.md
**Length:** ~500 lines  
**Type:** Working example  
**Audience:** Learning by example  

**Contents:**
- Complete HTTP server implementation
- Middleware chain pattern
- Routing system
- Type-safe Python with full hints
- Demonstrates:
  - Named chunks (⟨ name ⟩)
  - Additive composition (⟨ name ⟩+)
  - Chunk references
  - Hierarchical organization
  - Documentation with code

**When to read:**
- After QUICKSTART
- When learning advanced patterns
- As template for your own projects

**How to use:**
```bash
# Extract executable code
literate tangle example-webserver.lit.md --chunk "*" -o server.py

# Run it
python server.py

# Test endpoints
curl http://localhost:8080/health
```

---

## 🔍 Find Information By Topic

### Basic Concepts

| Topic | Document | Section |
|-------|----------|---------|
| What are chunks? | QUICKSTART.md | "Key Concepts" |
| Chunk syntax | RFC-001 | Section 2.1 |
| Named chunks | README.md | "Core Concepts" |
| Chunk references | QUICKSTART.md | "Pattern 1" |

### Chunk Composition

| Topic | Document | Section |
|-------|----------|---------|
| Definition vs Extension | RFC-001 | Section 3.1 |
| Additive chunks (+) | QUICKSTART.md | "Key Concepts #3" |
| Indentation rules | RFC-001 | Section 3.2 |
| Root chunks | QUICKSTART.md | "Key Concepts #5" |

### Namespaces

| Topic | Document | Section |
|-------|----------|---------|
| Why namespaces? | RFC-001 | Section 4.1 |
| File-level namespace | RFC-001 | Section 4.2 |
| Explicit namespace | RFC-001 | Section 4.3 |
| Hierarchical scoping | RFC-001 | Section 4.4 |
| Resolution algorithm | RFC-001 | Section 4.6 |

### Operations

| Topic | Document | Section |
|-------|----------|---------|
| Tangle (extract code) | QUICKSTART.md | "Commands" |
| Tangle specification | RFC-001 | Section 5.1 |
| Weave (documentation) | RFC-001 | Section 5.2 |
| Check (validate) | QUICKSTART.md | "Commands" |
| List chunks | RFC-001 | Section 5.4 |
| Graph dependencies | RFC-001 | Section 5.5 |

### Formats

| Topic | Document | Section |
|-------|----------|---------|
| Markdown format | RFC-001 | Section 2.2 |
| Typst format | RFC-001 | Section 2.3 |
| Metadata block | RFC-001 | Section 2.4 |
| Grammar (formal) | RFC-001 | Appendix B |

### LSP Integration

| Topic | Document | Section |
|-------|----------|---------|
| Dual-mode architecture | RFC-001 | Section 6.1 |
| Language features | RFC-001 | Section 6.2 |
| Document symbols | RFC-001 | Section 6.2.1 |
| Go to definition | RFC-001 | Section 6.2.2 |
| Diagnostics | RFC-001 | Section 6.2.6 |
| Code actions | RFC-001 | Section 6.2.7 |
| Server capabilities | RFC-001 | Section 6.3 |

### Project Organization

| Topic | Document | Section |
|-------|----------|---------|
| Single-file programs | RFC-001 | Section 7.1 |
| Multi-file projects | RFC-001 | Section 7.2 |
| Project configuration | RFC-001 | Section 7.3 |
| Example structure | README.md | "Project Organization" |

### IDE Support

| Topic | Document | Section |
|-------|----------|---------|
| VS Code extension | RFC-001 | Section 8.2 |
| Neovim plugin | RFC-001 | Section 8.2 |
| General IDE support | QUICKSTART.md | "IDE Support" |

### Implementation

| Topic | Document | Section |
|-------|----------|---------|
| Implementation plan | RFC-001 | Section 10 |
| Phase breakdown | RFC-001 | Section 10 |
| Parser design | RFC-001 | Section 10, Phase 1 |
| LSP server design | RFC-001 | Section 10, Phases 3-5 |

### Troubleshooting

| Topic | Document | Section |
|-------|----------|---------|
| Common errors | QUICKSTART.md | "Troubleshooting" |
| Error messages | RFC-001 | Appendix D |
| Undefined chunks | QUICKSTART.md | "Troubleshooting" |
| Circular dependencies | RFC-001 | Appendix D.2 |

---

## 🎨 Examples By Complexity

### Minimal (Hello World)
- **Location:** QUICKSTART.md, "Your First Literate Program"
- **Lines:** ~20
- **Features:** Basic chunks, tangle

### Simple (Fibonacci)
- **Location:** QUICKSTART.md, "Pattern 1"
- **Lines:** ~50
- **Features:** Multiple chunks, references

### Medium (Algorithm)
- **Location:** QUICKSTART.md, "Pattern 3"
- **Lines:** ~100
- **Features:** Complex decomposition, explanation

### Complete (Web Server)
- **Location:** example-webserver.lit.md
- **Lines:** ~500
- **Features:** All features, multi-layer architecture

---

## 📊 Statistics

### Documentation Coverage

| Topic | Coverage |
|-------|----------|
| Format specification | 100% (RFC-001, Sections 2-4) |
| Operations | 100% (RFC-001, Section 5) |
| LSP integration | 100% (RFC-001, Section 6) |
| Tooling | 90% (RFC-001, Section 8) |
| Examples | 100% (QUICKSTART + example-webserver) |
| Implementation | 100% (RFC-001, Section 10) |

### Document Statistics

| Document | Lines | Words | Sections |
|----------|-------|-------|----------|
| README.md | ~400 | ~3,000 | 15 |
| QUICKSTART.md | ~500 | ~4,000 | 12 |
| RFC-001 | ~1,500 | ~15,000 | 20 + 5 appendices |
| example-webserver.lit.md | ~500 | ~3,000 | 8 |
| **Total** | **~2,900** | **~25,000** | **55** |

---

## ✅ Checklist: Learning Path

### Beginner
- [ ] Read README.md overview
- [ ] Work through QUICKSTART.md tutorial
- [ ] Create hello.lit.md
- [ ] Tangle and run hello.py
- [ ] Try one pattern from QUICKSTART

### Intermediate
- [ ] Read RFC-001 Sections 1-5 (format & operations)
- [ ] Study example-webserver.lit.md
- [ ] Convert an existing program to .lit.md
- [ ] Use multiple chunks with references
- [ ] Add namespaces

### Advanced
- [ ] Read RFC-001 Sections 6-8 (LSP & tooling)
- [ ] Set up LSP server
- [ ] Organize multi-file project
- [ ] Create literate.toml configuration
- [ ] Write custom middleware/handlers

### Expert / Implementer
- [ ] Read complete RFC-001
- [ ] Study grammar (Appendix B)
- [ ] Study LSP messages (Appendix C)
- [ ] Follow implementation plan (Section 10)
- [ ] Contribute to toolchain

---

## 🔗 Quick Links

### External Resources

**Literate Programming History:**
- Knuth, "Literate Programming" (1984)
- Ramsey, "Literate Programming Simplified" (1994)

**Related Technologies:**
- LSP Specification: https://microsoft.github.io/language-server-protocol/
- Typst Documentation: https://typst.app/docs/
- Markdown (CommonMark): https://commonmark.org/

**Similar Systems:**
- noweb: https://www.cs.tufts.edu/~nr/noweb/
- Jupyter: https://jupyter.org/
- Org-mode: https://orgmode.org/

---

## 📝 Document Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2025-11-11 | 1.0.0 | Initial release of all documents |

---

## 🤝 Contributing

Found an error? Have a suggestion? Want to add examples?

See `../../CONTRIBUTING.md` for how to contribute.

---

## 📧 Questions?

- **General questions:** GitHub Discussions
- **Bug reports:** GitHub Issues
- **Feature requests:** GitHub Issues with "enhancement" label

---

**Happy Reading and Coding!** 📚✨


