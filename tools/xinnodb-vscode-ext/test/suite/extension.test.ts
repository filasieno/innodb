import * as assert from 'assert';

// You can import and use all API from the 'vscode' module
// as well as import your extension to test it
import * as vscode from 'vscode';
// import * as myExtension from '../../src/extension';

suite('Extension Test Suite', () => {
  vscode.window.showInformationMessage('Start all tests.');

  test('Sample test', () => {
    assert.strictEqual(-1, [1, 2, 3].indexOf(5));
    assert.strictEqual(-1, [1, 2, 3].indexOf(0));
  });

  test('Hello World command is registered', async () => {
    const commands = await vscode.commands.getCommands(true);
    const helloWorldCommand = commands.find(cmd => cmd === 'xinnodb.helloWorld');
    assert.ok(helloWorldCommand, 'xinnodb.helloWorld command should be registered');
  });
});
