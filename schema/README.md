# JSON Schemas

This directory contains JSON Schema definitions for validation and autocomplete in the project.

## Schemas

### `kaitai.schema.json`

- **Purpose**: Validates Kaitai Struct `.ksy` files
- **Source**: https://github.com/kaitai-io/kaitai_struct_compiler
- **Applies to**: `**/*.ksy`, `**/*.ksy.yaml`
- **Documentation**: https://doc.kaitai.io/

Kaitai Struct is a declarative language used to describe binary data structures. This schema ensures that `.ksy` files are valid and provides IDE autocomplete support.

### `lsp-metamodel.schema.json`

- **Purpose**: Validates LSP metamodel JSON files
- **Source**: Project-specific (tools/ib-lsp-model/)
- **Applies to**: `tools/ib-lsp-model/metaModel.json`

The LSP metamodel schema defines the structure for Language Server Protocol feature definitions used in this project.

## Usage

Schemas are automatically applied via `.vscode/settings.json`:

```json
{
  "yaml.schemas": {
    "./schema/kaitai.schema.json": ["**/*.ksy", "**/*.ksy.yaml"]
  },
  "json.schemas": [
    {
      "fileMatch": ["**/tools/ib-lsp-model/metaModel.json"],
      "url": "./schema/lsp-metamodel.schema.json"
    }
  ]
}
```

### In-file Schema Reference

You can also reference schemas directly in files:

**YAML files:**

```yaml
# yaml-language-server: $schema=../../schema/kaitai.schema.json
```

**JSON files:**
```json
{
  "$schema": "../../schema/lsp-metamodel.schema.json"
}
```

## Updating Schemas

### Kaitai Schema
To update the Kaitai Struct schema:

```bash
nix develop -c bash -c "curl -L 'https://github.com/kaitai-io/kaitai_struct_compiler/raw/master/shared/src/main/resources/ksy_schema.json' -o schema/kaitai.schema.json"
```

### LSP Metamodel Schema
This is project-specific. Update the source in `tools/ib-lsp-model/metaModel.schema.json`, then:

```bash
cp tools/ib-lsp-model/metaModel.schema.json schema/lsp-metamodel.schema.json
```

## Benefits

- ✅ **Real-time validation**: Catch errors as you type
- ✅ **Autocomplete**: IDE suggests valid keys and values
- ✅ **Documentation**: Hover tooltips with field descriptions
- ✅ **Type checking**: Ensures correct value types
- ✅ **Consistency**: Enforces project standards

