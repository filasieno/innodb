import * as vscode from 'vscode';

/**
 * @brief Represents a clickable link within terminal output in the InnoDB extension.
 */
export interface IBTerminalLink extends vscode.TerminalLink {
  startIndex: number;
  length: number;
  tooltip?: string;
}
