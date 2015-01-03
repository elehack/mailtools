set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (src maildir.c logging.c)

add_library (mt-utils ${src})
