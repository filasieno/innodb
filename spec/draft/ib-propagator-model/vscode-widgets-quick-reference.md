# VSCode Widgets - Quick Reference

## Widget Categories

### 1. Core Editing Widgets

| Widget | Purpose | Key Features |
|--------|---------|--------------|
| **Network Canvas** | Visual propagator network editor | Drag-drop, auto-layout, zoom, connections |
| **Property Inspector** | Edit selected element properties | Adaptive UI, live updates, validation |
| **Propagator Palette** | Library of available propagators | Search, drag-to-add, categorized |

### 2. Debugging & Analysis Widgets

| Widget | Purpose | Key Features |
|--------|---------|--------------|
| **Execution Timeline** | Visualize propagator executions | Time-based view, click for details, zoom |
| **Cell History** | Time-travel through cell values | Version timeline, diff, provenance |
| **Propagator Debugger** | Debug execution in real-time | Breakpoints, step, inspect I/O |
| **AST Viewer** | Visualize syntax trees | Expand/collapse, search, export |

### 3. Performance Widgets

| Widget | Purpose | Key Features |
|--------|---------|--------------|
| **Flame Graph** | Profile CPU/memory usage | Interactive, drill-down, hotspot ID |
| **Performance Dashboard** | Network-wide metrics | Throughput, latency, trends |
| **Diff Viewer** | Compare code versions | Side-by-side, syntax highlighted |

### 4. Generated Artifacts Widgets

| Widget | Purpose | Key Features |
|--------|---------|--------------|
| **Documentation Panel** | Auto-generated docs | Markdown, diagrams, examples |
| **Custom Inspector** | Type-specific debugger (generated) | Auto-layout, property view, validation |
| **Code Preview** | Live generated code view | Syntax highlighting, tabs, export |

### 5. Advanced Widgets

| Widget | Purpose | Key Features |
|--------|---------|--------------|
| **Constraint Editor** | Define bidirectional constraints | Visual relationship editor, test |
| **Batch Transformation** | Bulk operations on elements | Preview, impact analysis, undo |
| **Cell Value Visualizer** | Complex value visualization | Tree/graph/table views, interactive |

---

## Notebook Cell Types

### Network Cell
**Primary cell for propagator networks**
- **Edit Mode**: Visual network canvas
- **View Mode**: Generated code/docs/debuggers
- **Toggle**: `Ctrl+Shift+E` / `Ctrl+Shift+V`

### Code Cell
Traditional imperative code

### Markdown Cell
Documentation with literate programming

### Test Cell
Unit tests that run automatically

---

## Key Interaction Patterns

### 1. Edit/View Toggle
```
EDIT: Visual network → VIEW: Generated code
[Ctrl+Shift+E] ⟷ [Ctrl+Shift+V]
```

### 2. Drag-Drop Propagator
```
Palette → Drag → Canvas → Drop → Configure
```

### 3. Connect Cells
```
Click port → Drag → Drop on compatible cell → Validate
```

### 4. Time-Travel Debug
```
Right-click cell → View History → Scrub timeline → See provenance
```

### 5. Live Preview
```
Adjust parameter → Code updates instantly → View diff
```

---

## Workflow Comparison

### Traditional (Zig/Rust/C++)
```
Edit text → Compile → Wait → Fix errors → Repeat
• Slow feedback (minutes)
• Opaque compilation
• Separate tools
```

### CPSC (Propagator CAD)
```
Edit network → Instant compile → View result → Iterate
• Fast feedback (milliseconds)
• Transparent compilation  
• Integrated tools
```

---

## Core Scenarios

### 1. Code Generation (No Macros)
Edit propagator network → Adjust parameters → See generated code → Done

### 2. Aspect Orientation
Select functions → Apply aspect → Preview injection → Commit

### 3. Documentation Generation
Network auto-generates docs from structure + comments

### 4. Debugger Generation
Type definition → Auto-generate custom inspector widget

### 5. Performance Profiling
Run with profiling → View flame graph → Click to source → Optimize

### 6. Code Transformation
View AST at each compilation pass → See diffs → Understand optimizations

---

## Panel Layout

```
┌─────────────────────────────────────────────────┐
│ VSCode Window                                   │
├──────────┬──────────────────────┬───────────────┤
│ EXPLORER │ EDITOR (Notebooks)   │ SIDE PANEL    │
│          │                      │               │
│ • Networks│ ┌─Network Cell─┐   │ • Inspector   │
│ • Cells   │ │ [EDIT|VIEW]  │   │ • Palette     │
│ • Props   │ │              │   │ • Debugger    │
│          │ │ Canvas/Code  │   │ • Profiler    │
│          │ └──────────────┘   │               │
└──────────┴──────────────────────┴───────────────┘
```

---

## Quick Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| Command Palette | `Ctrl+Shift+P` | All commands |
| Toggle Mode | `Ctrl+Shift+E/V` | Edit ↔ View |
| Add Propagator | `Ctrl+Shift+A` | Open palette |
| Run Cell | `Shift+Enter` | Execute network |
| View History | `Ctrl+H` | Cell time-travel |
| Debug | `F5` | Start debugging |
| Profile | `Ctrl+Shift+F5` | Start profiling |

---

## Widget Properties

### Good Widget Design Principles

1. **Context-Aware**: Adapts to selection
2. **Live Updates**: Changes reflect immediately
3. **Keyboard Friendly**: All operations accessible via keys
4. **Visual Clarity**: Information dense but clear
5. **Undo/Redo**: All operations reversible
6. **Export**: Can export data/results

### Widget Communication

Widgets communicate via:
- **Selection events**: Clicking element updates all widgets
- **Value change events**: Modifying propagates to dependents
- **Execution events**: Running network updates timeline/profiler
- **History events**: Time-travel updates all views

---

## Example Workflow

### Building a Parser with Interactive Code Generation

1. **Create Network Cell** in notebook
2. **Switch to EDIT MODE** (default)
3. **Drag "Parser" propagator** from palette to canvas
4. **Connect cells**: Source → Parser → AST
5. **Open Property Inspector**: Adjust parser parameters
6. **Switch to VIEW MODE**: See generated parser code
7. **Click "Documentation" tab**: See auto-generated docs
8. **Click "Debugger" tab**: See custom AST inspector
9. **Run tests**: Test cell executes automatically
10. **Profile**: Check performance with flame graph
11. **Iterate**: Back to EDIT MODE, adjust, repeat

**Time to first working parser: ~5 minutes**
**vs Traditional approach: ~hours**

---

## Advanced Features

### Constraint Programming
Define bidirectional relationships between cells
```
[Celsius] ⚖️ [Fahrenheit]
C = (F - 32) × 5/9
```

### Batch Operations
Apply transformations to multiple elements at once
```
Select all → Add logging aspect → Preview → Apply
```

### Provenance Tracking
Understand why cell has current value
```
Click cell → View history → See which propagator produced it
```

### Custom Visualizations
Auto-generate visualizers from type definitions
```
B-Tree type → Tree visualizer widget
Graph type → Graph layout widget
```

---

## Technology Stack

- **VSCode Extension API**
- **Webviews** (React/Svelte for custom widgets)
- **Canvas API** (Network visualization)
- **D3.js** (Data visualization)
- **Monaco Editor** (Code editing)
- **LSP Integration** (Language server connection)

---

## Performance Tips

1. **Virtual Rendering**: Large networks render only visible nodes
2. **Debounced Updates**: Live previews throttle during fast changes
3. **Web Workers**: Heavy computation offloaded
4. **Incremental**: Only re-render changed portions

---

## Accessibility

- ✓ Full keyboard navigation
- ✓ Screen reader support (ARIA labels)
- ✓ High contrast theme support
- ✓ Zoom support

---

**See full specification**: `vscode-ui-specification.md`



