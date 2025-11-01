cmake_minimum_required(VERSION 3.29)

project(xinnodb_project VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD            23)
set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_EXTENSIONS          OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Keep compile_commands.json at repo root for clangd on each configure
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    RESULT_VARIABLE _cc_symlink_result
  )
endif()

# Ensure consistent stack protector mode across all targets to avoid PCM mismatch
add_compile_options(-fstack-protector-strong)

# Setup testing framework
enable_testing()
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

# Root folder configuration
set(XINNODB_INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/xinnodb/include)
set(XINNODB_SRC_ROOT     ${CMAKE_SOURCE_DIR}/xinnodb/src)
set(XINNODB_TEST_ROOT    ${CMAKE_SOURCE_DIR}/xinnodb/test)

# Public headers interface, consumable by all modules and library users
add_library(xinnodb_headers INTERFACE)
target_include_directories(xinnodb_headers INTERFACE ${XINNODB_INCLUDE_ROOT})

# Aggregated libraries composed of per-module object/interface libraries
# Ensure at least one source exists to satisfy CMake when no modules add sources
file(WRITE "${CMAKE_BINARY_DIR}/xinnodb_dummy.cpp" "// generated placeholder\n")
add_library(xinnodb_static STATIC "${CMAKE_BINARY_DIR}/xinnodb_dummy.cpp")
add_library(xinnodb_shared SHARED "${CMAKE_BINARY_DIR}/xinnodb_dummy.cpp")
target_link_libraries(xinnodb_static PUBLIC xinnodb_headers)
target_link_libraries(xinnodb_shared PUBLIC xinnodb_headers)

# Versioned output names
set_target_properties(xinnodb_shared PROPERTIES
    OUTPUT_NAME "xinnodb.${PROJECT_VERSION}"
)
set_target_properties(xinnodb_static PROPERTIES
    OUTPUT_NAME "xinnodb.${PROJECT_VERSION}"
)

# Collect all unit tests in a single runner
set(XINNODB_ALL_TESTS)
set(XINNODB_TEST_INCLUDE_DIRS)

# Function to register a module and its unit tests
function(xinnodb_module MODULE_NAME)

    # Parse optional dependencies as: xinnodb_module(<name> DEPS dep1 dep2 ...)
    set(oneValueArgs)
    set(multiValueArgs DEPS)
    cmake_parse_arguments(XMODULE "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(module_src_dir        "${XINNODB_SRC_ROOT}/${MODULE_NAME}")
    set(module_include_dir    "${module_src_dir}/include")
    set(module_impl_src_dir   "${module_src_dir}/src")
    set(module_test_src_dir   "${module_src_dir}/test")
    set(module_object_library "${MODULE_NAME}_obj_lib")

    # Define the module library either as a header only library or an object library
    file(GLOB module_src_files CONFIGURE_DEPENDS "${module_impl_src_dir}/*.cpp")
    if(module_src_files)
        add_library(${module_object_library} OBJECT ${module_src_files})
        target_link_libraries(${module_object_library} PRIVATE xinnodb_headers)
        target_include_directories(${module_object_library} PRIVATE ${module_include_dir} ${XINNODB_INCLUDE_ROOT})
        # Add include paths for declared dependencies
        foreach(dep IN LISTS XMODULE_DEPS)
            target_include_directories(${module_object_library} PRIVATE "${XINNODB_SRC_ROOT}/${dep}/include")
        endforeach()

        # Add to the aggregated libraries
        target_sources(xinnodb_static PRIVATE $<TARGET_OBJECTS:${module_object_library}>)
        target_sources(xinnodb_shared PRIVATE $<TARGET_OBJECTS:${module_object_library}>)
    else()
        # Header-only module: create an interface library to propagate includes
        set(module_interface_library "${MODULE_NAME}_headers")
        add_library(${module_interface_library} INTERFACE)
        target_include_directories(${module_interface_library} INTERFACE ${module_include_dir})
        # Propagate dependency include paths to consumers
        foreach(dep IN LISTS XMODULE_DEPS)
            target_include_directories(${module_interface_library} INTERFACE "${XINNODB_SRC_ROOT}/${dep}/include")
        endforeach()
        target_link_libraries(${module_interface_library} INTERFACE xinnodb_headers)
        # Link interface into aggregated libraries so consumers inherit includes
        target_link_libraries(xinnodb_static PUBLIC ${module_interface_library})
        target_link_libraries(xinnodb_shared PUBLIC ${module_interface_library})
    endif()

    # Module unit tests (optional) — accumulate into a single test target
    file(GLOB module_test_srcs CONFIGURE_DEPENDS "${module_test_src_dir}/*.cpp")
    if(module_test_srcs)
        list(APPEND XINNODB_ALL_TESTS ${module_test_srcs})
        list(APPEND XINNODB_TEST_INCLUDE_DIRS ${module_include_dir})
        # Tests also need include paths from dependencies
        foreach(dep IN LISTS XMODULE_DEPS)
            list(APPEND XINNODB_TEST_INCLUDE_DIRS "${XINNODB_SRC_ROOT}/${dep}/include")
        endforeach()
        list(APPEND XINNODB_TEST_INCLUDE_DIRS ${module_test_src_dir})
        # Update the global variables
        set(XINNODB_ALL_TESTS         "${XINNODB_ALL_TESTS}"         PARENT_SCOPE)
        set(XINNODB_TEST_INCLUDE_DIRS "${XINNODB_TEST_INCLUDE_DIRS}" PARENT_SCOPE)
    endif()

    # Install module public headers from its include/ directory
    if(EXISTS "${module_include_dir}")
      install(
        DIRECTORY "${module_include_dir}/"
        DESTINATION "include/xinnodb/${MODULE_NAME}"
        COMPONENT dev
        FILES_MATCHING
          PATTERN "*.hpp"
          PATTERN "*.h"
          PATTERN "*.inl"
      )
    endif()
endfunction()

# ---------------------------------------------------------------------------------
# XInnoDB standard module definitions
# ---------------------------------------------------------------------------------
xinnodb_module(defs)
xinnodb_module(io DEPS defs)
xinnodb_module(ut)


# ---------------------------------------------------------------------------------
# Definition of the single unit test executable
# ---------------------------------------------------------------------------------
if(XINNODB_ALL_TESTS)
    list(REMOVE_DUPLICATES XINNODB_TEST_INCLUDE_DIRS)
    message(STATUS "Unit test sources:")
    foreach(test_src IN LISTS XINNODB_ALL_TESTS)
        message(STATUS "  ${test_src}")
    endforeach()
    add_executable(xinnodb_tests ${XINNODB_ALL_TESTS})
    target_include_directories(xinnodb_tests
        PRIVATE ${XINNODB_INCLUDE_ROOT}
        PRIVATE ${XINNODB_SRC_ROOT}           # allow including module headers like "io.hpp"
        PRIVATE ${XINNODB_TEST_INCLUDE_DIRS}
    )
    # Unit tests should only use the static library
    target_link_libraries(xinnodb_tests PRIVATE xinnodb_static GTest::gtest_main)
    gtest_discover_tests(xinnodb_tests)
    install(TARGETS xinnodb_tests
        RUNTIME DESTINATION bin
        COMPONENT dev
    )
endif()

# ---------------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------------
install(TARGETS xinnodb_shared
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
install(FILES $<TARGET_FILE:xinnodb_shared>
    DESTINATION lib
    COMPONENT runtime
)
install(TARGETS xinnodb_static
    ARCHIVE DESTINATION lib
    COMPONENT dev
)
install(
    DIRECTORY "${XINNODB_INCLUDE_ROOT}/"
    DESTINATION include
    COMPONENT dev
    FILES_MATCHING
      PATTERN "*.hpp"
      PATTERN "*.h"
)


