import * as vscode from 'vscode';

/**
 * @brief Options for configuring terminal behavior in the InnoDB extension.
 */
export interface IBTerminalOptions extends vscode.TerminalOptions {
  name?: string;
  shellPath?: string;
  shellArgs?: string[];
  cwd?: string | vscode.Uri;
  env?: { [key: string]: string | null };
  hideFromUser?: boolean;
  iconPath?: vscode.Uri | { light: vscode.Uri; dark: vscode.Uri } | vscode.ThemeIcon;
  useDefaultProfile?: boolean;
}
