cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS -std=gnu++11)

include(FindPkgConfig)

find_package(Qt4 REQUIRED)
pkg_check_modules(VMIME REQUIRED vmime)

include_directories ("${PROJECT_SOURCE_DIR}/viewmail")
add_subdirectory (viewmail)
