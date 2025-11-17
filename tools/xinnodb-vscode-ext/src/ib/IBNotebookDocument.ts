import * as vscode from 'vscode';
import { IBUri } from './IBUri';

/**
 * @brief Notebook document for the InnoDB extension.
 */
export interface IBNotebookDocument extends vscode.NotebookDocument {
  readonly uri: IBUri;
  readonly cells: vscode.NotebookCell[];
  readonly metadata: { [key: string]: any };
  readonly isDirty: boolean;
  readonly isClosed: boolean;
}
