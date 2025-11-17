import * as vscode from 'vscode';
import { IBUri } from './IBUri';
import { IBProviderResult } from './IBProviderResult';
import { IBNotebookData } from './IBNotebookData';

/**
 * @brief Notebook content provider for the InnoDB extension.
 */
export interface IBNotebookContentProvider {
  provideNotebookContent(uri: IBUri, token: vscode.CancellationToken): IBProviderResult<vscode.NotebookData>;
  resolveNotebook(uri: IBUri, notebook: vscode.NotebookData, token: vscode.CancellationToken): IBProviderResult<void>;
  saveNotebook(uri: IBUri, notebook: vscode.NotebookData, token: vscode.CancellationToken): IBProviderResult<void>;
  saveNotebookAs(uri: IBUri, notebook: vscode.NotebookData, targetUri: IBUri, token: vscode.CancellationToken): IBProviderResult<void>;
  onDidChangeNotebook?: vscode.Event<vscode.NotebookDocument>;
}
