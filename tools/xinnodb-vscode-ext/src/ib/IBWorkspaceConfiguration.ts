import * as vscode from 'vscode';

/**
 * @brief Workspace configuration for the InnoDB extension.
 */
export interface IBWorkspaceConfiguration extends vscode.WorkspaceConfiguration {
  get<T>(section: string, defaultValue?: T): T;
  update(section: string, value: any, configurationTarget?: vscode.ConfigurationTarget): Promise<void>;
  has(section: string): boolean;
  inspect<T>(section: string): { key: string; defaultValue?: T; globalValue?: T; workspaceValue?: T; workspaceFolderValue?: T } | undefined;
}
