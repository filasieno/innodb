/**
 * @fileoverview Index file for InnoDB TypeScript interfaces
 *
 * @brief This file serves as the main entry point for all InnoDB type definitions,
 * re-exporting all interfaces and types from their individual files.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-16
 * @since 1.0.0
 * @version 1.0.0
 */

// Basic types
export { IBUri } from './IBUri';
export { IBFileStat } from './IBFileStat';
export type { IBFileType } from './IBFileType';

// Core infrastructure types
export { IBCancellationToken } from './IBCancellationToken';
export type { IBProviderResult } from './IBProviderResult';
export { IBFuture } from './IBFuture';

// Event system types
export { IBEvent } from './IBEvent';
export { IBDisposable } from './IBDisposable';

// Command system types
export { IBCommand } from './IBCommand';

// Terminal types
export { IBTerminalProfile } from './IBTerminalProfile';
export { IBTerminalOptions } from './IBTerminalOptions';
export { IBExtensionTerminalOptions } from './IBExtensionTerminalOptions';
export { IBTerminalLink } from './IBTerminalLink';
export { IBTerminalLinkContext } from './IBTerminalLinkContext';
export { IBTerminalDimensions } from './IBTerminalDimensions';

// Configuration types
export { IBWorkspaceConfiguration } from './IBWorkspaceConfiguration';

// File system types
export { IBFileChangeEvent } from './IBFileChangeEvent';
export type { IBFileChangeType } from './IBFileChangeType';

// Notebook types
export { IBNotebookData } from './IBNotebookData';
export { IBNotebookDocument } from './IBNotebookDocument';

// Source control types
export { IBSourceControlResourceDecorations } from './IBSourceControlResourceDecorations';
export { IBSourceControlInputBox } from './IBSourceControlInputBox';
export { IBSourceControlResourceState } from './IBSourceControlResourceState';
export { IBSourceControlResourceGroup } from './IBSourceControlResourceGroup';
export { IBQuickDiffProvider } from './IBQuickDiffProvider';
export { IBSourceControl } from './IBSourceControl';

// Provider types
export { IBFileSystemProvider } from './IBFileSystemProvider';
export { IBTextDocumentContentProvider } from './IBTextDocumentContentProvider';
export { IBNotebookContentProvider } from './IBNotebookContentProvider';
export { IBTerminalProfileProvider } from './IBTerminalProfileProvider';
export { IBTerminalLinkProvider } from './IBTerminalLinkProvider';
export { IBPseudoterminal } from './IBPseudoterminal';

// Virtual workspace types
export { IBVirtualWorkspace } from './IBVirtualWorkspace';
export { IBVirtualWorkspaceProvider } from './IBVirtualWorkspaceProvider';
