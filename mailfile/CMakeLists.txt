set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NOTMUCH_LIBRARY_PATH "" CACHE PATH "Path to libnotmuch")
find_package(TCL)
include(CheckLibraryExists)
check_library_exists(notmuch notmuch_database_open "${NOTMUCH_LIBRARY_PATH}" NOTMUCH_LIB)
include(CheckIncludeFile)
check_include_file(notmuch.h NOTMUCH_H)

include_directories (BEFORE ${TCL_INCLUDE_PATH} ${GLIB_INCLUDE_DIRS})
set(src mailfile.c filterscript.c message.c logging.c)

add_executable (mailfile ${src})
target_link_libraries(mailfile ${TCL_LIBRARY} -lnotmuch -lpopt ${GLIB_LIBRARIES})
