# VSCode Propagator CAD System - UI/UX Specification

## Table of Contents

1. [Overview](#overview)
2. [Workflow Model Comparison](#workflow-model-comparison)
3. [Core Scenarios](#core-scenarios)
4. [UI Architecture](#ui-architecture)
5. [Notebook Cell Design](#notebook-cell-design)
6. [Widget Catalog](#widget-catalog)
7. [Panel Specifications](#panel-specifications)
8. [Interaction Patterns](#interaction-patterns)

---

## Overview

This document specifies a **Software CAD System** for building propagator networks within VSCode. The system enables:

- **Literate Programming**: Code lives in notebooks with rich documentation
- **Visual Network Editing**: CAD-like interface for propagator networks
- **Compile-Time Interactivity**: Generate code, aspects, docs, and debuggers without macros
- **Dual-Mode Editing**: Edit network structure ↔ View generated artifacts
- **Professional Tooling**: Designed for experienced engineers expecting IDE-grade quality

### Design Philosophy

**"CAD for Computation"** - Treat propagator networks as first-class visual artifacts that generate code, not the other way around.

**Key Principles:**

- **Immediate feedback**: Changes to network immediately show impact
- **Reversible exploration**: All edits are tracked and revertible
- **Contextual intelligence**: Tools adapt to what you're editing
- **Zero ceremony**: Common operations are one click or keystroke
- **Professional aesthetics**: Clean, functional, information-dense

---

## Workflow Model Comparison

### Traditional Workflow (Zig/Rust/C++)

```text
┌─────────────────────────────────────────────────────────────┐
│  TRADITIONAL: Text-First, Compile-Execute Cycle             │
└─────────────────────────────────────────────────────────────┘

Edit Source Code (Linear Text)
   ↓
Invoke Compiler (batch process)
   ↓
Wait for Compilation
   ↓
Fix Errors → Edit Again
   ↓
Run Program
   ↓
Debug with External Tools

Characteristics:
├─ Edit-compile-run loop (slow feedback)
├─ Compilation is opaque (no visibility into process)
├─ Debugging is separate from authoring
├─ Documentation separate from code
├─ No compile-time interactivity
├─ Text is the only interface
└─ Tools are disconnected (editor, compiler, debugger, profiler)
```

### CPSC Workflow Model

```text
┌─────────────────────────────────────────────────────────────┐
│  CPSC: Notebook-First, Interactive Compilation              │
└─────────────────────────────────────────────────────────────┘

Notebook Cell (Literate Programming)
   ↓
Edit Propagator Network (Visual CAD)  ←─┐
   ↓                                     │
Continuous Compilation                   │
   ↓                                     │
View Generated Code                      │
   ↓                                     │
Interactive Aspects (injected at compile time) │
   ↓                                     │
Documentation Generated                  │
   ↓                                     │
Custom Debuggers Generated               │
   ↓                                     │
All Results in Same Notebook ───────────┘

Characteristics:
├─ Instant feedback (incremental compilation)
├─ Compilation is visible (inspect intermediate steps)
├─ Debugging is integrated (custom inspectors generated)
├─ Documentation is generated (always in sync)
├─ Compile-time interactivity (CAD-like manipulation)
├─ Visual + text interfaces (dual representation)
└─ All tools integrated in notebook environment

Key Innovation: EDIT/VIEW MODE TOGGLE
  - EDIT mode: manipulate propagator network structure
  - VIEW mode: see generated code, docs, debuggers
  - Seamless switching preserves context
```

### Comparison Matrix

| Aspect                 | Traditional (Zig/Rust/C++)         | CPSC (Propagator CAD)        |
|:-----------------------|:-----------------------------------|:-----------------------------|
| **Primary Interface**  | Text editor                        | Notebook + Visual CAD        |
| **Feedback Loop**      | Minutes (full compile)             | Milliseconds (incremental)   |
| **Code Generation**    | Macros (limited)                   | Full metaprogramming         |
| **Aspect Orientation** | External frameworks                | Built-in at compile time     |
| **Documentation**      | Separate tools (Doxygen)           | Auto-generated from network  |
| **Debugging**          | GDB/LLDB (generic)                 | Custom inspectors per type   |
| **Visibility**         | Opaque compilation                 | Full transparency            |
| **Explorability**      | Trial and error                    | Interactive simulation       |
| **Literate Programming**| Comments only                     | First-class notebooks        |
| **Tool Integration**   | Fragmented                         | Unified environment          |

---

## Core Scenarios

### Scenario 1: Code Generation at Compile Time (No Macros, CAD-Interactive)

**Goal**: Generate boilerplate code interactively without macro magic.

**Traditional Approach (C++ Templates/Rust Macros)**:

```cpp
// Write macro invocation
GENERATE_SERIALIZATION(MyStruct);

// Invoke compiler to see result
// If wrong, edit macro source and recompile
// No visibility into generation process
```

**CPSC Approach**:

1. **Create Propagator Network** (Edit Mode):

   ```text
   [Type Definition Cell] 
      → [Serialization Generator Propagator]
          → [Serialize Function Cell]
          → [Deserialize Function Cell]
          → [Schema Cell]
   ```

2. **Manipulate in CAD Interface**:

   - Drag type definition into network
   - Connect to serialization generator
   - Instantly see generated functions
   - Tweak generator parameters with sliders/dropdowns
   - See results update live

3. **View Generated Code** (View Mode):

   - Generated `serialize()` function with syntax highlighting
   - Generated `deserialize()` function
   - Generated schema definition
   - All in separate tabs within same notebook cell

**UI Components**:

- **Network Canvas**: Drag-and-drop propagator network editor
- **Property Inspector**: Real-time parameter adjustment
- **Code Preview Panel**: Side-by-side generated code view
- **Diff Viewer**: Show changes as parameters adjust

**Workflow**:

```
Edit Network → Adjust Parameters → See Code → Iterate
     ↑                                            ↓
     └────────────────────────────────────────────┘
            (Instant feedback loop)
```

---

### Scenario 2: Interactive Aspect Orientation at Compile Time

**Goal**: Inject cross-cutting concerns (logging, tracing, metrics) interactively.

**Traditional Approach (AOP Frameworks)**:

```java
// Define pointcut (separate file)
@Aspect
public class LoggingAspect {
  @Before("execution(* com.example.*.*(..))")
  public void log(JoinPoint jp) { ... }
}

// Compile with aspect weaver
// No visibility into what gets woven where
```

**CPSC Approach**:

1. **Define Aspect as Propagator**:

   ```text
   [Function AST Cell]
      → [Logging Aspect Propagator]
          ↓
      [Instrumented AST Cell]
   ```

2. **Interactive Configuration**:

   - Select which functions to instrument (visual selector)
   - Choose logging level per function
   - Preview instrumented code before committing
   - See performance impact estimate

3. **Compile-Time Application**:

   - Aspect applied during compilation
   - Generated code includes injected logic
   - No runtime overhead (all compile-time)

**UI Components**:

- **Aspect Selector**: Multi-select tree of all functions
- **Policy Editor**: Define when aspect applies (predicates)
- **Impact Visualizer**: Show AST diff before/after aspect
- **Performance Estimator**: Predict overhead in cycles

**Widget Example: Aspect Selector**:

```text
┌─ Aspect: Logging ────────────────────────────┐
│ Target Functions:                            │
│  ☑ module::function_a                        │
│  ☑ module::function_b                        │
│  ☐ module::function_c                        │
│                                              │
│ Logging Level: [Debug ▼]                     │
│ Include Args: [Yes ▼]                        │
│ Include Return: [Yes ▼]                      │
│                                              │
│ Estimated Overhead: ~45 cycles/call          │
│                                              │
│ [Apply] [Preview Diff] [Cancel]              │
└──────────────────────────────────────────────┘
```

---

### Scenario 3: Documentation Generation

**Goal**: Auto-generate comprehensive documentation from propagator network.

**Traditional Approach (Doxygen/rustdoc)**:

```rust
/// Manual comment - can drift from code
/// @param x The input value
/// @return The computed result
fn compute(x: i32) -> i32 { ... }

// Run doxygen separately
// Generated docs often outdated
```

**CPSC Approach**:

1. **Documentation Propagator**:

   ```text
   [Type Definitions]
   [Function Signatures]  → [Doc Generator] → [Markdown Docs]
   [Network Structure]                        [API Reference]
                                              [Architecture Diagrams]
   ```

2. **Rich Documentation**:

   - API reference (auto-generated)
   - Architecture diagrams (from network topology)
   - Example code (from test cells)
   - Performance characteristics (from profiling data)
   - Provenance (which propagators produced what)

3. **Always Current**:

   - Docs regenerate on any change
   - Impossible to be out of sync
   - Embedded in notebook for literate programming

**UI Components**:

- **Doc Preview Panel**: Live markdown rendering
- **Diagram Generator**: Auto-layout network as architecture diagram
- **Example Extractor**: Pull code from test cells
- **Export Widget**: Generate standalone docs site

---

### Scenario 4: Custom Debugger Tool Generation

**Goal**: Generate type-specific debuggers and inspectors automatically.

**Traditional Approach (GDB)**:

```text
(gdb) print my_complex_structure
$1 = {field1 = 0x7fff..., field2 = 0x...}  // Opaque pointers
```

**CPSC Approach**:

1. **Debugger Generator Propagator**:

   ```text
   [Type Definition]
      → [Debugger Generator]
          → [Inspector Widget Code]
          → [Visualizer Code]
          → [Pretty Printer Code]
   ```

2. **Generated Tools**:

   - **Type Inspector**: Custom UI for struct/class
   - **Graph Visualizer**: For trees/graphs (auto-layout)
   - **Timeline View**: For event sequences
   - **Memory Visualizer**: For buffers/arrays

3. **Integrated Debugging**:

   - Debugger widgets embedded in notebook
   - Attach to running process
   - Use generated inspectors
   - All synchronized with source

**UI Components**:

- **Inspector Canvas**: Container for custom debugger widgets
- **Data Binding**: Connect live variables to widgets
- **Layout Editor**: Arrange debugger UI
- **Widget Library**: Pre-built components (trees, tables, graphs)

**Example: Generated B-Tree Inspector**:

```text
┌─ B-Tree Inspector ────────────────────────────┐
│ Address: 0x7fff8a3c2000                       │
│ Height: 4                                     │
│ Node Count: 127                               │
│                                               │
│ [Tree Visualization]                          │
│         ┌─[50]─┐                              │
│    ┌─[25]─┐  ┌─[75]─┐                         │
│  [10][40] [60][80][90]                        │
│   ...                                         │
│                                               │
│ Selected Node: 0x...                          │
│  Keys: [60, 65, 68]                           │
│  Children: [0x..., 0x..., 0x..., 0x...]       │
│                                               │
│ [Step Into] [Step Over] [Watch Node]          │
└───────────────────────────────────────────────┘
```

---

### Scenario 5: Interactive Performance Profiling

**Goal**: Profile code and visualize performance interactively.

**CPSC Approach**:

1. **Profiling Propagator**:

   ```text
   [Compiled Code]
      → [Profiler Instrumenter]
          → [Instrumented Binary]
              → [Execution]
                  → [Profile Data]
                      → [Flame Graph]
                      → [Call Tree]
                      → [Hotspot List]
   ```

2. **Interactive Analysis**:

   - Run code with profiling
   - Visualize results in notebook
   - Click flame graph to jump to source
   - Adjust code and re-profile
   - Compare before/after

**UI Components**:

- **Flame Graph Widget**: Interactive flame chart
- **Hotspot Panel**: Sortable function list
- **Timeline View**: Execution timeline
- **Comparison View**: Side-by-side profiling runs

---

### Scenario 6: Live Code Transformation Preview

**Goal**: See code transformations (optimizations, transpilation) live.

**CPSC Approach**:

1. **Transformation Chain**:

   ```text
   [Source AST]
      → [Optimization Pass 1] → [Intermediate AST 1]
      → [Optimization Pass 2] → [Intermediate AST 2]
      → [Code Generator]      → [Machine Code]
   ```

2. **Visualization**:

   - Show AST at each step
   - Diff between steps
   - Highlight what changed
   - Explain why transformation applied

**UI Components**:

- **AST Viewer**: Tree widget for syntax trees
- **Transformation Pipeline**: Visual flow of passes
- **Diff Highlighter**: Show AST changes
- **Explanation Panel**: Why each transformation occurred

---

## UI Architecture

### Layout Structure

```text
┌────────────────────────────────────────────────────────────┐
│ VSCode Window                                              │
├────────────────────────────────────────────────────────────┤
│ ┌────────────────┬────────────────────────┬──────────────┐ │
│ │                │                        │              │ │
│ │  EXPLORER      │  EDITOR AREA           │  SIDE PANEL  │ │
│ │                │  (Notebooks)           │              │ │
│ │  • Files       │                        │  • Inspector │ │
│ │  • Networks    │  ┌──────────────────┐  │  • Palette   │ │
│ │  • Cells       │  │ Notebook Cell    │  │  • Debugger  │ │
│ │  • Propagators │  │                  │  │  • Profiler  │ │
│ │                │  │ [EDIT/VIEW mode] │  │  • Docs      │ │
│ │                │  │                  │  │              │ │
│ │                │  │ ┌──────────────┐ │  │              │ │
│ │                │  │ │Network Canvas│ │  │              │ │
│ │                │  │ │   or         │ │  │              │ │
│ │                │  │ │ Code View    │ │  │              │ │
│ │                │  │ └──────────────┘ │  │              │ │
│ │                │  └──────────────────┘  │              │ │
│ └────────────────┴────────────────────────┴──────────────┘ │
├────────────────────────────────────────────────────────────┤
│ STATUS BAR: Network Status | Compilation | Diagnostics     │
└────────────────────────────────────────────────────────────┘
```

### Panel Hierarchy

```text
  VSCode Extension: "Propagator CAD"
  ├─ Explorer Views
  │  ├─ Network Explorer
  │  ├─ Cell Browser
  │  ├─ Propagator Library
  │  └─ Execution History
  │
  ├─ Editor Providers
  │  ├─ Notebook Editor (CPSC Notebooks)
  │  │  ├─ Cell Renderer
  │  │  │  ├─ Network Canvas (Edit Mode)
  │  │  │  └─ Code/Doc Viewer (View Mode)
  │  │  └─ Cell Toolbar
  │  │     ├─ Mode Toggle (Edit ↔ View)
  │  │     ├─ Run Cell
  │  │     └─ Export
  │  │
  │  └─ Network Canvas Editor (standalone)
  │
  ├─ Side Panels
  │  ├─ Property Inspector
  │  ├─ Propagator Palette
  │  ├─ Debugger Panel
  │  ├─ Profiler Panel
  │  └─ Documentation Panel
  │
  ├─ Webview Panels (Custom UI)
  │  ├─ Network Visualizer
  │  ├─ Flame Graph Widget
  │  ├─ AST Viewer
  │  └─ Custom Inspectors
  │
  └─ Commands & Menus
     ├─ Create New Network
     ├─ Add Propagator
     ├─ Connect Cells
     ├─ Toggle Edit/View Mode
     ├─ Run Network
     └─ Generate Docs/Debugger
```

---

## Notebook Cell Design

### Cell Types

#### 1. **Network Cell** (Primary cell type)

**Purpose**: Contains propagator network definition and generated artifacts.

**Structure**:

```text
┌─ Network Cell ─────────────────────────────────────────────┐
│ ┌────────────────────────────────────────────────────────┐ │
│ │ 📘 CELL HEADER                                         │ │
│ │  Type: Network | Name: "parser_network"                │ │
│ │  [✎ EDIT MODE] [👁 VIEW MODE] [▶ Run] [⚙ Settings]     │ │
│ └────────────────────────────────────────────────────────┘ │
│                                                            │
│ ┌────────────────────────────────────────────────────────┐ │
│ │ 🗺 NETWORK CANVAS (Edit Mode)                          │ │
│ │                                                        │ │
│ │  [Source Cell] → [Parser] → [AST Cell]                 │ │
│ │                     ↓                                  │ │
│ │                 [Errors]                               │ │
│ │                                                        │ │
│ │  (Drag propagators from palette, connect cells)        │ │
│ └────────────────────────────────────────────────────────┘ │
│                                                            │
│ OR (when in VIEW MODE)                                     │
│                                                            │
│ ┌────────────────────────────────────────────────────────┐ │
│ │ 📄 GENERATED CODE                                      │ │
│ │                                                        │ │
│ │  ```rust                                               │ │
│ │  fn parse(input: &str) -> Result<AST> {                │ │
│ │    // Generated from parser_network                    │ │
│ │    ...                                                 │ │
│ │  }                                                     │ │
│ │  ```                                                   │ │
│ └────────────────────────────────────────────────────────┘ │
│                                                            │
│ ┌────────────────────────────────────────────────────────┐ │
│ │ 📊 CELL OUTPUTS                                        │ │
│ │  • Compilation: ✓ Success (234ms)                      │ │
│ │  • Tests: 15/15 passed                                 │ │
│ │  • Generated: parser.rs, parser_test.rs                │ │
│ └────────────────────────────────────────────────────────┘ │
└────────────────────────────────────────────────────────────┘
```

**Cell Header Components**:

- **Mode Toggle**: Switch between EDIT (network) and VIEW (code)
- **Run Button**: Execute propagator network
- **Settings**: Cell-level configuration

**Edit Mode - Network Canvas**:

- Drag-and-drop interface
- Pan and zoom
- Mini-map for large networks
- Connection validation (type checking)

**View Mode - Tabbed Output**:

```text
┌─ View Mode ────────────────────────────────────────────────┐
│ [Generated Code] [Documentation] [Debugger] [Tests]        │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  (Selected tab content with syntax highlighting)           │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

---

#### 2. **Code Cell** (Traditional code)

**Purpose**: Write imperative code that doesn't need network representation.

```text
┌─ Code Cell ──────────────────────────────────────────────┐
│ 🔧 Helper Functions                                      │
│ [▶ Run] [⚙ Settings]                                     │
├──────────────────────────────────────────────────────────┤
│ ```rust                                                  │
│ fn utility_function(x: i32) -> i32 {                     │
│   x * 2                                                  │
│ }                                                        │
│ ```                                                      │
├──────────────────────────────────────────────────────────┤
│ ✓ Compiled successfully                                  │
└──────────────────────────────────────────────────────────┘
```

---

#### 3. **Markdown Cell** (Documentation)

**Purpose**: Write rich documentation with literate programming.

```text
┌─ Markdown Cell ───────────────────────────────────────────┐
│ ## Parser Implementation                                  │
│                                                           │
│ This network implements a recursive descent parser for... │
│                                                           │
│ Key design decisions:                                     │
│ - Use PEG parsing for unambiguous grammar                 │
│ - Generate AST directly (no parse tree)                   │
│ - ...                                                     │
└───────────────────────────────────────────────────────────┘
```

---

#### 4. **Test Cell** (Unit tests)

**Purpose**: Define tests that run automatically when network changes.

```text
┌─ Test Cell ──────────────────────────────────────────────┐
│ 🧪 Parser Tests                                          │
│ [▶ Run All] [⚙ Settings]                                 │
├──────────────────────────────────────────────────────────┤
│ ✓ test_parse_integer (12ms)                              │
│ ✓ test_parse_string (8ms)                                │
│ ✗ test_parse_complex (45ms)                              │
│   Expected: AST { ... }                                  │
│   Got: ParseError("unexpected token")                    │
├──────────────────────────────────────────────────────────┤
│ 2/3 tests passed                                         │
└──────────────────────────────────────────────────────────┘
```

---

## Widget Catalog

### 1. Network Canvas Widget

**Purpose**: Visual editor for propagator networks.

**Features**:

- Infinite canvas with pan/zoom
- Snap-to-grid alignment
- Auto-layout algorithms (force-directed, hierarchical)
- Multi-select and bulk operations
- Undo/redo
- Copy/paste networks

**Visual Design**:

```text
┌─ Network Canvas ──────────────────────────────────────────┐
│ 🔍 [100%] [⊞ Auto-layout] [⊟ Minimap] [⚙ Settings]        │
├───────────────────────────────────────────────────────────┤
│                                                           │
│      ┌───────────┐           ┌───────────┐                │
│      │ Source    │           │ Parser    │                │
│      │ Cell      │──────────▶│           │                │
│      │ TextDoc   │           │ Primitive │                │
│      └───────────┘           └─────┬─────┘                │
│                                    │                      │
│                                    ▼                      │
│                            ┌───────────┐                  │
│                            │ AST Cell  │                  │
│                            │ ATerm     │                  │
│                            └───────────┘                  │
│                                                           │
│ (Right-click for context menu, drag to connect)           │
└───────────────────────────────────────────────────────────┘
```

**Node Styles**:

- **Cells**: Rounded rectangles, color-coded by type
  - TextDocument: Blue
  - ATerm: Green
  - Diagnostic: Red
  - WorkspaceEdit: Yellow

- **Propagators**: Sharp rectangles with icons
  - Primitive: ⚙️
  - Compound: 📦
  - Conditional: 🔀
  - Constraint: ⚖️

**Connections**:

- Solid lines: Data flow
- Dashed lines: Control dependencies
- Animated dots: Active propagation

---

### 2. Property Inspector Widget

**Purpose**: Edit properties of selected network element.

**Adaptive Interface**: Changes based on selection.

**Cell Selected**:

```text
┌─ Property Inspector ──────────────────────────────────────┐
│ Cell: "source_file"                                       │
├───────────────────────────────────────────────────────────┤
│ Type: TextDocument                                        │
│ Name: [source_file      ]                                 │
│ Role: INPUT                                               │
│                                                           │
│ Current Value:                                            │
│  Version: 5                                               │
│  Length: 1234 chars                                       │
│  Modified: 2 minutes ago                                  │
│                                                           │
│ [View History] [Clear Cell]                               │
└───────────────────────────────────────────────────────────┘
```

**Propagator Selected**:

```text
┌─ Property Inspector ──────────────────────────────────────┐
│ Propagator: "parser"                                      │
├───────────────────────────────────────────────────────────┤
│ Type: Primitive                                           │
│ Function: parse_typescript                                │
│ Enabled: ☑                                                │
│                                                           │
│ Parameters:                                               │
│  Strict Mode: ☑                                           │
│  Target: [ES2022 ▼]                                       │
│  JSX: [React ▼]                                           │
│                                                           │
│ Performance:                                              │
│  Avg Execution: 45ms                                      │
│  Last Run: 2s ago                                         │
│  Runs Today: 127                                          │
│                                                           │
│ [Edit Function] [View Logs]                               │
└───────────────────────────────────────────────────────────┘
```

---

### 3. Propagator Palette Widget

**Purpose**: Library of available propagators to drag into network.

```text
┌─ Propagator Palette ──────────────────────────────────────┐
│ [Search propagators...]                                   │
├───────────────────────────────────────────────────────────┤
│ 📁 Parsing                                                │
│   ⚙️ Parse TypeScript                                     │
│   ⚙️ Parse JSON                                           │
│   ⚙️ Parse Markdown                                       │
│                                                           │
│ 📁 Type Checking                                          │
│   ⚙️ Infer Types                                          │
│   ⚙️ Check Constraints                                    │
│                                                           │
│ 📁 Code Generation                                        │
│   ⚙️ Generate Serialization                               │
│   ⚙️ Generate Tests                                       │
│   ⚙️ Generate Documentation                               │
│                                                           │
│ 📁 Analysis                                               │
│   ⚙️ Find References                                      │
│   ⚙️ Compute Metrics                                      │
│                                                           │
│ 📁 Custom                                                 │
│   ⚙️ My Propagator 1                                      │
│   [+ Create New]                                          │
└───────────────────────────────────────────────────────────┘
```

**Interaction**:

- Drag propagator to canvas
- Double-click to add at center
- Hover for description tooltip

---

### 4. Execution Timeline Widget

**Purpose**: Visualize propagator executions over time.

```text
┌─ Execution Timeline ───────────────────────────────────────┐
│ [⏸ Pause] [▶ Resume] [⏮ Step] Time: 12:34:56              │
├────────────────────────────────────────────────────────────┤
│                                                            │
│ Parser     ████░░░░░░░░░░░░████░░░░░░░                     │
│ TypeCheck          ████████░░░░░░░░░░░                     │
│ Linter         ██░░░░░░░░░░░░░░░██                         │
│ CodeGen                      ████████████                  │
│                                                            │
│            └────────┬────────┬────────┬────────┘           │
│                  0.0s     0.5s     1.0s     1.5s           │
│                                                            │
│ Selected: TypeCheck execution at 0.3s (duration: 200ms)    │
│  Inputs: AST v3                                            │
│  Outputs: Types v3, Diagnostics v5                         │
│                                                            │
│ [Jump to Cell] [View Inputs] [View Outputs]                │
└────────────────────────────────────────────────────────────┘
```

**Features**:

- Zoom into time range
- Click bar to see execution details
- Color-coded by status (green=success, red=fail)
- Overlay shows dependencies

---

### 5. Diff Viewer Widget

**Purpose**: Show differences between generated code versions.

```text
┌─ Code Diff: parser.rs ────────────────────────────────────┐
│ Version 4 ← → Version 5                                   │
├────────────────────────┬──────────────────────────────────┤
│ fn parse(input: &str) { │ fn parse(input: &str) {         │
│   let mut tokens = ... │   let mut tokens = ...           │
│ -  tokenize(input);     │ +  tokenize_fast(input);        │
│   ...                   │   ...                           │
│ }                       │   ...                           │
│                         │ +  // Optimized path for common │
│                         │ +  if is_simple(input) {        │
│                         │ +    return fast_path(input);   │
│                         │ +  }                            │
│                         │ }                               │
└─────────────────────────┴─────────────────────────────────┘
```

**Features**:

- Side-by-side or inline diff
- Syntax highlighting preserved
- Navigate between changes
- Three-way merge for conflicts

---

### 6. AST Viewer Widget

**Purpose**: Visualize abstract syntax trees.

```text
┌─ AST Viewer ──────────────────────────────────────────────┐
│ [▽ Expand All] [△ Collapse All] [🔍 Search]               │
├───────────────────────────────────────────────────────────┤
│ ▽ Program                                                 │
│   ▽ FunctionDeclaration "parse"                           │
│     ▽ Parameters                                          │
│       • Parameter "input" (type: &str)                    │
│     ▽ Body                                                │
│       ▽ BlockStatement                                    │
│         ▽ LetDeclaration "tokens"                         │
│           • CallExpression "tokenize"                     │
│         ▽ ForLoop                                         │
│           ...                                             │
│                                                           │
│ [Export as JSON] [Copy Path] [Jump to Source]             │
└───────────────────────────────────────────────────────────┘
```

**Features**:

- Tree view with expand/collapse
- Search/filter nodes
- Highlight corresponding source code
- Export to JSON/XML

---

### 7. Flame Graph Widget

**Purpose**: Performance profiling visualization.

```text

┌─ Flame Graph ─────────────────────────────────────────────┐
│ [⚡ Profile Mode: CPU ▼] [🎯 Focus: All ▼] [↻ Refresh]     │
├───────────────────────────────────────────────────────────┤
│                                                           │
│ parse ████████████████████████████████████████████ 1.2s   │
│  tokenize ████████████████████ 0.6s                       │
│   next_token ██████████ 0.3s                              │
│   ...                                                     │
│  build_ast ████████████████ 0.5s                          │
│   create_node ████████ 0.3s                               │
│    allocate ████ 0.1s                                     │
│   ...                                                     │
│                                                           │
│ (Click box to zoom in, hover for details)                 │
│                                                           │
│ Selected: tokenize (0.6s, 50% of parse)                   │
│  Called: 1,234 times                                      │
│  Avg: 0.5ms per call                                      │
│                                                           │
│ [Jump to Source] [View Callers] [View Callees]            │
└───────────────────────────────────────────────────────────┘
```

---

### 8. Cell History Widget

**Purpose**: Time-travel through cell value history.

```text
┌─ Cell History: "ast_cell" ─────────────────────────────────┐
│ [◀ Prev] [▶ Next] [⏮ First] [⏭ Latest]                    │
├────────────────────────────────────────────────────────────┤
│ Timeline:                                                  │
│  • v5 (current) - 2 min ago - Added by: parser             │
│  • v4          - 5 min ago - Added by: parser              │
│  • v3          - 8 min ago - Added by: parser              │
│  • v2          - 15 min ago - Added by: parser             │
│  • v1          - 1 hour ago - Added by: parser             │
│                                                            │
│ Currently viewing: v5                                      │
│                                                            │
│ Value Preview:                                             │
│  Program(                                                  │
│    body: [                                                 │
│      FunctionDecl("parse", ...),                           │
│      ...                                                   │
│    ]                                                       │
│  )                                                         │
│                                                            │
│ [Diff with v4] [Revert to this] [Export]                   │
└────────────────────────────────────────────────────────────┘
```

---

### 9. Custom Inspector Widget (Generated)

**Purpose**: Type-specific debugger UI (auto-generated).

**Example: B-Tree Inspector** (generated from type definition):

```text
┌─ B-Tree Inspector ────────────────────────────────────────┐
│ Instance: tree_instance @ 0x7fff8a3c2000                  │
├───────────────────────────────────────────────────────────┤
│ Properties:                                               │
│  Height: 4                                                │
│  Node Count: 127                                          │
│  Key Count: 1,523                                         │
│  Fill Factor: 78%                                         │
│                                                           │
│ [Tree View] [Memory View] [Stats]                         │
├───────────────────────────────────────────────────────────┤
│ Tree Visualization:                                       │
│                                                           │
│            ┌────[50]────┐                                 │
│      ┌────[25]────┐   ┌────[75]────┐                      │
│   [10][15][20] [30][40] [60][65] [80][90]                 │
│     ...            ...      ...      ...                  │
│                                                           │
│ Selected Node: [25] (0x7fff8a3c2100)                      │
│  Keys: [25, 28, 30]                                       │
│  Children: 4 pointers                                     │
│  Parent: [50] (0x7fff8a3c2000)                            │
│                                                           │
│ [Expand Node] [Find Key] [Validate Tree]                  │
└───────────────────────────────────────────────────────────┘
```

**Features** (all auto-generated from type definition):

- Property display
- Visualizations appropriate to type
- Navigation (follow pointers)
- Validation (invariant checking)
- Memory layout view

---

### 10. Documentation Panel Widget

**Purpose**: Show auto-generated docs side-by-side with code.

```text
┌─ Documentation ────────────────────────────────────────────┐
│ [📖 Overview] [🔧 API] [🏗 Architecture] [📊 Examples]     │
├────────────────────────────────────────────────────────────┤
│ # Parser Network                                           │
│                                                            │
│ ## Overview                                                │
│ This network implements a TypeScript parser using PEG...   │
│                                                            │
│ ## Architecture                                            │
│ [Network Diagram]                                          │
│   Source Cell → Parser Propagator → AST Cell               │
│                      ↓                                     │
│                 Error Cell                                 │
│                                                            │
│ ## API                                                     │
│ ### Input Cells                                            │
│ - `source_file` (TextDocument): Source code to parse       │
│                                                            │
│ ### Output Cells                                           │
│ - `ast` (ATerm): Parsed abstract syntax tree               │
│ - `errors` (Diagnostic): Parse errors if any               │
│                                                            │
│ ## Performance                                             │
│ - Average: 45ms for typical file (1000 LOC)                │
│ - Peak memory: 2.3 MB                                      │
│                                                            │
│ ## Examples                                                │
│ [See test cells below]                                     │
│                                                            │
│ [Export as Markdown] [Copy Link]                           │
└────────────────────────────────────────────────────────────┘
```

---

## Panel Specifications

### Explorer Panel: Network Explorer

**Purpose**: Navigate project's propagator networks.

```text
┌─ PROPAGATOR NETWORKS ─────────────────────────────────────┐
│ [🔍 Search networks...]                                   │
├───────────────────────────────────────────────────────────┤
│ 📁 parser_project                                         │
│   📁 networks                                             │
│     🗺 parser_network ⚙️ RUNNING                          │
│       🔵 source_cell (INPUT)                              │
│       🟢 ast_cell (OUTPUT)                                │
│       🔴 error_cell (OUTPUT)                              │
│       ⚙️ parser_propagator                                │
│       ⚙️ validator_propagator                             │
│     🗺 typechecker_network 💤 IDLE                        │
│       ...                                                 │
│   📁 tests                                                │
│     🧪 parser_tests.nb                                    │
│     🧪 typecheck_tests.nb                                 │
│   📁 docs                                                 │
│     📄 architecture.md                                    │
│                                                           │
│ [+ New Network] [↻ Refresh]                               │
└───────────────────────────────────────────────────────────┘
```

**Features**:

- Hierarchical view of networks
- Status indicators (running, idle, error)
- Quick navigation to cells/propagators
- Context menu for operations

---

### Side Panel: Propagator Debugger

**Purpose**: Debug propagator execution in real-time.

```text
┌─ PROPAGATOR DEBUGGER ─────────────────────────────────────┐
│ Network: parser_network                                   │
│ Status: ⏸ PAUSED                                         │
├───────────────────────────────────────────────────────────┤
│ Breakpoints:                                              │
│  ✓ parser_propagator (on execution)                       │
│  ✓ ast_cell (on value change)                             │
│                                                           │
│ Current Execution:                                        │
│  Propagator: parser_propagator                            │
│  Triggered by: source_cell v5                             │
│  Started: 12:34:56.123                                    │
│  Status: PAUSED at breakpoint                             │
│                                                           │
│ Inputs:                                                   │
│  • source_cell: TextDocument v5 (1234 chars)              │
│                                                           │
│ Outputs (pending):                                        │
│  • ast_cell: (not yet written)                            │
│  • error_cell: (not yet written)                          │
│                                                           │
│ Stack Trace:                                              │
│  → parser_propagator::execute()                           │
│      parse_typescript()                                   │
│        tokenize()                                         │
│                                                           │
│ [▶ Continue] [⏭ Step Over] [⏬ Step Into] [⏸ Pause]      │
│                                                           │
│ Watches:                                                  │
│  • source_cell.version = 5                                │
│  • ast_cell.version = 4                                   │
│                                                           │
│ [+ Add Watch] [⚙ Settings]                                │
└───────────────────────────────────────────────────────────┘
```

---

### Side Panel: Performance Profiler

```text
┌─ PROFILER ────────────────────────────────────────────────┐
│ [▶ Start Profiling] [⏹ Stop] [📊 View Results]           │
├───────────────────────────────────────────────────────────┤
│ Last Profile: parser_network (2 min ago)                  │
│                                                           │
│ Hotspots:                                                 │
│  1. parser_propagator       850ms (65%)                   │
│  2. typechecker_propagator  300ms (23%)                   │
│  3. linter_propagator       150ms (12%)                   │
│                                                           │
│ [View Flame Graph]                                        │
│                                                           │
│ Detailed Breakdown:                                       │
│  ▽ parser_propagator (850ms)                              │
│    ├─ tokenize: 420ms (49%)                               │
│    ├─ parse_expr: 280ms (33%)                             │
│    └─ build_ast: 150ms (18%)                              │
│                                                           │
│ Recommendations:                                          │
│  ⚠ tokenize() is slow - consider caching                  │
│  💡 parse_expr() called 1,234 times - optimize?           │
│                                                           │
│ [Export Report] [Compare Profiles]                        │
└───────────────────────────────────────────────────────────┘
```

---

## Interaction Patterns

### Pattern 1: Edit/View Mode Toggle

**Context**: Working on network cell.

**Flow**:

1. **EDIT MODE** (default when creating network)
   - Canvas shows propagator network
   - Drag/drop propagators
   - Connect cells
   - Adjust properties

2. **Switch to VIEW MODE** (click button or `Ctrl+Shift+V`)
   - Canvas transitions to code view
   - Show generated code with syntax highlighting
   - Tabs for different outputs (code, docs, tests, debugger)
   - All read-only (can copy but not edit directly)

3. **Switch back to EDIT MODE** (`Ctrl+Shift+E`)
   - Preserves scroll position in canvas
   - Highlights recently modified nodes

**Visual Transition**:

```text
EDIT MODE                        VIEW MODE
┌──────────────┐    Toggle    ┌──────────────┐
│  Network     │  ─────────▶  │  Generated   │
│  Canvas      │  ◀─────────  │  Code        │
│  [Nodes]     │              │  [Syntax]    │
└──────────────┘              └──────────────┘
```

---

### Pattern 2: Drag-and-Drop Propagator

**Context**: Adding propagator to network.

**Flow**:

1. Open Propagator Palette
2. Find desired propagator (search or browse)
3. Drag propagator icon to canvas
4. Drop at desired location
5. Propagator appears with default configuration
6. Property inspector opens automatically
7. Configure parameters
8. Connect to cells

**Visual Feedback**:

- Drag: Ghost image follows cursor
- Valid drop zone: Canvas highlights green
- Invalid drop: Red highlight + tooltip explanation
- After drop: Animate node appearance

---

### Pattern 3: Connect Cells

**Context**: Wiring propagator to cells.

**Flow**:

1. Hover over propagator input/output port
2. Click and drag to start connection
3. Elastic line follows cursor
4. Compatible target cells highlight
5. Drop on target cell
6. Connection validates (type checking)
7. If valid: Connection appears
8. If invalid: Error tooltip + connection disappears

**Visual Feedback**:

```text
Dragging Connection:
  ┌──────────┐
  │ PropA    │
  │ Output ●─┼──────▶ (elastic line follows cursor)
  └──────────┘

Compatible Target Found:
  ┌──────────┐
  │ Cell B   │ ◀─ Green highlight
  │ ●        │
  └──────────┘

Connection Made:
  ┌──────────┐          ┌──────────┐
  │ PropA    │          │ Cell B   │
  │ Output ●─┼──────────┼● Input   │
  └──────────┘          └──────────┘
```

---

### Pattern 4: Live Code Preview

**Context**: Adjusting propagator parameters.

**Flow**:

1. Select propagator in EDIT MODE
2. Property inspector shows parameters
3. Adjust parameter (e.g., slider, dropdown)
4. Generated code updates immediately in background
5. Diff indicator shows change
6. Click "View Diff" to see side-by-side comparison
7. Accept or revert change

**Visual Feedback**:

```text
Property Inspector:        Generated Code Preview:
┌─────────────────┐       ┌──────────────────────┐
│ Optimization: 2 │ ───▶  │ // Code updates live │
│ ◄───────────▶   │       │ fn parse(...) {      │
│ 0  1  2  3      │       │   // Level 2 opts    │
└─────────────────┘       │   ...                │
                          └──────────────────────┘
                              ↑
                          [3 changes]
```

---

### Pattern 5: Time-Travel Debugging

**Context**: Investigating why cell has current value.

**Flow**:

1. Right-click cell → "View History"
2. Cell History Widget opens
3. Timeline shows all versions
4. Scrub through versions (like video player)
5. For each version, see:
   - Value content
   - Which propagator produced it
   - What inputs were used
   - Timestamp
6. Compare any two versions (diff)
7. Click "Why this value?" to see full provenance graph

**Visual Feedback**:

```text
Timeline:
├─ v1 ──── v2 ──── v3 ──── v4 ──── v5 (current)
   2hr     1hr     30min   10min   now
   ↑
   (Scrub here to view v2 state)

Provenance Graph for v5:
  source_file v5
       ↓
  parser_propagator (exec #127)
       ↓
  ast_cell v5 ◀── YOU ARE HERE
```

---

### Pattern 6: Multi-Cursor Network Editing

**Context**: Bulk operations on similar elements.

**Flow**:

1. Select multiple propagators (Ctrl+Click or drag-select)
2. Property inspector shows "Multiple Selection (3 items)"
3. Change shared properties → applies to all
4. Different properties → shows "[Mixed]"
5. Connect all to same cell simultaneously
6. Delete all together

**Visual Feedback**:

```text
Multiple Selection:
┌──────────┐ ┌──────────┐ ┌──────────┐
│ Parser 1 │ │ Parser 2 │ │ Parser 3 │ ◀─ All selected
└──────────┘ └──────────┘ └──────────┘

Property Inspector:
┌──────────────────────┐
│ 3 Propagators        │
│ Type: Parser         │
│ Strict: [Mixed]      │ ◀─ Different values
│ Target: ES2022       │ ◀─ Shared value
│                      │
│ [Apply to All]       │
└──────────────────────┘
```

---

### Pattern 7: Quick Command Palette

**Context**: Keyboard-driven workflow.

**Flow**:

1. Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
2. Command palette opens with fuzzy search
3. Type partial command: "add par" → "Add Propagator"
4. Press Enter
5. Propagator palette opens with focus on search
6. Type propagator name: "parse"
7. Press Enter
8. Propagator appears at canvas center

**Commands**:


Propagator Network Commands:

- Add Propagator
- Add Cell
- Connect Selected
- Auto-Layout Network
- Run Network
- Toggle Edit/View Mode
- Export Generated Code
- Generate Documentation
- Generate Debugger
- View Execution History
- Debug Propagator
- Profile Network
- Compare Versions

---

## Advanced Widget Examples

### Widget: Interactive Constraint Editor

**Purpose**: Define bidirectional constraints visually.

```text
┌─ Constraint Editor ───────────────────────────────────────┐
│ Constraint Type: [Temperature Conversion ▼]               │
├───────────────────────────────────────────────────────────┤
│ Cells:                                                    │
│  [Celsius Cell  ▼] ⚖️ [Fahrenheit Cell ▼]                 │
│                                                           │
│ Relationship:                                             │
│  C = (F - 32) × 5/9                                       │
│                                                           │
│ [✓] Bidirectional (both cells can be inputs)              │
│                                                           │
│ Projection Functions:                                     │
│  compute_C(F) = (F - 32) * 5 / 9                          │
│  compute_F(C) = C * 9 / 5 + 32                            │
│                                                           │
│ Test:                                                     │
│  F = [100 ] → C = 37.78 ✓                                 │
│  C = [0   ] → F = 32.00 ✓                                 │
│                                                           │
│ [Create Constraint] [Test] [Cancel]                       │
└───────────────────────────────────────────────────────────┘
```

---

### Widget: Network Performance Dashboard

```text
┌─ Network Performance ──────────────────────────────────────┐
│ Network: parser_network                                    │
│ Uptime: 2h 34m | Executions: 1,247                         │
├────────────────────────────────────────────────────────────┤
│ 📊 Throughput                                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │   ╱╲    ╱╲                                          │   │
│  │  ╱  ╲  ╱  ╲╱╲    ╱╲                                 │   │
│  │ ╱    ╲╱      ╲  ╱  ╲      Current: 12 exec/min      │   │
│  │╱              ╲╱    ╲╱╲   Peak: 28 exec/min         │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                            │
│ ⚡ Latency (p50/p95/p99)                                    │
│  Parser:        42ms / 85ms / 120ms                        │
│  TypeChecker:   28ms / 65ms / 95ms                         │
│  CodeGen:       15ms / 30ms / 45ms                         │
│                                                            │
│ 💾 Resource Usage                                          │
│  Memory: 145 MB (peak: 230 MB)                             │
│  CPU: 12% avg (peak: 45%)                                  │
│                                                            │
│ 📈 Trends (vs yesterday)                                   │
│  Throughput: +15% ↑                                        │
│  Latency: -8% ↓                                            │
│  Memory: +5% ↑                                             │
│                                                            │
│ [Detailed Report] [Export Metrics] [Set Alerts]            │
└────────────────────────────────────────────────────────────┘
```

---

### Widget: Cell Value Visualizer

**Purpose**: Visualize complex cell values interactively.

```text
┌─ Cell Value: ast_cell v5 ─────────────────────────────────┐
│ [Tree View] [Graph View] [JSON] [Table]                   │
├───────────────────────────────────────────────────────────┤
│ Graph View:                                               │
│                                                           │
│           ┌─────────────┐                                 │
│           │  Program    │                                 │
│           └──────┬──────┘                                 │
│          ╭───────┴───────╮                                │
│    ┌─────────┐     ┌─────────┐                            │
│    │  Func   │     │  Func   │                            │
│    │ "parse" │     │ "check" │                            │
│    └────┬────┘     └────┬────┘                            │
│       ...             ...                                 │
│                                                           │
│ (Click node to expand, right-click for actions)           │
│                                                           │
│ Selected Node: FunctionDeclaration "parse"                │
│  Type: FunctionDeclaration                                │
│  Name: "parse"                                            │
│  Parameters: 1                                            │
│  Body: BlockStatement (23 statements)                     │
│                                                           │
│ [Copy Path] [Find References] [Export]                    │
└───────────────────────────────────────────────────────────┘
```

---

### Widget: Batch Transformation Studio

**Purpose**: Apply transformations to multiple propagators/cells at once.

```text
┌─ Batch Transformation ────────────────────────────────────┐
│ 📦 Operation: Add Logging                                 │
├───────────────────────────────────────────────────────────┤
│ Target Selection:                                         │
│  ☑ All propagators in network (5 selected)                │
│  ☐ Only propagators matching: [filter...]                 │
│                                                           │
│ Transformation:                                           │
│  [Add Aspect ▼]                                           │
│  Aspect: [Logging ▼]                                      │
│  Level: [Debug ▼]                                         │
│  Include: ☑ Args  ☑ Return  ☐ Duration                    │
│                                                           │
│ Preview Impact:                                           │
│  ▽ parser_propagator                                      │
│    + logging wrapper (estimated: +20 cycles)              │
│  ▽ typechecker_propagator                                 │
│    + logging wrapper (estimated: +15 cycles)              │
│  ...                                                      │
│                                                           │
│ Total Estimated Overhead: +175 cycles (0.002%)            │
│                                                           │
│ [Preview Diff] [Apply] [Cancel]                           │
└───────────────────────────────────────────────────────────┘
```

---

## Summary

This specification defines a comprehensive **Software CAD System** for VSCode that treats propagator networks as first-class visual artifacts. The system provides:

1. **Dual-Mode Editing**: Seamless toggle between network structure (EDIT) and generated code (VIEW)

2. **Rich Widget Library**:

   - Network Canvas (visual editor)
   - Property Inspector (context-sensitive)
   - Execution Timeline (temporal debugging)
   - Custom Inspectors (auto-generated from types)
   - Performance Profiler (integrated)

3. **Professional Interactions**:

   - Drag-and-drop with validation
   - Multi-cursor editing
   - Time-travel debugging
   - Live code preview
   - Keyboard-driven workflow

4. **Advanced Scenarios**:

   - Code generation without macros
   - Compile-time aspect orientation
   - Auto-generated documentation
   - Custom debugger generation
   - Interactive performance profiling

5. **Literate Programming First**:

   - Notebooks as primary interface
   - Multiple cell types (Network, Code, Markdown, Test)
   - Rich outputs with multiple views
   - Full execution history

The design prioritizes **immediate feedback**, **visual clarity**, and **professional-grade tooling** for experienced engineers building complex systems with propagator networks.

---

## Implementation Notes

### Technology Stack

- **VSCode Extension API**: Core extension framework
- **Webviews**: Custom widget rendering (React/Svelte)
- **Canvas API**: Network visualization
- **D3.js**: Data visualization (flame graphs, timelines)
- **Monaco Editor**: Code editing with syntax highlighting
- **LSP Integration**: Connect to CPSC language server

### Performance Considerations

- **Virtual Rendering**: Only render visible nodes in large networks
- **Debounced Updates**: Throttle live previews during fast parameter changes
- **Web Workers**: Offload heavy computations (layout, diff)
- **Incremental Updates**: Only re-render changed portions

### Accessibility

- **Keyboard Navigation**: All operations accessible via keyboard
- **Screen Reader Support**: ARIA labels on all interactive elements
- **High Contrast**: Support for high-contrast themes
- **Zoom Support**: All widgets scale with VSCode zoom
