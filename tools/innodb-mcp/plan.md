# Innodb refactoring mcp

We shall create refacting MVP tool.

The key point is symbolic analisys of the source code.

## Symbolic analysis

The first objective is to collect all symbols.

We shall design the database use a graph approach.

Nodes and links.

First we shall describe the key nodes.


```text

node MOD_MODULE:
  - module_name: String (PK) # "buf", "srv"

node TYB_BASE_TYPE:
  - tyb_name: String (PK) 
  - tyb_kind: TY_BASE_TYPE_KIND

enum TYB_BASE_TYPE_KIND:
   1: FUNCTION   
   2: STRUCT
   3: ALIAS (of typedef)
   4: ENUM
   5: PRIMITIVE
   6: UNION

node TY_TYPE:
  - ty_id: UUID (PK)
  - ty_base_id: UUID (FK to TY_BASE_TYPE) # the base type (int, struct foo, etc)
  - tyc_pointer_depth: INT # 0=value, 1=*, 2=**, etc.
  - tyc_is_const: Boolean # const qualifier
  - tyc_is_volatile: Boolean # volatile qualifier
  - tyc_is_restrict: Boolean # restrict qualifier (C99)
  - tyc_array_dimensions: JSON # null or [10, 20] for int arr[10][20]
  - tyc_is_static_array: Boolean # for static arrays in function params
  - tyc_signature: String # computed: "const int**", "char[256]", etc.
  # This enables: int*, const char**, void*, struct foo[10], etc.

node TYSF_STRUCT_FIELD:
  - tysf_id: UUID (random UUID, PK)
  - ty_struct_id: UUID (FK to TY_BASE_TYPE - parent struct)
  - tyc_field_type_id: UUID (FK to TY_COMPLETE_TYPE)
  - tysf_name: String (field name)
  - tysf_index: INT # field order in struct
  - tysf_bit_field_width: INT # for bit fields, null otherwise

node TYEF_ENUM_FIELD:
  - tyef_id: UUID (random UUID, PK)
  - ty_enum_id: UUID (FK to TY_BASE_TYPE - parent enum)
  - tyef_name: String (enum field name)
  - tyef_value: INT

node GC_GLOBAL_CONST:
  - gc_id: UUID (random UUID, pk)
  - module_id: UUID (FK to MODULE)
  - gc_name: String 
  - tyc_type_id: UUID (FK to TY_COMPLETE_TYPE)
  UNIQUE(module_id, gc_name)

node GV_GLOBAL_VARIABLE:
  - gv_id: UUID (random UUID, pk)
  - module_id: UUID (FK to MODULE)
  - gv_name: String
  - tyc_type_id: UUID (FK to TY_COMPLETE_TYPE)
  UNIQUE(module_id, gv_name)

node RT_ROUTINE: 
  - rt_id: UUID (random UUID, pk)
  - module_id: UUID (FK to MODULE)
  - rt_name: String
  - rt_is_static: Boolean # static/internal linkage
  - rt_is_inline: Boolean
  - rt_is_variadic: Boolean # for functions with ... (varargs)
  UNIQUE(module_id, rt_name)

node RT_RETURN_TYPE:
  - rtr_id: UUID (PK)
  - rt_id: UUID (FK to RT_ROUTINE, UNIQUE)
  - tyc_type_id: UUID (FK to TY_COMPLETE_TYPE, nullable for void)

node RTP_ROUTINE_PARAMETER: 
  - rtp_id: UUID (PK)
  - rt_id: UUID (FK to RT_ROUTINE)
  - rtp_name: String
  - tyc_type_id: UUID (FK to TY_COMPLETE_TYPE)
  - rtp_index: INT # 0=first, 1=second, etc.
  - rtp_is_output: Boolean # for output parameters

node IMPL_ROUTINE_IMPLEMENTATION: 
  - impl_id: UUID (PK)
  - rt_id: UUID (FK to RT_ROUTINE)
  - version: INT # Starts with 0
  - impl_code: String # full source code
  - impl_file: URI
  - impl_line_begin: INT
  - impl_col_begin: INT
  - impl_line_end: INT
  - impl_col_end: INT

node MACRO_SIMPLE:
  - macro_id: UUID (PK)
  - module_id: UUID (FK to MODULE)
  - macro_name: String
  - macro_value: String # the replacement text
  - macro_file: URI
  - macro_line: INT
  UNIQUE(module_id, macro_name)

node MACRO_FUNCTION:
  - macfn_id: UUID (PK)
  - module_id: UUID (FK to MODULE)
  - macfn_name: String
  - macfn_parameters: JSON # ["a", "b", "c"] parameter names
  - macfn_body: String # the replacement text with parameters
  - macfn_file: URI
  - macfn_line: INT
  UNIQUE(module_id, macfn_name)

node DOC_DOCUMENTATION:
  - doc_id: UUID (PK)
  - doc_entity_type: ROUTINE | PARAMETER | RETURN_TYPE | TYPE | FIELD | MODULE
  - doc_entity_id: UUID # FK to the entity being documented
  - doc_brief: String # brief description (one line)
  - doc_details: Text # detailed description
  - doc_author: String # author name
  - doc_date: String # date (ISO 8601 or free form)
  - doc_notes: Text # additional notes
  - doc_see_also: JSON # ["other_function", "related_struct"]
  - doc_params: JSON # for routines: {param_name: "description", ...}
  - doc_returns: String # for routines: description of return value
```