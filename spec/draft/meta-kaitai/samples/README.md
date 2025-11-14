# KSM1 Sample Files

This directory contains example KSM1 binary files demonstrating the format.

## minimal.ksm1

Minimal valid KSM1 file with only header and empty SPEC segment.

- Magic: KSM1
- Version: 1.0
- Flags: 0 (LE, LE bit-endian)
- Segments: 1 (SPEC at offset 25, size 0)

Hex dump:
```
4B534D31 01000000 00000000 01 00 1900000000000000 00000000
```

## typical.ksm1

Typical KSM1 with a simple type spec.

Includes:
- Strings: "my_type", "len", "data"
- Identifiers: 0,1,2
- Meta: id="example", ks-version=0.11
- Types: one type with seq attribute

## edge_case.ksm1

Edge cases for validation:
- Invalid identifier charset
- Out-of-bounds indices
- Conflicting attribute fields
- Recursive type reference

Use the KSY parser to validate these files.
