## XInnoDB VSCode Extension

Basic "Hello World" example extension for the XInnoDB workspace.

### Commands
- **XInnoDB: Hello World** (`xinnodb.helloWorld`): Shows a greeting notification.

### Features
- **VSCode Provider Interfaces**: Comprehensive TypeScript interfaces for all VSCode providers that interact with files, VCS, text documents, notebooks, and filesystem providers. All interfaces follow the naming convention `IB<ProviderName>`.

### Develop
1) Install dependencies:
```bash
cd tools/xinnodb-vscode-ext
npm install
```
2) Build once (or run a watcher):
```bash
npm run compile
# or: npm run watch
```
3) Run tests:
```bash
npm test
```
4) Debug:
- Open this folder in VS Code.
- Press F5 or run "Run Extension" from the debug panel.
- In the Extension Development Host window, open the Command Palette and run "XInnoDB: Hello World".

### Package
Create a `.vsix`:
```bash
npm run package
```

### Deterministic Builds
This extension is designed to be built deterministically using Nix. The `default.nix` file ensures:
- All Node.js dependencies are pinned with exact versions
- No network access during builds
- Reproducible builds across different systems


