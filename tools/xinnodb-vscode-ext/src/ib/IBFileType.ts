import * as vscode from 'vscode';

/**
 * @fileoverview InnoDB file type enumeration
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @typedef IBFileType
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileType}
 * @example
 * @brief ```typescript
 * import { IBFileType } from './ib';
 *
 * if (fileType === IBFileType.Directory) {
 *   console.log('This is a directory');
 * }
 * ```
 */

/**
 * @brief Enumeration of file types supported by the InnoDB extension's file system.
 *
 * This type alias mirrors vscode.FileType and provides the same enumeration
 * values for identifying different types of files and directories.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @typedef {vscode.FileType} IBFileType
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileType}
 * @example
 * ```typescript
 * import { IBFileType } from './ib';
 *
 * // Check file type
 * switch (fileType) {
 *   case IBFileType.File:
 *     console.log('Regular file');
 *     break;
 *   case IBFileType.Directory:
 *     console.log('Directory');
 *     break;
 *   case IBFileType.SymbolicLink:
 *     console.log('Symbolic link');
 *     break;
 *   default:
 *     console.log('Unknown file type');
 * }
 * ```
 */
export type IBFileType = vscode.FileType;

/**
 * @brief The file type is unknown or not specified.
 *
 * @const {IBFileType} Unknown
 * @default 0
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileType}
 */
export const IBFileType = {
  /**
   * The file type is unknown.
   *
   * This value is used when the file system cannot determine the type
   * of a file or when the type information is not available.
   *
   * @property {number} Unknown
   * @readonly
   * @default 0
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileType}
   * @example
   * ```typescript
   * if (stat.type === IBFileType.Unknown) {
   *   console.log('File type could not be determined');
   * }
   * ```
   */
  Unknown: vscode.FileType.Unknown,

  /**
   * A regular file.
   *
   * This represents a standard file that contains data or executable code.
   *
   * @property {number} File
   * @readonly
   * @default 1
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileType}
   * @example
   * ```typescript
   * if (stat.type === IBFileType.File) {
   *   console.log('This is a regular file');
   * }
   * ```
   */
  File: vscode.FileType.File,

  /**
   * A directory.
   *
   * This represents a container that can hold files and other directories.
   *
   * @property {number} Directory
   * @readonly
   * @default 2
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileType}
   * @example
   * ```typescript
   * if (stat.type === IBFileType.Directory) {
   *   console.log('This is a directory');
   * }
   * ```
   */
  Directory: vscode.FileType.Directory,

  /**
   * A symbolic link.
   *
   * This represents a file that points to another file or directory.
   *
   * @property {number} SymbolicLink
   * @readonly
   * @default 64
   * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileType}
   * @example
   * ```typescript
   * if (stat.type === IBFileType.SymbolicLink) {
   *   console.log('This is a symbolic link');
   * }
   * ```
   */
  SymbolicLink: vscode.FileType.SymbolicLink
} as const;
