cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS -std=gnu99)
set(CMAKE_CXX_FLAGS -std=gnu++11)
set(CMAKE_CXX_FLAGS_RELEASE "-DQT_NO_DEBUG -DQT_NO_DEBUG_OUTPUT")

include(CheckCSourceCompiles)
include(CheckCSourceRuns)

include(FindPkgConfig)

find_package(Qt4 REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GMIME REQUIRED gmime-2.6 glib-2.0)

set(ENABLE_ADDRESS_BOOK false CACHE BOOL "Enable address book code")


add_subdirectory (mailfile)
add_subdirectory (viewmail)

if(${ENABLE_ADDRESS_BOOK})
    pkg_check_modules(YAML REQUIRED yaml-cpp)

    add_subdirectory (addresses)
endif()
