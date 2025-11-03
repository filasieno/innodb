# Module Code Layout Documentation

## Overview

This document describes the standardized code layout structure for modules and submodules in the InnoDB project. This layout provides clear separation of concerns, supports multiple build systems, and ensures consistent organization across all project components.

## Directory Structure

```
module/
├── src/                    # Implementation files and private headers
├── include/               # Public API headers
├── doc/                   # Documentation, diagrams, and templates
├── examples/              # Example programs and usage demonstrations
├── test/                  # Unit tests, integration tests, and test utilities
├── nix/                   # Nix build system utilities and expressions
├── cmake/                 # CMake build system utilities and modules
├── modules/               # Submodules and component modules
├── default.nix           # Nix package definition
├── CMakeLists.txt        # CMake build configuration
└── README.md             # Module-specific documentation
```

## Directory Descriptions

### `src/` - Source Implementation
**Purpose**: Contains all implementation files (.cpp, .c) and private header files (.hpp, .h) that are not part of the public API.

**Contents**:
- Implementation of classes and functions
- Private/internal headers not exposed to users
- Platform-specific implementations
- Internal utilities and helpers

**Guidelines**:
- Private headers should be included only by files in `src/`
- Avoid including private headers in `include/` files
- Use relative includes for private headers: `#include "detail/helper.hpp"`

### `include/` - Public API Headers
**Purpose**: Contains the public API headers that define the module's interface for external consumers.

**Contents**:
- Public class definitions and declarations
- API functions and types
- Public constants and enumerations
- Forward declarations for public interfaces

**Guidelines**:
- Headers in `include/` should be self-contained
- Use include guards or `#pragma once`
- Avoid implementation details in public headers
- Include only necessary dependencies

### `doc/` - Documentation
**Purpose**: Contains all documentation-related files, templates, and assets.

**Contents**:
- Doxygen configuration and templates
- Markdown documentation files (.md)
- Architecture diagrams (Mermaid, PlantUML, etc.)
- API documentation
- Design documents

**Guidelines**:
- Use consistent documentation format
- Include usage examples in documentation
- Keep diagrams and documentation synchronized with code
- Use relative paths in documentation

### `examples/` - Example Programs
**Purpose**: Contains example programs that demonstrate how to use the module's API.

**Contents**:
- Complete, runnable example programs
- Usage demonstrations
- Tutorial code snippets
- Integration examples

**Guidelines**:
- Examples should compile and run independently
- Include build instructions for examples
- Demonstrate common use cases and best practices
- Keep examples up-to-date with API changes

### `test/` - Testing
**Purpose**: Contains all testing-related files and infrastructure.

**Contents**:
- Unit tests
- Integration tests
- Test fixtures and utilities
- Test data files
- Performance benchmarks

**Guidelines**:
- Tests should be runnable independently
- Use established testing frameworks consistently
- Include both positive and negative test cases
- Tests should validate both functionality and performance

### `nix/` - Nix Build System
**Purpose**: Contains Nix-specific build utilities and expressions.

**Contents**:
- Nix package definitions
- Build utilities specific to Nix
- Development environment configurations
- Nix shell configurations

**Guidelines**:
- Follow Nix package conventions
- Include reproducible build configurations
- Document Nix-specific build requirements

### `cmake/` - CMake Build System
**Purpose**: Contains CMake-specific build utilities and modules.

**Contents**:
- CMake modules and find scripts
- Build configuration utilities
- Platform-specific CMake configurations
- Custom CMake functions and macros

**Guidelines**:
- Follow CMake best practices
- Include comprehensive build configurations
- Support multiple platforms and compilers
- Document CMake-specific requirements

### `modules/` - Submodules
**Purpose**: Contains submodules and component modules that are part of this module.

**Contents**:
- Submodules following the same layout structure
- Component libraries
- Plugin modules
- Optional components

**Guidelines**:
- Submodules should follow the same layout convention
- Clearly document submodule dependencies
- Include integration instructions for submodules

### Root Files

#### `default.nix` - Nix Package Definition
**Purpose**: Defines the Nix package for this module.

**Contents**:
- Package metadata (name, version, description)
- Dependencies and build requirements
- Build instructions for Nix
- Test configurations

#### `CMakeLists.txt` - CMake Configuration
**Purpose**: Defines the CMake build configuration for this module.

**Contents**:
- Target definitions
- Dependency specifications
- Installation rules
- Test configurations
- Submodule inclusions

#### `README.md` - Module Documentation
**Purpose**: Provides module-specific documentation and usage instructions.

**Contents**:
- Module description and purpose
- Build and installation instructions
- API usage examples
- Dependencies and requirements

## Build System Integration

### CMake Integration
- `CMakeLists.txt` serves as the main build configuration
- `cmake/` directory contains reusable CMake utilities
- Supports both standalone builds and integration into larger projects

### Nix Integration
- `default.nix` provides reproducible builds
- `nix/` directory contains Nix-specific utilities
- Enables declarative package management and development environments

## Benefits of This Layout

### 1. Clear Separation of Concerns
- Public API clearly separated from private implementation
- Documentation and examples are easily accessible
- Testing infrastructure is isolated and comprehensive

### 2. Multi-Build System Support
- Simultaneous support for CMake and Nix
- Flexible deployment options
- Cross-platform compatibility

### 3. Consistent Organization
- Predictable file locations across all modules
- Easy navigation and maintenance
- Standardized development workflow

### 4. Scalability
- Submodules follow the same structure
- Easy to add new components
- Maintainable at any project size

### 5. Developer Experience
- Clear project structure reduces onboarding time
- Standardized locations for common tasks
- Comprehensive documentation and examples

## Usage Guidelines

### For Module Developers
1. Follow the exact directory structure outlined above
2. Place public headers in `include/`, private headers in `src/`
3. Include comprehensive tests in `test/`
4. Provide working examples in `examples/`
5. Document thoroughly in `doc/`

### For Build System Maintainers
1. Ensure both CMake and Nix configurations are maintained
2. Keep build configurations synchronized
3. Test builds on all supported platforms

### For Documentation Maintainers
1. Keep documentation synchronized with code changes
2. Update diagrams when architecture changes
3. Maintain comprehensive API documentation

## Migration Guide

When migrating existing modules to this structure:

1. Create the directory structure
2. Move public headers to `include/`
3. Move private headers and sources to `src/`
4. Move documentation to `doc/`
5. Move tests to `test/`
6. Move examples to `examples/`
7. Create appropriate build system files
8. Update all include paths and references

## Examples

### Small Utility Module
```
myutil/
├── src/
│   ├── implementation.cpp
│   └── detail/
│       └── internal.hpp
├── include/
│   └── myutil.hpp
├── test/
│   └── test_myutil.cpp
├── doc/
│   └── README.md
├── CMakeLists.txt
└── default.nix
```

### Large Application Module
```
myapp/
├── src/
│   ├── main.cpp
│   ├── core/
│   └── ui/
├── include/
│   ├── myapp/api.hpp
│   └── myapp/types.hpp
├── modules/
│   ├── networking/
│   └── database/
├── test/
├── examples/
├── doc/
├── cmake/
├── nix/
├── CMakeLists.txt
├── default.nix
└── README.md
```

This layout provides a solid foundation for maintainable, scalable, and well-documented C/C++ projects.
