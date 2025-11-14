# KSM1: Kaitai Source-level Metamodel (Binary Format)

KSM1 is a forward-compatible binary container format that encodes the full source-level Kaitai Struct metamodel. It preserves the concepts and structures that a modeller uses when writing `.ksy` files, enabling validation, transformation, and reimplementation of Kaitai parsers/compilers.

## Overview

KSM1 files consist of a header followed by contiguous, non-overlapping segments. Each segment is a self-contained table of records, designed for efficient parsing and strong validation using Kaitai Struct features (enums, `valid`, sized substreams, switch-on unions).

### File Layout

- **Header** (24 bytes)
  - Magic: `KSM1` (4 bytes)
  - Version: major (2), minor (2)
  - Flags: bitset (4) - default endianness, bit-endianness, features
  - Segment count: uleb128
  - Segment entries: array of {type (1), offset (8), size (8), reserved (4)}

- **Segments** (contiguous, in order)
  - SPEC: Core metamodel structures
  - DOCS: Documentation texts and assets
  - SYMBOLS: Symbol definitions and scopes
  - SYMTAB: Symbol lookup indices
  - EXTRA: Extensible TLV for future fields

## Segments

### SPEC Segment

Encodes the metamodel's structural elements.

- Strings table: UTF-8 NFC strings with lengths
- Identifiers table: indices into strings, validated charset `[a-z][a-z0-9_]*`
- Meta: file-level metadata (id, title, apps, exts, license, ks-version, debug, opaque, imports, encoding, endian, bit-endian, xref)
- Expressions: AST pool for all expressions in the spec
- Enums: global enums with members (order preserved, uniqueness validated)
- Types: type specifications (recursive, with params, seq, instances, nested types, local enums)

### DOCS Segment

Holds large documentation and assets.

- Doc texts: UTF-8 NFC text blobs
- Doc map: construct → doc text indices
- Assets: binary files (images, etc.) with MIME and filenames

### SYMBOLS Segment

Manages symbol resolution.

- Scopes: hierarchy (root, types, nested)
- Symbols: definitions with kinds (type, param, attr, instance, enum, etc.), scope, target location

### SYMTAB Segment

Fast lookup tables.

- Name index: name → symbol list
- Qualified path index: path → symbol

### EXTRA Segment

TLV-encoded extensible fields for forward compatibility.

## Mapping to KSY Concepts

KSM1 directly models the source-level language:

- **Meta**: Mirrors `.ksy` top-level meta fields
- **Types**: Recursive type specs with params, seq attributes, instances, enums
- **Attributes**: Full field definitions with all options (type, repeat, size, process, enum, encoding, etc.)
- **Instances**: Lazy-evaluated fields
- **Enums**: Named integer constants with scoped references
- **Expressions**: Complete AST for all computations, validations, and references
- **Symbols/Scopes**: Resolution of names, imports, and namespaces

## Validation Features

KSM1 uses Kaitai's declarative validation extensively:

- Enum constraints on types, operators, etc.
- Index bounds checking across tables
- Mutual exclusivity (value vs type vs contents)
- Conditional invariants (repeat requires expr/until; process requires size)
- UTF-8 and charset validation
- Substream sizing for robustness

## Usage

Parse KSM1 files with the provided `kaitai.ksy` spec. Generate parsers in any Kaitai-supported language to read, validate, or transform Kaitai metamodels.

## Examples

See `samples/` for minimal, typical, and edge-case KSM1 files.

## References

- [Kaitai Struct User Guide](https://doc.kaitai.io/user_guide.html) - embedded and mirrored
- `schema/kaitai.schema.json` - JSON schema for comparison
- Kaitai compiler internals - for semantic alignment
