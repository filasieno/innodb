# Kaitai Struct Visualization Tools

## VS Code Setup (Local Editing)

### 1. Install Required Extension

Install the **YAML extension by Red Hat**:
```
ext install redhat.vscode-yaml
```

Or use the command palette:
- Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
- Type "Install Extensions"
- Search for "YAML" by Red Hat
- Click Install

### 2. Reload VS Code

After installing, reload your window:
- Press `Ctrl+Shift+P`
- Type "Reload Window"
- Hit Enter

### Features You'll Get

✅ **Syntax Highlighting** - Different colors for keys, values, strings, numbers  
✅ **Schema Validation** - Real-time error checking  
✅ **Autocomplete** - Suggests valid Kaitai keys  
✅ **Hover Documentation** - Descriptions on hover  
✅ **Formatting** - Auto-format with `Shift+Alt+F`

### Custom Color Scheme

Your `.vscode/settings.json` now includes:
- **Cyan** (#4EC9B0) - YAML keys (`id:`, `type:`, etc.)
- **Orange** (#CE9178) - String values
- **Green** (#B5CEA8) - Numeric values

---

## Kaitai Web IDE (Visual Testing & Debugging)

The **best tool for visualizing** and testing Kaitai Struct files:

### 🌐 Access Online

**Stable Version:**  
https://ide.kaitai.io/

**Development Version (latest features):**  
https://ide.kaitai.io/devel/

### Features

1. **Live Preview**
   - See parsed structure in real-time
   - Hex viewer with field highlighting
   - Navigate between hex and structure

2. **Visual Debugging**
   - Click fields to see their hex location
   - Inspect byte-by-byte parsing
   - See parsed values instantly

3. **Code Generation**
   - Generate parser code in multiple languages
   - Download generated parsers
   - Test on sample files

### How to Use

1. **Upload your `.ksy` file**
   - Click "Open .ksy file" button
   - Select `sprite-lfs.ksy`

2. **Upload sample data** (optional)
   - Click "Open binary file"
   - Load a Sprite LFS disk image

3. **Explore**
   - Left panel: Hex view with highlighting
   - Right panel: Parsed structure tree
   - Bottom: Generated code preview

### Local Web IDE Setup (Optional)

If you want to run the Web IDE locally:

```bash
# Clone the repository
git clone https://github.com/kaitai-io/kaitai_struct_webide.git
cd kaitai_struct_webide

# Install dependencies
npm install

# Run local server
npm start

# Open browser to http://localhost:8000
```

---

## Alternative VS Code Extensions

### Indent Rainbow
Colorizes indentation levels - helpful for YAML:
```
ext install oderwat.indent-rainbow
```

### Better Comments
Enhances comment visibility:
```
ext install aaron-bond.better-comments
```

---

## Command Line Tools

### Kaitai Struct Compiler
Compile `.ksy` to parsers:

```bash
nix develop -c bash -c "ksc -t python sprite-lfs.ksy -d output/"
```

Supported targets: `cpp_stl`, `csharp`, `java`, `javascript`, `python`, `ruby`, `rust`

### Kaitai Struct Visualizer (ksv)
Console-based hex viewer:

```bash
nix develop -c bash -c "ksv sprite-lfs.ksy sample.img"
```

---

## Recommended Workflow

1. **Edit** in VS Code with syntax highlighting
2. **Validate** using VS Code schema validation
3. **Test** in Kaitai Web IDE with sample data
4. **Debug** using Web IDE hex viewer
5. **Generate** parsers with `ksc`

---

## Troubleshooting

### YAML Still One Color?

1. Check file association:
   - Bottom-right corner should say "YAML"
   - If not, click and select "YAML"

2. Reload VS Code:
   - `Ctrl+Shift+P` → "Reload Window"

3. Check extension is installed:
   - `Ctrl+Shift+X` → Search "YAML"
   - Should see "Red Hat YAML" installed

4. Verify settings:
   - Open `.vscode/settings.json`
   - Should have `"*.ksy": "yaml"` in `files.associations`

### Schema Not Working?

1. Check schema path:
   ```yaml
   # First line of .ksy file should be:
   # $schema=../../schema/kaitai.schema.json
   ```

2. Verify schema exists:
   ```bash
   ls -lh schema/kaitai.schema.json
   ```

3. YAML extension settings:
   - Ensure `yaml.validate: true` in settings

---

## Links

- **Kaitai Struct Documentation**: https://doc.kaitai.io/
- **Web IDE**: https://ide.kaitai.io/
- **Format Gallery**: https://formats.kaitai.io/
- **GitHub**: https://github.com/kaitai-io/kaitai_struct

