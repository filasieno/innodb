# Widget Design System for Propagator CAD

## Design Tokens

### Color Palette

```css
/* Primary Colors */
--primary-blue: #0066CC;
--primary-green: #00AA66;
--primary-red: #DD4444;
--primary-yellow: #FFAA00;

/* Cell Types */
--cell-textdocument: #3B82F6;  /* Blue */
--cell-aterm: #10B981;          /* Green */
--cell-diagnostic: #EF4444;     /* Red */
--cell-workspace-edit: #F59E0B; /* Amber */
--cell-nothing: #9CA3AF;        /* Gray */
--cell-contradiction: #DC2626;  /* Dark Red */

/* Propagator Types */
--prop-primitive: #8B5CF6;      /* Purple */
--prop-compound: #EC4899;       /* Pink */
--prop-conditional: #14B8A6;    /* Teal */
--prop-constraint: #F97316;     /* Orange */

/* Semantic Colors */
--success: #10B981;
--warning: #F59E0B;
--error: #EF4444;
--info: #3B82F6;

/* Neutral Palette */
--gray-50: #F9FAFB;
--gray-100: #F3F4F6;
--gray-200: #E5E7EB;
--gray-300: #D1D5DB;
--gray-400: #9CA3AF;
--gray-500: #6B7280;
--gray-600: #4B5563;
--gray-700: #374151;
--gray-800: #1F2937;
--gray-900: #111827;

/* Background */
--bg-primary: #FFFFFF;
--bg-secondary: #F9FAFB;
--bg-tertiary: #F3F4F6;
--bg-elevated: #FFFFFF;
--bg-overlay: rgba(0, 0, 0, 0.5);

/* Dark Mode (VSCode respects theme) */
--bg-primary-dark: #1E1E1E;
--bg-secondary-dark: #252526;
--bg-tertiary-dark: #2D2D30;
```

### Typography

```css
/* Font Families */
--font-sans: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
--font-mono: "Fira Code", "Source Code Pro", Consolas, monospace;

/* Font Sizes */
--text-xs: 0.75rem;    /* 12px */
--text-sm: 0.875rem;   /* 14px */
--text-base: 1rem;     /* 16px */
--text-lg: 1.125rem;   /* 18px */
--text-xl: 1.25rem;    /* 20px */
--text-2xl: 1.5rem;    /* 24px */
--text-3xl: 1.875rem;  /* 30px */

/* Font Weights */
--font-normal: 400;
--font-medium: 500;
--font-semibold: 600;
--font-bold: 700;

/* Line Heights */
--leading-tight: 1.25;
--leading-normal: 1.5;
--leading-relaxed: 1.75;
```

### Spacing

```css
/* Spacing Scale (4px base unit) */
--space-1: 0.25rem;   /* 4px */
--space-2: 0.5rem;    /* 8px */
--space-3: 0.75rem;   /* 12px */
--space-4: 1rem;      /* 16px */
--space-5: 1.25rem;   /* 20px */
--space-6: 1.5rem;    /* 24px */
--space-8: 2rem;      /* 32px */
--space-10: 2.5rem;   /* 40px */
--space-12: 3rem;     /* 48px */
--space-16: 4rem;     /* 64px */
```

### Border Radius

```css
--radius-sm: 0.25rem;  /* 4px */
--radius-md: 0.375rem; /* 6px */
--radius-lg: 0.5rem;   /* 8px */
--radius-xl: 0.75rem;  /* 12px */
--radius-full: 9999px; /* Pill shape */
```

### Shadows

```css
--shadow-sm: 0 1px 2px 0 rgba(0, 0, 0, 0.05);
--shadow-md: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
--shadow-lg: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
--shadow-xl: 0 20px 25px -5px rgba(0, 0, 0, 0.1);
```

### Transitions

```css
--transition-fast: 150ms ease-in-out;
--transition-base: 200ms ease-in-out;
--transition-slow: 300ms ease-in-out;
```

---

## Base Components

### Button

```typescript
interface ButtonProps {
  variant: 'primary' | 'secondary' | 'ghost' | 'danger';
  size: 'sm' | 'md' | 'lg';
  icon?: React.ReactNode;
  disabled?: boolean;
  loading?: boolean;
  onClick: () => void;
}
```

**Visual Styles**:
```css
/* Primary Button */
.button-primary {
  background: var(--primary-blue);
  color: white;
  padding: var(--space-2) var(--space-4);
  border-radius: var(--radius-md);
  font-weight: var(--font-medium);
  transition: all var(--transition-fast);
}

.button-primary:hover {
  background: #0052A3;
  box-shadow: var(--shadow-md);
}

.button-primary:active {
  transform: scale(0.98);
}

/* Secondary Button */
.button-secondary {
  background: var(--gray-100);
  color: var(--gray-700);
  border: 1px solid var(--gray-300);
}

/* Ghost Button */
.button-ghost {
  background: transparent;
  color: var(--gray-600);
}

.button-ghost:hover {
  background: var(--gray-100);
}
```

**Examples**:
```tsx
<Button variant="primary" size="md" icon={<PlayIcon />}>
  Run Network
</Button>

<Button variant="secondary" size="sm">
  Cancel
</Button>

<Button variant="danger" disabled>
  Delete
</Button>
```

---

### Input Field

```typescript
interface InputProps {
  type: 'text' | 'number' | 'password';
  label?: string;
  placeholder?: string;
  error?: string;
  icon?: React.ReactNode;
  onChange: (value: string) => void;
}
```

**Visual Styles**:
```css
.input-wrapper {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
}

.input-label {
  font-size: var(--text-sm);
  font-weight: var(--font-medium);
  color: var(--gray-700);
}

.input-field {
  padding: var(--space-2) var(--space-3);
  border: 1px solid var(--gray-300);
  border-radius: var(--radius-md);
  font-size: var(--text-sm);
  transition: all var(--transition-fast);
}

.input-field:focus {
  outline: none;
  border-color: var(--primary-blue);
  box-shadow: 0 0 0 3px rgba(0, 102, 204, 0.1);
}

.input-field.error {
  border-color: var(--error);
}

.input-error-message {
  font-size: var(--text-xs);
  color: var(--error);
}
```

---

### Dropdown / Select

```typescript
interface DropdownProps {
  options: Array<{value: string; label: string}>;
  value: string;
  onChange: (value: string) => void;
  placeholder?: string;
}
```

**Visual Styles**:
```css
.dropdown {
  position: relative;
  display: inline-block;
}

.dropdown-trigger {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  padding: var(--space-2) var(--space-3);
  border: 1px solid var(--gray-300);
  border-radius: var(--radius-md);
  background: white;
  cursor: pointer;
}

.dropdown-menu {
  position: absolute;
  top: calc(100% + var(--space-1));
  left: 0;
  min-width: 200px;
  background: white;
  border: 1px solid var(--gray-200);
  border-radius: var(--radius-md);
  box-shadow: var(--shadow-lg);
  z-index: 1000;
}

.dropdown-item {
  padding: var(--space-2) var(--space-3);
  cursor: pointer;
  transition: background var(--transition-fast);
}

.dropdown-item:hover {
  background: var(--gray-100);
}

.dropdown-item.selected {
  background: var(--primary-blue);
  color: white;
}
```

---

### Slider

```typescript
interface SliderProps {
  min: number;
  max: number;
  value: number;
  step?: number;
  label?: string;
  onChange: (value: number) => void;
}
```

**Visual Styles**:
```css
.slider-wrapper {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.slider-header {
  display: flex;
  justify-content: space-between;
  font-size: var(--text-sm);
}

.slider-track {
  position: relative;
  height: 4px;
  background: var(--gray-200);
  border-radius: var(--radius-full);
}

.slider-fill {
  position: absolute;
  height: 100%;
  background: var(--primary-blue);
  border-radius: var(--radius-full);
}

.slider-thumb {
  position: absolute;
  width: 16px;
  height: 16px;
  background: white;
  border: 2px solid var(--primary-blue);
  border-radius: var(--radius-full);
  cursor: grab;
  transform: translate(-50%, -50%);
  top: 50%;
}

.slider-thumb:active {
  cursor: grabbing;
  transform: translate(-50%, -50%) scale(1.2);
}
```

---

### Tabs

```typescript
interface TabsProps {
  tabs: Array<{id: string; label: string; icon?: React.ReactNode}>;
  activeTab: string;
  onChange: (tabId: string) => void;
}
```

**Visual Styles**:
```css
.tabs-container {
  display: flex;
  flex-direction: column;
}

.tabs-header {
  display: flex;
  border-bottom: 1px solid var(--gray-200);
}

.tab {
  padding: var(--space-3) var(--space-4);
  font-size: var(--text-sm);
  font-weight: var(--font-medium);
  color: var(--gray-600);
  cursor: pointer;
  border-bottom: 2px solid transparent;
  transition: all var(--transition-fast);
}

.tab:hover {
  color: var(--gray-900);
  background: var(--gray-50);
}

.tab.active {
  color: var(--primary-blue);
  border-bottom-color: var(--primary-blue);
}

.tab-content {
  padding: var(--space-4);
}
```

---

### Badge / Pill

```typescript
interface BadgeProps {
  variant: 'default' | 'success' | 'warning' | 'error' | 'info';
  size?: 'sm' | 'md';
  children: React.ReactNode;
}
```

**Visual Styles**:
```css
.badge {
  display: inline-flex;
  align-items: center;
  padding: var(--space-1) var(--space-2);
  border-radius: var(--radius-full);
  font-size: var(--text-xs);
  font-weight: var(--font-medium);
}

.badge-default {
  background: var(--gray-100);
  color: var(--gray-700);
}

.badge-success {
  background: rgba(16, 185, 129, 0.1);
  color: #065F46;
}

.badge-warning {
  background: rgba(245, 158, 11, 0.1);
  color: #92400E;
}

.badge-error {
  background: rgba(239, 68, 68, 0.1);
  color: #991B1B;
}
```

---

### Tooltip

```typescript
interface TooltipProps {
  content: string | React.ReactNode;
  position?: 'top' | 'right' | 'bottom' | 'left';
  children: React.ReactNode;
}
```

**Visual Styles**:
```css
.tooltip-wrapper {
  position: relative;
  display: inline-block;
}

.tooltip {
  position: absolute;
  background: var(--gray-900);
  color: white;
  padding: var(--space-2) var(--space-3);
  border-radius: var(--radius-md);
  font-size: var(--text-xs);
  white-space: nowrap;
  box-shadow: var(--shadow-lg);
  z-index: 10000;
  pointer-events: none;
  opacity: 0;
  transition: opacity var(--transition-fast);
}

.tooltip.visible {
  opacity: 1;
}

/* Arrow */
.tooltip::after {
  content: '';
  position: absolute;
  width: 0;
  height: 0;
  border: 4px solid transparent;
}

.tooltip.top::after {
  bottom: -8px;
  left: 50%;
  transform: translateX(-50%);
  border-top-color: var(--gray-900);
}
```

---

## Complex Widgets

### Network Node (Cell/Propagator)

```typescript
interface NetworkNodeProps {
  id: string;
  type: 'cell' | 'propagator';
  subtype: string; // e.g., 'TextDocument', 'Primitive'
  label: string;
  position: {x: number; y: number};
  selected: boolean;
  ports: Array<{id: string; direction: 'input' | 'output'}>;
  status?: 'idle' | 'running' | 'error';
  onMove: (newPos: {x: number; y: number}) => void;
  onClick: () => void;
}
```

**Visual Design**:

```css
/* Cell Node */
.network-node.cell {
  min-width: 120px;
  min-height: 60px;
  background: white;
  border: 2px solid var(--cell-textdocument);
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-md);
  cursor: move;
  transition: all var(--transition-fast);
}

.network-node.cell:hover {
  box-shadow: var(--shadow-lg);
  transform: translateY(-2px);
}

.network-node.cell.selected {
  border-width: 3px;
  box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.2);
}

.network-node.cell.running {
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0%, 100% { border-color: var(--cell-textdocument); }
  50% { border-color: rgba(59, 130, 246, 0.5); }
}

/* Node Header */
.node-header {
  padding: var(--space-2);
  border-bottom: 1px solid var(--gray-200);
  background: var(--gray-50);
  border-radius: var(--radius-lg) var(--radius-lg) 0 0;
}

.node-title {
  font-size: var(--text-sm);
  font-weight: var(--font-semibold);
  color: var(--gray-900);
}

.node-subtitle {
  font-size: var(--text-xs);
  color: var(--gray-500);
}

/* Node Body */
.node-body {
  padding: var(--space-3);
}

/* Ports */
.node-port {
  position: absolute;
  width: 12px;
  height: 12px;
  background: white;
  border: 2px solid var(--primary-blue);
  border-radius: var(--radius-full);
  cursor: crosshair;
  transition: all var(--transition-fast);
}

.node-port:hover {
  transform: scale(1.5);
  box-shadow: 0 0 0 3px rgba(0, 102, 204, 0.2);
}

.node-port.input {
  left: -6px;
}

.node-port.output {
  right: -6px;
}

/* Propagator Node (distinct style) */
.network-node.propagator {
  border-radius: var(--radius-md); /* Sharper corners */
  border-color: var(--prop-primitive);
  background: linear-gradient(135deg, white 0%, var(--gray-50) 100%);
}

.propagator-icon {
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--prop-primitive);
  color: white;
  border-radius: var(--radius-md);
}
```

**Component Structure**:
```tsx
<div className="network-node cell selected running">
  <div className="node-header">
    <div className="node-title">source_file</div>
    <div className="node-subtitle">TextDocument</div>
  </div>
  <div className="node-body">
    <div className="node-info">
      <div>Version: 5</div>
      <div>Size: 1.2 KB</div>
    </div>
  </div>
  <div className="node-port input" style={{top: '50%'}} />
  <div className="node-port output" style={{top: '50%'}} />
</div>
```

---

### Connection Line

```typescript
interface ConnectionProps {
  from: {x: number; y: number};
  to: {x: number; y: number};
  type: 'data' | 'control';
  animated?: boolean;
  status?: 'idle' | 'active' | 'error';
}
```

**Visual Design**:

```css
.connection-line {
  fill: none;
  stroke: var(--gray-400);
  stroke-width: 2;
  transition: stroke var(--transition-fast);
}

.connection-line:hover {
  stroke: var(--primary-blue);
  stroke-width: 3;
}

.connection-line.active {
  stroke: var(--primary-blue);
}

.connection-line.error {
  stroke: var(--error);
  stroke-dasharray: 4 4;
}

/* Animated flow */
.connection-line.animated {
  stroke-dasharray: 5 5;
  animation: dash 1s linear infinite;
}

@keyframes dash {
  to {
    stroke-dashoffset: -10;
  }
}

/* Arrow marker */
.connection-arrow {
  fill: var(--gray-400);
}
```

**SVG Rendering**:
```tsx
<svg>
  <defs>
    <marker id="arrowhead" markerWidth="10" markerHeight="10" 
            refX="9" refY="3" orient="auto">
      <polygon points="0 0, 10 3, 0 6" className="connection-arrow" />
    </marker>
  </defs>
  
  <path
    className="connection-line animated active"
    d={`M ${from.x} ${from.y} C ${controlPoint1.x} ${controlPoint1.y}, 
         ${controlPoint2.x} ${controlPoint2.y}, ${to.x} ${to.y}`}
    markerEnd="url(#arrowhead)"
  />
</svg>
```

---

### Property Inspector Panel

```typescript
interface PropertyInspectorProps {
  selectedElement: NetworkElement | null;
  onPropertyChange: (key: string, value: any) => void;
}
```

**Layout Structure**:
```tsx
<div className="property-inspector">
  <div className="inspector-header">
    <h3>Properties</h3>
    <button className="close-btn">×</button>
  </div>
  
  <div className="inspector-body">
    {selectedElement?.type === 'cell' && (
      <CellProperties cell={selectedElement} onChange={onPropertyChange} />
    )}
    
    {selectedElement?.type === 'propagator' && (
      <PropagatorProperties prop={selectedElement} onChange={onPropertyChange} />
    )}
  </div>
</div>
```

**Visual Styles**:
```css
.property-inspector {
  width: 300px;
  height: 100%;
  background: var(--bg-primary);
  border-left: 1px solid var(--gray-200);
  display: flex;
  flex-direction: column;
}

.inspector-header {
  padding: var(--space-4);
  border-bottom: 1px solid var(--gray-200);
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.inspector-body {
  flex: 1;
  overflow-y: auto;
  padding: var(--space-4);
}

.property-group {
  margin-bottom: var(--space-6);
}

.property-group-title {
  font-size: var(--text-sm);
  font-weight: var(--font-semibold);
  color: var(--gray-700);
  margin-bottom: var(--space-2);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.property-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: var(--space-2) 0;
  border-bottom: 1px solid var(--gray-100);
}

.property-label {
  font-size: var(--text-sm);
  color: var(--gray-600);
}

.property-value {
  font-size: var(--text-sm);
  color: var(--gray-900);
  font-weight: var(--font-medium);
}
```

---

### Execution Timeline

```typescript
interface ExecutionTimelineProps {
  executions: Array<{
    id: string;
    propagatorId: string;
    startTime: number;
    duration: number;
    status: 'success' | 'failed';
  }>;
  timeWindow: {start: number; end: number};
  onSelectExecution: (id: string) => void;
}
```

**Visual Design**:

```css
.execution-timeline {
  height: 100%;
  display: flex;
  flex-direction: column;
  background: var(--bg-primary);
}

.timeline-header {
  padding: var(--space-3);
  border-bottom: 1px solid var(--gray-200);
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.timeline-controls {
  display: flex;
  gap: var(--space-2);
}

.timeline-canvas {
  flex: 1;
  position: relative;
  overflow-x: auto;
  overflow-y: auto;
}

.timeline-axis {
  position: sticky;
  top: 0;
  height: 30px;
  background: var(--bg-secondary);
  border-bottom: 1px solid var(--gray-200);
  display: flex;
  align-items: center;
  padding: 0 var(--space-4);
}

.timeline-tick {
  position: absolute;
  font-size: var(--text-xs);
  color: var(--gray-500);
}

.timeline-row {
  height: 40px;
  border-bottom: 1px solid var(--gray-100);
  position: relative;
  display: flex;
  align-items: center;
}

.timeline-row-label {
  width: 150px;
  padding: 0 var(--space-3);
  font-size: var(--text-sm);
  font-weight: var(--font-medium);
  background: var(--bg-secondary);
  border-right: 1px solid var(--gray-200);
  position: sticky;
  left: 0;
}

.timeline-bar {
  position: absolute;
  height: 24px;
  background: var(--primary-blue);
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: all var(--transition-fast);
}

.timeline-bar:hover {
  height: 28px;
  box-shadow: var(--shadow-md);
}

.timeline-bar.success {
  background: var(--success);
}

.timeline-bar.failed {
  background: var(--error);
}

.timeline-bar.selected {
  outline: 2px solid var(--gray-900);
  outline-offset: 2px;
}
```

---

### Flame Graph Widget

```typescript
interface FlameGraphProps {
  data: FlameGraphNode;
  width: number;
  height: number;
  onClickNode: (node: FlameGraphNode) => void;
}

interface FlameGraphNode {
  name: string;
  value: number;
  children: FlameGraphNode[];
}
```

**Visual Design**:

```css
.flame-graph {
  width: 100%;
  height: 100%;
  background: var(--bg-primary);
}

.flame-graph-svg {
  width: 100%;
  height: 100%;
}

.flame-rect {
  stroke: white;
  stroke-width: 1;
  cursor: pointer;
  transition: opacity var(--transition-fast);
}

.flame-rect:hover {
  opacity: 0.8;
  stroke-width: 2;
}

.flame-text {
  font-size: 12px;
  font-family: var(--font-mono);
  fill: white;
  pointer-events: none;
}

.flame-tooltip {
  position: absolute;
  background: var(--gray-900);
  color: white;
  padding: var(--space-2);
  border-radius: var(--radius-md);
  font-size: var(--text-xs);
  pointer-events: none;
  z-index: 1000;
}
```

**Color Scheme for Functions**:
```javascript
// Hash-based coloring for consistency
function getColor(name: string): string {
  const hash = name.split('').reduce((acc, char) => {
    return char.charCodeAt(0) + ((acc << 5) - acc);
  }, 0);
  
  const hue = hash % 360;
  return `hsl(${hue}, 70%, 50%)`;
}
```

---

### AST Tree Widget

```typescript
interface ASTTreeProps {
  root: ASTNode;
  expandedNodes: Set<string>;
  selectedNode: string | null;
  onToggleExpand: (nodeId: string) => void;
  onSelectNode: (nodeId: string) => void;
}
```

**Visual Design**:

```css
.ast-tree {
  font-family: var(--font-mono);
  font-size: var(--text-sm);
  padding: var(--space-4);
}

.ast-node {
  display: flex;
  align-items: center;
  padding: var(--space-1) 0;
  cursor: pointer;
  transition: background var(--transition-fast);
}

.ast-node:hover {
  background: var(--gray-100);
}

.ast-node.selected {
  background: rgba(59, 130, 246, 0.1);
  border-left: 3px solid var(--primary-blue);
  padding-left: calc(var(--space-2) - 3px);
}

.ast-node-toggle {
  width: 16px;
  height: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  margin-right: var(--space-1);
  color: var(--gray-500);
  transition: transform var(--transition-fast);
}

.ast-node-toggle.expanded {
  transform: rotate(90deg);
}

.ast-node-icon {
  width: 16px;
  height: 16px;
  margin-right: var(--space-2);
}

.ast-node-type {
  color: var(--primary-blue);
  font-weight: var(--font-medium);
}

.ast-node-name {
  color: var(--gray-900);
  margin-left: var(--space-2);
}

.ast-node-info {
  color: var(--gray-500);
  margin-left: var(--space-2);
  font-size: var(--text-xs);
}

.ast-node-children {
  margin-left: var(--space-6);
  border-left: 1px solid var(--gray-200);
  padding-left: var(--space-3);
}
```

---

## Animation Library

### Keyframe Animations

```css
/* Fade In */
@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}

/* Slide In From Right */
@keyframes slideInRight {
  from {
    transform: translateX(100%);
    opacity: 0;
  }
  to {
    transform: translateX(0);
    opacity: 1;
  }
}

/* Pulse (for active elements) */
@keyframes pulse {
  0%, 100% {
    opacity: 1;
  }
  50% {
    opacity: 0.5;
  }
}

/* Bounce (for notifications) */
@keyframes bounce {
  0%, 100% {
    transform: translateY(0);
  }
  50% {
    transform: translateY(-10px);
  }
}

/* Spinner */
@keyframes spin {
  from {
    transform: rotate(0deg);
  }
  to {
    transform: rotate(360deg);
  }
}

/* Shimmer (for loading skeletons) */
@keyframes shimmer {
  0% {
    background-position: -1000px 0;
  }
  100% {
    background-position: 1000px 0;
  }
}
```

### Loading States

```css
.skeleton {
  background: linear-gradient(
    90deg,
    var(--gray-200) 25%,
    var(--gray-100) 50%,
    var(--gray-200) 75%
  );
  background-size: 2000px 100%;
  animation: shimmer 2s infinite;
  border-radius: var(--radius-md);
}

.spinner {
  width: 24px;
  height: 24px;
  border: 3px solid var(--gray-200);
  border-top-color: var(--primary-blue);
  border-radius: var(--radius-full);
  animation: spin 1s linear infinite;
}
```

---

## Responsive Behavior

### Breakpoints

```css
/* Mobile First Approach */
--breakpoint-sm: 640px;   /* Small devices */
--breakpoint-md: 768px;   /* Tablets */
--breakpoint-lg: 1024px;  /* Laptops */
--breakpoint-xl: 1280px;  /* Desktops */
--breakpoint-2xl: 1536px; /* Large screens */
```

### Adaptive Layouts

```css
/* Network Canvas: Adjust grid size */
@media (max-width: 1024px) {
  .network-canvas {
    --grid-size: 10px; /* Smaller grid on tablets */
  }
}

/* Property Inspector: Collapse on small screens */
@media (max-width: 768px) {
  .property-inspector {
    position: fixed;
    bottom: 0;
    left: 0;
    right: 0;
    width: 100%;
    height: 40vh;
    border-left: none;
    border-top: 1px solid var(--gray-200);
  }
}

/* Timeline: Vertical on mobile */
@media (max-width: 640px) {
  .execution-timeline {
    flex-direction: column;
  }
  
  .timeline-row {
    flex-direction: column;
  }
}
```

---

## Accessibility

### Focus Indicators

```css
/* Global focus style */
*:focus-visible {
  outline: 2px solid var(--primary-blue);
  outline-offset: 2px;
}

/* Interactive elements */
button:focus-visible,
a:focus-visible,
input:focus-visible {
  outline: 2px solid var(--primary-blue);
  outline-offset: 2px;
  box-shadow: 0 0 0 4px rgba(0, 102, 204, 0.1);
}
```

### Screen Reader Support

```tsx
// ARIA labels
<button aria-label="Run network" title="Run network">
  <PlayIcon />
</button>

// ARIA live regions for dynamic content
<div aria-live="polite" aria-atomic="true">
  {statusMessage}
</div>

// ARIA expanded for collapsible sections
<button
  aria-expanded={expanded}
  aria-controls="section-content"
>
  Toggle Section
</button>
```

### Keyboard Navigation

```typescript
// Keyboard shortcuts
const shortcuts = {
  'Ctrl+Shift+E': () => setMode('edit'),
  'Ctrl+Shift+V': () => setMode('view'),
  'Ctrl+Shift+P': () => openCommandPalette(),
  'Ctrl+S': () => saveNetwork(),
  'Ctrl+Z': () => undo(),
  'Ctrl+Y': () => redo(),
  'Delete': () => deleteSelected(),
  'Escape': () => clearSelection(),
};
```

---

## Performance Optimizations

### Virtual Scrolling

```typescript
// For large lists (e.g., AST tree, execution history)
import { VirtualScroll } from 'react-virtual';

<VirtualScroll
  height={600}
  itemCount={items.length}
  itemSize={40}
  renderItem={(index) => <ASTNode node={items[index]} />}
/>
```

### Memoization

```typescript
// Prevent unnecessary re-renders
const NetworkNode = React.memo(({ node, selected, onMove }) => {
  return <div>...</div>;
}, (prevProps, nextProps) => {
  return (
    prevProps.selected === nextProps.selected &&
    prevProps.node.id === nextProps.node.id &&
    prevProps.node.position === nextProps.node.position
  );
});
```

### Debouncing

```typescript
// Debounce expensive operations
import { debounce } from 'lodash';

const handleParameterChange = debounce((value) => {
  regenerateCode(value);
}, 300);
```

### Canvas Rendering

```typescript
// Use requestAnimationFrame for smooth animations
function animateConnections() {
  requestAnimationFrame(() => {
    ctx.clearRect(0, 0, width, height);
    drawConnections();
    animateConnections();
  });
}
```

---

## Dark Mode Support

### Color Adjustments

```css
@media (prefers-color-scheme: dark) {
  :root {
    --bg-primary: #1E1E1E;
    --bg-secondary: #252526;
    --bg-tertiary: #2D2D30;
    
    --gray-50: #2D2D30;
    --gray-100: #3E3E42;
    --gray-900: #CCCCCC;
    
    --shadow-md: 0 4px 6px rgba(0, 0, 0, 0.3);
  }
  
  .network-node {
    background: var(--bg-secondary);
    border-color: var(--gray-600);
  }
  
  .connection-line {
    stroke: var(--gray-600);
  }
}
```

---

## Testing Guidelines

### Visual Regression Testing

```typescript
// Jest + Puppeteer
describe('Network Node', () => {
  it('renders correctly', async () => {
    await page.goto('http://localhost:3000/storybook');
    const screenshot = await page.screenshot();
    expect(screenshot).toMatchImageSnapshot();
  });
});
```

### Interaction Testing

```typescript
// Testing Library
import { render, fireEvent } from '@testing-library/react';

test('node selection works', () => {
  const { getByTestId } = render(<NetworkNode id="node1" />);
  const node = getByTestId('network-node-node1');
  
  fireEvent.click(node);
  expect(node).toHaveClass('selected');
});
```

---

**End of Design System**

For implementation examples, see: `/examples/widget-components/`



