import * as vscode from 'vscode';
import { IBUri } from './IBUri';

/**
 * @brief Provider for quick diff information in the InnoDB extension.
 */
export interface IBQuickDiffProvider {
  readonly label: string;
  provideOriginalResource(uri: IBUri, token: vscode.CancellationToken): vscode.ProviderResult<IBUri>;
}
