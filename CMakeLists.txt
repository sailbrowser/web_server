cmake_minimum_required( VERSION 2.6 )
set( BIN final )
project( ${BIN} )
# libev
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set( SOURCE_ROOT ./src )
aux_source_directory( ${SOURCE_ROOT} SOURCES )
find_package( Libev REQUIRED )
include_directories( ${LIBEV_INCLUDE_DIR} )
# set (LIBS "-pthread")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -pedantic -std=c++11")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -std=c99")
# set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_POSIX_C_SOURCE=200809L")

add_executable( ${BIN} ${SOURCES} )
target_link_libraries( ${BIN} ${LIBEV_LIBRARY} )
