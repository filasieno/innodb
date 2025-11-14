# VSCode Source Control Provider API Reference

## Overview

This document provides a comprehensive reference for the Visual Studio Code Source Control Management (SCM) API. The SCM API enables extension developers to integrate custom version control systems seamlessly into VS Code, providing a unified interface for source control operations.

**Key Capabilities:**
- Create custom source control providers
- Manage resource states (files, changes)
- Organize changes into groups (staged, unstaged, conflicts, etc.)
- Display inline diffs with QuickDiff
- Handle commit messages and source control commands
- Provide status bar integration
- Support contextual menus and decorations

---

## API Namespace

All Source Control API functionality is accessed through the `vscode.scm` namespace.

### Primary Entry Point

```typescript
namespace vscode.scm {
  /**
   * Creates a new source control provider.
   * 
   * @param id - Unique identifier for the provider (e.g., 'git', 'hg', 'svn')
   * @param label - Human-readable name displayed in UI (e.g., 'Git', 'Mercurial')
   * @param rootUri - Optional root URI of the repository
   * @returns A new SourceControl instance
   */
  export function createSourceControl(
    id: string, 
    label: string, 
    rootUri?: Uri
  ): SourceControl;
  
  /**
   * The input box for the last source control created by the extension.
   */
  export const inputBox: SourceControlInputBox;
}
```

---

## Core Interfaces

### 1. SourceControl Interface

Represents the primary interface for a source control provider. This is the root object that manages all source control functionality for a repository.

```typescript
interface SourceControl {
  /**
   * The unique identifier of this source control provider.
   * This value is used internally and should be stable across sessions.
   */
  readonly id: string;
  
  /**
   * The human-readable label of this source control provider.
   * Displayed in the Source Control view title.
   */
  readonly label: string;
  
  /**
   * The optional root URI of this source control provider.
   * Typically points to the repository root directory.
   */
  readonly rootUri: Uri | undefined;
  
  /**
   * The input box for this source control provider.
   * Used for commit messages and other text input.
   */
  readonly inputBox: SourceControlInputBox;
  
  /**
   * An optional count badge displayed on the Source Control view.
   * Typically represents the number of changes or pending operations.
   * Set to undefined to hide the count badge.
   */
  count: number | undefined;
  
  /**
   * An optional commit message template.
   * Pre-fills the input box when user initiates a commit.
   */
  commitTemplate: string | undefined;
  
  /**
   * An optional command executed when the user accepts the input.
   * This is typically used to trigger the commit operation.
   */
  acceptInputCommand: Command | undefined;
  
  /**
   * An optional provider for quick diff functionality.
   * Enables inline diff decorations in the editor.
   */
  quickDiffProvider: QuickDiffProvider | undefined;
  
  /**
   * Optional commands to be displayed in the status bar.
   * These commands are shown when this source control provider is active.
   */
  statusBarCommands: Command[] | undefined;
  
  /**
   * Creates a new resource group.
   * Resource groups organize related changes (e.g., staged, unstaged).
   * 
   * @param id - Unique identifier for the group
   * @param label - Human-readable label displayed in UI
   * @returns A new SourceControlResourceGroup instance
   */
  createResourceGroup(id: string, label: string): SourceControlResourceGroup;
  
  /**
   * Disposes of this source control provider.
   * Removes it from the Source Control view and releases all resources.
   */
  dispose(): void;
}
```

**Usage Example:**

```typescript
const mySourceControl = vscode.scm.createSourceControl(
  'myVcs',
  'My Version Control',
  vscode.Uri.file('/path/to/repo')
);

mySourceControl.count = 5;
mySourceControl.commitTemplate = 'feat: ';
mySourceControl.acceptInputCommand = {
  command: 'myVcs.commit',
  title: 'Commit',
  arguments: [mySourceControl]
};
```

---

### 2. SourceControlResourceGroup Interface

Represents a group of related source control resources. Groups are used to organize changes into logical categories (e.g., "Staged Changes", "Unstaged Changes", "Merge Conflicts").

```typescript
interface SourceControlResourceGroup {
  /**
   * The unique identifier of this resource group.
   * Used for programmatic access and state management.
   */
  readonly id: string;
  
  /**
   * The human-readable label of this resource group.
   * Displayed as a collapsible section header in the Source Control view.
   */
  label: string;
  
  /**
   * Whether this resource group should be hidden when empty.
   * When true, the group is not displayed if resourceStates is empty.
   * Default: false
   */
  hideWhenEmpty: boolean | undefined;
  
  /**
   * The array of resource states within this group.
   * Each state represents an individual file or resource.
   * Setting this property replaces all existing resource states.
   */
  resourceStates: SourceControlResourceState[];
  
  /**
   * Disposes of this resource group.
   * Removes it from the parent source control provider.
   */
  dispose(): void;
}
```

**Usage Example:**

```typescript
const stagedGroup = mySourceControl.createResourceGroup('staged', 'Staged Changes');
stagedGroup.hideWhenEmpty = true;

const unstagedGroup = mySourceControl.createResourceGroup('unstaged', 'Changes');
unstagedGroup.resourceStates = [
  // Resource states here
];
```

---

### 3. SourceControlResourceState Interface

Represents the state of an individual resource (typically a file) within a source control system.

```typescript
interface SourceControlResourceState {
  /**
   * The URI of the underlying resource.
   * This is the file or resource that this state represents.
   */
  readonly resourceUri: Uri;
  
  /**
   * The command to execute when this resource is selected.
   * Typically opens a diff view or the file itself.
   */
  readonly command: Command | undefined;
  
  /**
   * The decorations for this resource state.
   * Controls the appearance in the Source Control view.
   */
  readonly decorations: SourceControlResourceDecorations | undefined;
  
  /**
   * Context value for this resource state.
   * Used for conditional visibility of menu items via 'when' clauses.
   * 
   * Example: 'staged', 'unstaged', 'untracked', 'conflict'
   */
  readonly contextValue: string | undefined;
}
```

**Usage Example:**

```typescript
const resourceState: SourceControlResourceState = {
  resourceUri: vscode.Uri.file('/path/to/file.ts'),
  command: {
    command: 'myVcs.openDiff',
    title: 'Open Diff',
    arguments: [vscode.Uri.file('/path/to/file.ts')]
  },
  decorations: {
    strikeThrough: false,
    faded: false,
    tooltip: 'Modified',
    iconPath: new vscode.ThemeIcon('diff-modified')
  },
  contextValue: 'modified'
};

unstagedGroup.resourceStates = [resourceState];
```

---

### 4. SourceControlResourceDecorations Interface

Defines the visual decorations for a source control resource in the Source Control view.

```typescript
interface SourceControlResourceDecorations {
  /**
   * Whether the resource should be rendered with a strikethrough.
   * Typically used for deleted files.
   */
  readonly strikeThrough: boolean | undefined;
  
  /**
   * Whether the resource should be rendered as faded.
   * Typically used for ignored or less important files.
   */
  readonly faded: boolean | undefined;
  
  /**
   * The tooltip text for this resource.
   * Displayed when the user hovers over the resource.
   */
  readonly tooltip: string | undefined;
  
  /**
   * The light theme icon for this resource.
   * Can be a ThemeIcon, Uri, or object with light/dark variants.
   */
  readonly light: Uri | ThemeIcon | undefined;
  
  /**
   * The dark theme icon for this resource.
   * Can be a ThemeIcon, Uri, or object with light/dark variants.
   */
  readonly dark: Uri | ThemeIcon | undefined;
  
  /**
   * The icon path for this resource.
   * Alternative to light/dark when the same icon is used for both themes.
   */
  readonly iconPath: Uri | ThemeIcon | { light: Uri; dark: Uri } | undefined;
}
```

**Common Icon Examples:**

```typescript
// Using ThemeIcon (built-in codicons)
const modifiedDecoration = {
  iconPath: new vscode.ThemeIcon('diff-modified'),
  tooltip: 'Modified'
};

const addedDecoration = {
  iconPath: new vscode.ThemeIcon('diff-added'),
  tooltip: 'Added'
};

const deletedDecoration = {
  iconPath: new vscode.ThemeIcon('diff-removed'),
  strikeThrough: true,
  tooltip: 'Deleted'
};

const conflictDecoration = {
  iconPath: new vscode.ThemeIcon('warning'),
  tooltip: 'Conflict'
};

// Using custom icon paths
const customDecoration = {
  light: vscode.Uri.file('/path/to/light-icon.svg'),
  dark: vscode.Uri.file('/path/to/dark-icon.svg'),
  tooltip: 'Custom state'
};
```

---

### 5. SourceControlInputBox Interface

Represents the input box in the Source Control view, typically used for commit messages.

```typescript
interface SourceControlInputBox {
  /**
   * The current value of the input box.
   * Can be read and written to programmatically.
   */
  value: string;
  
  /**
   * The placeholder string displayed when the input box is empty.
   */
  placeholder: string;
  
  /**
   * Controls the visibility of the input box.
   * When false, the input box is hidden from the Source Control view.
   */
  visible: boolean;
  
  /**
   * Event fired when the value of the input box changes.
   * This event is fired for every character typed or deleted.
   */
  readonly onDidChange: Event<string>;
  
  /**
   * Event fired when the user accepts the input.
   * Typically triggered by Ctrl+Enter or the accept button.
   */
  readonly onDidAccept: Event<void>;
}
```

**Usage Example:**

```typescript
mySourceControl.inputBox.placeholder = 'Enter commit message (Ctrl+Enter to commit)';
mySourceControl.inputBox.value = '';

// Listen to input changes for validation
const disposable = mySourceControl.inputBox.onDidChange(value => {
  if (value.length === 0) {
    // Show validation error
  }
});

// Handle commit
mySourceControl.inputBox.onDidAccept(() => {
  const message = mySourceControl.inputBox.value;
  performCommit(message);
  mySourceControl.inputBox.value = ''; // Clear after commit
});
```

---

### 6. QuickDiffProvider Interface

Enables inline diff decorations in the editor by providing the original content of modified resources.

```typescript
interface QuickDiffProvider {
  /**
   * Provide the original resource for a given URI.
   * 
   * This method is called when VS Code needs to show inline diff decorations
   * (the green/red lines in the editor gutter indicating additions/deletions).
   * 
   * @param uri - The URI of the modified resource
   * @param token - A cancellation token
   * @returns The URI of the original resource, or undefined if not available
   * 
   * The returned URI typically uses a custom scheme that your extension handles
   * via a TextDocumentContentProvider.
   */
  provideOriginalResource(
    uri: Uri, 
    token: CancellationToken
  ): ProviderResult<Uri>;
}
```

**Usage Example:**

```typescript
// Implement QuickDiffProvider
mySourceControl.quickDiffProvider = {
  provideOriginalResource: (uri: vscode.Uri) => {
    // Return a URI with a custom scheme that provides the original content
    return uri.with({ scheme: 'myVcs-original' });
  }
};

// Register TextDocumentContentProvider for the custom scheme
vscode.workspace.registerTextDocumentContentProvider('myVcs-original', {
  provideTextDocumentContent: (uri: vscode.Uri) => {
    // Return the original content for this file
    return getOriginalContent(uri);
  }
});
```

---

### 7. Command Interface

Represents a command that can be executed, typically associated with UI elements.

```typescript
interface Command {
  /**
   * The title of the command.
   * Displayed in UI elements like buttons and menus.
   */
  title: string;
  
  /**
   * The identifier of the command to execute.
   * Must be registered via vscode.commands.registerCommand.
   */
  command: string;
  
  /**
   * Optional arguments passed to the command handler.
   */
  arguments?: any[];
  
  /**
   * Optional tooltip for the command.
   * Displayed when user hovers over the command in UI.
   */
  tooltip?: string;
}
```

**Usage Example:**

```typescript
// Register command handler
vscode.commands.registerCommand('myVcs.openDiff', (uri: vscode.Uri) => {
  // Open diff view for the file
  vscode.commands.executeCommand('vscode.diff', 
    uri.with({ scheme: 'myVcs-original' }),
    uri,
    `Diff: ${path.basename(uri.fsPath)}`
  );
});

// Use command in resource state
const command: Command = {
  title: 'Open Diff',
  command: 'myVcs.openDiff',
  arguments: [fileUri],
  tooltip: 'View changes'
};
```

---

## Supporting Types

### Uri Interface

Represents a Uniform Resource Identifier (URI), used throughout VS Code for file and resource identification.

```typescript
interface Uri {
  /**
   * Scheme component of the URI (e.g., 'file', 'http', 'myCustomScheme')
   */
  readonly scheme: string;
  
  /**
   * Authority component of the URI (e.g., 'example.com:8080')
   */
  readonly authority: string;
  
  /**
   * Path component of the URI (e.g., '/path/to/file')
   */
  readonly path: string;
  
  /**
   * Query component of the URI (e.g., 'key=value&foo=bar')
   */
  readonly query: string;
  
  /**
   * Fragment component of the URI (e.g., 'section-1')
   */
  readonly fragment: string;
  
  /**
   * The file system path of this URI.
   * Only available for URIs with 'file' scheme.
   */
  readonly fsPath: string;
  
  /**
   * Creates a new URI by modifying properties of this URI.
   * 
   * @param change - Object specifying which properties to change
   * @returns A new URI instance
   */
  with(change: {
    scheme?: string;
    authority?: string;
    path?: string;
    query?: string;
    fragment?: string;
  }): Uri;
  
  /**
   * Returns a string representation of this URI.
   * 
   * @param skipEncoding - If true, special characters are not encoded
   * @returns URI as a string
   */
  toString(skipEncoding?: boolean): string;
  
  /**
   * Returns a JSON representation of this URI.
   */
  toJSON(): any;
}

namespace Uri {
  /**
   * Creates a URI from a file system path.
   * 
   * @param path - A file system path (e.g., '/path/to/file' or 'C:\\path\\to\\file')
   * @returns A new URI with 'file' scheme
   */
  export function file(path: string): Uri;
  
  /**
   * Creates a URI from a string representation.
   * 
   * @param value - A URI string (e.g., 'file:///path/to/file')
   * @returns A new URI instance
   */
  export function parse(value: string): Uri;
  
  /**
   * Creates a URI from its components.
   */
  export function from(components: {
    scheme: string;
    authority?: string;
    path?: string;
    query?: string;
    fragment?: string;
  }): Uri;
}
```

---

### Event<T> Type

Represents an event that can be subscribed to.

```typescript
interface Event<T> {
  /**
   * Subscribe to this event.
   * 
   * @param listener - The callback function to invoke when the event fires
   * @param thisArgs - Optional 'this' context for the listener
   * @param disposables - Optional array to add the subscription disposable to
   * @returns A disposable that removes the listener when disposed
   */
  (
    listener: (e: T) => any, 
    thisArgs?: any, 
    disposables?: Disposable[]
  ): Disposable;
}
```

---

### Disposable Interface

Represents a resource that can be disposed (cleaned up).

```typescript
interface Disposable {
  /**
   * Disposes of this resource, releasing any associated resources.
   * Should be called when the resource is no longer needed.
   */
  dispose(): void;
}

namespace Disposable {
  /**
   * Creates a disposable from a callback function.
   * 
   * @param callOnDispose - Function to call when disposed
   * @returns A new Disposable instance
   */
  export function from(...disposables: { dispose(): any }[]): Disposable;
}
```

---

### CancellationToken Interface

Represents a token that signals cancellation of an operation.

```typescript
interface CancellationToken {
  /**
   * Whether cancellation has been requested.
   */
  readonly isCancellationRequested: boolean;
  
  /**
   * Event fired when cancellation is requested.
   */
  readonly onCancellationRequested: Event<any>;
}
```

---

### ProviderResult<T> Type

A type alias for the result of provider methods, which can be synchronous or asynchronous.

```typescript
type ProviderResult<T> = T | undefined | null | Thenable<T | undefined | null>;
```

This allows provider methods to return:
- A direct value: `T`
- Nothing: `undefined` or `null`
- A promise: `Thenable<T | undefined | null>`

---

### ThemeIcon Class

Represents a reference to a built-in VS Code icon (codicon).

```typescript
class ThemeIcon {
  /**
   * Creates a reference to a theme icon.
   * 
   * @param id - The identifier of the icon (codicon name)
   * @param color - Optional theme color for the icon
   */
  constructor(id: string, color?: ThemeColor);
  
  /**
   * The identifier of this icon.
   */
  readonly id: string;
  
  /**
   * The optional color of this icon.
   */
  readonly color: ThemeColor | undefined;
}

namespace ThemeIcon {
  /**
   * Reference to the built-in file icon theme icon.
   */
  export const File: ThemeIcon;
  
  /**
   * Reference to the built-in folder icon theme icon.
   */
  export const Folder: ThemeIcon;
}
```

**Common Codicon IDs for Source Control:**
- `'diff-modified'` - Modified file icon
- `'diff-added'` - Added file icon
- `'diff-removed'` - Removed/deleted file icon
- `'diff-renamed'` - Renamed file icon
- `'diff-ignored'` - Ignored file icon
- `'warning'` - Conflict or warning icon
- `'check'` - Success or staged icon
- `'circle-outline'` - Untracked file icon
- `'git-merge'` - Merge icon
- `'git-branch'` - Branch icon
- `'git-commit'` - Commit icon

---

## Complete Implementation Example

Here's a complete example of implementing a custom source control provider:

```typescript
import * as vscode from 'vscode';
import * as path from 'path';

export function activate(context: vscode.ExtensionContext) {
  // Create the source control provider
  const myScm = vscode.scm.createSourceControl(
    'myVcs',
    'My Version Control System',
    vscode.workspace.workspaceFolders?.[0]?.uri
  );
  
  // Configure input box
  myScm.inputBox.placeholder = 'Enter commit message (Ctrl+Enter to commit)';
  myScm.inputBox.visible = true;
  
  // Set accept command (triggered when user presses Ctrl+Enter)
  myScm.acceptInputCommand = {
    command: 'myVcs.commit',
    title: 'Commit',
    arguments: [myScm]
  };
  
  // Create resource groups
  const stagedGroup = myScm.createResourceGroup('staged', 'Staged Changes');
  stagedGroup.hideWhenEmpty = true;
  
  const unstagedGroup = myScm.createResourceGroup('changes', 'Changes');
  const untrackedGroup = myScm.createResourceGroup('untracked', 'Untracked Files');
  untrackedGroup.hideWhenEmpty = true;
  
  // Implement QuickDiff provider
  myScm.quickDiffProvider = {
    provideOriginalResource(uri: vscode.Uri): vscode.ProviderResult<vscode.Uri> {
      if (uri.scheme !== 'file') {
        return;
      }
      return uri.with({ scheme: 'myVcs-original' });
    }
  };
  
  // Register TextDocumentContentProvider for original content
  context.subscriptions.push(
    vscode.workspace.registerTextDocumentContentProvider('myVcs-original', {
      provideTextDocumentContent(uri: vscode.Uri): string {
        // Return the HEAD version of the file
        return getOriginalContent(uri.with({ scheme: 'file' }));
      }
    })
  );
  
  // Register commands
  context.subscriptions.push(
    vscode.commands.registerCommand('myVcs.commit', async (scm: vscode.SourceControl) => {
      const message = scm.inputBox.value;
      if (!message) {
        vscode.window.showWarningMessage('Please enter a commit message');
        return;
      }
      
      // Perform commit
      await performCommit(message, stagedGroup.resourceStates);
      
      // Clear input and refresh
      scm.inputBox.value = '';
      await refresh();
    })
  );
  
  context.subscriptions.push(
    vscode.commands.registerCommand('myVcs.openDiff', (uri: vscode.Uri) => {
      const originalUri = uri.with({ scheme: 'myVcs-original' });
      vscode.commands.executeCommand(
        'vscode.diff',
        originalUri,
        uri,
        `${path.basename(uri.fsPath)} (Working Tree)`
      );
    })
  );
  
  context.subscriptions.push(
    vscode.commands.registerCommand('myVcs.stage', async (resource: vscode.SourceControlResourceState) => {
      await stageFile(resource.resourceUri);
      await refresh();
    })
  );
  
  context.subscriptions.push(
    vscode.commands.registerCommand('myVcs.unstage', async (resource: vscode.SourceControlResourceState) => {
      await unstageFile(resource.resourceUri);
      await refresh();
    })
  );
  
  context.subscriptions.push(
    vscode.commands.registerCommand('myVcs.refresh', async () => {
      await refresh();
    })
  );
  
  // Status bar commands
  myScm.statusBarCommands = [
    {
      command: 'myVcs.refresh',
      title: '$(sync) Refresh',
      tooltip: 'Refresh source control status'
    }
  ];
  
  // Initial refresh
  refresh();
  
  // Watch for file changes
  const watcher = vscode.workspace.createFileSystemWatcher('**/*');
  context.subscriptions.push(
    watcher.onDidChange(() => refresh()),
    watcher.onDidCreate(() => refresh()),
    watcher.onDidDelete(() => refresh()),
    watcher
  );
  
  context.subscriptions.push(myScm);
  
  // Helper function to refresh status
  async function refresh() {
    const status = await getRepositoryStatus();
    
    // Update count badge
    myScm.count = status.stagedFiles.length + status.changedFiles.length;
    
    // Update staged group
    stagedGroup.resourceStates = status.stagedFiles.map(file => ({
      resourceUri: file.uri,
      command: {
        command: 'myVcs.openDiff',
        title: 'Open Diff',
        arguments: [file.uri]
      },
      decorations: {
        iconPath: new vscode.ThemeIcon('check'),
        tooltip: 'Staged'
      },
      contextValue: 'staged'
    }));
    
    // Update unstaged group
    unstagedGroup.resourceStates = status.changedFiles.map(file => ({
      resourceUri: file.uri,
      command: {
        command: 'myVcs.openDiff',
        title: 'Open Diff',
        arguments: [file.uri]
      },
      decorations: {
        iconPath: new vscode.ThemeIcon(
          file.status === 'modified' ? 'diff-modified' :
          file.status === 'added' ? 'diff-added' :
          'diff-removed'
        ),
        strikeThrough: file.status === 'deleted',
        tooltip: file.status.charAt(0).toUpperCase() + file.status.slice(1)
      },
      contextValue: file.status
    }));
    
    // Update untracked group
    untrackedGroup.resourceStates = status.untrackedFiles.map(file => ({
      resourceUri: file.uri,
      command: {
        command: 'vscode.open',
        title: 'Open',
        arguments: [file.uri]
      },
      decorations: {
        iconPath: new vscode.ThemeIcon('circle-outline'),
        tooltip: 'Untracked'
      },
      contextValue: 'untracked'
    }));
  }
  
  // Mock implementation functions (replace with actual VCS operations)
  function getOriginalContent(uri: vscode.Uri): string {
    // Return the HEAD version of the file from your VCS
    return 'Original content';
  }
  
  async function getRepositoryStatus() {
    // Query your VCS for current status
    return {
      stagedFiles: [],
      changedFiles: [],
      untrackedFiles: []
    };
  }
  
  async function performCommit(message: string, resources: vscode.SourceControlResourceState[]) {
    // Perform the actual commit operation
    vscode.window.showInformationMessage(`Committed: ${message}`);
  }
  
  async function stageFile(uri: vscode.Uri) {
    // Stage the file in your VCS
  }
  
  async function unstageFile(uri: vscode.Uri) {
    // Unstage the file in your VCS
  }
}
```

---

## Context Values and Menu Contributions

To provide contextual menus for your source control resources, use the `contextValue` property and contribute menu items in your `package.json`.

### Setting Context Values

```typescript
const resourceState: vscode.SourceControlResourceState = {
  resourceUri: uri,
  contextValue: 'modified', // or 'staged', 'untracked', 'conflict', etc.
  // ... other properties
};
```

### Menu Contribution in package.json

```json
{
  "contributes": {
    "menus": {
      "scm/resourceState/context": [
        {
          "command": "myVcs.stage",
          "when": "scmResourceState == modified",
          "group": "inline"
        },
        {
          "command": "myVcs.unstage",
          "when": "scmResourceState == staged",
          "group": "inline"
        },
        {
          "command": "myVcs.discardChanges",
          "when": "scmResourceState == modified",
          "group": "1_modification"
        },
        {
          "command": "myVcs.openFile",
          "when": "scmResourceState",
          "group": "navigation"
        }
      ],
      "scm/resourceGroup/context": [
        {
          "command": "myVcs.stageAll",
          "when": "scmResourceGroup == changes",
          "group": "inline"
        },
        {
          "command": "myVcs.unstageAll",
          "when": "scmResourceGroup == staged",
          "group": "inline"
        }
      ],
      "scm/title": [
        {
          "command": "myVcs.refresh",
          "when": "scmProvider == myVcs",
          "group": "navigation"
        },
        {
          "command": "myVcs.pull",
          "when": "scmProvider == myVcs",
          "group": "1_sync"
        },
        {
          "command": "myVcs.push",
          "when": "scmProvider == myVcs",
          "group": "1_sync"
        }
      ]
    },
    "commands": [
      {
        "command": "myVcs.stage",
        "title": "Stage Changes",
        "icon": "$(add)"
      },
      {
        "command": "myVcs.unstage",
        "title": "Unstage Changes",
        "icon": "$(remove)"
      },
      {
        "command": "myVcs.refresh",
        "title": "Refresh",
        "icon": "$(sync)"
      }
    ]
  }
}
```

---

## Best Practices

### 1. Resource Management

Always dispose of resources properly:

```typescript
// Store disposables
const disposables: vscode.Disposable[] = [];

// Add to disposables array
disposables.push(myScm);
disposables.push(watcher);
disposables.push(commandDisposable);

// Clean up on deactivation
export function deactivate() {
  disposables.forEach(d => d.dispose());
}
```

### 2. Performance Optimization

- **Debounce file system watchers** to avoid excessive refreshes
- **Cache original content** for QuickDiff to reduce I/O
- **Use incremental updates** instead of rebuilding entire resource lists
- **Lazy load** resource states only when needed

```typescript
let refreshTimeout: NodeJS.Timeout | undefined;

function scheduleRefresh() {
  if (refreshTimeout) {
    clearTimeout(refreshTimeout);
  }
  refreshTimeout = setTimeout(() => refresh(), 500); // Debounce 500ms
}

watcher.onDidChange(() => scheduleRefresh());
```

### 3. Error Handling

Provide clear error messages and handle failures gracefully:

```typescript
try {
  await performCommit(message, resources);
  vscode.window.showInformationMessage('Successfully committed changes');
} catch (error) {
  vscode.window.showErrorMessage(`Commit failed: ${error.message}`);
  // Keep the commit message in the input box for retry
}
```

### 4. User Feedback

- Show progress for long-running operations
- Provide clear status messages
- Use appropriate icons and decorations

```typescript
await vscode.window.withProgress(
  {
    location: vscode.ProgressLocation.SourceControl,
    title: 'Committing changes...',
    cancellable: false
  },
  async (progress) => {
    await performCommit(message, resources);
  }
);
```

### 5. Input Validation

Validate user input before executing operations:

```typescript
myScm.inputBox.onDidChange(value => {
  if (value.trim().length === 0) {
    // Could show error decoration or message
  }
  if (value.length > 72) {
    // Warn about long commit message
  }
});
```

---

## Common Patterns

### Pattern 1: Multi-Repository Support

Support multiple repositories in a workspace:

```typescript
const sourceControls = new Map<string, vscode.SourceControl>();

for (const folder of vscode.workspace.workspaceFolders || []) {
  if (isRepository(folder.uri)) {
    const scm = vscode.scm.createSourceControl(
      `myVcs-${folder.name}`,
      `My VCS - ${folder.name}`,
      folder.uri
    );
    sourceControls.set(folder.uri.fsPath, scm);
  }
}
```

### Pattern 2: Conflict Resolution UI

Display merge conflicts with special decorations:

```typescript
const conflictsGroup = myScm.createResourceGroup('conflicts', 'Merge Conflicts');

conflictsGroup.resourceStates = conflictFiles.map(file => ({
  resourceUri: file.uri,
  command: {
    command: 'myVcs.resolveConflict',
    title: 'Resolve Conflict',
    arguments: [file.uri]
  },
  decorations: {
    iconPath: new vscode.ThemeIcon('warning'),
    tooltip: 'Merge conflict - click to resolve'
  },
  contextValue: 'conflict'
}));
```

### Pattern 3: Custom URI Schemes for Diffs

Use custom schemes to show different versions:

```typescript
// Register multiple content providers
context.subscriptions.push(
  // HEAD version
  vscode.workspace.registerTextDocumentContentProvider('myVcs-head', {
    provideTextDocumentContent(uri: vscode.Uri): string {
      return getVersionContent(uri, 'HEAD');
    }
  }),
  
  // Staged version
  vscode.workspace.registerTextDocumentContentProvider('myVcs-staged', {
    provideTextDocumentContent(uri: vscode.Uri): string {
      return getVersionContent(uri, 'staged');
    }
  })
);

// Show three-way diff
vscode.commands.registerCommand('myVcs.showThreeWayDiff', (uri: vscode.Uri) => {
  const baseUri = uri.with({ scheme: 'myVcs-head' });
  const stagedUri = uri.with({ scheme: 'myVcs-staged' });
  
  // Open multiple diff editors
  vscode.commands.executeCommand('vscode.diff', baseUri, stagedUri, 'Base ↔ Staged');
  vscode.commands.executeCommand('vscode.diff', stagedUri, uri, 'Staged ↔ Working');
});
```

### Pattern 4: Status Bar Integration

Show repository status in the status bar:

```typescript
const statusBarItem = vscode.window.createStatusBarItem(
  vscode.StatusBarAlignment.Left,
  100
);

statusBarItem.command = 'myVcs.showQuickPick';
statusBarItem.show();

function updateStatusBar(branch: string, changes: number) {
  statusBarItem.text = `$(git-branch) ${branch}`;
  if (changes > 0) {
    statusBarItem.text += ` $(diff) ${changes}`;
  }
  statusBarItem.tooltip = `My VCS: ${branch} (${changes} changes)`;
}

context.subscriptions.push(statusBarItem);
```

---

## API Evolution and Compatibility

The VS Code Source Control API has been stable since VS Code 1.11. However, some features have been added over time:

- **VS Code 1.11**: Initial SCM API release
- **VS Code 1.18**: Added `statusBarCommands` property
- **VS Code 1.27**: Added `count` property
- **VS Code 1.44**: Enhanced `SourceControlInputBox` with events
- **VS Code 1.66**: Added support for `ThemeColor` in decorations

Always specify the minimum VS Code version in your `package.json`:

```json
{
  "engines": {
    "vscode": "^1.66.0"
  }
}
```

---

## Debugging Tips

### 1. Enable SCM Logging

Use the VS Code output channel for debugging:

```typescript
const outputChannel = vscode.window.createOutputChannel('My VCS');

function log(message: string) {
  outputChannel.appendLine(`[${new Date().toISOString()}] ${message}`);
}

log('SCM Provider activated');
```

### 2. Inspect SCM State

Use VS Code's command palette to inspect SCM state:
- `Developer: Inspect Context Keys` - View current context values
- `Developer: Toggle Developer Tools` - Access console logs

### 3. Test with Different Scenarios

Test your provider with:
- Empty repositories
- Large numbers of changes (1000+ files)
- Binary files
- Symbolic links
- Submodules or nested repositories
- Merge conflicts
- Network failures (for remote operations)

---

## Additional Resources

### Official Documentation
- [VS Code SCM Provider Guide](https://code.visualstudio.com/api/extension-guides/scm-provider)
- [VS Code API Reference](https://code.visualstudio.com/api/references/vscode-api)
- [Extension Samples - Git Provider](https://github.com/microsoft/vscode-extension-samples/tree/main/scm-provider-sample)

### Reference Implementations
- [VS Code Git Extension](https://github.com/microsoft/vscode/tree/main/extensions/git) - Official Git implementation
- [Mercurial Extension](https://marketplace.visualstudio.com/items?itemName=mrcrowl.hg) - Example of alternative VCS
- [SVN Extension](https://marketplace.visualstudio.com/items?itemName=johnstoncode.svn-scm) - Subversion integration

### Codicons Reference
- [VS Code Codicons](https://microsoft.github.io/vscode-codicons/dist/codicon.html) - Complete icon reference

---

## Summary

The VS Code Source Control API provides a comprehensive framework for integrating version control systems into VS Code. Key components include:

✅ **SourceControl** - Main provider interface  
✅ **SourceControlResourceGroup** - Organize changes into groups  
✅ **SourceControlResourceState** - Represent individual file states  
✅ **SourceControlInputBox** - Handle commit messages  
✅ **QuickDiffProvider** - Enable inline diff decorations  
✅ **Command & Decorations** - Rich UI integration  

By implementing these interfaces, you can create a fully-featured source control experience that integrates seamlessly with VS Code's native UI and workflows.


