# Propagator Platform - Naming Convention

## ID Field Naming Convention

### Physical ID (`_pid`)

- **Purpose**    : Internal database primary key
- **Format**     : `<entity>_pid`
- **Used for**   : Database operations, foreign key references within the database
- **Example**    : `cell_pid`, `cv_pid`, `prop_pid`
- **Properties** : Primary key, used in FK relationships

### Logical ID (`_id`)

- **Purpose**    : External stable identifier
- **Format**     : `<entity>_id`
- **Used for**   : UNIX socket protocol, external references, survives migrations
- **Example**    : `cell_id`, `cv_id`, `prop_id`
- **Properties** : Unique constraint, used in APIs and serialization

## Systematic Naming Pattern

For every entity `<ENTITY>`:

```text
[entity] Entity:
  - <entity>_pid    : UUID <<PK>>      # Physical ID
  - <entity>_id     : UUID <<UNIQUE>>  # Logical ID
  - ... other fields ...
```

### Foreign Key References

Always reference via Physical ID:

```text
- parent_entity_pid : UUID <<FK parent_entity>>  # FK to parent_entity.parent_entity_pid
```

### Variant Tables (Single-Table Inheritance)

```text
[variant] EntityVariant:
  - variant_pid     : UUID <<PK>>                  # Physical ID for variant
  - entity_pid      : UUID <<FK entity>> <<UNIQUE>>  # FK to parent entity Physical ID
  - ... variant-specific fields ...
```

Constraints:

- UNIQUE: (entity_pid)  # Enforces 1:1 relationship
- FK: entity_pid → [entity].entity_pid

