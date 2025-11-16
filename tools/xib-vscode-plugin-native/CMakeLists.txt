cmake_minimum_required(VERSION 3.18)

project(xib_vscode_plugin_native LANGUAGES C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Allow passing NODE_INCLUDE_DIR; default to node include path in Nix dev shell if provided
if(NOT DEFINED NODE_INCLUDE_DIR AND DEFINED ENV{NODE_INCLUDE_DIR})
  set(NODE_INCLUDE_DIR "$ENV{NODE_INCLUDE_DIR}")
endif()

if(NOT NODE_INCLUDE_DIR)
  message(STATUS "NODE_INCLUDE_DIR not provided; attempting common locations")
  find_path(NODE_INCLUDE_DIR
    NAMES node_api.h
    PATH_SUFFIXES node include/node
  )
endif()

if(NOT NODE_INCLUDE_DIR)
  message(FATAL_ERROR "Could not locate Node-API headers (node_api.h). Set NODE_INCLUDE_DIR to Node's include directory.")
endif()

message(STATUS "Using NODE_INCLUDE_DIR=${NODE_INCLUDE_DIR}")

add_library(addon MODULE
  src/addon.c
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

# Install the built addon into lib/ for consumption by other packages
install(FILES
  "${_addon_out}/addon.node"
  DESTINATION lib
)