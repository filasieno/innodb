cmake_minimum_required(VERSION 4.1)

project(xib_vscode_plugin_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# find_package(GTest CONFIG REQUIRED)

# enable_testing()

# Allow passing NODE_INCLUDE_DIR; default to node include path in Nix dev shell if provided
if(NOT DEFINED NODE_INCLUDE_DIR AND DEFINED ENV{NODE_INCLUDE_DIR})
  set(NODE_INCLUDE_DIR "$ENV{NODE_INCLUDE_DIR}")
endif()

if(NOT NODE_INCLUDE_DIR)
  # Try pkg-config first
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(NODE QUIET node)
    if(NODE_FOUND)
      set(NODE_INCLUDE_DIR ${NODE_INCLUDE_DIRS})
      message(STATUS "Found Node-API via pkg-config: ${NODE_INCLUDE_DIR}")
    endif()
  endif()

  # Fallback to manual search
  if(NOT NODE_INCLUDE_DIR)
    message(STATUS "NODE_INCLUDE_DIR not provided and pkg-config failed; attempting common locations")
    find_path(NODE_INCLUDE_DIR
      NAMES node_api.h
      PATH_SUFFIXES node include/node
    )
  endif()
endif()

if(NOT NODE_INCLUDE_DIR)
  message(FATAL_ERROR "Could not locate Node-API headers (node_api.h). Set NODE_INCLUDE_DIR to Node's include directory or install node.pc.")
endif()

message(STATUS "Using NODE_INCLUDE_DIR=${NODE_INCLUDE_DIR}")

add_library(addon MODULE
  src/ib_node_plugin.cpp
)

target_include_directories(addon PRIVATE
  ${NODE_INCLUDE_DIR}
)

# Produce a .node binary with no 'lib' prefix
set_target_properties(addon PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)

# Output directory for the addon
set(_addon_out "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(addon PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${_addon_out}"
  RUNTIME_OUTPUT_DIRECTORY "${_addon_out}"
  ARCHIVE_OUTPUT_DIRECTORY "${_addon_out}"
)

# Test executable
# add_executable(test_addon
#   test/test_addon.cpp
# )

# target_link_libraries(test_addon
#   GTest::gtest_main
# )

# add_test(NAME addon_tests COMMAND test_addon)

# Install the built addon into lib/ for consumption by other packages
install(TARGETS addon
  LIBRARY DESTINATION lib
)