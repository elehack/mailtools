set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NOTMUCH_LIBRARY_PATH "" CACHE PATH "Path to libnotmuch")
find_package(TCL REQUIRED)
include(CheckLibraryExists)
check_library_exists(notmuch notmuch_database_open "${NOTMUCH_LIBRARY_PATH}" NOTMUCH_LIB)
include(CheckIncludeFile)
check_include_file(notmuch.h NOTMUCH_H)

include_directories (BEFORE ${TCL_INCLUDE_PATH} ${GLIB_INCLUDE_DIRS})
include_directories (BEFORE ${Project_SOURCE_DIR}/util)

set(common_src filterscript.c message.c strutil.c)

set(app_libs mt-utils ${TCL_LIBRARY} -lnotmuch -lpopt ${GLIB_LIBRARIES})

add_library (mf-internals ${common_src})

add_executable (mailfile mailfile.c)
target_link_libraries(mailfile mf-internals ${app_libs})
install (TARGETS mailfile DESTINATION bin)

file(GLOB test_src test_*.c)

add_executable(run-tests run-tests.c ${test_src})
target_link_libraries(run-tests mf-internals ${app_libs})

enable_testing()
add_test(NAME run-tests COMMAND run-tests)
