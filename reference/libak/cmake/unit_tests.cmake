# ---------------------------------------------------------------------------------------------------------------------
# libak Unit Testing Framework
# ---------------------------------------------------------------------------------------------------------------------
#
# This file configures the comprehensive unit testing infrastructure for libak using Google Test.
# It automatically discovers and builds all unit tests in the test directory structure.
#
# TEST ORGANIZATION:
# -----------------
# Tests are organized by module:
# - alloc/: Memory allocation tests
# - base/: Core utilities tests
# - json/: JSON parsing tests
# - runtime/: Runtime system tests
# - storage/: Storage engine tests
# - sync/: Synchronization tests
#
# Each test directory contains individual test files that are automatically discovered and built.
#

# ====================================================================================================================
# 1. TEST SOURCE DISCOVERY
# ====================================================================================================================
#
# Automatically discover all test source files

file(GLOB_RECURSE LIBAK_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../test/*.cpp"
)

# ====================================================================================================================
# 2. UNIT TEST EXECUTABLE TARGET
# ====================================================================================================================
#
# Create the main unit test executable that runs all libak tests

add_executable(libak_unit_tests ${LIBAK_TEST_SOURCES})

# Set target properties
set_target_properties(libak_unit_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Include directories
target_include_directories(libak_unit_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../test
        ${LIBAK_PRIVATE_GENERATED_INCLUDE_DIR}
)

# Link dependencies
target_link_libraries(libak_unit_tests
    PRIVATE
        libak::libak
        PkgConfig::GTEST
        PkgConfig::GBENCHMARK
        PkgConfig::GMOCK
        Threads::Threads
)

# Compile options
target_compile_options(libak_unit_tests PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-tautological-constant-out-of-range-compare>
)

# Precompiled header support (if enabled)
if(LIBAK_ENABLE_PCH)
    target_precompile_headers(libak_unit_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/precompiled.hpp"
    )
endif()

# Unity build support (if enabled)
if(LIBAK_ENABLE_UNITY_BUILD)
    set_target_properties(libak_unit_tests PROPERTIES UNITY_BUILD ON)
endif()

# IPO/LTO support (if enabled)
if(LIBAK_ENABLE_IPO)
    set_target_properties(libak_unit_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# ====================================================================================================================
# 3. DEBUG AND ANALYSIS FEATURES
# ====================================================================================================================

# Address Sanitizer
if(LIBAK_ENABLE_ASAN)
    target_compile_options(libak_unit_tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(libak_unit_tests PRIVATE -fsanitize=address)
endif()

# Thread Sanitizer
if(LIBAK_ENABLE_TSAN)
    target_compile_options(libak_unit_tests PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
    target_link_options(libak_unit_tests PRIVATE -fsanitize=thread)
endif()

# Undefined Behavior Sanitizer
if(LIBAK_ENABLE_UBSAN)
    target_compile_options(libak_unit_tests PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
    target_link_options(libak_unit_tests PRIVATE -fsanitize=undefined)
endif()

# Code Coverage (GCOV)
if(LIBAK_ENABLE_GCOV)
    target_compile_options(libak_unit_tests PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(libak_unit_tests PRIVATE -fprofile-arcs -ftest-coverage)
endif()

# ====================================================================================================================
# 4. TEST EXECUTION
# ====================================================================================================================
#
# Add test target for running unit tests

add_test(NAME libak_unit_tests
    COMMAND libak_unit_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Custom target to run tests
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS libak_unit_tests
    COMMENT "Running libak unit tests"
)

# ====================================================================================================================
# 5. BUILD INFORMATION
# ====================================================================================================================

message(STATUS "libak unit tests configuration:")
message(STATUS "  Test sources: ${LIBAK_TEST_SOURCES}")
message(STATUS "  Output: ${CMAKE_BINARY_DIR}/tests/libak_unit_tests")
