# VSCode UI Specification - Documentation Overview

## 📚 Document Set

This directory contains a complete UI/UX specification for building a **Software CAD System** for propagator networks in VSCode. The specification is designed for experienced engineers and focuses on creating a professional, IDE-grade experience for visual programming with propagator networks.

---

## 🎯 Purpose

Define a comprehensive UI system that treats propagator networks as **first-class visual artifacts** that generate code, documentation, debuggers, and other artifacts—all within a literate programming environment (notebooks).

**Core Innovation**: **Edit/View Mode Toggle**
- **EDIT MODE**: Visual CAD interface for constructing propagator networks
- **VIEW MODE**: See all generated artifacts (code, docs, tests, debuggers)
- Seamless switching with full context preservation

---

## 📄 Documents in This Set

### 1. **vscode-ui-specification.md** (Main Specification)
**~18,000 words | Comprehensive design document**

#### Contents:
- **Overview & Philosophy**: "CAD for Computation" design principles
- **Workflow Comparison**: Traditional (Zig/Rust/C++) vs CPSC model
- **Core Scenarios** (6 detailed):
  - Code generation at compile time (no macros, CAD-interactive)
  - Interactive aspect orientation
  - Documentation generation
  - Custom debugger tool generation
  - Interactive performance profiling
  - Live code transformation preview
- **UI Architecture**: Panel layout and hierarchy
- **Notebook Cell Design**: Network, Code, Markdown, Test cells
- **Widget Catalog**: 10 core widgets with specifications
- **Panel Specifications**: Explorer, Side Panels, Webviews
- **Interaction Patterns**: 7 key user workflows
- **Advanced Examples**: Constraint editor, performance dashboard, batch transformations

**Target Audience**: Architects, lead developers, UX designers

---

### 2. **vscode-widgets-quick-reference.md** (Quick Reference)
**~3,000 words | Quick lookup guide**

#### Contents:
- Widget categories table
- Notebook cell types summary
- Key interaction patterns
- Workflow comparison matrix
- Panel layout diagram
- Quick commands reference
- Example end-to-end workflow
- Technology stack

**Target Audience**: Developers actively implementing the UI

---

### 3. **widget-design-system.md** (Design System)
**~8,000 words | Implementation-ready design tokens and components**

#### Contents:
- **Design Tokens**: Colors, typography, spacing, shadows, transitions
- **Base Components**: Button, Input, Dropdown, Slider, Tabs, Badge, Tooltip
- **Complex Widgets**: Network Node, Connection Line, Property Inspector, Timeline, Flame Graph, AST Tree
- **Animation Library**: Keyframes, transitions, loading states
- **Responsive Behavior**: Breakpoints, adaptive layouts
- **Accessibility**: Focus indicators, ARIA labels, keyboard navigation
- **Performance**: Virtual scrolling, memoization, debouncing
- **Dark Mode Support**: Color scheme adjustments
- **Testing**: Visual regression, interaction testing

**Target Audience**: Frontend developers, designers implementing components

---

### 4. **THIS FILE** (README)
**Navigation and overview**

---

## 🎨 Design Philosophy

### Core Principles

1. **Immediate Feedback**: Changes show impact instantly (milliseconds, not minutes)
2. **Reversible Exploration**: All edits tracked and revertible (time-travel debugging)
3. **Contextual Intelligence**: Tools adapt to what you're editing
4. **Zero Ceremony**: Common operations are one click or keystroke
5. **Professional Aesthetics**: Clean, functional, information-dense

### Key Innovations

- **Literate Programming First**: Notebooks as primary development environment
- **Dual Representation**: Visual (network) + textual (code) in harmony
- **Compile-Time Interactivity**: CAD-like manipulation generates code without macros
- **Auto-Generated Tooling**: Docs and debuggers generated from network structure
- **Full Transparency**: Every compilation step is visible and explorable

---

## 🚀 Quick Start

### For Architects/Designers
1. Read: **vscode-ui-specification.md** (Sections 1-3: Overview, Comparison, Scenarios)
2. Review: **Scenario examples** to understand user workflows
3. Explore: **UI Architecture** and **Panel Specifications**

### For Frontend Developers
1. Read: **vscode-widgets-quick-reference.md** for overview
2. Reference: **widget-design-system.md** for component specs
3. Implement: Start with base components, then complex widgets

### For UX Researchers
1. Read: **Workflow Comparison** (vscode-ui-specification.md, Section 2)
2. Analyze: **Interaction Patterns** (Section 8)
3. Test: Use **Scenario workflows** as usability test scripts

---

## 📊 Comparison: Traditional vs CPSC

| Aspect | Traditional (Zig/Rust/C++) | CPSC Propagator CAD |
|--------|---------------------------|---------------------|
| **Interface** | Text editor only | Visual CAD + notebooks |
| **Feedback** | Minutes (full compile) | Milliseconds (incremental) |
| **Code Gen** | Macros (limited, opaque) | Full metaprogramming (transparent) |
| **Aspects** | External frameworks | Built-in at compile time |
| **Docs** | Separate tools (can drift) | Auto-generated (always in sync) |
| **Debugging** | Generic GDB/LLDB | Custom inspectors per type |
| **Visibility** | Opaque compilation | Full transparency |
| **Exploration** | Trial and error | Interactive simulation |

---

## 🧩 Key Components

### Notebook Cells (4 types)

```
┌─ Network Cell ──────────────────────────────────────┐
│ [EDIT MODE: Visual network] ↔ [VIEW MODE: Code]    │
│ Toggle with Ctrl+Shift+E / Ctrl+Shift+V            │
└─────────────────────────────────────────────────────┘

┌─ Code Cell ─────────────────────────────────────────┐
│ Traditional imperative code (when needed)           │
└─────────────────────────────────────────────────────┘

┌─ Markdown Cell ─────────────────────────────────────┐
│ Rich documentation (literate programming)           │
└─────────────────────────────────────────────────────┘

┌─ Test Cell ─────────────────────────────────────────┐
│ Unit tests (run automatically on changes)           │
└─────────────────────────────────────────────────────┘
```

### Core Widgets (10 primary)

| Widget                  | Purpose                                |
|:------------------------|:---------------------------------------|
| **Network Canvas**      | Visual propagator network editor       |
| **Property Inspector**  | Edit selected element properties       |
| **Propagator Palette**  | Library of available propagators       |
| **Execution Timeline**  | Visualize propagator executions        |
| **Cell History**        | Time-travel through cell values        |
| **Propagator Debugger** | Debug execution in real-time           |
| **Flame Graph**         | Performance profiling                  |
| **AST Viewer**          | Visualize syntax trees                 |
| **Custom Inspector**    | Type-specific debugger (generated)     |
| **Documentation Panel** | Auto-generated docs                    |

---

## 🎓 Example Scenario: Building a Parser

**Traditional Workflow (C++/Rust)**:

1. Write parser code (30-60 minutes)
2. Write test cases (15 minutes)
3. Compile (30 seconds - 2 minutes)
4. Fix errors, iterate (repeat 2-10 times)
5. Write documentation separately (30 minutes)
6. Debug with GDB (print AST pointers manually)

**Total Time**: ~2-4 hours  
**Feedback Loop**: Minutes per iteration

---

**CPSC Workflow (Propagator CAD)**:

1. Create Network Cell (30 seconds)
2. Drag "Parser" propagator to canvas (10 seconds)
3. Connect: Source Cell → Parser → AST Cell (20 seconds)
4. Adjust parameters in Property Inspector (live preview, 1 minute)
5. Switch to View Mode: see generated parser code (instant)
6. Click "Documentation" tab: see auto-generated docs (instant)
7. Click "Debugger" tab: see custom AST inspector (instant)
8. Run test cell: automatic validation (instant)
9. Profile: flame graph shows performance (5 seconds)
10. Iterate: adjust in Edit Mode, see results immediately

**Total Time**: ~5 minutes  
**Feedback Loop**: Milliseconds per iteration

---

## 🔧 Technology Stack

### Core Technologies

- **VSCode Extension API**: Foundation
- **Webviews**: Custom widget rendering
- **React/Svelte**: Component framework
- **Canvas API**: Network visualization
- **D3.js**: Data visualization (graphs, timelines)
- **Monaco Editor**: Code editing with syntax highlighting
- **LSP Integration**: Language server connection

### Performance

- **Virtual Rendering**: Handle large networks
- **Web Workers**: Offload heavy computation
- **Debounced Updates**: Smooth live previews
- **Incremental Rendering**: Only update changes

---

## 📐 Layout Structure



## ⌨️ Key Shortcuts

| Shortcut         | Action                  |
|------------------|-------------------------|
| `Ctrl+Shift+E`   | Switch to EDIT mode     |
| `Ctrl+Shift+V`   | Switch to VIEW mode     |
| `Ctrl+Shift+P`   | Command palette         |
| `Ctrl+Shift+A`   | Add propagator          |
| `Shift+Enter`    | Run cell                |
| `Ctrl+H`         | View cell history       |
| `F5`             | Start debugger          |
| `Ctrl+Shift+F5`  | Start profiler          |
| `Ctrl+Z`         | Undo                    |
| `Ctrl+Y`         | Redo                    |
| `Delete`         | Delete selected         |
| `Escape`         | Clear selection         |

---

## 🎨 Visual Design Language

### Colors

- **Cells**: Color-coded by type
  - TextDocument: Blue
  - ATerm: Green
  - Diagnostic: Red
  - WorkspaceEdit: Yellow

- **Propagators**: Distinct colors
  - Primitive: Purple
  - Compound: Pink
  - Conditional: Teal
  - Constraint: Orange

### Typography

- **Sans**: System fonts (-apple-system, Segoe UI, Roboto)
- **Mono**: Fira Code, Source Code Pro, Consolas
- **Sizes**: 12px (xs) to 30px (3xl)

### Spacing

- **Base Unit**: 4px
- **Range**: 4px to 64px (geometric progression)

### Animations

- **Fast**: 150ms (hover, focus)
- **Base**: 200ms (transitions)
- **Slow**: 300ms (complex animations)

---

## ♿ Accessibility

- ✅ **Keyboard Navigation**: All operations accessible via keyboard
- ✅ **Screen Reader Support**: ARIA labels on all interactive elements
- ✅ **High Contrast**: Support for high-contrast themes
- ✅ **Zoom Support**: All widgets scale with VSCode zoom
- ✅ **Focus Indicators**: Visible focus states for all interactive elements

---

## 🧪 Testing

### Visual Regression

- Storybook for component library
- Jest + Puppeteer for screenshot comparison

### Interaction Testing

- Testing Library for user interactions
- Jest for unit tests
- Cypress for E2E tests

### Performance Testing

- Lighthouse for initial load
- Chrome DevTools for runtime performance
- Memory profiling for large networks

---

## 🗺️ Implementation Roadmap

### Phase 1: Core Infrastructure (Weeks 1-2)

- VSCode extension setup
- Basic notebook provider
- Simple network canvas (drag-drop)

### Phase 2: Essential Widgets (Weeks 3-4)

- Network Canvas (full featured)
- Property Inspector
- Propagator Palette
- Edit/View mode toggle

### Phase 3: Visualization (Weeks 5-6)

- Execution Timeline
- Cell History
- AST Viewer
- Diff Viewer

### Phase 4: Advanced Features (Weeks 7-8)

- Debugger integration
- Performance profiler
- Custom inspector generation
- Documentation generation

### Phase 5: Polish & Optimization (Weeks 9-10)

- Performance optimization
- Accessibility audit
- Visual polish
- User testing

---

## 📚 Related Documentation

This UI specification builds on:

- **lsp-object-model-plan.md**: LSP data model (Workspace, Documents, Notebooks)
- **propagator-object-model-plan.md**: Propagator network data model (Cells, Propagators, Values)
- **propagator-runtime-plan.md**: Runtime execution model (Notifications, Executions, Provenance)
- **propagator-cpsc.md**: CPSC language design (Coroutines, Compilation model)
