import * as vscode from 'vscode';

// Interface for FileSystemProvider
export interface IBFileSystemProvider extends vscode.FileSystemProvider {
  // Core filesystem operations
  stat(uri: vscode.Uri): vscode.FileStat | Thenable<vscode.FileStat>;
  readDirectory(uri: vscode.Uri): [string, vscode.FileType][] | Thenable<[string, vscode.FileType][]>;
  createDirectory(uri: vscode.Uri): void | Thenable<void>;
  readFile(uri: vscode.Uri): Uint8Array | Thenable<Uint8Array>;
  writeFile(uri: vscode.Uri, content: Uint8Array, options?: { create: boolean, overwrite: boolean }): void | Thenable<void>;
  delete(uri: vscode.Uri, options?: { recursive: boolean }): void | Thenable<void>;
  rename(oldUri: vscode.Uri, newUri: vscode.Uri, options?: { overwrite: boolean }): void | Thenable<void>;
  copy?(source: vscode.Uri, destination: vscode.Uri, options?: { overwrite: boolean }): void | Thenable<void>;

  // Optional methods
  watch?(uri: vscode.Uri, options: { recursive: boolean; excludes: string[] }): vscode.Disposable;
  onDidChangeFile?: vscode.Event<vscode.FileChangeEvent[]>;
  onDidChangeCapabilities?: vscode.Event<void>;
}

// Interface for TextDocumentContentProvider
export interface IBTextDocumentContentProvider extends vscode.TextDocumentContentProvider {
  provideTextDocumentContent(uri: vscode.Uri, token: vscode.CancellationToken): vscode.ProviderResult<string>;
  onDidChange?: vscode.Event<vscode.Uri>;
}

// Interface for DocumentLinkProvider
export interface IBDocumentLinkProvider extends vscode.DocumentLinkProvider {
  provideDocumentLinks(document: vscode.TextDocument, token: vscode.CancellationToken): vscode.ProviderResult<vscode.DocumentLink[]>;
  resolveDocumentLink?(link: vscode.DocumentLink, token: vscode.CancellationToken): vscode.ProviderResult<vscode.DocumentLink>;
}

// Interface for DocumentSymbolProvider
export interface IBDocumentSymbolProvider extends vscode.DocumentSymbolProvider {
  provideDocumentSymbols(document: vscode.TextDocument, token: vscode.CancellationToken): vscode.ProviderResult<vscode.SymbolInformation[] | vscode.DocumentSymbol[]>;
}

// Interface for WorkspaceSymbolProvider
export interface IBWorkspaceSymbolProvider extends vscode.WorkspaceSymbolProvider {
  provideWorkspaceSymbols(query: string, token: vscode.CancellationToken): vscode.ProviderResult<vscode.SymbolInformation[]>;
  resolveWorkspaceSymbol?(symbol: vscode.SymbolInformation, token: vscode.CancellationToken): vscode.ProviderResult<vscode.SymbolInformation>;
}

// Interface for DefinitionProvider
export interface IBDefinitionProvider extends vscode.DefinitionProvider {
  provideDefinition(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.Definition | vscode.DefinitionLink[]>;
}

// Interface for DeclarationProvider
export interface IBDeclarationProvider extends vscode.DeclarationProvider {
  provideDeclaration(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.Declaration | vscode.DeclarationLink[]>;
}

// Interface for ImplementationProvider
export interface IBImplementationProvider extends vscode.ImplementationProvider {
  provideImplementation(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.Definition | vscode.DefinitionLink[]>;
}

// Interface for TypeDefinitionProvider
export interface IBTypeDefinitionProvider extends vscode.TypeDefinitionProvider {
  provideTypeDefinition(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.Definition | vscode.DefinitionLink[]>;
}

// Interface for HoverProvider
export interface IBHoverProvider extends vscode.HoverProvider {
  provideHover(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.Hover>;
}

// Interface for CompletionItemProvider
export interface IBCompletionItemProvider extends vscode.CompletionItemProvider {
  provideCompletionItems(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken, context: vscode.CompletionContext): vscode.ProviderResult<vscode.CompletionItem[] | vscode.CompletionList>;
  resolveCompletionItem?(item: vscode.CompletionItem, token: vscode.CancellationToken): vscode.ProviderResult<vscode.CompletionItem>;
}

// Interface for CodeActionProvider
export interface IBCodeActionProvider extends vscode.CodeActionProvider {
  provideCodeActions(document: vscode.TextDocument, range: vscode.Range | vscode.Selection, context: vscode.CodeActionContext, token: vscode.CancellationToken): vscode.ProviderResult<(vscode.Command | vscode.CodeAction)[]>;
  resolveCodeAction?(codeAction: vscode.CodeAction, token: vscode.CancellationToken): vscode.ProviderResult<vscode.CodeAction>;
}

// Interface for DocumentFormattingEditProvider
export interface IBDocumentFormattingEditProvider extends vscode.DocumentFormattingEditProvider {
  provideDocumentFormattingEdits(document: vscode.TextDocument, options: vscode.FormattingOptions, token: vscode.CancellationToken): vscode.ProviderResult<vscode.TextEdit[]>;
}

// Interface for DocumentRangeFormattingEditProvider
export interface IBDocumentRangeFormattingEditProvider extends vscode.DocumentRangeFormattingEditProvider {
  provideDocumentRangeFormattingEdits(document: vscode.TextDocument, range: vscode.Range, options: vscode.FormattingOptions, token: vscode.CancellationToken): vscode.ProviderResult<vscode.TextEdit[]>;
}

// Interface for OnTypeFormattingEditProvider
export interface IBOnTypeFormattingEditProvider extends vscode.OnTypeFormattingEditProvider {
  provideOnTypeFormattingEdits(document: vscode.TextDocument, position: vscode.Position, ch: string, options: vscode.FormattingOptions, token: vscode.CancellationToken): vscode.ProviderResult<vscode.TextEdit[]>;
}

// Interface for RenameProvider
export interface IBRenameProvider extends vscode.RenameProvider {
  provideRenameEdits(document: vscode.TextDocument, position: vscode.Position, newName: string, token: vscode.CancellationToken): vscode.ProviderResult<vscode.WorkspaceEdit>;
  prepareRename?(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.Range | { range: vscode.Range; placeholder: string }>;
}

// Interface for FoldingRangeProvider
export interface IBFoldingRangeProvider extends vscode.FoldingRangeProvider {
  provideFoldingRanges(document: vscode.TextDocument, context: vscode.FoldingContext, token: vscode.CancellationToken): vscode.ProviderResult<vscode.FoldingRange[]>;
}

// Interface for SelectionRangeProvider
export interface IBSelectionRangeProvider extends vscode.SelectionRangeProvider {
  provideSelectionRanges(document: vscode.TextDocument, positions: vscode.Position[], token: vscode.CancellationToken): vscode.ProviderResult<vscode.SelectionRange[]>;
}

// Interface for CallHierarchyProvider
export interface IBCallHierarchyProvider extends vscode.CallHierarchyProvider {
  prepareCallHierarchy(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.CallHierarchyItem | vscode.CallHierarchyItem[]>;
  provideCallHierarchyIncomingCalls(item: vscode.CallHierarchyItem, token: vscode.CancellationToken): vscode.ProviderResult<vscode.CallHierarchyIncomingCall[]>;
  provideCallHierarchyOutgoingCalls(item: vscode.CallHierarchyItem, token: vscode.CancellationToken): vscode.ProviderResult<vscode.CallHierarchyOutgoingCall[]>;
}

// Interface for SemanticTokensProvider
export interface IBSemanticTokensProvider extends vscode.DocumentSemanticTokensProvider {
  provideDocumentSemanticTokens(document: vscode.TextDocument, token: vscode.CancellationToken): vscode.ProviderResult<vscode.SemanticTokens>;
  onDidChangeSemanticTokens?: vscode.Event<void>;
}

// Interface for LinkedEditingRangeProvider
export interface IBLinkedEditingRangeProvider extends vscode.LinkedEditingRangeProvider {
  provideLinkedEditingRanges(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken): vscode.ProviderResult<vscode.LinkedEditingRanges>;
}

// Interface for DocumentDropEditProvider
export interface IBDocumentDropEditProvider extends vscode.DocumentDropEditProvider {
  provideDocumentDropEdits(document: vscode.TextDocument, position: vscode.Position, dataTransfer: vscode.DataTransfer, token: vscode.CancellationToken): vscode.ProviderResult<vscode.DocumentDropEdits>;
}

// Interface for NotebookContentProvider
export interface IBNotebookContentProvider extends vscode.NotebookSerializer {
  deserializeNotebook(content: Uint8Array, token: vscode.CancellationToken): vscode.NotebookData | Thenable<vscode.NotebookData>;
  serializeNotebook(data: vscode.NotebookData, token: vscode.CancellationToken): Uint8Array | Thenable<Uint8Array>;
  onDidChangeNotebook?: vscode.Event<vscode.NotebookDocument>;
}

// Interface for NotebookKernelProvider
export interface IBNotebookKernelProvider extends vscode.NotebookKernelProvider {
  provideKernels(document: vscode.NotebookDocument, token: vscode.CancellationToken): vscode.ProviderResult<vscode.NotebookKernel[]>;
  onDidChangeKernels?: vscode.Event<vscode.NotebookDocument | undefined>;
}

// Interface for NotebookRendererMessaging
export interface IBNotebookRendererMessaging extends vscode.NotebookRendererMessaging {
  onDidReceiveMessage?: vscode.Event<{ readonly editor: vscode.NotebookEditor; readonly message: any }>;
  postMessage?(message: any): void;
}

// Interface for WebviewViewProvider
export interface IBWebviewViewProvider extends vscode.WebviewViewProvider {
  resolveWebviewView(webviewView: vscode.WebviewView, context: vscode.WebviewViewResolveContext, token: vscode.CancellationToken): void | Thenable<void>;
}

// Interface for CustomEditorProvider
export interface IBCustomEditorProvider extends vscode.CustomEditorProvider {
  openCustomDocument(uri: vscode.Uri, openContext: vscode.CustomDocumentOpenContext, token: vscode.CancellationToken): vscode.CustomDocument | Thenable<vscode.CustomDocument>;
  resolveCustomEditor(document: vscode.CustomDocument, webviewPanel: vscode.WebviewPanel, token: vscode.CancellationToken): void | Thenable<void>;
  saveCustomDocument(document: vscode.CustomDocument, cancellation: vscode.CancellationToken): void | Thenable<void>;
  saveCustomDocumentAs(document: vscode.CustomDocument, destination: vscode.Uri, cancellation: vscode.CancellationToken): void | Thenable<void>;
  revertCustomDocument(document: vscode.CustomDocument, cancellation: vscode.CancellationToken): void | Thenable<void>;
  backupCustomDocument(document: vscode.CustomDocument, context: vscode.CustomDocumentBackupContext, cancellation: vscode.CancellationToken): vscode.CustomDocumentBackup | Thenable<vscode.CustomDocumentBackup>;
  onDidChangeCustomDocument?: vscode.Event<vscode.CustomDocumentEditEvent | vscode.CustomDocumentContentChangeEvent>;
}

// Interface for TreeDataProvider
export interface IBTreeDataProvider<T> extends vscode.TreeDataProvider<T> {
  getTreeItem(element: T): vscode.TreeItem | Thenable<vscode.TreeItem>;
  getChildren(element?: T): vscode.ProviderResult<T[]>;
  getParent?(element: T): vscode.ProviderResult<T>;
  resolveTreeItem?(item: vscode.TreeItem, element: T, token: vscode.CancellationToken): vscode.ProviderResult<vscode.TreeItem>;
  onDidChangeTreeData?: vscode.Event<T | T[] | undefined>;
}

// Interface for TaskProvider
export interface IBTaskProvider extends vscode.TaskProvider {
  provideTasks(token: vscode.CancellationToken): vscode.ProviderResult<vscode.Task[]>;
  resolveTask(task: vscode.Task, token: vscode.CancellationToken): vscode.ProviderResult<vscode.Task>;
}

// Interface for DebugConfigurationProvider
export interface IBDebugConfigurationProvider extends vscode.DebugConfigurationProvider {
  provideDebugConfigurations?(folder: vscode.WorkspaceFolder | undefined, token?: vscode.CancellationToken): vscode.ProviderResult<vscode.DebugConfiguration[]>;
  resolveDebugConfiguration?(folder: vscode.WorkspaceFolder | undefined, debugConfiguration: vscode.DebugConfiguration, token?: vscode.CancellationToken): vscode.ProviderResult<vscode.DebugConfiguration>;
  resolveDebugConfigurationWithSubstitutedVariables?(folder: vscode.WorkspaceFolder | undefined, debugConfiguration: vscode.DebugConfiguration, token?: vscode.CancellationToken): vscode.ProviderResult<vscode.DebugConfiguration>;
  provideDebugAdapterExecutable?(folder: vscode.WorkspaceFolder | undefined, debugConfiguration: vscode.DebugConfiguration, token?: vscode.CancellationToken): vscode.ProviderResult<vscode.DebugAdapterExecutable>;
  debugAdapterExecutable?: vscode.DebugAdapterExecutable;
}
