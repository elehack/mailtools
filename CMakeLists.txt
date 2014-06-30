cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS -std=gnu++11)

include(FindPkgConfig)

find_package(Qt4 REQUIRED)
pkg_check_modules(VMIME REQUIRED vmime)
pkg_check_modules(YAML REQUIRED yaml-cpp)

add_library (mustache mustache/src/mustache.cpp)

include_directories ("${PROJECT_SOURCE_DIR}/viewmail", "${PROJECT_SOURCE_DIR}/addresses")
add_subdirectory (addresses)
add_subdirectory (viewmail)
