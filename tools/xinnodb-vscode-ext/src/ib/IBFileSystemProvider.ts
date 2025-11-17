import * as vscode from 'vscode';
import { IBUri } from './IBUri';
import { IBProviderResult } from './IBProviderResult';

/**
 * @brief File system provider for the InnoDB extension.
 */
export interface IBFileSystemProvider {
  readonly isCaseSensitive?: boolean;
  readonly isReadonly?: boolean;
  readFile(uri: IBUri): IBProviderResult<Uint8Array>;
  writeFile(uri: IBUri, content: Uint8Array, options?: { create?: boolean; overwrite?: boolean }): void;
  delete(uri: IBUri, options?: { recursive?: boolean }): void;
  rename(oldUri: IBUri, newUri: IBUri, options?: { overwrite?: boolean }): void;
  createDirectory(uri: IBUri): void;
  readDirectory(uri: IBUri): IBProviderResult<[string, vscode.FileType][]>;
  stat(uri: IBUri): IBProviderResult<vscode.FileStat>;
  watch(uri: IBUri, options?: { recursive?: boolean; excludes?: string[] }): vscode.Disposable;
}
