import * as vscode from 'vscode';
import { IBTerminalDimensions } from './IBTerminalDimensions';

/**
 * @brief Represents a pseudoterminal in the InnoDB extension.
 */
export interface IBPseudoterminal extends vscode.Pseudoterminal {
  open(initialDimensions: IBTerminalDimensions | undefined): void;
  setDimensions(dimensions: IBTerminalDimensions): void;
  handleInput(data: string): void;
  close(): void;
  onDidClose?: vscode.Event<void>;
  onDidWrite: vscode.Event<string>;
  onDidChangeTitle?: vscode.Event<string>;
}
