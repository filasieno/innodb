import * as vscode from 'vscode';
import { IBTerminalLink } from './IBTerminalLink';
import { IBTerminalLinkContext } from './IBTerminalLinkContext';

/**
 * @brief Provider for terminal links in the InnoDB extension.
 */
export interface IBTerminalLinkProvider extends vscode.TerminalLinkProvider<IBTerminalLink> {
  provideTerminalLinks(context: IBTerminalLinkContext, token: vscode.CancellationToken): vscode.ProviderResult<IBTerminalLink[]>;
  handleTerminalLink(link: IBTerminalLink): vscode.ProviderResult<void>;
}
