cmake_minimum_required(VERSION 3.14)

project(tree-sitter-cps
        VERSION "0.1.0"
        DESCRIPTION "Cps grammar for tree-sitter"
        HOMEPAGE_URL "https://github.com/tree-sitter/tree-sitter-cps"
        LANGUAGES C CXX)

option(BUILD_SHARED_LIBS           "Build using shared libraries" ON)
option(TREE_SITTER_REUSE_ALLOCATOR "Reuse the library allocator"  OFF)
option(BUILD_TESTS                 "Build tests" ON)

set(TREE_SITTER_ABI_VERSION 15 CACHE STRING "Tree-sitter ABI version")

if(NOT ${TREE_SITTER_ABI_VERSION} MATCHES "^[0-9]+$")
    unset(TREE_SITTER_ABI_VERSION CACHE)
    message(FATAL_ERROR "TREE_SITTER_ABI_VERSION must be an integer")
endif()

include(GNUInstallDirs)

find_program(TREE_SITTER_CLI tree-sitter DOC "Tree-sitter CLI")

add_custom_command(OUTPUT            "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c"
                   DEPENDS           "${CMAKE_CURRENT_SOURCE_DIR}/src/grammar.json"
                   COMMAND           "${TREE_SITTER_CLI}" generate src/grammar.json --abi=${TREE_SITTER_ABI_VERSION}
                   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                   COMMENT           "Generating parser.c")

add_custom_target(generate_parser DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c")

add_library(tree-sitter-cps src/parser.c)
add_dependencies(tree-sitter-cps generate_parser)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scanner.c)
  target_sources(tree-sitter-cps PRIVATE src/scanner.c)
endif()

target_include_directories(tree-sitter-cps
    PRIVATE   src
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bindings/c>
    	      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_definitions(tree-sitter-cps PRIVATE
	$<$<BOOL:${TREE_SITTER_REUSE_ALLOCATOR}>:TREE_SITTER_REUSE_ALLOCATOR>
    $<$<CONFIG:Debug>:TREE_SITTER_DEBUG>)

set_target_properties(tree-sitter-cps
    PROPERTIES
    C_STANDARD 11
    POSITION_INDEPENDENT_CODE ON
    SOVERSION "${TREE_SITTER_ABI_VERSION}.${PROJECT_VERSION_MAJOR}"
    DEFINE_SYMBOL "")

configure_file(bindings/c/tree-sitter-cps.pc.in
    "${CMAKE_CURRENT_BINARY_DIR}/tree-sitter-cps.pc" @ONLY)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bindings/c/tree_sitter"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING PATTERN "*.h")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tree-sitter-cps.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
install(TARGETS tree-sitter-cps
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

file(GLOB QUERIES queries/*.scm)
install(
    FILES ${QUERIES}
    DESTINATION "${CMAKE_INSTALL_DATADIR}/tree-sitter/queries/cps")

add_custom_target(ts-test "${TREE_SITTER_CLI}" test
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "tree-sitter test")

add_dependencies(ts-test generate_parser)
# # Tests
# if(BUILD_TESTS)
#     # Force Ninja generator for faster builds
#     if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
#         message(FATAL_ERROR "This project requires Ninja generator. Please run: cmake -G Ninja ..")
#     endif()

#     # Find dependencies
#     find_package(GTest REQUIRED)

#     # Try to find tree-sitter using pkg-config, but fall back to manual configuration
#     find_package(PkgConfig QUIET)
#     if(PkgConfig_FOUND)
#         pkg_check_modules(TREE_SITTER QUIET tree-sitter)
#     endif()

#     # If pkg-config didn't find tree-sitter, set manual paths
#     if(NOT TREE_SITTER_FOUND)
#         set(TREE_SITTER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/bindings/c" "/nix/store/2hcp80sh45psnz1i014806np18m91yg5-tree-sitter-0.25.10/include")
#         set(TREE_SITTER_LIBRARIES "/nix/store/2hcp80sh45psnz1i014806np18m91yg5-tree-sitter-0.25.10/lib/libtree-sitter.so")
#         message(STATUS "Using manual tree-sitter configuration")
#     endif()

#     # Test executable
#     add_executable(cps_parser_test test/src/cps_parser_test.cpp)

#     # Include directories
#     target_include_directories(cps_parser_test PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/src
#         ${CMAKE_CURRENT_SOURCE_DIR}/bindings/c
#         ${TREE_SITTER_INCLUDE_DIRS}
#     )

#     # Also include the tree_sitter subdirectory
#     target_include_directories(cps_parser_test PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/bindings/c/tree_sitter
#     )

#     # Link libraries
#     target_link_libraries(cps_parser_test PRIVATE
#         tree-sitter-cps
#         ${TREE_SITTER_LIBRARIES}
#         GTest::gtest_main
#         GTest::gtest
#         pthread
#     )

#     # Enable testing and configure CTest
#     enable_testing()

#     # Ensure actual directory exists in build folder
#     add_custom_target(create_actual_dir ALL
#         COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/actual
#         COMMENT "Creating actual output directory"
#     )

#     # Ensure dependencies are met before building the test executable
#     add_dependencies(cps_parser_test create_actual_dir generate_parser)

#     # Add the main test
#     add_test(
#         NAME cps_parser_test
#         COMMAND cps_parser_test
#         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     )

#     # Set test properties
#     set_tests_properties(cps_parser_test PROPERTIES
#         TIMEOUT 30
#         LABELS "parser;unit"
#         ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}"
#     )

#     # Add a convenience target to run tests
#     add_custom_target(run_tests
#         COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
#         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#         COMMENT "Running CPS parser tests"
#         USES_TERMINAL
#     )

#     # Add target to run tests with verbose output
#     add_custom_target(run_tests_verbose
#         COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -V
#         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#         COMMENT "Running CPS parser tests with verbose output"
#         USES_TERMINAL
#     )
# endif()

