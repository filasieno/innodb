import * as vscode from 'vscode';

export function activate(context: vscode.ExtensionContext): void {
  const disposable = vscode.commands.registerCommand('xinnodb.helloWorld', () => {
    vscode.window.showInformationMessage('Hello World from XInnoDB!');
  });

  context.subscriptions.push(disposable);
}

export function deactivate(): void {
  // nothing to clean up
}


