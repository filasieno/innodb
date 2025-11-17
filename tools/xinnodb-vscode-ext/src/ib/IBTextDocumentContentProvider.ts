import * as vscode from 'vscode';
import { IBUri } from './IBUri';
import { IBProviderResult } from './IBProviderResult';

/**
 * @brief Text document content provider for the InnoDB extension.
 */
export interface IBTextDocumentContentProvider {
  provideTextDocumentContent(uri: IBUri, token: vscode.CancellationToken): IBProviderResult<string>;
  onDidChange?: vscode.Event<IBUri>;
}
