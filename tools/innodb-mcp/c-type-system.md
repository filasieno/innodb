# C Type System - Relational Database Schema

## Core Tables

Let's define the C type system:

```text

entity TYS_TYPE_SPEC
  tys_spec: String (PK)
  ty_id: UUID (FK TY_TYPE.ty_id)

entity TYN_TYPE_NAME: 
  tyn_name: String (PK)  # Just the identifier, without struct/union/enum prefix
  ty_id: UUID (FK TY_TYPE.ty_id)
  # Note: Type kind (struct/union/enum/class) is determined by TY_TYPE.tyk_id
  # For C codebases, this avoids redundancy
  # For C++, this matches native name lookup (class Foo can be used as "Foo")
  # Example: "point" not "struct point", "string" not "class string"

entity TY_TYPE:
   ty_id: UUID (PK)
   tyk_id: INT (FK TYK_TYPE_KIND.tyk_id)

entity TYK_TYPE_KIND:
   tyk_id: INT (PK)
   tyk_name: String

entity TY_CONST_TYPE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   ty_id_ref: UUID (FK TY_TYPE.ty_id)

entity TY_VOLATILE_TYPE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   ty_id_ref: UUID (FK TY_TYPE.ty_id)

entity TY_RESTRICT_TYPE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   ty_id_ref: UUID (FK TY_TYPE.ty_id)

entity TY_PTR_TYPE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   ty_id_ref: UUID (FK TY_TYPE.ty_id)

entity TY_STATIC_ARRAY_TYPE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   ty_id_ref: UUID (FK TY_TYPE.ty_id)
   ty_size: Int

entity TY_ARRAY_TYPE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   ty_id_ref: UUID (FK TY_TYPE.ty_id)

entity TY_ALIAS_TYPE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   ty_id_ref: UUID (FK TY_TYPE.ty_id)

entity TY_PRIMITIVE:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   prim_kind: String  # INTEGER | FLOAT | CHAR | BOOL | VOID
   size_bytes: Int
   alignment: Int
   is_signed: Boolean?  # NULL for non-numeric types

entity TY_STRUCT:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   size_bytes: Int
   alignment: Int

entity TY_STRUCT_FIELD:
   field_id: UUID (PK)
   ty_struct_id: UUID (FK TY_STRUCT.ty_id)
   field_name: String
   ty_id: UUID (FK TY_TYPE.ty_id)
   offset_bytes: Int
   bit_offset: Int?
   bit_width: Int?
   position: Int

entity TY_FUNCTION_PTR:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   return_ty_id: UUID (FK TY_TYPE.ty_id)
   is_variadic: Boolean

entity TY_ENUM:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   underlying_ty_id: UUID (FK TY_TYPE.ty_id)  # int, long, etc.

entity TY_ENUM_VALUE:
   enum_value_id: UUID (PK)
   ty_enum_id: UUID (FK TY_ENUM.ty_id)
   name: String
   value: Int
   position: Int

entity TY_UNION:
   ty_id: UUID (PK, FK TY_TYPE.ty_id)
   size_bytes: Int
   alignment: Int

entity TY_UNION_FIELD:
   field_id: UUID (PK)
   ty_union_id: UUID (FK TY_UNION.ty_id)
   field_name: String
   ty_id: UUID (FK TY_TYPE.ty_id)
   position: Int

entity TY_FUNCTION_PARAM:
   param_id: UUID (PK)
   ty_function_ptr_id: UUID (FK TY_FUNCTION_PTR.ty_id)
   param_ty_id: UUID (FK TY_TYPE.ty_id)
   position: Int   
```

For instance, a set of primitives should be predefined:

### TYK_TYPE_KIND

| tyk_id  | tyk_name                  |
|---------|---------------------------|
| 0       | Primitive                 |
| 1       | Const                     |
| 2       | Volatile                  |
| 3       | Restrict                  |
| 4       | Pointer                   |
| 5       | Static Array              |
| 6       | Array                     |
| 7       | Enum                      |
| 8       | Struct                    |
| 9       | Union                     |
| 10      | Alias                     |
| 11      | Function Pointer          |

### TY_TYPE

| ty_id   | tyk_id |
|---------|--------|
| UUID1   |   0    |
| UUID2   |   0    |
| UUID3   |   0    |
| UUID4   |   0    |
| UUID5   |   0    |
| UUID6   |   0    |
| UUID7   |   0    |
| UUID8   |   0    |
| UUID9   |   0    |
| UUID10  |   0    |
| UUID11  |   0    |
| UUID12  |   0    |
| UUID13  |   0    |
| UUID14  |   0    |
| UUID15  |   0    |

### TY_PRIMITIVE

| ty_id  | prim_kind | size_bytes | alignment | is_signed |
|--------|-----------|------------|-----------|-----------|
| UUID1  | CHAR      | 1          | 1         | true      |
| UUID2  | INTEGER   | 2          | 2         | true      |
| UUID3  | INTEGER   | 4          | 4         | true      |
| UUID4  | INTEGER   | 8          | 8         | true      |
| UUID5  | INTEGER   | 8          | 8         | true      |
| UUID6  | CHAR      | 1          | 1         | false     |
| UUID7  | INTEGER   | 2          | 2         | false     |
| UUID8  | INTEGER   | 4          | 4         | false     |
| UUID9  | INTEGER   | 8          | 8         | false     |
| UUID10 | INTEGER   | 8          | 8         | false     |
| UUID11 | FLOAT     | 4          | 4         | NULL      |
| UUID12 | FLOAT     | 8          | 8         | NULL      |
| UUID13 | FLOAT     | 16         | 16        | NULL      |
| UUID14 | BOOL      | 1          | 1         | NULL      |
| UUID15 | VOID      | 0          | 1         | NULL      |

### TYN_TYPE_NAME

| tyn_name             | ty_id   |
|----------------------|---------|
| "signed char"        | UUID1   |
| "signed short"       | UUID2   |
| "signed int"         | UUID3   |
| "signed long"        | UUID4   |
| "signed long long"   | UUID5   |
| "unsigned char"      | UUID6   |
| "unsigned short"     | UUID7   |
| "unsigned int"       | UUID8   |
| "unsigned long"      | UUID9   |
| "unsigned long long" | UUID10  |
| "float"              | UUID11  |
| "double"             | UUID12  |
| "long double"        | UUID13  |
| "bool"               | UUID14  |
| "void"               | UUID15  |

### TYS_TYPE_SPEC

| tys_spec             | ty_id   |
|----------------------|---------|
| "signed char"        | UUID1   |
| "signed short"       | UUID2   |
| "signed int"         | UUID3   |
| "signed long"        | UUID4   |
| "signed long long"   | UUID5   |
| "unsigned char"      | UUID6   |
| "unsigned short"     | UUID7   |
| "unsigned int"       | UUID8   |
| "unsigned long"      | UUID9   |
| "unsigned long long" | UUID10  |
| "float"              | UUID11  |
| "double"             | UUID12  |
| "long double"        | UUID13  |
| "bool"               | UUID14  |
| "void"               | UUID15  |

The spec type is the type as represented in C with a String:

`const char*`
`char []`
`char [10]`
`char [10][200][]`
`char **[10][200][]`

## Complex Type Examples

Let's show how these complex types are represented:

### Example 1: `const char*` (pointer to const char)

Requires 3 type entries: `char` (primitive) → `const char` → `const char*`

#### TY_TYPE (Example 1)

| ty_id   | tyk_id | comment             |
|---------|--------|---------------------|
| UUID1   | 0      | char (primitive)    |
| UUID11  | 1      | const char          |
| UUID12  | 4      | const char* (ptr)   |

#### TY_PRIMITIVE (Example 1)

| ty_id | prim_kind | size_bytes | alignment | is_signed |
|-------|-----------|------------|-----------|-----------|
| UUID1 | CHAR      | 1          | 1         | true      |

#### TY_CONST_TYPE (Example 1)

| ty_id  | ty_id_ref | comment          |
|--------|-----------|------------------|
| UUID11 | UUID1     | points to char   |

#### TY_PTR_TYPE (Example 1)

| ty_id  | ty_id_ref | comment               |
|--------|-----------|----------------------|
| UUID12 | UUID11    | points to const char |

#### TYS_TYPE_SPEC (Example 1)

| tys_spec      | ty_id  |
|---------------|--------|
| "char"        | UUID1  |
| "const char"  | UUID11 |
| "const char*" | UUID12 |

---

### Example 2: `char[10]` (static array of 10 chars)

#### TY_TYPE (Example 2)

| ty_id   | tyk_id |
|---------|--------|
| UUID1   | 0      |
| UUID21  | 5      |

#### TY_STATIC_ARRAY_TYPE (Example 2)

| ty_id  | ty_id_ref | ty_size |
|--------|-----------|---------|
| UUID21 | UUID1     | 10      |

#### TYS_TYPE_SPEC (Example 2)

| tys_spec   | ty_id  |
|------------|--------|
| "char"     | UUID1  |
| "char[10]" | UUID21 |

---

### Example 3: `char[]` (unsized array)

#### TY_TYPE (Example 3)

| ty_id   | tyk_id |
|---------|--------|
| UUID1   | 0      |
| UUID31  | 6      |

#### TY_ARRAY_TYPE (Example 3)

| ty_id  | ty_id_ref |
|--------|-----------|
| UUID31 | UUID1     |

#### TYS_TYPE_SPEC (Example 3)

| tys_spec | ty_id  |
|----------|--------|
| "char"   | UUID1  |
| "char[]" | UUID31 |

---

### Example 4: `double*` (pointer to float type)

Demonstrates FLOAT primitive kind

#### TY_TYPE (Example 4)

| ty_id   | tyk_id |
|---------|--------|
| UUID12  | 0      |
| UUID32  | 4      |

#### TY_PRIMITIVE (Example 4)

| ty_id  | prim_kind | size_bytes | alignment | is_signed |
|--------|-----------|------------|-----------|-----------|
| UUID12 | FLOAT     | 8          | 8         | NULL      |

#### TY_PTR_TYPE (Example 4)

| ty_id  | ty_id_ref |
|--------|-----------|
| UUID32 | UUID12    |

#### TYS_TYPE_SPEC (Example 4)

| tys_spec | ty_id  |
|----------|--------|
| "double" | UUID12 |
| "double\*" | UUID32 |

---

### Example 5: `char[10][200][]` (3D array: static, static, dynamic)

This is: array-of-unknown-size of (array-of-200 of (array-of-10 chars))

#### TY_TYPE (Example 5)

| ty_id   | tyk_id |
|---------|--------|
| UUID1   | 0      |
| UUID41  | 5      |
| UUID42  | 5      |
| UUID43  | 6      |

#### TY_STATIC_ARRAY_TYPE (Example 5)

| ty_id  | ty_id_ref | ty_size |
|--------|-----------|---------|
| UUID41 | UUID1     | 10      |
| UUID42 | UUID41    | 200     |

#### TY_ARRAY_TYPE (Example 5)

| ty_id  | ty_id_ref |
|--------|-----------|
| UUID43 | UUID42    |

#### TYS_TYPE_SPEC (Example 5)

| tys_spec           | ty_id  |
|--------------------|--------|
| "char\[10\]"         | UUID41 |
| "char\[10\]\[200\]"    | UUID42 |
| "char\[10\]\[200\]\[\]"  | UUID43 |

---

### Example 6: `char**[10][200][]` (arrays of pointer-to-pointer-to-char)

Build from inside out: `char` → `char*` → `char**` → `char**[10]` → `char**[10][200]` → `char**[10][200][]`

#### TY_TYPE (Example 6)

| ty_id   | tyk_id |
|---------|--------|
| UUID1   | 0      |
| UUID51  | 4      |
| UUID52  | 4      |
| UUID53  | 5      |
| UUID54  | 5      |
| UUID55  | 6      |

#### TY_PTR_TYPE (Example 6)

| ty_id  | ty_id_ref |
|--------|-----------|
| UUID51 | UUID1     |
| UUID52 | UUID51    |

#### TY_STATIC_ARRAY_TYPE (Example 6)

| ty_id  | ty_id_ref | ty_size |
|--------|-----------|---------|
| UUID53 | UUID52    | 10      |
| UUID54 | UUID53    | 200     |

#### TY_ARRAY_TYPE (Example 6)

| ty_id  | ty_id_ref |
|--------|-----------|
| UUID55 | UUID54    |

#### TYS_TYPE_SPEC (Example 6)

| tys_spec              | ty_id  |
|-----------------------|--------|
| "char\*"               | UUID51 |
| "char\*\*"              | UUID52 |
| "char\*\*\[10\]"          | UUID53 |
| "char\*\*\[10\]\[200\]"     | UUID54 |
| "char\*\*\[10\]\[200\]\[\]"   | UUID55 |

---

### Example 7: Function Pointer `int (*callback)(const char* restrict, volatile int*)`

Type: pointer-to-function that takes (const char\* restrict, volatile int\*) and returns int

This demonstrates const, restrict, and volatile qualifiers in function parameters.

#### TY_TYPE (Example 7)

| ty_id   | tyk_id | description                    |
|---------|--------|--------------------------------|
| UUID3   | 0      | int (primitive)                |
| UUID1   | 0      | char (primitive)               |
| UUID61  | 0      | int (primitive)                |
| UUID62  | 1      | const char                     |
| UUID63  | 4      | const char*                    |
| UUID64  | 3      | const char* restrict           |
| UUID65  | 2      | volatile int                   |
| UUID66  | 4      | volatile int*                  |
| UUID67  | 11     | function pointer type          |

#### TY_CONST_TYPE (Example 7)

| ty_id  | ty_id_ref | description      |
|--------|-----------|------------------|
| UUID62 | UUID1     | const char       |

#### TY_PTR_TYPE (Example 7)

| ty_id  | ty_id_ref |
|--------|-----------|
| UUID63 | UUID62    |
| UUID66 | UUID65    |

#### TY_RESTRICT_TYPE (Example 7)

| ty_id  | ty_id_ref | description              |
|--------|-----------|--------------------------|
| UUID64 | UUID63    | const char* restrict     |

#### TY_VOLATILE_TYPE (Example 7)

| ty_id  | ty_id_ref | description      |
|--------|-----------|------------------|
| UUID65 | UUID61    | volatile int     |

#### TY_FUNCTION_PTR (Example 7)

| ty_id  | return_ty_id | is_variadic |
|--------|--------------|-------------|
| UUID67 | UUID3        | false       |

#### TY_FUNCTION_PARAM (Example 7)

| param_id | ty_function_ptr_id | param_ty_id | position | description                |
|----------|-------------------|-------------|----------|----------------------------|
| PID1     | UUID67            | UUID64      | 0        | const char* restrict       |
| PID2     | UUID67            | UUID66      | 1        | volatile int*              |

#### TYS_TYPE_SPEC (Example 7)

| tys_spec                                           | ty_id  |
|----------------------------------------------------|--------|
| "int (*)(const char* restrict, volatile int*)"     | UUID67 |

---

### Example 8: Struct with Nested Types

```c
struct point {
    int x;
    int y;
};

struct line {
    struct point start;
    struct point end;
    const char* label;
};
```

#### TY_TYPE (Example 8)

| ty_id    | tyk_id |
|----------|--------|
| UUID3    | 0      |
| UUID1    | 0      |
| UUID71   | 8      |
| UUID72   | 8      |
| UUID73   | 1      |
| UUID74   | 4      |

#### TY_STRUCT - struct point (Example 8)

| ty_id  | size_bytes | alignment |
|--------|------------|-----------|
| UUID71 | 8          | 4         |

#### TY_STRUCT_FIELD - struct point (Example 8)

| field_id | ty_struct_id | field_name | ty_id | offset_bytes | position |
|----------|--------------|------------|-------|--------------|----------|
| FID1     | UUID71       | "x"        | UUID3 | 0            | 0        |
| FID2     | UUID71       | "y"        | UUID3 | 4            | 1        |

#### TY_STRUCT - struct line (Example 8)

| ty_id  | size_bytes | alignment |
|--------|------------|-----------|
| UUID72 | 24         | 8         |

#### TY_STRUCT_FIELD - struct line (Example 8)

| field_id | ty_struct_id | field_name | ty_id  | offset_bytes | position |
|----------|--------------|------------|--------|--------------|----------|
| FID3     | UUID72       | "start"    | UUID71 | 0            | 0        |
| FID4     | UUID72       | "end"      | UUID71 | 8            | 1        |
| FID5     | UUID72       | "label"    | UUID74 | 16           | 2        |

#### TYN_TYPE_NAME (Example 8)

| tyn_name | ty_id  |
|----------|--------|
| "point"  | UUID71 |
| "line"   | UUID72 |

Note: Type names are stored without struct/union/enum prefix. To reconstruct C-style names, combine with type kind from `TY_TYPE.tyk_id`.

#### TYS_TYPE_SPEC (Example 8)

| tys_spec       | ty_id  |
|----------------|--------|
| "struct point" | UUID71 |
| "struct line"  | UUID72 |
