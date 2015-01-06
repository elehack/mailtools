set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NOTMUCH_INCLUDE_PATH "" CACHE PATH "Path to notmuch.h")

find_package(TCL)
if(NOT TCL_FOUND)
    message(SEND_ERROR "TCL was not found")
endif()

find_library(NOTMUCH_LIBRARY notmuch
    PATHS /usr/lib /usr/local/lib
    DOC "Path to libnotmuch")
find_library(POPT_LIBRARY popt
    PATHS /usr/lib /usr/local/lib
    DOC "Path to libpopt")

if(NOT NOTMUCH_LIBRARY)
    message(SEND_ERROR "libnotmuch was not found")
endif()
if(NOT POPT_LIBRARY)
    message(SEND_ERROR "libpopt was not found")
endif()

link_directories (${GLIB_LIBRARY_DIRS})
include_directories (BEFORE ${NOTMUCH_INCLUDE_PATH} ${TCL_INCLUDE_PATH} ${GLIB_INCLUDE_DIRS})

include(CheckIncludeFile)
check_include_file(notmuch.h NOTMUCH_H)
if(NOT NOTMUCH_H_FOUND)
    message(WARNING "notmuch.h was not found")
endif()

include_directories (BEFORE ${Project_SOURCE_DIR}/util)
set(common_src filterscript.c message.c strutil.c)

set(app_libs mt-utils ${TCL_LIBRARY} ${NOTMUCH_LIBRARY} ${POPT_LIBRARY} ${GLIB_LIBRARIES})

add_library (mf-internals ${common_src})

add_executable (mailfile mailfile.c)
target_link_libraries(mailfile mf-internals ${app_libs})
install (TARGETS mailfile DESTINATION bin)

file(GLOB test_src test_*.c)

add_executable(run-tests run-tests.c ${test_src})
target_link_libraries(run-tests mf-internals ${app_libs})

add_test(NAME run-tests COMMAND run-tests)
