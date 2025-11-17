import * as vscode from 'vscode';
import { IBUri } from './IBUri';

/**
 * @brief Decorations for source control resources in the InnoDB extension.
 */
export interface IBSourceControlResourceDecorations {
  strikeThrough: boolean;
  faded: boolean;
  tooltip: string;
  iconPath: string | IBUri | vscode.ThemeIcon | { light: string | IBUri; dark: string | IBUri };
  letter: string;
  color: vscode.ThemeColor | string;
}
