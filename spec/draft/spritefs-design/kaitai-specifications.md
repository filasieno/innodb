# SpriteFS Record Format Specifications

## Kaitai Struct Definitions

**Version:** 1.4
**Date:** November 17, 2025

---

## Common Types

```yaml
meta:
  id: spritefs_record
  title: SpriteFS Log Record Format
  endian: le
  file-extension: [fwd, bwd, hist]

types:
  header:
    seq:
      - id: magic
        contents: [0x53]  # 'S'
      - id: record_type
        type: u1
      - id: flags
        type: u2
        doc:
          bit 0: compressed (zstd)
          bit 1: incremental_chain
          bit 2: has_snapshot
          bit 3: is_encrypted
          bits 4-15: reserved
      - id: body_len
        type: u4
      - id: timestamp_ns
        type: u8
      - id: prev_lsn
        type: u8
        doc: LSN in same stream, 0 for first record

  blake3_hash:
    seq:
      - id: bytes
        size: 32
    doc: BLAKE3-256 content address

  varint:
    seq:
      - id: bytes
        type: u1
        repeat: until
        repeat-until: "(_ & 0x80) == 0"
    instances:
      value:
        value: >-
          bytes.inject(0) { |acc, b| (acc << 7) | (b & 0x7F) }

  var_str:
    seq:
      - id: len
        type: vlq_base128_le
      - id: data
        type: str
        size: len
        encoding: UTF-8

  person:
    seq:
      - id: name
        type: var_str
      - id: email
        type: var_str
      - id: timestamp_ns
        type: u8
```

---

## History Stream Records

```yaml
types:
  commit_record:
    seq:
      - id: parent_count
        type: u1
      - id: parents
        type: blake3_hash
        repeat: expr
        repeat-expr: parent_count
      - id: tree_hash
        type: blake3_hash
      - id: author
        type: person
      - id: committer
        type: person
      - id: message
        type: var_str
      - id: extra_metadata_len
        type: u4
      - id: extra_metadata
        size: extra_metadata_len

  tag_record:
    seq:
      - id: object_hash
        type: blake3_hash
      - id: tagger
        type: person
      - id: tag_name
        type: var_str
      - id: message
        type: var_str
      - id: target_type
        type: u1
        enum: object_type

  branch_create:
    seq:
      - id: branch_name
        type: var_str
      - id: target_commit
        type: blake3_hash

  branch_update:
    seq:
      - id: branch_name
        type: var_str
      - id: old_commit
        type: blake3_hash
      - id: new_commit
        type: blake3_hash

enums:
  object_type:
    1: commit
    2: tree
    3: blob
    4: tag
```

---

## Forward Stream Records

```yaml
types:
  begin_tx:
    seq:
      - id: tx_id
        type: u8

  create_object:
    seq:
      - id: object_hash
        type: blake3_hash
      - id: object_type
        type: u1
        enum: object_type
      - id: content
        type: var_str

  incremental_delta:
    seq:
      - id: blob_hash
        type: blake3_hash
      - id: base_hash
        type: blake3_hash
      - id: change_count
        type: varint
      - id: changes
        type: change_event
        repeat: expr
        repeat-expr: change_count.value
      - id: is_snapshot
        type: u1
        if: (../hdr.flags & 4) != 0

  change_event:
    doc: Exact VS Code TextDocumentContentChangeEvent format
    seq:
      - id: range_start_line
        type: u4
      - id: range_start_character
        type: u4
      - id: range_end_line
        type: u4
      - id: range_end_character
        type: u4
      - id: text_len
        type: varint
      - id: text
        type: str
        size: text_len.value
        encoding: UTF-8

  commit_tx:
    seq:
      - id: tx_id
        type: u8
      - id: commit_hash
        type: blake3_hash

  create_tree:
    seq:
      - id: new_tree_hash
        type: blake3_hash

  tree_add_entry:
    seq:
      - id: tree_hash
        type: blake3_hash
      - id: name
        type: var_str
      - id: mode
        type: u4
        doc: POSIX-like mode (040000 dir, 100644 file, 120000 symlink)
      - id: object_hash
        type: blake3_hash

  tree_remove_entry:
    seq:
      - id: tree_hash
        type: blake3_hash
      - id: name
        type: var_str

  tree_rename_entry:
    seq:
      - id: tree_hash
        type: blake3_hash
      - id: old_name
        type: var_str
      - id: new_name
        type: var_str

  create_symlink:
    seq:
      - id: symlink_hash
        type: blake3_hash
      - id: target_path
        type: var_str

  range_lock_acquire:
    seq:
      - id: blob_hash
        type: blake3_hash
      - id: start_line
        type: u4
      - id: end_line
        type: u4
      - id: lock_mode
        type: u1
        enum: lock_mode
      - id: owner_id
        type: var_str
      - id: timeout_ms
        type: u4

enums:
  lock_mode:
    1: shared
    2: exclusive
```

---

## Backward Stream Records

```yaml
types:
  undo_incremental_delta:
    seq:
      - id: blob_hash
        type: blake3_hash
      - id: inverse_count
        type: varint
      - id: inverse_changes
        type: change_event
        repeat: expr
        repeat-expr: inverse_count.value

  compensation_clr:
    seq:
      - id: clr_lsn
        type: u8
      - id: undo_next_lsn
        type: u8
      - id: redo_payload_len
        type: u4
      - id: redo_payload
        size: redo_payload_len

  undo_tree_add:
    seq:
      - id: tree_hash
        type: blake3_hash
      - id: name
        type: var_str

  undo_tree_remove:
    seq:
      - id: tree_hash
        type: blake3_hash
      - id: name
        type: var_str
      - id: mode
        type: u4
      - id: object_hash
        type: blake3_hash

  undo_tree_rename:
    seq:
      - id: tree_hash
        type: blake3_hash
      - id: new_name
        type: var_str
      - id: old_name
        type: var_str

  range_lock_release:
    seq:
      - id: blob_hash
        type: blake3_hash
      - id: start_line
        type: u4
      - id: end_line
        type: u4
      - id: owner_id
        type: var_str
```

---

## Root Sequence

```yaml
seq:
  - id: records
    type: record
    repeat: eos

types:
  record:
    seq:
      - id: hdr
        type: header
      - id: payload
        size: hdr.body_len
        type:
          switch-on: hdr.record_type
          cases:
            # History Stream (1-20)
            1: commit_record
            2: tag_record
            3: branch_create
            4: branch_update
            5: ref_delete
            6: merge_record
            7: rebase_record
            8: cherry_pick_record
            9: revert_record
            10: stash_create
            11: stash_apply

            # Forward Stream (10-49)
            10: begin_tx
            11: create_object
            12: incremental_delta
            13: commit_tx
            14: create_tree
            15: tree_add_entry
            16: tree_remove_entry
            17: tree_rename_entry
            18: create_symlink
            25: range_lock_acquire

            # Backward Stream (50-99)
            50: undo_incremental_delta
            51: compensation_clr
            52: undo_tree_add
            53: undo_tree_remove
            54: undo_tree_rename
            56: range_lock_release
```

---

## Stream File Organization

### Forward Stream (forward.stream)

- Contains redo operations for crash recovery
- Sequential append for normal operation
- Random access during recovery phase

### Backward Stream (backward.stream)

- Contains undo operations for transaction rollback
- Sequential append during normal operation
- Sequential scan during undo phase

### History Stream (history.stream)

- Contains version control metadata
- Indexed for fast history queries
- Triple-mirrored for redundancy

### Optional Streams

- **Snapshots**: Periodic full state snapshots for faster recovery
- **Indexes**: Rebuildable secondary indexes for performance

---

## Compression and Encryption

### Compression

- **Algorithm**: Zstd with trained dictionaries per record type
- **Dictionary Training**: Based on representative workload samples
- **Flag**: bit 0 in record header flags

### Encryption (Future)

- **Algorithm**: AES-256-GCM
- **Key Management**: Repository-specific keys
- **Flag**: bit 3 in record header flags

---

## Extensibility

### Versioning

- **Format Version**: Encoded in magic number extensions
- **Forward Compatibility**: Unknown record types are preserved
- **Backward Compatibility**: Graceful degradation for older formats

### Custom Record Types

- **Range**: 100-255 available for extensions
- **Registration**: Runtime type registration system
- **Validation**: Schema-based validation for custom types
