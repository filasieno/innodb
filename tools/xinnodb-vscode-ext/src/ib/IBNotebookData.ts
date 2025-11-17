import * as vscode from 'vscode';

/**
 * @brief Notebook data structure for the InnoDB extension.
 */
export interface IBNotebookData extends vscode.NotebookData {
  cells: vscode.NotebookCellData[];
  metadata?: { [key: string]: any };
}
