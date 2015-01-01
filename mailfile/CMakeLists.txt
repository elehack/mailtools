set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(TCL)

include_directories (BEFORE ${TCL_INCLUDE_PATH} ${GLIB_INCLUDE_DIRS})
set(src mailfile.c filterscript.c message.c logging.c)

add_executable (mailfile ${src})
target_link_libraries(mailfile ${TCL_LIBRARY} -lnotmuch -lpopt ${GLIB_LIBRARIES})
