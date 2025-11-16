import * as vscode from 'vscode';
import * as path from 'path';

export function activate(context: vscode.ExtensionContext): void {
  const disposable = vscode.commands.registerCommand('xib.helloWorld', () => {
    try {
      const nativePath = path.join(context.extensionPath, 'native', 'index.js');
      // eslint-disable-next-line @typescript-eslint/no-var-requires
      const native = require(nativePath);
      const msg = typeof native?.greet === 'function' ? native.greet() : 'Native module did not export greet()';
      vscode.window.showInformationMessage(`Hello World from XIB! ${msg}`);
    } catch (err) {
      vscode.window.showInformationMessage('Hello World from XIB! (native module not available)');
    }
  });

  context.subscriptions.push(disposable);
}

export function deactivate(): void {
  // nothing to clean up
}


