import * as vscode from 'vscode';
import { IBFileChangeType } from './IBFileChangeType';

/**
 * @brief Represents a file change event in the InnoDB virtual workspace.
 *
 * This interface extends vscode.FileChangeEvent and provides information
 * about changes to files and directories in the virtual file system.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-16
 * @since 1.0.0
 * @version 1.0.0
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#FileChangeEvent}
 */
export interface IBFileChangeEvent {
  /**
   * The type of change that occurred.
   */
  readonly type: IBFileChangeType;

  /**
   * The URI of the file that changed.
   */
  readonly uri: vscode.Uri;
}
