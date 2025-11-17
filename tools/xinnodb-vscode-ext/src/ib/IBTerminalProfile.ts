import * as vscode from 'vscode';

/**
 * @brief Defines a terminal profile for the InnoDB extension.
 */
export interface IBTerminalProfile extends vscode.TerminalProfile {
  profileName: string;
  path: string;
  args?: string[];
  env?: { [key: string]: string | null };
  isDefault?: boolean;
  icon?: vscode.Uri | { light: vscode.Uri; dark: vscode.Uri } | vscode.ThemeIcon;
}
