/**
 * TypeScript definitions for Language Server Protocol types
 * Generated from LSP metamodel v3.17.0
 */

// ============================================================================
// Core Types
// ============================================================================

export interface Position {
  /** Line position in a document (zero-based) */
  line: number;
  /** Character offset on a line in a document (zero-based) */
  character: number;
}

export interface Range {
  /** The range's start position */
  start: Position;
  /** The range's end position */
  end: Position;
}

export interface Location {
  /** The text document's URI */
  uri: string;
  /** The range within the document */
  range: Range;
}

export interface LocationLink {
  /** Span of the origin of this link */
  originSelectionRange?: Range;
  /** The target resource identifier of this link */
  targetUri: string;
  /** The full target range of this link */
  targetRange: Range;
  /** The range that should be selected and revealed when this link is being followed */
  targetSelectionRange: Range;
}

// ============================================================================
// Document Changes
// ============================================================================

export interface TextEdit {
  /** The range of the text document to be manipulated */
  range: Range;
  /** The string to be inserted. For delete operations use an empty string. */
  newText: string;
}

export interface InsertReplaceEdit {
  /** The string to be inserted */
  newText: string;
  /** The range if the insert is requested */
  insert: Range;
  /** The range if the replace is requested */
  replace: Range;
}

export interface AnnotatedTextEdit extends TextEdit {
  /** The annotation that describes the change */
  annotationId: string;
}

export interface TextDocumentEdit {
  /** The text document to change */
  textDocument: VersionedTextDocumentIdentifier;
  /** The edits to be applied */
  edits: (TextEdit | InsertReplaceEdit | AnnotatedTextEdit)[];
}

export interface CreateFile {
  /** The resource to create */
  uri: string;
  /** Additional options */
  options?: CreateFileOptions;
  /** An optional annotation identifier */
  annotationId?: string;
}

export interface CreateFileOptions {
  /** Overwrite existing file. Overwrite wins over `ignoreIfExists` */
  overwrite?: boolean;
  /** Ignore if exists */
  ignoreIfExists?: boolean;
}

export interface RenameFile {
  /** The old (existing) location */
  oldUri: string;
  /** The new location */
  newUri: string;
  /** Rename options */
  options?: RenameFileOptions;
  /** An optional annotation identifier */
  annotationId?: string;
}

export interface RenameFileOptions {
  /** Overwrite target if existing. Overwrite wins over `ignoreIfExists` */
  overwrite?: boolean;
  /** Ignores if target exists */
  ignoreIfExists?: boolean;
}

export interface DeleteFile {
  /** The file to delete */
  uri: string;
  /** Delete options */
  options?: DeleteFileOptions;
  /** An optional annotation identifier */
  annotationId?: string;
}

export interface DeleteFileOptions {
  /** Delete the content recursively if a folder is denoted */
  recursive?: boolean;
  /** Ignore the operation if the file doesn't exist */
  ignoreIfNotExists?: boolean;
}

export type DocumentChange =
  | TextDocumentEdit
  | CreateFile
  | RenameFile
  | DeleteFile;

export interface WorkspaceEdit {
  /** Holds changes to existing resources */
  changes?: { [uri: string]: TextEdit[] };
  /** Depending on the client capability `workspace.workspaceEdit.resourceOperations` */
  documentChanges?: DocumentChange[];
  /** A map of change annotations that can be referenced in `AnnotatedTextEdit`s */
  changeAnnotations?: { [id: string]: ChangeAnnotation };
}

export interface ChangeAnnotation {
  /** A human-readable string describing the actual change */
  label: string;
  /** A flag which indicates that user confirmation is needed */
  needsConfirmation?: boolean;
  /** A human-readable string which is rendered less prominent in the user interface */
  description?: string;
}

// ============================================================================
// Text Documents
// ============================================================================

export interface TextDocumentIdentifier {
  /** The text document's URI */
  uri: string;
}

export interface TextDocumentItem {
  /** The text document's URI */
  uri: string;
  /** The text document's language identifier */
  languageId: string;
  /** The version number of this document (it will increase after each change) */
  version: number;
  /** The content of the opened text document */
  text: string;
}

export interface VersionedTextDocumentIdentifier extends TextDocumentIdentifier {
  /** The version number of this document */
  version: number;
}

export interface OptionalVersionedTextDocumentIdentifier extends TextDocumentIdentifier {
  /** The version number of this document */
  version: number | null;
}

export interface TextDocumentPositionParams {
  /** The text document */
  textDocument: TextDocumentIdentifier;
  /** The position inside the text document */
  position: Position;
}

// ============================================================================
// Diagnostics
// ============================================================================

export enum DiagnosticSeverity {
  /** Reports an error */
  Error = 1,
  /** Reports a warning */
  Warning = 2,
  /** Reports an information */
  Information = 3,
  /** Reports a hint */
  Hint = 4,
}

export interface DiagnosticRelatedInformation {
  /** The location of this related diagnostic information */
  location: Location;
  /** The message of this related diagnostic information */
  message: string;
}

export interface Diagnostic {
  /** The range at which the message applies */
  range: Range;
  /** The diagnostic's severity. Can be omitted. If omitted it is up to the client to interpret diagnostics as error, warning, info or hint */
  severity?: DiagnosticSeverity;
  /** The diagnostic's code, which might appear in the user interface */
  code?: number | string;
  /** An optional property to describe the error code */
  codeDescription?: CodeDescription;
  /** A human-readable string describing the source of this diagnostic */
  source?: string;
  /** The diagnostic's message */
  message: string;
  /** Additional metadata about the diagnostic */
  tags?: DiagnosticTag[];
  /** An array of related diagnostic information */
  relatedInformation?: DiagnosticRelatedInformation[];
  /** The diagnostic's data */
  data?: unknown;
}

export enum DiagnosticTag {
  /** Unused or unnecessary code */
  Unnecessary = 1,
  /** Deprecated or obsolete code */
  Deprecated = 2,
}

export interface CodeDescription {
  /** An URI to open with more information about the diagnostic error */
  href: string;
}

// ============================================================================
// Completion
// ============================================================================

export enum CompletionTriggerKind {
  /** Completion was triggered by typing an identifier (24x7 code complete) */
  Invoked = 1,
  /** Completion was triggered by a trigger character */
  TriggerCharacter = 2,
  /** Completion was re-triggered as the current completion list is incomplete */
  TriggerForIncompleteCompletions = 3,
}

export interface CompletionContext {
  /** How the completion was triggered */
  triggerKind: CompletionTriggerKind;
  /** The trigger character (a single character) that has trigger code complete */
  triggerCharacter?: string;
}

export enum InsertTextFormat {
  /** The primary text to be inserted is treated as a plain string */
  PlainText = 1,
  /** The primary text to be inserted is treated as a snippet */
  Snippet = 2,
}

export enum CompletionItemKind {
  Text = 1,
  Method = 2,
  Function = 3,
  Constructor = 4,
  Field = 5,
  Variable = 6,
  Class = 7,
  Interface = 8,
  Module = 9,
  Property = 10,
  Unit = 11,
  Value = 12,
  Enum = 13,
  Keyword = 14,
  Snippet = 15,
  Color = 16,
  File = 17,
  Reference = 18,
  Folder = 19,
  EnumMember = 20,
  Constant = 21,
  Struct = 22,
  Event = 23,
  Operator = 24,
  TypeParameter = 25,
}

export interface CompletionItemLabelDetails {
  /** An optional string which is rendered less prominently directly after {@link CompletionItem.label label} */
  detail?: string;
  /** An optional string which is rendered less prominently after {@link CompletionItem.detail} */
  description?: string;
}

export interface CompletionItem {
  /** The label of this completion item */
  label: string;
  /** Additional details for the label */
  labelDetails?: CompletionItemLabelDetails;
  /** The kind of this completion item */
  kind?: CompletionItemKind;
  /** Tags for this completion item */
  tags?: CompletionItemTag[];
  /** A human-readable string with additional information about this item */
  detail?: string;
  /** A human-readable string that represents a doc-comment */
  documentation?: string | MarkupContent;
  /** Indicates if this item is deprecated */
  deprecated?: boolean;
  /** Select this item when showing */
  preselect?: boolean;
  /** A string that should be inserted into a document when selecting this completion */
  insertText?: string;
  /** The format of the insert text */
  insertTextFormat?: InsertTextFormat;
  /** How whitespace and indentation is handled during completion item insertion */
  insertTextMode?: InsertTextMode;
  /** An edit which is applied to a document when selecting this completion */
  textEdit?: TextEdit | InsertReplaceEdit;
  /** The edit text used if the completion item is part of a CompletionList and CompletionList defines an item default for `editRange` */
  textEditText?: string;
  /** An optional array of additional text edits that are applied when selecting this completion */
  additionalTextEdits?: TextEdit[];
  /** An optional command that is executed *after* inserting this completion */
  command?: Command;
  /** A data entry field that is preserved on a completion item between a completion and a completion resolve request */
  data?: unknown;
}

export enum CompletionItemTag {
  /** Render a completion as obsolete, usually using a strike-out */
  Deprecated = 1,
}

export enum InsertTextMode {
  /** The insertion or replace strings is taken as it is */
  asIs = 1,
  /** The editor adjusts leading whitespace of new lines so that they match the indentation of the line for which the item is accepted */
  adjustIndentation = 2,
}

export interface CompletionList {
  /** This list is not complete. Further typing should result in recomputing this list */
  isIncomplete: boolean;
  /** The completion items */
  items: CompletionItem[];
}

export interface CompletionParams extends TextDocumentPositionParams {
  /** The completion context */
  context?: CompletionContext;
}

// ============================================================================
// Hover
// ============================================================================

export interface Hover {
  /** The hover's content */
  contents: MarkupContent | MarkedString | MarkedString[];
  /** An optional range is a range inside a text document that is used to visualize a hover */
  range?: Range;
}

export type MarkedString = string | { language: string; value: string };

// ============================================================================
// Signature Help
// ============================================================================

export interface SignatureHelp {
  /** One or more signatures */
  signatures: SignatureInformation[];
  /** The active signature */
  activeSignature?: number;
  /** The active parameter of the active signature */
  activeParameter?: number;
}

export interface SignatureInformation {
  /** The label of this signature */
  label: string;
  /** The human-readable doc-comment of this signature */
  documentation?: string | MarkupContent;
  /** The parameters of this signature */
  parameters?: ParameterInformation[];
  /** The index of the active parameter */
  activeParameter?: number;
}

export interface ParameterInformation {
  /** The label of this parameter information */
  label: string | [number, number];
  /** The human-readable doc-comment of this parameter */
  documentation?: string | MarkupContent;
}

export interface SignatureHelpParams extends TextDocumentPositionParams {
  /** The signature help context */
  context?: SignatureHelpContext;
}

export enum SignatureHelpTriggerKind {
  /** Signature help was invoked manually by the user or by a command */
  Invoked = 1,
  /** Signature help was triggered by a trigger character */
  TriggerCharacter = 2,
  /** Signature help was triggered by the cursor moving or by the document content changing */
  ContentChange = 3,
}

export interface SignatureHelpContext {
  /** Action that caused signature help to be triggered */
  triggerKind: SignatureHelpTriggerKind;
  /** Character that caused signature help to be triggered */
  triggerCharacter?: string;
  /** `true` if signature help was already showing when it was triggered */
  isRetrigger: boolean;
  /** The currently active `SignatureHelp` */
  activeSignatureHelp?: SignatureHelp;
}

// ============================================================================
// Document Symbols
// ============================================================================

export enum SymbolKind {
  File = 1,
  Module = 2,
  Namespace = 3,
  Package = 4,
  Class = 5,
  Method = 6,
  Property = 7,
  Field = 8,
  Constructor = 9,
  Enum = 10,
  Interface = 11,
  Function = 12,
  Variable = 13,
  Constant = 14,
  String = 15,
  Number = 16,
  Boolean = 17,
  Array = 18,
  Object = 19,
  Key = 20,
  Null = 21,
  EnumMember = 22,
  Struct = 23,
  Event = 24,
  Operator = 25,
  TypeParameter = 26,
}

export enum SymbolTag {
  /** Render a symbol as obsolete, usually using a strike-out */
  Deprecated = 1,
}

export interface DocumentSymbol {
  /** The name of this symbol */
  name: string;
  /** More detail for this symbol, e.g. the signature of a function */
  detail?: string;
  /** The kind of this symbol */
  kind: SymbolKind;
  /** Tags for this document symbol */
  tags?: SymbolTag[];
  /** Indicates if this symbol is deprecated */
  deprecated?: boolean;
  /** The range enclosing this symbol not including leading/trailing whitespace but everything else, e.g. comments and code */
  range: Range;
  /** The range that should be selected and revealed when this symbol is being picked, e.g. the name of a function */
  selectionRange: Range;
  /** Children of this symbol, e.g. properties of a class */
  children?: DocumentSymbol[];
}

export interface SymbolInformation {
  /** The name of this symbol */
  name: string;
  /** The kind of this symbol */
  kind: SymbolKind;
  /** Tags for this symbol */
  tags?: SymbolTag[];
  /** Indicates if this symbol is deprecated */
  deprecated?: boolean;
  /** The location of this symbol */
  location: Location;
  /** The name of the symbol containing this symbol */
  containerName?: string;
}

// ============================================================================
// Commands
// ============================================================================

export interface Command {
  /** Title of the command, like `save` */
  title: string;
  /** The identifier of the actual command handler */
  command: string;
  /** Arguments that the command handler should be invoked with */
  arguments?: unknown[];
}

// ============================================================================
// Markup Content
// ============================================================================

export enum MarkupKind {
  /** Plain text is supported as a content format */
  PlainText = 'plaintext',
  /** Markdown is supported as a content format */
  Markdown = 'markdown',
}

export interface MarkupContent {
  /** The type of the Markup */
  kind: MarkupKind;
  /** The content itself */
  value: string;
}

// ============================================================================
// Work Done Progress
// ============================================================================

export interface WorkDoneProgressBegin {
  kind: 'begin';
  /** Mandatory title of the progress operation */
  title: string;
  /** Controls if a cancel button should show to allow the user to cancel the long-running operation */
  cancellable?: boolean;
  /** Optional, more detailed associated progress message */
  message?: string;
  /** Optional progress percentage to display (value 100 is considered 100%) */
  percentage?: number;
}

export interface WorkDoneProgressReport {
  kind: 'report';
  /** Controls if a cancel button should show to allow the user to cancel the long-running operation */
  cancellable?: boolean;
  /** Optional, more detailed associated progress message */
  message?: string;
  /** Optional progress percentage to display (value 100 is considered 100%) */
  percentage?: number;
}

export interface WorkDoneProgressEnd {
  kind: 'end';
  /** Optional, more detailed associated progress message */
  message?: string;
}

export type WorkDoneProgress = WorkDoneProgressBegin | WorkDoneProgressReport | WorkDoneProgressEnd;

export interface WorkDoneProgressParams {
  /** An optional token that a server can use to report work done progress */
  workDoneToken?: ProgressToken;
}

export type ProgressToken = number | string;

// ============================================================================
// JSON-RPC Message Types
// ============================================================================

export interface RequestMessage {
  jsonrpc: '2.0';
  id: number | string;
  method: string;
  params?: unknown;
}

export interface ResponseMessage {
  jsonrpc: '2.0';
  id: number | string | null;
  result?: unknown;
  error?: ResponseError;
}

export interface NotificationMessage {
  jsonrpc: '2.0';
  method: string;
  params?: unknown;
}

export interface ResponseError {
  /** A number indicating the error type that occurred */
  code: number;
  /** A string providing a short description of the error */
  message: string;
  /** A primitive or structured value that contains additional information about the error */
  data?: unknown;
}

// ============================================================================
// Error Codes
// ============================================================================

export const ErrorCodes = {
  // Defined by JSON-RPC
  ParseError: -32700,
  InvalidRequest: -32600,
  MethodNotFound: -32601,
  InvalidParams: -32602,
  InternalError: -32603,

  // Defined by LSP
  ServerNotInitialized: -32002,
  UnknownErrorCode: -32001,

  // Defined by the protocol
  RequestFailed: -32803,
  ServerCancelled: -32802,
  ContentModified: -32801,
  RequestCancelled: -32800,
} as const;

export type ErrorCode = typeof ErrorCodes[keyof typeof ErrorCodes];
