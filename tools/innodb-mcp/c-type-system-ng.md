# C Type System - Unified Design for Global Analysis

## Overview

This schema models C types in a global, single-file view (all forward declarations resolved upfront). It supports composition via hierarchical references (name_id to TYN_TYPE_NAME for symbolic linking), unified qualifiers, functions (base + pointer), arrays (static/VLA/flexible), structs/unions/enums with bitfields, and C11 features like _Atomic. Layouts (sizes/offsets) are computed from primitives and fields. No scoping or incompletes—assumes full resolution, but supports incremental parsing via symbolic name_id refs (ty_id nullable in TYN for forwards).

All refs (e.g., ptr to type, array of type, qualifier on type) use name_id (to TYN), allowing use before definition: Insert TYN with null ty_id for forwards, link via name_id, resolve ty_id later. During ingestion, if base type undefined, create placeholder TYN (null ty_id) and link.

## Datamodel

### Core Tables

note:

- entity
- attribute
- attribute type
  - `*` marks a PK
  - `?` marks a nullable
- all tables are uniquely identified by a TLA
- all columns are identified by the parent TLA; if the TLA is different then then we have a foreign key

```text

#------------------------------------------------------------------------------------
# Symbol scopes
#------------------------------------------------------------------------------------

entity SCP_SCOPE: 
    scp_id        : *UUID

link SCP_SCP_PARENT:
    scp_id        : *UUID
    scp_id_parent : UUID

link SCP_TYN_DEF: 
    scp_id : UUID
    tyn_id : UUID

entity TYN_TYPE_NAME: 
    tyn_id   : *UUID
    tyn_name : String
    tydef_id : UUID?


#------------------------------------------------------------------------------------
# Type use
#------------------------------------------------------------------------------------

entity TY_TYPE: 
    ty_id  : *UUID
    tyk_id : Int

entity TYK_TYPE_KIND:
    tyk_id          : *Int
    tyk_name        : String # (B)uiltin Type | (S)truct Type | (U)nion Type | (T)ypedef Type | (E)num Type | (P)ointer Type | (A)rray Type | (V)ariable Lenght Array Type | (F)unction Type | (Q)ualified Type
    tyk_description : String?  

entity TYB_TYPE_BUILTIN:
    ty_id             : *UUID  # typ_ for TYP_PRIMITIVE
    tyn_id            : UUID   # the builtin name for example: UUID of "int" in TYN, UUID of "long long" in TYN, UUID of "unsigned long long" in TYN, ...
    tyb_size_bytes    : Int
    tyb_aligned_bytes : Int

entity TYS_TYPE_STRUCT:
    ty_id    : *UUID   
    tys_name : String

entity TYU_TYPE_UNION:
    ty_id    : *UUID
    tyu_name : String  

entity TYE_TYPE_ENUM:
    ty_id    : *UUID
    tye_name : String  

entity TYT_TYPE_REF:
    ty_id    : *UUID
    tyt_name : String

entity TYP_TYPE_POINTER:
    ty_id        : *UUID
    ty_id_ptr_to : UUID # Foreign key to TY_TYPE.ty_id 

entity TYA_TYPE_ARRAY:
    ty_id    : *UUID
    tya_size : INT

entity TYV_TYPE_VLA:
    ty_id      : *UUID
    ty_id_type : UUID # Foreign key to TY_TYPE.ty_id 

entity TYF_TYPE_FUNCTION:
    ty_id             : *UUID
    ty_id_return_type : UUID # Foreign key to TY_TYPE.ty_id 

entity TYQ_TYPE_QUALIFIED:
    ty_id               : *UUID
    ty_id_type          : UUID UUID # Foreign key to TY_TYPE.ty_id 
    tyq_is_const        : Boolean DEFAULT FALSE
    tyq_is_volatile     : Boolean DEFAULT FALSE
    tyq_is_restrict     : Boolean DEFAULT FALSE
    tyq_is_atomic       : Boolean DEFAULT FALSE
    tyq_is_thread_local : Boolean DEFAULT FALSE
    > constraint 
        rule "at least one of the boolean fields is TRUE"

#------------------------------------------------------------------------------------
# Type def
#------------------------------------------------------------------------------------

entity TYDEF_TYPE_DEFINITION:
    tydef_id   : *UUID
    tydef_kind : STRUCT | ENUM | UNION | BUILTIN

entity TYDEF_BUILTIN_DEF
    tydef_id : *UUID

entity TYDEF_UNION_DEF
    tydef_id   : *UUID
    src_loc_id :  UUID
    scp_id     :  UUID

entity TYDEF_ENUM_DEF:
    tydef_id : *UUID
    src_loc_id: UUID

entity TYDEF_STRUCT_DEF:
    tydef_id   : *UUID
    src_loc_id :  UUID
    scp_id     :  UUID

entity SRC_LOCATION_ID
    src_loc_id  : *UUID
    fil_id      : INT
    begin_line  : INT
    begin_col   : INT
    end_line    : INT
    end_col     : INT
   
entity FIL_FILE_RESOURCE: 
    fil_id  : INT
    path    : String
   
#------------------------------------------------------------------------------------
# Expr def
#------------------------------------------------------------------------------------






























#------------------------------------------------------------------------------------
# TODO ....
#------------------------------------------------------------------------------------


entity TYC_COMPLETE_TYPE:
    tyc_id              : *UUID
    tyck_id             : Int                  # FK to TYCK

entity TYCK_COMPLETE_TYPE_KIND:
    tyck_id             : *Int
    tyck_name           : String
    tyck_description    : String?


entity PRIMK_PRIMITIVE_KIND:
    primk_id            : *Int
    primk_name          : String
    primk_description   : String?

entity TYPT_PTR_TYPE:
    typt_id             : *UUID                # typt_ for TYPT_PTR_TYPE
    typt_base_tyn_id    : UUID                 # FK to TYN

entity TYSA_STATIC_ARRAY_TYPE:
    tysa_id             : *UUID                # tysa_ for TYSA_STATIC_ARRAY_TYPE
    tysa_base_tyn_id    : UUID                 # FK
    tysa_ty_size        : Int
    tysa_is_flexible    : Boolean DEFAULT FALSE

entity TYV_VLA_TYPE:
    tyv_id              : *UUID                # tyv_ for TYV_VLA_TYPE
    tyv_base_tyn_id     : UUID
    tyv_size_expr       : String?

entity TYA_ALIAS_TYPE:
    tya_id              : *UUID                # tya_ for TYA_ALIAS_TYPE
    tya_base_tyn_id     : UUID

entity TYQ_QUALIFIER:
    tyq_id              : *UUID                # tyq_ for TYQ_QUALIFIER
    tyq_base_tyn_id     : UUID




entity TYFP_FUNCTION_PARAM:
    tyfp_id             : *UUID                # tyfp_ for TYFP_FUNCTION_PARAM
    tyc_id              : UUID                 # FK to TYC (TYF_FUNCTION)
    tyfp_base_tyn_id    : UUID
    tyfp_param_name     : String?
    tyfp_position       : Int



entity TYSF_STRUCT_FIELD:
    tysf_id             : *UUID                # tysf_ for TYSF_STRUCT_FIELD
    tyc_id              : UUID                 # FK to TYC (TYS_STRUCT)
    tysf_field_name     : String?
    tysf_base_tyn_id    : UUID
    tysf_offset_bytes   : Int
    tysf_bit_offset     : Int?
    tysf_bit_width      : Int?
    tysf_position       : Int
    tysf_storage_tyc_id : UUID?
    tysf_bit_container_size : Int?



entity TYUF_UNION_FIELD:
    tyuf_id             : *UUID                # tyuf_ for TYUF_UNION_FIELD
    tyc_id              : UUID                 # FK to TYC (TYU_UNION)
    tyuf_field_name     : String?
    tyuf_base_tyn_id    : UUID
    tyuf_position       : Int

entity TYE_ENUM:
    tye_id              : *UUID                # tye_ for TYE_ENUM
    tye_underlying_tyn_id : UUID?
    tye_size_bytes      : Int
    tye_alignment       : Int

entity TYEC_ENUM_CONSTANT:
    tyec_id             : *UUID                # tyec_ for TYEC_ENUM_CONSTANT
    tyc_id              : UUID                 # FK to TYC (TYE_ENUM)
    tyec_const_name     : String
    tyec_const_value    : Int64?
    tyec_position       : Int

entity TYDEF_TYPE_DEFINITION:
    tydef_id            : *UUID                # tydef_ for TYDEF_TYPE_DEFINITION
    tyn_id              : UUID                 # FK to TYN
    tydef_file          : String
    tydef_begin_line    : Int
    tydef_begin_column  : Int
    tydef_end_line      : Int
    tydef_end_column    : Int

entity TYUSE_TYPE_USE:
    tyuse_id            : *UUID                # tyuse_ for TYUSE_TYPE_USE
    tyn_id              : UUID                 # FK
    tyuse_file          : String
    tyuse_begin_line    : Int
    tyuse_begin_column  : Int
    tyuse_end_line      : Int
    tyuse_end_column    : Int
    tyusek_id           : Int                  # FK to TYUSEK

entity TYUSEK_TYPE_USE_KIND:
    tyusek_id           : *Int                 # tyusek_ for TYUSEK_TYPE_USE_KIND
    tyusek_name         : String
    tyusek_description  : String?

entity VAR_VARIABLE:
    var_id                : *UUID                    # var_ for VAR_VARIABLE
    var_name              : String
    var_base_tyn_id       : UUID                     # FK to TYN
    stck_id               : Int?                     # FK to STCK
    var_is_global         : Boolean
    var_containing_fun_id : UUID?                    # FK to FUN
    var_is_parameter      : Boolean DEFAULT FALSE
    var_is_const          : Boolean DEFAULT FALSE

entity VARDEF_VARIABLE_DEFINITION:
    vardef_id           : *UUID           # vardef_ for VARDEF_VARIABLE_DEFINITION
    var_id              : UUID            # FK
    vardef_file         : String
    vardef_begin_line   : Int
    vardef_begin_column : Int
    vardef_end_line     : Int
    vardef_end_column   : Int
    vardef_is_definition: Boolean
    vardef_is_tentative : Boolean DEFAULT FALSE

entity VARUSE_VARIABLE_USE:
    varuse_id           : *UUID         # varuse_ for VARUSE_VARIABLE_USE
    var_id              : UUID          # FK
    varuse_file         : String
    varuse_begin_line   : Int
    varuse_begin_column : Int
    varuse_end_line     : Int
    varuse_end_column   : Int
    varusek_id          : Int           # FK to VARUSEK

entity VARUSEK_VARIABLE_USE_KIND:
    varusek_id          : *Int          # varusek_ for VARUSEK_VARIABLE_USE_KIND
    varusek_name        : String
    varusek_description : String?

entity STCK_STORAGE_CLASS_KIND:
    stck_id             : *Int          # stck_ for STCK_STORAGE_CLASS_KIND
    stck_name           : String
    stck_description    : String?

entity CCK_CALLING_CONVENTION_KIND:
    cck_id              : *Int         # cck_ for CCK_CALLING_CONVENTION_KIND
    cck_name            : String
    cck_description     : String?

entity FUN_FUNCTION:
    fun_id              : *UUID        # fun_ for FUN_FUNCTION
    fun_name            : String
    fun_tyc_id          : UUID         # FK to TYC
    stck_id             : Int?         # FK to STCK

entity FUNDECL_FUNCTION_DECLARATION:
    fundecl_id          : *UUID        # fundecl_ for FUNDECL_FUNCTION_DECLARATION
    fun_id              : UUID         # FK
    fundecl_file        : String
    fundecl_begin_line  : Int
    fundecl_begin_column: Int
    fundecl_end_line    : Int
    fundecl_end_column  : Int
    fundecl_is_forward  : Boolean DEFAULT FALSE

entity FUNIMPL_FUNCTION_IMPLEMENTATION:
    funimpl_id                : *UUID     # funimpl_ for FUNIMPL_FUNCTION_IMPLEMENTATION
    fun_id                    : UUID      # FK
    funimpl_file              : String
    funimpl_begin_line        : Int
    funimpl_begin_column      : Int
    funimpl_end_line          : Int
    funimpl_end_column        : Int
    funimpl_body_begin_line   : Int
    funimpl_body_begin_column : Int
    funimpl_body_end_line     : Int
    funimpl_body_end_column   : Int

entity FUNUSE_FUNCTION_USE:
    funuse_id           : *UUID       # funuse_ for FUNUSE_FUNCTION_USE
    fun_id              : UUID        # FK
    funuse_file         : String
    funuse_begin_line   : Int
    funuse_begin_column : Int
    funuse_end_line     : Int
    funuse_end_column   : Int
    funusek_id          : Int         # FK to FUNUSEK

entity FUNUSEK_FUNCTION_USE_KIND:
    funusek_id          : *Int        # funusek_ for FUNUSEK_FUNCTION_USE_KIND
    funusek_name        : String
    funusek_description : String?

entity ECDEF_ENUM_CONSTANT_DEFINITION:
    ecdef_id            : *UUID       # ecdef_ for ECDEF_ENUM_CONSTANT_DEFINITION
    ecdef_const_id      : UUID        # FK to TYEC_ENUM_CONSTANT
    ecdef_file          : String
    ecdef_begin_line    : Int
    ecdef_begin_column  : Int
    ecdef_end_line      : Int
    ecdef_end_column    : Int

entity ECUSE_ENUM_CONSTANT_USE:
    ecuse_id            : *UUID       # ecuse_ for ECUSE_ENUM_CONSTANT_USE
    ecuse_const_id      : UUID        # FK
    ecuse_file          : String
    ecuse_begin_line    : Int
    ecuse_begin_column  : Int
    ecuse_end_line      : Int
    ecuse_end_column    : Int
    ecusek_id           : Int         # FK to ECUSEK

entity ECUSEK_ENUM_CONSTANT_USE_KIND:
    ecusek_id           : *Int        # ecusek_ for ECUSEK_ENUM_CONSTANT_USE_KIND
    ecusek_name         : String
    ecusek_description  : String?
```

### Predefined Data

#### TYBK_TYPE_BASE_KIND

| tybk_id       | tybk_name   | tybk_description                          |
|:--------------|:------------|:------------------------------------------|
| 0             | BuiltIn     | Built-in types like int                   |
| 1             | Typedef     | Typedef identifiers                       |
| 2             | StructTag   | Struct tag names                          |
| 3             | UnionTag    | Union tag names                           |
| 4             | EnumTag     | Enum tag names                            |

#### TYCK_COMPLETE_TYPE_KIND

| tyck_id       | tyck_name    | tyck_description                      |
|:--------------|:-------------|:--------------------------------------|
| 0             | Primitive    | Basic types                           |
| 1             | Pointer      | Pointer types                         |
| 2             | StaticArray  | Fixed-size arrays                     |
| 3             | Enum         | Enumeration types                     |
| 4             | Struct       | Structure types                       |
| 5             | Union        | Union types                           |
| 6             | Alias        | Typedef aliases                       |
| 7             | Function     | Function types                        |
| 8             | Qualifier    | Qualified types                       |
| 9             | VLA          | Variable-length arrays                |
| 10            | Bitfield     | Bitfield types (if specialized)       |

#### PRIMK_PRIMITIVE_KIND

| primk_id      | primk_name   | primk_description                             |
|:--------------|:-------------|:----------------------------------------------|
| 0             | INTEGER      | Integer types (e.g., int, long)               |
| 1             | FLOAT        | Floating-point types (e.g., float, double)    |
| 2             | CHAR         | Character types (e.g., char, wchar_t)         |
| 3             | BOOL         | Boolean types (_Bool)                         |
| 4             | VOID         | Void type                                     |
| 5             | COMPLEX      | Complex types (_Complex)                      |
| 6             | IMAGINARY    | Imaginary types (_Imaginary)                  |

#### TYUSEK_TYPE_USE_KIND

| tyusek_id     | tyusek_name        | tyusek_description                       |
|:--------------|:-------------------|:-----------------------------------------|
| 0             | Parameters         | Function parameters                      |
| 1             | StructsUnions      | Fields in struct/union                   |
| 2             | StaticGlobals      | Static variables at file scope           |
| 3             | StaticConstants    | Static const at file scope               |
| 4             | LocalVariables     | Local variables in functions             |
| 5             | GlobalVariables    | Non-static variables at file scope       |
| 6             | FunctionReturns    | Return types                             |
| 7             | Typedefs           | Typedef declarations                     |
| 8             | EnumDeclarations   | Enum types/constants                     |
| 9             | ExternDeclarations | Extern declarations                      |
| 10            | Casts              | Types in cast expressions                |
| 11            | Sizeof             | Types in sizeof operators                |
| 12            | Alignof            | Types in _Alignof                        |

#### STCK_STORAGE_CLASS_KIND

| stck_id       | stck_name    | stck_description                             |
|:--------------|:-------------|:---------------------------------------------|
| 0             | NONE         | Default (extern for globals, auto for locals)|
| 1             | STATIC       | Static storage                               |
| 2             | EXTERN       | External linkage                             |
| 3             | AUTO         | Automatic storage                            |
| 4             | REGISTER     | Register storage                             |
| 5             | THREAD_LOCAL | Thread-local storage (_Thread_local)         |
| 6             | TYPEDEF      | Typedef (for compatibility)                  |

#### CCK_CALLING_CONVENTION_KIND

| cck_id        | cck_name     | cck_description                          |
|:--------------|:-------------|:-----------------------------------------|
| 0             | CDECL        | Standard C calling convention            |
| 1             | STDCALL      | Standard call (Windows)                  |
| 2             | FASTCALL     | Fast call                                |
| 3             | VECTORCALL   | Vector call                              |
| 4             | THISCALL     | This call (for methods)                  |
| 5             | SYSCALL      | System call                              |
| 6             | API_CALL     | API call (e.g., attribute((apicall)))    |
| 7             | DEFAULT      | Compiler default                         |

#### FUNUSEK_FUNCTION_USE_KIND

| funusek_id    | funusek_name  | funusek_description                           |
|:--------------|:--------------|:----------------------------------------------|
| 0             | Call          | Function call                                 |
| 1             | AddressTaken  | Function pointer taken                        |
| 2             | Callback      | Used as callback                              |
| 3             | Inline        | Inline expansion (if tracked)                 |
| 4             | VirtualCall   | Virtual function call (C++ extension)         |

#### ECUSEK_ENUM_CONSTANT_USE_KIND

| ecusek_id     | ecusek_name    | ecusek_description                   |
|:--------------|:---------------|:-------------------------------------|
| 0             | Reference      | Used in expression                   |
| 1             | SwitchCase     | Used in switch case                  |
| 2             | Initialization | Used in initializer                  |
| 3             | Comparison     | Used in comparison                   |

## Examples

