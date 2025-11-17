import * as vscode from 'vscode';

/**
 * @brief Enumeration of file change types in the InnoDB file system.
 *
 * IBFileChangeType mirrors the VS Code FileChangeType enum but provides
 * additional context and documentation specific to the InnoDB virtual workspace.
 * These values are used to indicate what kind of change occurred to a file or directory.
 *
 * @enum {number}
 * @author Fabio N. Filasieno
 * @date 2025-01-16
 * @since 1.0.0
 * @version 1.0.0
 *
 * @example
 * ```typescript
 * // Check the type of file change
 * if (change.type === IBFileChangeType.Created) {
 *   console.log('A new file was created');
 * } else if (change.type === IBFileChangeType.Deleted) {
 *   console.log('A file was deleted');
 * } else if (change.type === IBFileChangeType.Changed) {
 *   console.log('A file was modified');
 * }
 * ```
 */
export enum IBFileChangeType {
  /**
   * The contents or metadata of a file have changed.
   *
   * This type is used when a file's content is modified, or when its metadata
   * (such as permissions, timestamps, or other attributes) is updated. This
   * is the most common type of file change in development workflows.
   *
   * @member {number} Changed
   * @since 1.0.0
   *
   * @example
   * ```typescript
   * // File content was modified
   * const change = { type: IBFileChangeType.Changed, uri: fileUri };
   * ```
   */
  Changed = 1,

  /**
   * A file has been created.
   *
   * This type indicates that a new file or directory has been added to the
   * file system. This commonly occurs when users create new files, copy files,
   * or when build processes generate output files.
   *
   * @member {number} Created
   * @since 1.0.0
   *
   * @example
   * ```typescript
   * // New file created
   * const change = { type: IBFileChangeType.Created, uri: newFileUri };
   * ```
   */
  Created = 2,

  /**
   * A file has been deleted.
   *
   * This type indicates that a file or directory has been removed from the
   * file system. This occurs when users delete files, or when cleanup processes
   * remove temporary or generated files.
   *
   * @member {number} Deleted
   * @since 1.0.0
   *
   * @example
   * ```typescript
   * // File was deleted
   * const change = { type: IBFileChangeType.Deleted, uri: deletedFileUri };
   * ```
   */
  Deleted = 3,
}
