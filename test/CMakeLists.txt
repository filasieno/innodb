cmake_minimum_required(VERSION 3.14)

# CPS Parser Tests
project(cps_parser_tests)

# Find tree-sitter
find_package(PkgConfig REQUIRED)
pkg_check_modules(TREE_SITTER REQUIRED tree-sitter)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../bindings/c
    ${TREE_SITTER_INCLUDE_DIRS}
)

# Link directories
link_directories(${TREE_SITTER_LIBRARY_DIRS})

# Test executable
add_executable(cps_parser_test src/cps_parser_test.cpp)

# Link libraries
target_link_libraries(cps_parser_test
    ${TREE_SITTER_LIBRARIES}
    gtest
    gtest_main
    pthread
)

# Enable testing
enable_testing()
add_test(NAME cps_parser_test COMMAND cps_parser_test)

# Copy test data to build directory
add_custom_command(TARGET cps_parser_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:cps_parser_test>/data
)
