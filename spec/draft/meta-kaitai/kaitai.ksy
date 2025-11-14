# $schema=../../schema/kaitai.schema.json

meta:
  id: ksm1
  title: Kaitai Source-level Metamodel (KSM1)
  application: Kaitai Struct compiler
  file-extension: ksm1
  endian: le
  doc: |
    KSM1 is a binary format that models the full source-level Kaitai Struct metamodel.
    It captures concepts like types, attributes, expressions, enums, etc., preserving declaration order.
    This specification is self-documenting with embedded user guide excerpts.
    See https://doc.kaitai.io/user_guide.html for the full guide.
  doc-ref: https://doc.kaitai.io/user_guide.html

seq:
  - id: magic
    contents: "KSM1"
    doc: |
      Magic bytes "KSM1" (Kaitai Source-level Metamodel 1).
      From section 1: Introduction - Kaitai Struct is a domain-specific language designed for binary formats.
  - id: version_major
    type: u2
    doc: Major version number. Must be validated for compatibility.
  - id: version_minor
    type: u2
    doc: Minor version number. Forward-compatible within major version.
  - id: flags
    type: u4
    doc: Global flags bitset including default endianness and bit-endianness.
  - id: seg_count
    type: u4
    doc: Number of segments in the file.
  - id: seg_entries
    type: seg_entry
    repeat: expr
    repeat-expr: seg_count
    doc: Array of segment directory entries, defining segment types and positions.

instances:
  segments:
    type: segment
    repeat: expr
    repeat-expr: seg_count
    doc: Parsed segments, each positioned according to directory.

types:
  seg_entry:
    seq:
      - id: seg_type
        type: u1
        enum: seg_type_enum
        doc: Type of the segment (SPEC, DOCS, etc.).
      - id: offset
        type: u8
        doc: Absolute offset of the segment in the file.
      - id: size
        type: u8
        doc: Size of the segment in bytes.
      - id: reserved
        type: u4
        doc: Reserved for future flags.

  segment:
    seq:
      - id: body
        type:
          switch-on: _parent.seg_entries[_index].seg_type
          cases:
            seg_type_enum::spec: spec_segment
            seg_type_enum::docs: docs_segment
            seg_type_enum::symbols: symbols_segment
            seg_type_enum::symtab: symtab_segment
            seg_type_enum::extra: extra_segment
        size: _parent.seg_entries[_index].size
        doc: Segment body, sized substream for validation.

enums:
  seg_type_enum:
    0: spec
    1: docs
    2: symbols
    3: symtab
    4: extra

types:
  # ULEB128 variable-length unsigned integer
  uleb128:
    seq:
      - id: groups
        type: u1
        repeat: until
        repeat-until: (_ & 0x80) == 0
    instances:
      value:
        value: >-
          groups[0] & 0x7F |
          ((groups[1] & 0x7F) << 7 if groups.size > 1 else 0) |
          ((groups[2] & 0x7F) << 14 if groups.size > 2 else 0) |
          ((groups[3] & 0x7F) << 21 if groups.size > 3 else 0) |
          ((groups[4] & 0x7F) << 28 if groups.size > 4 else 0) |
          ((groups[5] & 0x7F) << 35 if groups.size > 5 else 0) |
          ((groups[6] & 0x7F) << 42 if groups.size > 6 else 0) |
          ((groups[7] & 0x7F) << 49 if groups.size > 7 else 0) |
          ((groups[8] & 0x7F) << 56 if groups.size > 8 else 0)
        doc: Decoded ULEB128 value

  # SPEC segment
  spec_segment:
    seq:
      - id: strings_count
        type: uleb128
        doc: Number of strings in the table
      - id: strings
        type: string_entry
        repeat: expr
        repeat-expr: strings_count.value
        doc: Array of UTF-8 strings
      - id: identifiers_count
        type: uleb128
        doc: Number of identifiers
      - id: identifiers
        type: uleb128
        repeat: expr
        repeat-expr: identifiers_count.value
        doc: Array of string indices for identifiers
      - id: meta_record
        type: spec_meta
        doc: Meta information
      - id: expressions_count
        type: uleb128
        doc: Number of expression AST nodes
      - id: expressions
        type: expr_node
        repeat: expr
        repeat-expr: expressions_count.value
        doc: Expression AST pool
      - id: enums_count
        type: uleb128
        doc: Number of enums
      - id: enums
        type: spec_enum
        repeat: expr
        repeat-expr: enums_count.value
        doc: Array of enums
      - id: types_count
        type: uleb128
        doc: Number of types
      - id: types
        type: spec_type
        repeat: expr
        repeat-expr: types_count.value
        doc: Array of type specs

  expr_node:
    seq:
      - id: tag
        type: u1
        enum: expr_tag
        doc: Expression node type tag
      - id: payload
        type:
          switch-on: tag
          cases:
            'expr_tag::int_literal': expr_int_literal
            'expr_tag::uint_literal': expr_uint_literal
            'expr_tag::bool_literal': expr_bool_literal
            'expr_tag::str_literal': expr_str_literal
            'expr_tag::bytes_literal': expr_bytes_literal
            'expr_tag::null_literal': expr_null_literal
            'expr_tag::name': expr_name
            'expr_tag::special_name': expr_special_name
            'expr_tag::unary_op': expr_unary_op
            'expr_tag::binary_op': expr_binary_op
            'expr_tag::ternary_op': expr_ternary_op
            'expr_tag::indexing': expr_indexing
            'expr_tag::field_access': expr_field_access
            'expr_tag::method_call': expr_method_call
            'expr_tag::switch_case_expr': expr_switch_case_expr
        doc: Tagged payload

  expr_int_literal:
    seq:
      - id: value
        type: s8
        doc: Signed integer literal value

  expr_uint_literal:
    seq:
      - id: value
        type: u8
        doc: Unsigned integer literal value

  expr_bool_literal:
    seq:
      - id: value
        type: u1
        doc: Boolean literal value

  expr_str_literal:
    seq:
      - id: str_idx
        type: uleb128
        doc: String index for literal

  expr_bytes_literal:
    seq:
      - id: len
        type: uleb128
        doc: Length of bytes
      - id: value
        size: len.value
        doc: Bytes literal value

  expr_null_literal:
    seq: []  # No payload

  expr_name:
    seq:
      - id: id_idx
        type: uleb128
        doc: Identifier index for name

  expr_special_name:
    seq:
      - id: name_type
        type: u1
        enum: special_name_type
        doc: Special name type

  expr_unary_op:
    seq:
      - id: op
        type: u1
        enum: unary_op_type
        doc: Unary operator
      - id: operand_idx
        type: uleb128
        doc: Operand expression index

  expr_binary_op:
    seq:
      - id: op
        type: u1
        enum: binary_op_type
        doc: Binary operator
      - id: left_idx
        type: uleb128
        doc: Left operand expression index
      - id: right_idx
        type: uleb128
        doc: Right operand expression index

  expr_ternary_op:
    seq:
      - id: cond_idx
        type: uleb128
        doc: Condition expression index
      - id: true_idx
        type: uleb128
        doc: True branch expression index
      - id: false_idx
        type: uleb128
        doc: False branch expression index

  expr_indexing:
    seq:
      - id: array_idx
        type: uleb128
        doc: Array expression index
      - id: index_idx
        type: uleb128
        doc: Index expression index

  expr_field_access:
    seq:
      - id: obj_idx
        type: uleb128
        doc: Object expression index
      - id: field_id_idx
        type: uleb128
        doc: Field identifier index

  expr_method_call:
    seq:
      - id: obj_idx
        type: uleb128
        doc: Object expression index
      - id: method_id_idx
        type: uleb128
        doc: Method identifier index
      - id: args_count
        type: uleb128
        doc: Number of arguments
      - id: args
        type: uleb128
        repeat: expr
        repeat-expr: args_count.value
        doc: Argument expression indices

  expr_switch_case_expr:
    seq:
      - id: switch_on_idx
        type: uleb128
        doc: Switch-on expression index
      - id: cases_count
        type: uleb128
        doc: Number of cases
      - id: cases
        type: expr_switch_case
        repeat: expr
        repeat-expr: cases_count.value
        doc: Switch cases

  expr_switch_case:
    seq:
      - id: key_expr_idx
        type: uleb128
        doc: Key expression index
      - id: value_expr_idx
        type: uleb128
        doc: Value expression index

  spec_enum:
    seq:
      - id: name_id_idx
        type: uleb128
        doc: Identifier index for enum name
      - id: members_count
        type: uleb128
        doc: Number of members
      - id: members
        type: enum_member
        repeat: expr
        repeat-expr: members_count.value
        doc: Enum members in declaration order

  enum_member:
    seq:
      - id: name_id_idx
        type: uleb128
        doc: Identifier index for member name
      - id: value
        type: s8
        doc: Member value (must be unique per enum)

  spec_type:
    seq:
      - id: name_id_idx
        type: uleb128
        doc: Identifier index for type name
      - id: params_count
        type: uleb128
        doc: Number of parameters
      - id: params
        type: type_param
        repeat: expr
        repeat-expr: params_count.value
        doc: Type parameters
      - id: seq_count
        type: uleb128
        doc: Number of seq attributes
      - id: seq
        type: attribute
        repeat: expr
        repeat-expr: seq_count.value
        doc: Sequential attributes
      - id: instances_count
        type: uleb128
        doc: Number of instances
      - id: instances
        type: instance
        repeat: expr
        repeat-expr: instances_count.value
        doc: Lazy instances
# TODO: Add nested types support
# - id: nested_types_count
#   type: uleb128
#   doc: Number of nested types
# - id: nested_types
#   type: spec_type
#   repeat: expr
#   repeat-expr: nested_types_count.value
#   doc: Nested type definitions
      - id: local_enums_count
        type: uleb128
        doc: Number of local enums
      - id: local_enums
        type: spec_enum
        repeat: expr
        repeat-expr: local_enums_count.value
        doc: Local enum definitions

  type_param:
    seq:
      - id: id_id_idx
        type: uleb128
        doc: Identifier index for param name
      - id: type_ref_present
        type: u1
        doc: Whether type_ref is present
      - id: type_ref
        type: type_ref
        if: type_ref_present == 1
        doc: Optional type reference
      - id: doc_idx
        type: uleb128
        doc: Doc index

  type_ref:
    seq:
      - id: kind
        type: u1
        doc: "0: named, 1: switch-on"
      - id: named_id_idx
        type: uleb128
        if: kind == 0
        doc: Named type identifier index
      - id: switch_on_expr_idx
        type: uleb128
        if: kind == 1
        doc: Switch expression index
      - id: cases_count
        type: uleb128
        if: kind == 1
        doc: Number of cases
      - id: cases
        type: switch_case
        repeat: expr
        repeat-expr: cases_count.value
        if: kind == 1
        doc: Switch cases

  switch_case:
    seq:
      - id: literal_expr_idx
        type: uleb128
        doc: Literal expression for case
      - id: type_ref
        type: type_ref
        doc: Type for this case

  attribute:
    seq:
      - id: id_id_idx
        type: uleb128
        doc: Identifier index for attribute id
      - id: doc_idx
        type: uleb128
        doc: Doc index
      - id: doc_ref_count
        type: uleb128
        doc: Number of doc-ref strings
      - id: doc_refs
        type: uleb128
        repeat: expr
        repeat-expr: doc_ref_count.value
        doc: Doc-ref string indices
      - id: contents_present
        type: u1
        doc: Whether contents is present
      - id: contents_type
        type: u1
        if: contents_present == 1
        doc: "Contents type: 0=str, 1=bytes, 2=int"
      - id: contents_str_idx
        type: uleb128
        if: contents_present == 1 and contents_type == 0
        doc: String index for contents
      - id: contents_bytes_len
        type: uleb128
        if: contents_present == 1 and contents_type == 1
        doc: Length of bytes
      - id: contents_bytes
        size: contents_bytes_len.value
        if: contents_present == 1 and contents_type == 1
        doc: Bytes contents
      - id: contents_int
        type: s8
        if: contents_present == 1 and contents_type == 2
        doc: Int contents
      - id: type_present
        type: u1
        doc: Whether type is present
      - id: type_ref
        type: type_ref
        if: type_present == 1
        doc: Type reference
      - id: repeat_type
        type: u1
        doc: "Repeat type: 0=none, 1=expr, 2=eos, 3=until"
      - id: repeat_expr_idx
        type: uleb128
        if: repeat_type == 1
        doc: Repeat expression index
      - id: repeat_until_idx
        type: uleb128
        if: repeat_type == 3
        doc: Repeat until expression index
      - id: if_present
        type: u1
        doc: Whether if is present
      - id: if_expr_idx
        type: uleb128
        if: if_present == 1
        doc: If expression index
      - id: size_present
        type: u1
        doc: Whether size is present
      - id: size_expr_idx
        type: uleb128
        if: size_present == 1
        doc: Size expression index
      - id: size_eos
        type: u1
        doc: Size-eos flag
      - id: process_present
        type: u1
        doc: Whether process is present
      - id: process_str_idx
        type: uleb128
        if: process_present == 1
        doc: Process string index
      - id: enum_present
        type: u1
        doc: Whether enum is present
      - id: enum_scope_id_idx
        type: uleb128
        if: enum_present == 1
        doc: Enum scope id index
      - id: enum_type_id_idx
        type: uleb128
        if: enum_present == 1
        doc: Enum type id index
      - id: encoding_present
        type: u1
        doc: Whether encoding is present
      - id: encoding_str_idx
        type: uleb128
        if: encoding_present == 1
        doc: Encoding string index
      - id: pad_right
        type: uleb128
        doc: Pad-right value
      - id: terminator
        type: uleb128
        doc: Terminator value
      - id: consume
        type: u1
        doc: Consume flag
      - id: include
        type: u1
        doc: Include flag
      - id: eos_error
        type: u1
        doc: Eos-error flag
      - id: pos_present
        type: u1
        doc: Whether pos is present
      - id: pos_expr_idx
        type: uleb128
        if: pos_present == 1
        doc: Pos expression index
      - id: io_present
        type: u1
        doc: Whether io is present
      - id: io_expr_idx
        type: uleb128
        if: io_present == 1
        doc: Io expression index
      - id: value_present
        type: u1
        doc: Whether value is present
      - id: value_expr_idx
        type: uleb128
        if: value_present == 1
        doc: Value expression index

  instance:
    seq:
      - id: id_id_idx
        type: uleb128
        doc: Identifier index for instance id
      - id: doc_idx
        type: uleb128
        doc: Doc index
      - id: doc_ref_count
        type: uleb128
        doc: Number of doc-ref strings
      - id: doc_refs
        type: uleb128
        repeat: expr
        repeat-expr: doc_ref_count.value
        doc: Doc-ref string indices
      - id: pos_present
        type: u1
        doc: Whether pos is present
      - id: pos_expr_idx
        type: uleb128
        if: pos_present == 1
        doc: Pos expression index
      - id: io_present
        type: u1
        doc: Whether io is present
      - id: io_expr_idx
        type: uleb128
        if: io_present == 1
        doc: Io expression index
      - id: value_present
        type: u1
        doc: Whether value is present
      - id: value_expr_idx
        type: uleb128
        if: value_present == 1
        doc: Value expression index
      - id: type_present
        type: u1
        doc: Whether type is present
      - id: type_ref
        type: type_ref
        if: type_present == 1
        doc: Type reference
      - id: size_present
        type: u1
        doc: Whether size is present
      - id: size_expr_idx
        type: uleb128
        if: size_present == 1
        doc: Size expression index
      - id: size_eos
        type: u1
        doc: Size-eos flag
      - id: process_present
        type: u1
        doc: Whether process is present
      - id: process_str_idx
        type: uleb128
        if: process_present == 1
        doc: Process string index
      - id: enum_present
        type: u1
        doc: Whether enum is present
      - id: enum_scope_id_idx
        type: uleb128
        if: enum_present == 1
        doc: Enum scope id index
      - id: enum_type_id_idx
        type: uleb128
        if: enum_present == 1
        doc: Enum type id index
      - id: encoding_present
        type: u1
        doc: Whether encoding is present
      - id: encoding_str_idx
        type: uleb128
        if: encoding_present == 1
        doc: Encoding string index
      - id: if_present
        type: u1
        doc: Whether if is present
      - id: if_expr_idx
        type: uleb128
        if: if_present == 1
        doc: If expression index
      - id: repeat_type
        type: u1
        doc: "Repeat type: 0=none, 1=expr, 2=eos, 3=until"
      - id: repeat_expr_idx
        type: uleb128
        if: repeat_type == 1
        doc: Repeat expression index
      - id: repeat_until_idx
        type: uleb128
        if: repeat_type == 3
        doc: Repeat until expression index

  string_entry:
    seq:
      - id: len
        type: uleb128
        doc: Length of string in bytes
      - id: payload
        type: str
        size: len.value
        encoding: utf-8
        doc: UTF-8 string content (expected NFC)

  spec_meta:
    seq:
      - id: id_str_idx
        type: uleb128
        doc: String index for id
      - id: title_str_idx
        type: uleb128
        doc: String index for title
      - id: application_count
        type: uleb128
        doc: Number of applications
      - id: application_str_indices
        type: uleb128
        repeat: expr
        repeat-expr: application_count.value
        doc: String indices for applications
      - id: file_extension_count
        type: uleb128
        doc: Number of file extensions
      - id: file_extension_str_indices
        type: uleb128
        repeat: expr
        repeat-expr: file_extension_count.value
        doc: String indices for extensions
      - id: license_str_idx
        type: uleb128
        doc: String index for license
      - id: ks_version_major
        type: uleb128
        doc: KS version major
      - id: ks_version_minor
        type: uleb128
        doc: KS version minor
      - id: ks_debug
        type: u1
        doc: KS debug flag
      - id: ks_opaque_types
        type: u1
        doc: KS opaque types flag
      - id: imports_count
        type: uleb128
        doc: Number of imports
      - id: imports_str_indices
        type: uleb128
        repeat: expr
        repeat-expr: imports_count.value
        doc: String indices for imports
      - id: encoding_str_idx
        type: uleb128
        doc: String index for encoding
      - id: endian
        type: u1
        enum: endian_enum
        doc: Default endianness
      - id: bit_endian
        type: u1
        enum: bit_endian_enum
        doc: Bit endianness
      - id: xref_count
        type: uleb128
        doc: Number of xref entries
      - id: xrefs
        type: xref_entry
        repeat: expr
        repeat-expr: xref_count.value
        doc: Extensible xref entries

  xref_entry:
    seq:
      - id: key_str_idx
        type: uleb128
        doc: Key string index
      - id: value_str_idx
        type: uleb128
        doc: Value string index


  expr_tag:
    0: int_literal
    1: uint_literal
    2: bool_literal
    3: str_literal
    4: bytes_literal
    5: null_literal
    6: name
    7: special_name
    8: unary_op
    9: binary_op
    10: ternary_op
    11: indexing
    12: field_access
    13: method_call
    14: switch_case_expr

  unary_op_type:
    0: plus
    1: minus
    2: bitwise_not
    3: logical_not

  binary_op_type:
    0: add
    1: sub
    2: mul
    3: div
    4: mod
    5: shift_left
    6: shift_right
    7: bitwise_and
    8: bitwise_or
    9: bitwise_xor
    10: logical_and
    11: logical_or
    12: eq
    13: ne
    14: lt
    15: le
    16: gt
    17: ge

  special_name_type:
    0: root
    1: parent
    2: io
    3: index

  scope_kind:
    0: root
    1: type
    2: enum

  symbol_kind:
    0: type
    1: param
    2: attribute
    3: instance
    4: enum
    5: enum_member
    6: method_builtin
    7: import_namespace

types:
  # DOCS segment
  docs_segment:
    seq:
      - id: doc_texts_count
        type: uleb128
        doc: Number of doc text entries
      - id: doc_texts
        type: doc_text_entry
        repeat: expr
        repeat-expr: doc_texts_count.value
        doc: Array of large doc text blobs
      - id: doc_map_count
        type: uleb128
        doc: Number of doc map entries
      - id: doc_map
        type: doc_map_entry
        repeat: expr
        repeat-expr: doc_map_count.value
        doc: Mapping constructs to doc indices
      - id: assets_count
        type: uleb128
        doc: Number of asset entries
      - id: assets
        type: asset_entry
        repeat: expr
        repeat-expr: assets_count.value
        doc: Binary assets (images, etc.)

  doc_text_entry:
    seq:
      - id: len
        type: uleb128
        doc: Length of doc text
      - id: payload
        type: str
        size: len.value
        encoding: utf-8
        doc: UTF-8 doc text (e.g., guide excerpts)

  doc_map_entry:
    seq:
      - id: construct_path_str_idx
        type: uleb128
        doc: String index for construct path (e.g., 'seq.attribute')
      - id: doc_indices_count
        type: uleb128
        doc: Number of doc text indices
      - id: doc_indices
        type: uleb128
        repeat: expr
        repeat-expr: doc_indices_count.value
        doc: Indices into doc_texts

  asset_entry:
    seq:
      - id: filename_str_idx
        type: uleb128
        doc: String index for filename
      - id: mime_str_idx
        type: uleb128
        doc: String index for MIME type
      - id: size
        type: uleb128
        doc: Size of asset payload
      - id: payload
        size: size.value
        doc: Binary asset data

  # SYMBOLS segment
  symbols_segment:
    seq:
      - id: scopes_count
        type: uleb128
        doc: Number of scope records
      - id: scopes
        type: scope_record
        repeat: expr
        repeat-expr: scopes_count.value
        doc: Scope hierarchy
      - id: symbols_count
        type: uleb128
        doc: Number of symbol records
      - id: symbols
        type: symbol_record
        repeat: expr
        repeat-expr: symbols_count.value
        doc: Symbol definitions and targets

  scope_record:
    seq:
      - id: parent_idx
        type: uleb128
        doc: Parent scope index (0 for root)
      - id: name_id_idx
        type: uleb128
        doc: Identifier index for scope name
      - id: kind
        type: u1
        enum: scope_kind
        doc: Scope kind (root, type, etc.)

  symbol_record:
    seq:
      - id: name_id_idx
        type: uleb128
        doc: Identifier index for symbol name
      - id: kind
        type: u1
        enum: symbol_kind
        doc: Symbol kind
      - id: scope_idx
        type: uleb128
        doc: Scope index
      - id: target_seg
        type: u1
        enum: seg_type_enum
        doc: Target segment
      - id: target_table
        type: uleb128
        doc: Target table kind
      - id: target_row
        type: uleb128
        doc: Target row index

  # SYMTAB segment
  symtab_segment:
    seq:
      - id: name_index_count
        type: uleb128
        doc: Number of name index entries
      - id: name_index
        type: name_index_entry
        repeat: expr
        repeat-expr: name_index_count.value
        doc: Name-to-symbols mapping
      - id: qual_index_count
        type: uleb128
        doc: Number of qualified path entries
      - id: qual_index
        type: qual_index_entry
        repeat: expr
        repeat-expr: qual_index_count.value
        doc: Qualified path to symbol mapping

  name_index_entry:
    seq:
      - id: name_id_idx
        type: uleb128
        doc: Identifier index
      - id: symbol_indices_count
        type: uleb128
        doc: Number of symbol indices
      - id: symbol_indices
        type: uleb128
        repeat: expr
        repeat-expr: symbol_indices_count.value
        doc: Indices into symbols table

  qual_index_entry:
    seq:
      - id: path_count
        type: uleb128
        doc: Number of path components
      - id: path
        type: uleb128
        repeat: expr
        repeat-expr: path_count.value
        doc: Identifier indices for qualified path
      - id: symbol_idx
        type: uleb128
        doc: Symbol index

  # EXTRA segment
  extra_segment:
    seq:
      - id: entries
        type: tlv_entry
        repeat: eos
        doc: Extensible TLV entries for forward compatibility

  tlv_entry:
    seq:
      - id: tag
        type: uleb128
        doc: TLV tag
      - id: len
        type: uleb128
        doc: Length of value
      - id: value
        size: len.value
        doc: TLV value bytes
