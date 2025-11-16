## XIB VSCode Plugin

Basic “Hello World” example extension for the innodb workspace.

### Commands
- **XIB: Hello World** (`xib.helloWorld`): Shows a greeting notification.

### Develop
1) Install dependencies:
```bash
cd tools/xib-vscode-plugin
npm install
```
2) Build once (or run a watcher):
```bash
npm run compile
# or: npm run watch
```
3) Debug:
- Open this folder in VS Code.
- Press F5 or run “Run Extension” from the debug panel.
- In the Extension Development Host window, open the Command Palette and run “XIB: Hello World”.

### Package
Create a `.vsix`:
```bash
npm run package
```


