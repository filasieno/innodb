import * as vscode from 'vscode';

/**
 * @brief Provides additional context for terminal links in the InnoDB extension.
 */
export interface IBTerminalLinkContext {
  readonly terminal: vscode.Terminal;
  readonly line: string;
  readonly linkText: string;
}
