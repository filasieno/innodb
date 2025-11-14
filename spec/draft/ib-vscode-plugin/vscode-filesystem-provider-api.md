# VSCode FileSystemProvider API Reference

This document provides a comprehensive reference of the Visual Studio Code FileSystemProvider API, including all interfaces, types, enumerations, and methods.

## Overview

The FileSystemProvider API enables VSCode extensions to integrate custom file systems, allowing the editor to interact with files and directories beyond the local disk, such as remote servers, in-memory structures, or proprietary storage solutions.

## Registration

To use a custom file system provider, register it with VSCode:

```typescript
vscode.workspace.registerFileSystemProvider(
  scheme: string,
  provider: FileSystemProvider,
  options?: { isCaseSensitive?: boolean; isReadonly?: boolean }
): Disposable
```

**Parameters:**

- `scheme`: The URI scheme to handle (e.g., "myfs")
- `provider`: An implementation of the FileSystemProvider interface
- `options`: Optional configuration
  - `isCaseSensitive`: Whether the file system is case-sensitive
  - `isReadonly`: Whether the file system is read-only

**Returns:** A `Disposable` that unregisters the provider when disposed

---

## Core Interface

### FileSystemProvider

The primary interface that defines the contract between VSCode and custom file system implementations.

#### Methods

##### stat

```typescript
stat(uri: Uri): FileStat | Thenable<FileStat>
```

Retrieves metadata about a file or directory.

**Parameters:**

- `uri`: The URI of the file or directory

**Returns:** `FileStat` object containing metadata

**Throws:**

- `FileSystemError.FileNotFound`: If the file does not exist
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### readDirectory

```typescript
readDirectory(uri: Uri): [string, FileType][] | Thenable<[string, FileType][]>
```

Reads the contents of a directory.

**Parameters:**

- `uri`: The URI of the directory

**Returns:** An array of tuples `[name, type]` representing directory entries

**Throws:**

- `FileSystemError.FileNotFound`: If the directory does not exist
- `FileSystemError.FileNotADirectory`: If the URI is not a directory
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### createDirectory

```typescript
createDirectory(uri: Uri): void | Thenable<void>
```

Creates a new directory.

**Parameters:**


- `uri`: The URI where the directory should be created

**Throws:**

- `FileSystemError.FileExists`: If a file or directory already exists at the URI
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### readFile

```typescript
readFile(uri: Uri): Uint8Array | Thenable<Uint8Array>
```

Reads the entire contents of a file.

**Parameters:**


- `uri`: The URI of the file

**Returns:** The file contents as a `Uint8Array`

**Throws:**

- `FileSystemError.FileNotFound`: If the file does not exist
- `FileSystemError.FileIsADirectory`: If the URI points to a directory
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### writeFile

```typescript
writeFile(
  uri: Uri,
  content: Uint8Array,
  options: { create: boolean; overwrite: boolean }
): void | Thenable<void>
```

Writes data to a file, replacing its entire contents.

**Parameters:**

- `uri`: The URI of the file
- `content`: The new file content as a `Uint8Array`
- `options`: Write operation options
  - `create`: If `true`, creates the file if it doesn't exist
  - `overwrite`: If `true`, overwrites the file if it exists

**Throws:**

- `FileSystemError.FileNotFound`: If the file does not exist and `create` is not set
- `FileSystemError.FileExists`: If the file exists, `create` is set, but `overwrite` is not set
- `FileSystemError.FileIsADirectory`: If the URI points to a directory
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### delete

```typescript
delete(uri: Uri, options: { recursive: boolean }): void | Thenable<void>
```

Deletes a file or directory.

**Parameters:**

- `uri`: The URI of the file or directory to delete
- `options`: Delete operation options
  - `recursive`: If `true`, recursively deletes directories and their contents

**Throws:**

- `FileSystemError.FileNotFound`: If the file or directory does not exist
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### rename

```typescript
rename(
  oldUri: Uri,
  newUri: Uri,
  options: { overwrite: boolean }
): void | Thenable<void>
```

Renames or moves a file or directory.

**Parameters:**

- `oldUri`: The current URI of the file or directory
- `newUri`: The new URI for the file or directory
- `options`: Rename operation options
  - `overwrite`: If `true`, overwrites the target if it exists

**Throws:**

- `FileSystemError.FileNotFound`: If the source does not exist
- `FileSystemError.FileExists`: If the target exists and `overwrite` is not set
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### copy (Optional)

```typescript
copy?(
  source: Uri,
  destination: Uri,
  options: { overwrite: boolean }
): void | Thenable<void>
```

Copies files or directories. This method is optional.

**Parameters:**

- `source`: The URI of the source file or directory
- `destination`: The URI of the destination
- `options`: Copy operation options
  - `overwrite`: If `true`, overwrites the destination if it exists

**Throws:**

- `FileSystemError.FileNotFound`: If the source does not exist
- `FileSystemError.FileExists`: If the destination exists and `overwrite` is not set
- `FileSystemError.NoPermissions`: If permissions are insufficient

---

##### watch

```typescript
watch(
  uri: Uri,
  options: { recursive: boolean; excludes: readonly string[] }
): Disposable
```

Creates a file system watcher to monitor changes to files or directories.

**Parameters:**

- `uri`: The URI to watch
- `options`: Watch operation options
  - `recursive`: If `true`, watches subdirectories recursively
  - `excludes`: An array of glob patterns to exclude from watching

**Returns:** A `Disposable` that stops watching when disposed

**Note:** The implementation can choose to ignore the `recursive` and `excludes` options. File changes should be reported through the `onDidChangeFile` event.

---

#### Events

##### onDidChangeFile

```typescript
onDidChangeFile: Event<FileChangeEvent[]>
```

An event that fires when files or directories change. Implementations should batch multiple changes and emit them together.

**Event Type:** `FileChangeEvent[]` - An array of file change events

---

## Supporting Interfaces

### FileStat

Represents metadata about a file or directory.

```typescript
interface FileStat {
  type: FileType;
  ctime: number;
  mtime: number;
  size: number;
  permissions?: FilePermission;
}
```

**Properties:**

- **`type: FileType`**  
  The type of the file system object (File, Directory, SymbolicLink, Unknown)

- **`ctime: number`**  
  Creation time in milliseconds since the Unix epoch (January 1, 1970, 00:00:00 UTC)

- **`mtime: number`**  
  Modification time in milliseconds since the Unix epoch

- **`size: number`**  
  Size of the file in bytes. Must be 0 for directories and symbolic links.

- **`permissions?: FilePermission`** (Optional)  
  File permissions (Readonly)

---

### FileChangeEvent

Describes a change to a file or directory.

```typescript
interface FileChangeEvent {
  type: FileChangeType;
  uri: Uri;
}
```

**Properties:**

- **`type: FileChangeType`**  
  The type of change that occurred

- **`uri: Uri`**  
  The URI of the file or directory that changed

---

## Enumerations

### FileType

Enumeration of file types.

```typescript
enum FileType {
  Unknown = 0,
  File = 1,
  Directory = 2,
  SymbolicLink = 64
}
```

**Values:**

- **`Unknown`** (0): An unknown file type
- **`File`** (1): A regular file
- **`Directory`** (2): A directory
- **`SymbolicLink`** (64): A symbolic link

**Note:** File types can be combined with bitwise OR. For example, a symbolic link to a file would be `FileType.File | FileType.SymbolicLink` (65).

---

### FileChangeType

Enumeration of file change types.

```typescript
enum FileChangeType {
  Changed = 1,
  Created = 2,
  Deleted = 3
}
```

**Values:**

- **`Changed`** (1): The file or directory was modified
- **`Created`** (2): The file or directory was created
- **`Deleted`** (3): The file or directory was deleted

---

### FilePermission

Enumeration of file permissions.

```typescript
enum FilePermission {
  Readonly = 1
}
```

**Values:**

- **`Readonly`** (1): The file is read-only

---

## Error Handling

### FileSystemError

A specialized error class for file system operations. Use the static factory methods to create specific error types.

#### Static Factory Methods

##### FileNotFound

```typescript
static FileNotFound(messageOrUri?: string | Uri): FileSystemError
```

Creates an error indicating that a file or directory was not found.

**Parameters:**

- `messageOrUri`: Optional custom message or the URI that was not found

---

##### FileExists

```typescript
static FileExists(messageOrUri?: string | Uri): FileSystemError
```

Creates an error indicating that a file or directory already exists.

**Parameters:**

- `messageOrUri`: Optional custom message or the URI that already exists

---

##### FileNotADirectory

```typescript
static FileNotADirectory(messageOrUri?: string | Uri): FileSystemError
```

Creates an error indicating that the URI is not a directory.

**Parameters:**

- `messageOrUri`: Optional custom message or the URI that is not a directory

---

##### FileIsADirectory

```typescript
static FileIsADirectory(messageOrUri?: string | Uri): FileSystemError
```

Creates an error indicating that the URI is a directory but a file was expected.

**Parameters:**

- `messageOrUri`: Optional custom message or the URI that is a directory

---

##### NoPermissions

```typescript
static NoPermissions(messageOrUri?: string | Uri): FileSystemError
```

Creates an error indicating insufficient permissions to perform the operation.

**Parameters:**

- `messageOrUri`: Optional custom message or the URI with insufficient permissions

---

##### Unavailable

```typescript
static Unavailable(messageOrUri?: string | Uri): FileSystemError
```

Creates an error indicating that the file system is unavailable or not responding.

**Parameters:**

- `messageOrUri`: Optional custom message or the URI that is unavailable

---

## Workspace FileSystem API

The `vscode.workspace.fs` namespace provides a file system interface that uses registered FileSystemProviders.

### Workspace FileSystem Methods

#### Workspace readFile

```typescript
vscode.workspace.fs.readFile(uri: Uri): Thenable<Uint8Array>
```

Reads the entire contents of a file.

**Parameters:**

- `uri`: The URI of the file

**Returns:** Promise resolving to the file contents

---

#### Workspace writeFile

```typescript
vscode.workspace.fs.writeFile(uri: Uri, content: Uint8Array): Thenable<void>
```

Writes data to a file, replacing its entire contents.

**Parameters:**

- `uri`: The URI of the file
- `content`: The new file content

---

#### Workspace stat

```typescript
vscode.workspace.fs.stat(uri: Uri): Thenable<FileStat>
```

Retrieves metadata about a file or directory.

**Parameters:**

- `uri`: The URI of the file or directory

**Returns:** Promise resolving to the file metadata

---

#### Workspace readDirectory

```typescript
vscode.workspace.fs.readDirectory(uri: Uri): Thenable<[string, FileType][]>
```

Reads the contents of a directory.

**Parameters:**

- `uri`: The URI of the directory

**Returns:** Promise resolving to an array of `[name, type]` tuples

---

#### Workspace createDirectory

```typescript
vscode.workspace.fs.createDirectory(uri: Uri): Thenable<void>
```

Creates a new directory.

**Parameters:**

- `uri`: The URI where the directory should be created

---

#### Workspace delete

```typescript
vscode.workspace.fs.delete(
  uri: Uri,
  options?: { recursive?: boolean; useTrash?: boolean }
): Thenable<void>
```

Deletes a file or directory.

**Parameters:**

- `uri`: The URI of the file or directory to delete
- `options`: Optional delete options
  - `recursive`: If `true`, recursively deletes directories
  - `useTrash`: If `true`, moves to trash/recycle bin instead of permanently deleting

---

#### Workspace rename

```typescript
vscode.workspace.fs.rename(
  source: Uri,
  target: Uri,
  options?: { overwrite?: boolean }
): Thenable<void>
```

Renames or moves a file or directory.

**Parameters:**

- `source`: The current URI
- `target`: The new URI
- `options`: Optional rename options
  - `overwrite`: If `true`, overwrites the target if it exists

---

#### copy

```typescript
vscode.workspace.fs.copy(
  source: Uri,
  target: Uri,
  options?: { overwrite?: boolean }
): Thenable<void>
```

Copies a file or directory.

**Parameters:**

- `source`: The URI of the source
- `target`: The URI of the destination
- `options`: Optional copy options
  - `overwrite`: If `true`, overwrites the destination if it exists

---

#### isWritableFileSystem

```typescript
vscode.workspace.fs.isWritableFileSystem(scheme: string): boolean | undefined
```

Checks if a file system scheme is writable.

**Parameters:**

- `scheme`: The URI scheme

**Returns:** `true` if writable, `false` if read-only, `undefined` if unknown

---

## Implementation Guidelines

### Basic Implementation Pattern

```typescript
import * as vscode from 'vscode';

class MyFileSystemProvider implements vscode.FileSystemProvider {
  private _emitter = new vscode.EventEmitter<vscode.FileChangeEvent[]>();
  readonly onDidChangeFile: vscode.Event<vscode.FileChangeEvent[]> = this._emitter.event;

  watch(uri: vscode.Uri, options: { recursive: boolean; excludes: readonly string[] }): vscode.Disposable {
    // Implement file watching
    return new vscode.Disposable(() => {});
  }

  stat(uri: vscode.Uri): vscode.FileStat | Thenable<vscode.FileStat> {
    // Implement stat
  }

  readDirectory(uri: vscode.Uri): [string, vscode.FileType][] | Thenable<[string, vscode.FileType][]> {
    // Implement directory reading
  }

  createDirectory(uri: vscode.Uri): void | Thenable<void> {
    // Implement directory creation
  }

  readFile(uri: vscode.Uri): Uint8Array | Thenable<Uint8Array> {
    // Implement file reading
  }

  writeFile(uri: vscode.Uri, content: Uint8Array, options: { create: boolean; overwrite: boolean }): void | Thenable<void> {
    // Implement file writing
  }

  delete(uri: vscode.Uri, options: { recursive: boolean }): void | Thenable<void> {
    // Implement deletion
  }

  rename(oldUri: vscode.Uri, newUri: vscode.Uri, options: { overwrite: boolean }): void | Thenable<void> {
    // Implement rename/move
  }
}

// Register the provider
export function activate(context: vscode.ExtensionContext) {
  const provider = new MyFileSystemProvider();
  context.subscriptions.push(
    vscode.workspace.registerFileSystemProvider('myscheme', provider, {
      isCaseSensitive: true,
      isReadonly: false
    })
  );
}
```

### Best Practices

1. **Error Handling**: Always use the appropriate `FileSystemError` static methods to create errors
2. **Batching**: Batch file change events and emit them together for better performance
3. **Async Operations**: Return `Thenable` for operations that require async processing
4. **Watch Implementation**: The `watch` method can be a no-op if your file system doesn't support watching
5. **URI Handling**: Always validate and normalize URIs before processing
6. **Permissions**: Implement proper permission checking and throw `FileSystemError.NoPermissions` when appropriate
7. **Resource Cleanup**: Properly dispose of resources in watch method disposables
8. **File Types**: Use bitwise operations correctly when combining file types (e.g., symbolic links)

---

## Resources

- **Official Sample**: [VSCode File System Provider Sample](https://github.com/microsoft/vscode-extension-samples/tree/main/fsprovider-sample)
- **API Documentation**: [VSCode Extension API](https://code.visualstudio.com/api/references/vscode-api)
- **Extension Guides**: [Virtual Documents Guide](https://code.visualstudio.com/api/extension-guides/virtual-documents)

---

## Version Compatibility

This API reference is based on VSCode API version 1.50.0 and later. Some features may not be available in earlier versions. Check the VSCode extension compatibility for your target version.

---

Last Updated: November 2025
