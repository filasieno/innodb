cmake_minimum_required(VERSION 3.20)
project(xinnodb VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
include(CTest)
enable_testing()

# Add Google Test
find_package(GTest REQUIRED)

# Add subdirectories
add_subdirectory(src/defs)
add_subdirectory(src/io)
add_subdirectory(src/ut)
