cmake_minimum_required(VERSION 4.1)

project(xinnodb_project VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD            23)
set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_EXTENSIONS          OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Ensure compile_commands.json is available at source-relative build/compilation_database
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/build/compilation_database")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/build/compilation_database/compile_commands.json
    RESULT_VARIABLE _cc_symlink_result
  )
  message(STATUS "Compile commands file: ${CMAKE_SOURCE_DIR}/build/compilation_database/compile_commands.json")
endif()

# Ensure consistent stack protector mode across all targets to avoid PCM mismatch
add_compile_options(-fstack-protector-strong)

# Setup testing framework
enable_testing()
find_package(Git      QUIET)
find_package(GTest    CONFIG REQUIRED)
find_package(Doxygen  REQUIRED)
find_package(fmt      12 CONFIG REQUIRED)

find_program(FLEX flex REQUIRED)
find_program(BISON bison REQUIRED)

include(GoogleTest)

# Root folder configuration
set(XINNODB_INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/xinnodb/include)
set(XINNODB_SRC_ROOT     ${CMAKE_SOURCE_DIR}/xinnodb/src)
set(XINNODB_TEST_ROOT    ${CMAKE_SOURCE_DIR}/xinnodb/test)

# Precompiled headers interface target
add_library(xinnodb_pch INTERFACE)
target_precompile_headers(xinnodb_pch INTERFACE
  $<$<COMPILE_LANGUAGE:CXX>:${XINNODB_SRC_ROOT}/defs/include/defs_precompiled.hpp>
)
# Provide fmt headers for the precompiled header that includes <fmt/core.h>
target_link_libraries(xinnodb_pch INTERFACE fmt::fmt-header-only)

# Public headers interface, consumable by all modules and library users
add_library(xinnodb_headers INTERFACE)
target_include_directories(xinnodb_headers INTERFACE ${XINNODB_INCLUDE_ROOT})

# Aggregated libraries composed of per-module object/interface libraries
# Ensure at least one source exists to satisfy CMake when no modules add sources
file(WRITE "${CMAKE_BINARY_DIR}/xinnodb_dummy.cpp" "// generated placeholder\n")

add_library(xinnodb_static STATIC "${CMAKE_BINARY_DIR}/xinnodb_dummy.cpp")
target_link_libraries(xinnodb_static PRIVATE xinnodb_pch)
target_link_libraries(xinnodb_static PUBLIC xinnodb_headers)

add_library(xinnodb_shared SHARED "${CMAKE_BINARY_DIR}/xinnodb_dummy.cpp")
target_link_libraries(xinnodb_shared PUBLIC xinnodb_headers)
target_link_libraries(xinnodb_shared PRIVATE xinnodb_pch)

# Versioned output names
set_target_properties(xinnodb_shared PROPERTIES
    OUTPUT_NAME "xinnodb.${PROJECT_VERSION}"
)
set_target_properties(xinnodb_static PROPERTIES
    OUTPUT_NAME "xinnodb.${PROJECT_VERSION}"
)

# Collect all unit tests in a single runner

if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")
  # Provide legacy variable name used by templates
  set(IB_GIT_COMMIT_HASH "${GIT_COMMIT_HASH}")
else()
    message(FATAL_ERROR "Git not found")
endif()

# ---------------------------------------------------------------------------------
# Function to register a module and its unit tests
# ---------------------------------------------------------------------------------

set(XINNODB_ALL_TESTS)
set(XINNODB_TEST_INCLUDE_DIRS)
set(XINNODB_INTERNAL_DOCS_FOLDERS "${CMAKE_SOURCE_DIR}/xinnodb/include/")

function(xinnodb_component MODULE_NAME)

    # Parse optional dependencies as: xinnodb_component(<name> DEPS dep1 dep2 ...)
    set(oneValueArgs)
    set(multiValueArgs DEPS)
    cmake_parse_arguments(XMODULE "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(module_src_dir               "${XINNODB_SRC_ROOT}/${MODULE_NAME}")
    set(module_include_dir           "${module_src_dir}/include")
    set(module_generated_include_dir "${CMAKE_BINARY_DIR}/generated/${MODULE_NAME}/include")
    set(module_generated_src_dir     "${CMAKE_BINARY_DIR}/generated/${MODULE_NAME}/src")
    set(module_impl_src_dir          "${module_src_dir}/src")
    set(module_test_src_dir          "${module_src_dir}/test")
    
    # Generate .in source files
    file(GLOB_RECURSE component_src_in_files CONFIGURE_DEPENDS "${module_impl_src_dir}/*.cpp.in")
    foreach(in_file IN LISTS component_src_in_files)
        file(RELATIVE_PATH rel "${module_impl_src_dir}" "${in_file}")
        set(out_file "${module_generated_src_dir}/${rel}")
        string(REGEX REPLACE "\\.cpp\\.in$" ".cpp" out_file "${out_file}")
        get_filename_component(out_dir "${out_file}" DIRECTORY)
        file(MAKE_DIRECTORY "${out_dir}")
        configure_file("${in_file}" "${out_file}" @ONLY)
    endforeach()

    # Generate .in header files
    # TODO

    # Define the module library either as a header only library or an object library
    file(GLOB module_src_files CONFIGURE_DEPENDS "${module_impl_src_dir}/*.cpp")

    # Exclude sources that have a corresponding .cpp.in (we'll use generated ones)
    set(_pruned_module_src_files)
    foreach(_src IN LISTS module_src_files)
        if(EXISTS "${_src}.in")
            # Skip: generated counterpart will be used
        else()
            list(APPEND _pruned_module_src_files "${_src}")
        endif()
    endforeach()

    set(module_src_files ${_pruned_module_src_files})
    file(GLOB_RECURSE module_generated_src_files CONFIGURE_DEPENDS "${module_generated_src_dir}/*.cpp")
    list(APPEND module_src_files ${module_generated_src_files})
    if(module_src_files)
        set(module_object_library "${MODULE_NAME}_obj_lib")
        add_library(${module_object_library} OBJECT ${module_src_files})
        target_link_libraries(${module_object_library} PRIVATE xinnodb_headers)
        target_link_libraries(${module_object_library} PRIVATE fmt::fmt-header-only)
        target_link_libraries(${module_object_library} PRIVATE xinnodb_pch)
        target_include_directories(${module_object_library} PRIVATE ${module_include_dir} ${XINNODB_INCLUDE_ROOT} ${module_generated_include_dir})
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

    # Add Doxygen internal documentation    
    list(APPEND XINNODB_INTERNAL_DOCS_FOLDERS "${module_include_dir}" )
    # Propagate updated list to parent so subsequent modules see the accumulation
    set(XINNODB_INTERNAL_DOCS_FOLDERS "${XINNODB_INTERNAL_DOCS_FOLDERS}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------------
# XInnoDB component definitions
# ---------------------------------------------------------------------------------

xinnodb_component(defs)
xinnodb_component(ut)
xinnodb_component(io    DEPS defs)
xinnodb_component(alloc DEPS defs ut)

# ---------------------------------------------------------------------------------
# XInnoDB documentation generation
# ---------------------------------------------------------------------------------

set(DOXYGEN_PROJECT_NAME       "XInnoDB")
set(DOXYGEN_GENERATE_HTML      YES)  # Generate HTML documentation
set(DOXYGEN_GENERATE_LATEX     NO)   # Skip LaTeX/PDF generation for faster builds
set(DOXYGEN_RECURSIVE          YES)  # Recursively parse all source files
set(DOXYGEN_GENERATE_XML       YES)  # Generate XML documentation
set(DOXYGEN_INTERNAL_DOCS      YES)
set(DOXYGEN_EXTRACT_STATIC     YES) 
set(DOXYGEN_EXTRACT_ALL        YES)
set(DOXYGEN_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/doc/public")
set(DOXYGEN_LAYOUT_FILE       "${CMAKE_SOURCE_DIR}/xinnodb/docs/DoxygenLayout.xml")
set(DOXYGEN_GENERATE_GROUPS   YES)
doxygen_add_docs(doc "${XINNODB_INTERNAL_DOCS_FOLDERS}")

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
    set_target_properties(xinnodb_tests PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    target_include_directories(xinnodb_tests
        PRIVATE ${XINNODB_INCLUDE_ROOT}
        PRIVATE ${XINNODB_SRC_ROOT}           # allow including module headers like "io.hpp"
        PRIVATE ${XINNODB_TEST_INCLUDE_DIRS}
    )
    # Unit tests should only use the static library
    target_link_libraries(xinnodb_tests PRIVATE xinnodb_static GTest::gtest_main fmt::fmt-header-only)
    target_link_libraries(xinnodb_tests PRIVATE xinnodb_pch)
    gtest_discover_tests(xinnodb_tests)
    install(TARGETS xinnodb_tests
        RUNTIME DESTINATION bin
        COMPONENT dev
    )
endif()

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS xinnodb_tests
  COMMENT "Running unit tests"
)

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

# Install Doxygen documentation (if generated)
install(
    DIRECTORY   "${CMAKE_BINARY_DIR}/doc/public/html"
    DESTINATION "doc"
    COMPONENT   dev
)



