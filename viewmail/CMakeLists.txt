set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories (BEFORE ${MUSTACHE_SOURCE_DIR})
QT4_ADD_RESOURCES (VIEWMAIL_RCC_SRCS assets.qrc)

set(src main.cpp mailview.cpp htmlmail.cpp mailnetworkmanager.cpp statichttpreply.cpp ${VIEWMAIL_RCC_SRCS})
add_executable (viewmail ${src})

target_link_libraries(viewmail Qt4::QtGui Qt4::QtNetwork Qt4::QtWebKit ${VMIME_LIBRARIES} mustache)

install (TARGETS viewmail DESTINATION bin)
