import * as vscode from 'vscode';
import { IBFileType } from './IBFileType';

/**
 * @brief Represents the stat information of a file or directory in the InnoDB file system.
 */
export interface IBFileStat extends vscode.FileStat {
  type: IBFileType;
  ctime: number;
  mtime: number;
  size: number;
  permissions?: vscode.FilePermission;
}
