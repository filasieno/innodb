# ---------------------------------------------------------------------------------------------------------------------
# libak Installation Configuration
# ---------------------------------------------------------------------------------------------------------------------
#
# This file defines the installation configuration for the libak library, including
# CMake package configuration for downstream consumers.
#
# INSTALLATION COMPONENTS:
# -----------------------
# - Library binaries (static and shared)
# - Header files
# - CMake configuration files
# - Documentation (if enabled)
#
# The installation follows standard CMake conventions and provides both development
# and runtime installation options.
#

# ====================================================================================================================
# 1. CMAKE PACKAGE CONFIGURATION
# ====================================================================================================================
#
# Generate CMake package configuration files for find_package(libak)

include(CMakePackageConfigHelpers)

# Generate version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libakConfigVersion.cmake"
    VERSION ${LIBAK_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Generate config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libakConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libakConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libak
    PATH_VARS
        CMAKE_INSTALL_INCLUDEDIR
        CMAKE_INSTALL_LIBDIR
)

# ====================================================================================================================
# 2. INSTALLATION TARGETS
# ====================================================================================================================
#
# Install CMake configuration files

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/libakConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/libakConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libak
)

# Install CMake target files
install(EXPORT libakTargets
    FILE libakTargets.cmake
    NAMESPACE libak::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libak
)

# ====================================================================================================================
# 3. UNINSTALL TARGET
# ====================================================================================================================
#
# Provide uninstall target for development convenience

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
        COMMENT "Uninstalling libak"
    )
endif()

# ====================================================================================================================
# 4. INSTALLATION SUMMARY
# ====================================================================================================================

message(STATUS "libak installation configuration:")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Library directory: ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "  Include directory: ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "  CMake config directory: ${CMAKE_INSTALL_LIBDIR}/cmake/libak")
