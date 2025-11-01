# ---------------------------------------------------------------------------------------------------------------------
# libak Library Build Configuration
# ---------------------------------------------------------------------------------------------------------------------
#
# This file defines the modular build structure for the libak library, organizing source files
# into logical modules for efficient compilation and maintenance.
#
# BUILD ORGANIZATION:
# ------------------
# The library is organized into the following modules:
# - alloc: Memory allocation and management
# - base: Core base utilities and data structures
# - json: JSON parsing and manipulation
# - runtime: Runtime system and task management
# - storage: Storage engine and data persistence
# - sync: Synchronization primitives and event handling
#
# Each module consists of header files (API and private) and implementation files.
# The build uses OBJECT libraries for efficient compilation and linking.
#

# ====================================================================================================================
# 1. SOURCE FILE DISCOVERY
# ====================================================================================================================
#
# Automatically discover all source files in the libak source directory
# This ensures that new files are automatically included in the build

# Collect all C++ source files recursively
file(GLOB_RECURSE LIBAK_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp"
)

# Collect all header files for installation
file(GLOB_RECURSE LIBAK_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.h"
)

# ====================================================================================================================
# 2. LIBAK LIBRARY TARGET
# ====================================================================================================================
#
# Create the main libak library target as both static and shared library
# The library provides the core functionality for the AK runtime system

add_library(libak ${LIBAK_SOURCES})
add_library(libak::libak ALIAS libak)

# Set target properties
set_target_properties(libak PROPERTIES
    VERSION ${LIBAK_VERSION}
    SOVERSION ${LIBAK_API_VERSION}
    OUTPUT_NAME "ak"
)

# Include directories
target_include_directories(libak
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${LIBAK_PRIVATE_GENERATED_INCLUDE_DIR}
)

# Compile definitions
target_compile_definitions(libak
    PRIVATE
        LIBAK_BUILDING_LIBRARY
)

# Link dependencies
target_link_libraries(libak
    PUBLIC
        Threads::Threads
    PRIVATE
        PkgConfig::LIBURING
        PkgConfig::BS_THREAD_POOL
        PkgConfig::LUAJIT
)

# Compile options
target_compile_options(libak PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-tautological-constant-out-of-range-compare>
)

# Precompiled header support (if enabled)
if(LIBAK_ENABLE_PCH)
    target_precompile_headers(libak PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/precompiled.hpp"
    )
endif()

# Unity build support (if enabled)
if(LIBAK_ENABLE_UNITY_BUILD)
    set_target_properties(libak PROPERTIES UNITY_BUILD ON)
endif()

# IPO/LTO support (if enabled)
if(LIBAK_ENABLE_IPO)
    set_target_properties(libak PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# ====================================================================================================================
# 3. DEBUG AND ANALYSIS FEATURES
# ====================================================================================================================

# Address Sanitizer
if(LIBAK_ENABLE_ASAN)
    target_compile_options(libak PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(libak PRIVATE -fsanitize=address)
endif()

# Thread Sanitizer
if(LIBAK_ENABLE_TSAN)
    target_compile_options(libak PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
    target_link_options(libak PRIVATE -fsanitize=thread)
endif()

# Undefined Behavior Sanitizer
if(LIBAK_ENABLE_UBSAN)
    target_compile_options(libak PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
    target_link_options(libak PRIVATE -fsanitize=undefined)
endif()

# Code Coverage (GCOV)
if(LIBAK_ENABLE_GCOV)
    target_compile_options(libak PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(libak PRIVATE -fprofile-arcs -ftest-coverage)
endif()

# ====================================================================================================================
# 4. INSTALLATION TARGETS
# ====================================================================================================================
#
# Define installation rules for the library and its headers

# Install the library
install(TARGETS libak
    EXPORT libakTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# ====================================================================================================================
# 5. BUILD INFORMATION
# ====================================================================================================================

message(STATUS "libak library configuration:")
message(STATUS "  Sources: ${LIBAK_SOURCES}")
message(STATUS "  Headers: ${LIBAK_HEADERS}")
message(STATUS "  Version: ${LIBAK_VERSION}")
message(STATUS "  API Version: ${LIBAK_API_VERSION_STRING}")
