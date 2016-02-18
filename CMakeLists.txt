cmake_minimum_required( VERSION 2.6 )
set( BIN final )
project( ${BIN} )

set( SOURCE_ROOT ./src )
aux_source_directory( ${SOURCE_ROOT} SOURCES )

set( CMAKE_CXX_FLAGS "-O0 -Wall -pedantic -std=c++11" )
# set( CMAKE_CXX_FLAGS "-std=c++11" )

add_executable( ${BIN} ${SOURCES} )
