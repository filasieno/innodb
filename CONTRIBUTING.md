# Contributing

## Physical Architecture

### Overview

XInnoDB follows a strict layered architecture designed for ABI stability, maintainability, and extensibility. The system is organized into distinct layers with clear dependency rules and no circular dependencies.

## Architecture Layers

```mermaid
graph TB
    subgraph Layer0["**Layer 0: Public API** (`xinnodb.hpp`)"]
        API["Type-erased, ABI-stable C++ coroutine interface<br/>Forever stable (additive-only changes after 1.0)<br/>Zero templates in ABI surface"]
    end
    
    subgraph Layer1["Layer 1: SDK (xinnodb/sdk/)"]
        SDK["Rich C++ API for plugin/extension developers<br/>RAII wrappers, modern C++ features<br/>Can evolve independently of public API"]
    end
    
    subgraph Layer2["Layer 2: Internal Utilities (xinnodb/internal/)"]
        Internal["Shared utilities across implementation modules<br/>Not exposed to users or SDK<br/>dlink, assert, comptime utilities"]
    end
    
    subgraph Layer3["Layer 3: Module APIs (xinnodb/src/*/include/)"]
        ModuleAPI["Inter-module communication interfaces<br/>Each module exposes public interface<br/>alloc, task, sched, sync, io, rt, etc."]
    end
    
    subgraph Layer4["Layer 4: Module Implementation (xinnodb/src/*/src/)"]
        ModuleImpl["Private implementation details<br/>Never included by other modules<br/>Implementation files and private headers"]
    end
    
    API <-- SDK
    SDK <-- Internal
    SDK <-- ModuleAPI
    Internal <-- ModuleAPI
    ModuleAPI <-- ModuleImpl
    
    style Layer0 fill:#e1f5ff,stroke:#01579b,stroke-width:3px
    style Layer1 fill:#f3e5f5,stroke:#4a148c,stroke-width:2px
    style Layer2 fill:#fff3e0,stroke:#e65100,stroke-width:2px
    style Layer3 fill:#e8f5e9,stroke:#1b5e20,stroke-width:2px
    style Layer4 fill:#fce4ec,stroke:#880e4f,stroke-width:2px
```

**Layer Hierarchy (Top to Bottom):**

- **Layer 4** (Top/Private): Module Implementation - Private details
- **Layer 3**: Module APIs - Inter-module interfaces
- **Layer 2**: Internal Utilities - Shared infrastructure
- **Layer 1**: SDK - Rich extension layer for developers (less stable)
- **Layer 0** (Leaf): Public API - What users see and use (stable)

## Directory Structure

```
xinnodb/
├── include/
│   ├── xinnodb.hpp                      # Layer 0: Public API (STABLE ABI)
│   └── xinnodb/
│       ├── sdk/                         # Layer 1: SDK
│       │   ├── database.hpp             # Rich Database wrapper
│       │   ├── transaction.hpp          # Rich Transaction wrapper
│       │   ├── cursor.hpp               # Rich Cursor wrapper
│       │   ├── plugin.hpp               # Plugin development interface
│       │   ├── allocator.hpp            # Custom allocator support
│       │   └── utilities.hpp            # SDK helper functions
│       │
│       └── internal/                    # Layer 2: Internal utilities
│           ├── dlink.hpp                # Intrusive linked lists
│           ├── assert.hpp               # Debug assertions
│           ├── comptime_string.hpp      # Compile-time utilities
│           └── common.hpp               # Common internal types
│
└── src/                                 # Implementation modules
    ├── defs/                            # Foundation: basic definitions
    │   ├── include/defs/
    │   │   ├── defs.hpp                 # Module public interface
    │   │   └── defs_types.hpp           # Types for other modules
    │   ├── src/
    │   │   └── defs_impl.cpp            # Private implementation
    │   └── test/
    │       └── test_defs.cpp
    │
    ├── alloc/                           # Memory allocator module
    │   ├── include/alloc/
    │   │   ├── alloc.hpp                # Allocator public API
    │   │   └── alloc_types.hpp          # Allocator types
    │   ├── src/
    │   │   ├── alloc_impl.hpp           # Private headers
    │   │   ├── alloc_table.cpp
    │   │   └── alloc_freeblock.cpp
    │   └── test/
    │
    ├── sync/                            # Synchronization primitives
    │   ├── include/sync/
    │   ├── src/
    │   └── test/
    │
    ├── task/                            # Coroutine task system
    │   ├── include/task/
    │   ├── src/
    │   └── test/
    │
    ├── sched/                           # Scheduler
    │   ├── include/sched/
    │   ├── src/
    │   └── test/
    │
    ├── io/                              # I/O operations
    │   ├── include/io/
    │   ├── src/
    │   └── test/
    │
    ├── pwa/                             # Private Worker Area
    │   ├── include/pwa/
    │   ├── src/
    │   └── test/
    │
    ├── sga/                             # Shared Global Area
    │   ├── include/sga/
    │   ├── src/
    │   └── test/
    │
    ├── msgbus/                          # Message bus
    │   ├── include/msgbus/
    │   ├── src/
    │   └── test/
    │
    └── rt/                              # Runtime orchestration
        ├── include/rt/
        ├── src/
        └── test/
```

## Layer 0: Public API (`xinnodb.hpp`)

### Purpose
The public API is the **forever-stable** interface for all XInnoDB users. It uses type erasure to achieve ABI stability while supporting C++20 coroutines.

### Design Principles

1. **Type Erasure**: All templates are wrappers around type-erased functions
2. **POD Types**: All handle types are Plain Old Data (fixed size, no vtables)
3. **Stable Values**: Enum values never change once published
4. **Additive Only**: New functions/enums can be added, never removed
5. **No Breaking Changes**: After 1.0 release, API is frozen

### Structure

```cpp
// xinnodb.hpp has two sections:

// SECTION 1: Type-Erased API (Stable ABI)
namespace xinnodb {
    // POD handle types
    struct ib_sga_hdl { ib_u64 hdl; };
    
    // Type-erased result container
    struct ib_async_result {
        ib_async_opaque opaque;
        const ib_async_ops* ops;
        char result_buffer[32];
    };
    
    // Type-erased functions (exported from library)
    ib_async_result ib_trx_commit_async(ib_trx_hdl) noexcept;
}

// SECTION 2: Template Wrappers (No ABI surface)
namespace xinnodb::inline templates {
    // Header-only template wrappers
    template<typename T>
    class ib_async_hdl { /* wraps ib_async_result */ };
    
    // Convenience template functions
    inline ib_async_hdl<ib_err> ib_trx_commit(ib_trx_hdl);
}
```

### Adding to Public API

**✅ ALLOWED:**
- Add new functions (additive)
- Add new enum values at the end
- Add new handle types
- Add overloads (carefully)

**❌ FORBIDDEN:**
- Change function signatures
- Remove functions
- Change enum values
- Change struct layouts
- Rename symbols

## Layer 1: SDK (`xinnodb/sdk/`)

### Purpose
Rich C++ API for plugin and extension developers. Built on top of the stable public API.

### Design Principles

1. **Depends on Public API**: Includes `xinnodb.hpp`
2. **Modern C++**: RAII, exceptions, smart pointers, concepts
3. **Can Evolve**: Free to add/change features without breaking public API
4. **Plugin Friendly**: Virtual interfaces for extensibility

### Example Module: `xinnodb/sdk/database.hpp`

```cpp
#pragma once
#include "xinnodb.hpp"  // Depends on public API

namespace xinnodb::sdk {
    class Database {
        ib_state_hdl handle_;
    public:
        // RAII wrapper
        static ib_async_hdl<Database> create(ib_sga_hdl);
        
        // Rich interface
        auto begin_transaction() -> ib_async_hdl<Transaction>;
        
        // Access underlying handle
        ib_state_hdl native_handle() const noexcept;
    };
}
```

### Versioning

SDK versions independently from public API:
- Public API: `v1.0.0` (frozen)
- SDK: `v1.5.2` (can evolve)

## Layer 2: Internal Utilities (`xinnodb/internal/`)

### Purpose
Shared utilities used by implementation modules but not exposed to users.

### Contents

- **dlink.hpp**: Intrusive linked list implementation
- **assert.hpp**: Debug assertion macros
- **comptime_string.hpp**: Compile-time string utilities
- **common.hpp**: Internal common types

### Usage Rules

1. Only included by implementation modules (`xinnodb/src/*/`)
2. NOT included by public API or SDK
3. Can change freely (internal implementation detail)
4. Must be header-only or compiled into internal object files

## Layer 3: Module APIs (`xinnodb/src/*/include/`)

### Purpose
Module APIs provide the public interface that each implementation module exposes to other modules. This is the inter-module communication layer.

### Design Principles

1. **Clean Interfaces**: Minimal, focused API for each module
2. **Type Safety**: Strong typing for inter-module communication
3. **No Circular Dependencies**: Enforced through dependency levels
4. **Documentation**: All APIs must be documented

### Example: Allocator Module API

```cpp
// xinnodb/src/alloc/include/alloc/alloc.hpp
#pragma once

#include "alloc/alloc_types.hpp"

namespace xinnodb::alloc {
    // Public API for other modules to use
    void* alloc_table_malloc(ib_alloc_table* table, size_t size) noexcept;
    void alloc_table_free(ib_alloc_table* table, void* ptr) noexcept;
    int alloc_table_init(ib_alloc_table* table, void* mem, size_t size) noexcept;
}
```

## Layer 4: Module Implementation (`xinnodb/src/*/src/`)

### Purpose
The module implementation layer contains all private implementation details, helper functions, and internal data structures. This is the "leaf" layer where actual work happens.

### Design Principles

1. **Encapsulation**: Implementation details never leak to other modules
2. **Private Headers**: Internal headers in `src/` directory only
3. **No External Includes**: Other modules cannot include implementation files
4. **Freedom to Refactor**: Can change freely without affecting other modules

### Example: Allocator Module Implementation

```cpp
// xinnodb/src/alloc/src/alloc_impl.hpp (PRIVATE)
#pragma once

#include "alloc/alloc.hpp"
#include "xinnodb/internal/assert.hpp"

namespace xinnodb::alloc::detail {
    // Private helper functions
    void* find_free_block(ib_alloc_table* table, size_t size) noexcept;
    void coalesce_blocks(ib_alloc_table* table) noexcept;
    
    // Private data structures
    struct FreeBlock {
        size_t size;
        FreeBlock* next;
    };
}
```

```cpp
// xinnodb/src/alloc/src/alloc_table.cpp (PRIVATE)
#include "alloc_impl.hpp"

namespace xinnodb::alloc {
    void* alloc_table_malloc(ib_alloc_table* table, size_t size) noexcept {
        // Implementation uses private helpers
        return detail::find_free_block(table, size);
    }
}
```

### Private Implementation Rules

1. **Header Location**: All private headers in `src/` subdirectory
2. **Include Guards**: Use `#pragma once`
3. **Namespace**: Use `detail` or `internal` sub-namespace for private APIs
4. **Documentation**: Document complex algorithms, not required for all helpers
5. **Testing**: Private details can be tested via white-box testing

## Module Structure Template

### Module Template

Each module follows this structure:

```text
xinnodb/src/<module>/
├── include/<module>/          # Public interface (to other modules)
│   ├── <module>.hpp           # Main module API
│   ├── <module>_types.hpp     # Types other modules need
│   └── <module>_fwd.hpp       # Forward declarations (optional)
│
├── src/                       # Private implementation
│   ├── <module>_impl.hpp      # Private headers
│   ├── <module>_impl.cpp      # Implementation
│   └── <module>_internal.hpp  # Private utilities
│
└── test/                      # Unit tests
    └── test_<module>.cpp
```

### Include Guidelines

**From other modules:**
```cpp
#include "<module>/<module>.hpp"  // ✅ Correct
```

**Never do this:**
```cpp
#include "<module>/src/impl.hpp"  // ❌ WRONG - private header
```

## Module Dependency Graph

Modules are organized in dependency levels to prevent circular dependencies:

### Level 0: Foundation
```
defs          # Basic types, configuration
ut            # Utilities
```

### Level 1: Core Infrastructure
```
alloc         # Memory allocator (depends: defs)
sync          # Synchronization primitives (depends: defs)
io            # I/O operations (depends: defs, sync)
```

### Level 2: Runtime Components
```
task          # Coroutine tasks (depends: defs, alloc)
sched         # Scheduler (depends: defs, task)
pwa           # Private Worker Area (depends: alloc, task)
```

### Level 3: System Services
```
sga           # Shared Global Area (depends: alloc, sync)
msgbus        # Message bus (depends: alloc, task, sched)
rt            # Runtime orchestration (depends: task, sched, pwa)
```

### Level 4: Public API Implementation
```
api           # Implements xinnodb.hpp (depends: all modules)
```

### Dependency Rules

1. **Acyclic**: No circular dependencies allowed
2. **Downward Only**: Modules can only depend on lower levels
3. **Explicit**: All dependencies documented in module `README.md`
4. **Minimal**: Depend only on what you need

### Checking Dependencies

```bash
# Generate dependency graph
./tools/check-dependencies.sh

# Verify no cycles
./tools/verify-acyclic.sh
```

## Adding a New Module

### Step 1: Create Module Structure

```bash
cd xinnodb/src
mkdir -p mymodule/{include/mymodule,src,test}
touch mymodule/include/mymodule/mymodule.hpp
touch mymodule/src/mymodule.cpp
touch mymodule/test/test_mymodule.cpp
touch mymodule/README.md
```

### Step 2: Document Dependencies

In `mymodule/README.md`:

```markdown
# MyModule

## Purpose
Brief description of module purpose.

## Dependencies
- **defs**: Basic types
- **alloc**: Memory allocation

## Dependents
Modules that depend on this module.
```

### Step 3: Implement Module API

```cpp
// include/mymodule/mymodule.hpp
#pragma once

#include "defs/defs.hpp"       // Foundation dependency
#include "alloc/alloc.hpp"     // Core dependency

namespace xinnodb::mymodule {
    // Public API for other modules
    class MyService {
    public:
        void do_something() noexcept;
    };
}
```

### Step 4: Implement Private Details

```cpp
// src/mymodule_impl.hpp
#pragma once

#include "mymodule/mymodule.hpp"
#include "xinnodb/internal/common.hpp"

namespace xinnodb::mymodule::detail {
    // Private implementation details
}
```

### Step 5: Add CMake Integration

```cmake
# CMakeLists.txt (module registration)
add_xinnodb_module(mymodule
    SOURCES
        src/mymodule.cpp
    PUBLIC_HEADERS
        include/mymodule/mymodule.hpp
    PRIVATE_HEADERS
        src/mymodule_impl.hpp
    DEPENDENCIES
        defs_obj
        alloc_obj
    TESTS
        test/test_mymodule.cpp
)
```

### Step 6: Write Tests

```cpp
// test/test_mymodule.cpp
#include <gtest/gtest.h>
#include "mymodule/mymodule.hpp"

TEST(MyModule, BasicFunctionality) {
    xinnodb::mymodule::MyService service;
    service.do_something();
    // Assertions...
}
```

### Step 7: Update Dependency Documentation

Add to `docs/DEPENDENCIES.md`:

```markdown
## mymodule
- **Level**: 1 (Core Infrastructure)
- **Depends on**: defs, alloc
- **Used by**: (none yet)
```

## Build System Integration

### Object Libraries

Each module is compiled as an object library:

```cmake
add_library(mymodule_obj OBJECT
    src/mymodule.cpp
)

target_include_directories(mymodule_obj
    PUBLIC include/           # Other modules can include
    PRIVATE src/             # Private implementation
)

target_link_libraries(mymodule_obj
    PUBLIC defs_obj          # Public dependencies
    PRIVATE xinnodb_internal # Internal utilities
)
```

### Final Libraries

Object libraries are linked into final static/shared libraries:

```cmake
add_library(xinnodb_static STATIC
    $<TARGET_OBJECTS:defs_obj>
    $<TARGET_OBJECTS:alloc_obj>
    $<TARGET_OBJECTS:mymodule_obj>
    # ... all modules
)

add_library(xinnodb_shared SHARED
    # Same as static
)
```

## Header Include Paths

### Public API Users

```cpp
#include "xinnodb.hpp"                    // Public API
#include "xinnodb/sdk/database.hpp"       // SDK
```

### SDK Implementation

```cpp
#include "xinnodb.hpp"                    // Public API dependency
```

### Module Implementation

```cpp
#include "mymodule/mymodule.hpp"          // Own public API
#include "defs/defs.hpp"                  // Other module public API
#include "xinnodb/internal/common.hpp"    // Internal utilities
```

### Private Implementation

```cpp
#include "mymodule_impl.hpp"              // Own private header
```

## Type Erasure Pattern

### Implementing Type-Erased Functions

```cpp
// Public API (xinnodb.hpp) - type-erased
namespace xinnodb {
    ib_async_result my_operation_async(ib_handle_t h) noexcept;
}

// Implementation (src/api/api_impl.cpp)
namespace xinnodb::detail {
    template<typename T>
    struct async_vtable {
        static bool await_ready(ib_async_opaque*) noexcept;
        static void await_resume(ib_async_opaque*, void*) noexcept;
        // ...
    };
    
    template<typename T>
    const ib_async_ops vtable_for = {
        &async_vtable<T>::await_ready,
        &async_vtable<T>::await_resume,
        // ...
    };
}

ib_async_result my_operation_async(ib_handle_t h) noexcept {
    ib_async_result result;
    result.ops = &detail::vtable_for<my_result_type>;
    // Setup type-erased promise...
    return result;
}
```

## Testing Strategy

### Unit Tests

Each module has unit tests in `test/`:

```cpp
// test/test_mymodule.cpp
#include <gtest/gtest.h>
#include "mymodule/mymodule.hpp"

TEST(MyModule, Feature) {
    // Test implementation
}
```

### Integration Tests

Cross-module tests in `xinnodb/test/integration/`:

```cpp
// test/integration/test_workflow.cpp
#include "xinnodb.hpp"

TEST(Integration, CompleteWorkflow) {
    // Test complete user workflow
}
```

### ABI Stability Tests

```cpp
// test/abi/test_stability.cpp
TEST(AbiStability, HandleSizes) {
    static_assert(sizeof(ib_sga_hdl) == 8);
    static_assert(alignof(ib_sga_hdl) == 8);
}

TEST(AbiStability, EnumValues) {
    ASSERT_EQ(static_cast<int>(ib_err::DB_SUCCESS), 10);
}
```

## Documentation Requirements

### Module README

Each module must have `README.md`:

```markdown
# Module Name

## Purpose
What this module does.

## Dependencies
- module1: Why it's needed
- module2: Why it's needed

## Public API
Brief description of main types/functions.

## Implementation Notes
Any important implementation details.

## Thread Safety
Thread safety guarantees.
```

### API Documentation

Use Doxygen comments:

```cpp
/// \brief Brief description
/// \details Detailed description
/// \param name Parameter description
/// \return Return value description
/// \ingroup module_name
ib_async_hdl<ib_err> my_function(ib_handle_t h) noexcept;
```

## Code Style Guidelines

### Naming Conventions

- **Types**: `ib_snake_case` (e.g., `ib_trx_hdl`)
- **Functions**: `ib_snake_case` (e.g., `ib_trx_begin`)
- **Enums**: `IB_SCREAMING_CASE` (e.g., `DB_SUCCESS`)
- **Namespaces**: `lowercase` (e.g., `xinnodb::sdk`)
- **Classes** (SDK): `PascalCase` (e.g., `Database`)

### File Naming

- **Headers**: `module_name.hpp`
- **Implementation**: `module_name.cpp`
- **Private headers**: `module_name_impl.hpp`
- **Tests**: `test_module_name.cpp`

## Review Checklist

Before submitting:

- [ ] No circular dependencies
- [ ] Module README updated
- [ ] Dependency graph updated
- [ ] Unit tests added
- [ ] Doxygen comments added
- [ ] Code style followed
- [ ] No ABI-breaking changes to public API
- [ ] CMake integration complete
- [ ] Tests pass locally

## Getting Help

- **Architecture questions**: See `docs/architecture.md`
- **Build issues**: See `docs/building.md`
- **API design**: Discuss in GitHub issues
- **Module design**: Review existing modules as examples

---

**Remember**: The public API (`xinnodb.hpp`) is **forever stable**. Think carefully before adding to it. Most features should go in the SDK layer.
