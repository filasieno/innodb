import * as vscode from 'vscode';

/**
 * @brief Command system for the InnoDB extension.
 *
 * IBCommand represents executable actions that can be invoked by users
 * through menus, keybindings, or programmatically. Commands provide a
 * consistent way to expose functionality in VS Code.
 *
 * @author Fabio N. Filasieno
 * @date 2025-01-01
 * @since 1.0.0
 * @version 1.0.0
 * @see {@link https://code.visualstudio.com/api/references/vscode-api#Command}
 */
export interface IBCommand extends vscode.Command {
  /**
   * The identifier of the command.
   */
  command: string;

  /**
   * Title of the command, like "Save".
   */
  title: string;

  /**
   * The tooltip text when you hover over the command.
   */
  tooltip?: string;

  /**
   * Arguments that the command handler should be invoked with.
   */
  arguments?: any[];
}
