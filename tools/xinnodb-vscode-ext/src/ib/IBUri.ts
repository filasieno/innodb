import * as vscode from 'vscode';

/**
 * @brief Represents a URI in the InnoDB extension.
 */
export interface IBUri extends vscode.Uri {
  readonly scheme: string;
  readonly authority: string;
  readonly path: string;
  readonly query: string;
  readonly fragment: string;
  readonly fsPath: string;
  with(scheme: { scheme: string }): IBUri;
  with(authority: { authority: string }): IBUri;
  with(path: { path: string }): IBUri;
  with(query: { query: string }): IBUri;
  with(fragment: { fragment: string }): IBUri;
  toString(): string;
  toJSON(): { scheme: string; authority: string; path: string; query: string; fragment: string };
}

export const IBUri = {
  parse: (value: string): IBUri => vscode.Uri.parse(value) as IBUri,
  file: (path: string): IBUri => vscode.Uri.file(path) as IBUri,
  joinPath: (base: IBUri, ...paths: string[]): IBUri => vscode.Uri.joinPath(base, ...paths) as IBUri,
};
