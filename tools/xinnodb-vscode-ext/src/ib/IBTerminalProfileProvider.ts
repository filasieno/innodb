import * as vscode from 'vscode';
import { IBTerminalProfile } from './IBTerminalProfile';

/**
 * @brief Provider for terminal profiles in the InnoDB extension.
 */
export interface IBTerminalProfileProvider extends vscode.TerminalProfileProvider {
  provideTerminalProfile(token: vscode.CancellationToken): vscode.ProviderResult<IBTerminalProfile>;
  provideTerminalProfiles(token: vscode.CancellationToken): vscode.ProviderResult<IBTerminalProfile[]>;
}
