cmake_minimum_required( VERSION 2.6 )
set( BIN final )
project( ${BIN} )
# libev
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set( SOURCE_ROOT ./src )
aux_source_directory( ${SOURCE_ROOT} SOURCES )
find_package(Libev REQUIRED)
include_directories(${LIBEV_INCLUDE_DIR})

set( CMAKE_CXX_FLAGS "-O2 -Wall -pedantic -std=c++11" )

add_executable( ${BIN} ${SOURCES} )
target_link_libraries(${BIN} ${LIBEV_LIBRARY} -pthread )
