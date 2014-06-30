add_library(addrbook addressbook.cpp)
target_link_libraries(addrbook Qt4::QtCore ${YAML_LIBRARIES})
