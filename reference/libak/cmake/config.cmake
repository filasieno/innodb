# libak CMake Configuration File
#
# This file configures libak for use with find_package() in CMake projects.
# It defines the libak::libak target and sets up all necessary dependencies.

@PACKAGE_INIT@

# Set up import targets
include("${CMAKE_CURRENT_LIST_DIR}/libakTargets.cmake")

# Check required components
check_required_components(libak)

# Set version information
set(libak_VERSION       "@LIBAK_VERSION@")
set(libak_VERSION_MAJOR "@LIBAK_API_VERSION@")
set(libak_VERSION_MINOR "@LIBAK_API_VERSION_REVISION@")
set(libak_VERSION_PATCH "@LIBAK_API_VERSION_AGE@")

# Provide legacy variables for backwards compatibility
set(LIBAK_LIBRARIES    libak::libak)
set(LIBAK_INCLUDE_DIRS "@CMAKE_INSTALL_FULL_INCLUDEDIR@")
set(LIBAK_LIBRARY_DIRS "@CMAKE_INSTALL_FULL_LIBDIR@")

# Set found variable
set(libak_FOUND TRUE)
