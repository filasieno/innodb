cmake_minimum_required(VERSION 4.1)

project(cps_parser_tool
    VERSION 0.1.0
    DESCRIPTION "CPS grammar for tree-sitter"
    LANGUAGES C CXX
)

# Find tree-sitter CLI for parser generation
find_program(TREE_SITTER_CLI tree-sitter DOC "Tree-sitter CLI")
if(NOT TREE_SITTER_CLI)
    message(FATAL_ERROR "Tree-sitter CLI not found. Please install it.")
endif()

# Tree-sitter ABI version
set(TREE_SITTER_ABI_VERSION 15 CACHE STRING "Tree-sitter ABI version")

# Generate parser.c from grammar.json
add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/grammar.json"
    COMMAND "${TREE_SITTER_CLI}" generate src/grammar.json --abi=${TREE_SITTER_ABI_VERSION}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating parser.c from CPS grammar")

add_custom_target(generate_cps_parser DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c")

# Find tree-sitter library
find_package(PkgConfig REQUIRED)
pkg_check_modules(TREE_SITTER REQUIRED tree-sitter)

# CPS parser libraries (both static and shared)
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c"
    PROPERTIES
    LANGUAGE C
    GENERATED TRUE
)

# Static library
add_library(tree-sitter-cps-static STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c")
add_dependencies(tree-sitter-cps-static generate_cps_parser)
set_target_properties(tree-sitter-cps-static PROPERTIES
    OUTPUT_NAME tree-sitter-cps
    POSITION_INDEPENDENT_CODE ON
)

# Shared library
add_library(tree-sitter-cps SHARED "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c")
add_dependencies(tree-sitter-cps generate_cps_parser)
set_target_properties(tree-sitter-cps PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# Common target properties
foreach(target tree-sitter-cps-static tree-sitter-cps)
    target_include_directories(${target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bindings/c>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src"
            ${TREE_SITTER_INCLUDE_DIRS}
    )
    target_link_libraries(${target} PUBLIC ${TREE_SITTER_LIBRARIES})
endforeach()

# CPS parser test executable
find_package(GTest REQUIRED)

add_executable(cps_parser_test "${CMAKE_CURRENT_SOURCE_DIR}/test/src/cps_parser_test.cpp")

target_include_directories(cps_parser_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/bindings/c"
    ${TREE_SITTER_INCLUDE_DIRS}
)

target_link_libraries(cps_parser_test PRIVATE
    tree-sitter-cps  # Links to shared library
    GTest::gtest_main
    GTest::gtest
    pthread
)

# No test data copying needed - all tests are in-memory

# Enable testing
enable_testing()
add_test(NAME cps_parser_test COMMAND cps_parser_test)

# Install rules
install(TARGETS tree-sitter-cps tree-sitter-cps-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bindings/c/tree_sitter"
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install pkg-config file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/bindings/c/tree-sitter-cps.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tree-sitter-cps.pc"
    @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tree-sitter-cps.pc"
    DESTINATION lib/pkgconfig
)
