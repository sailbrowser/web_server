cmake_minimum_required( VERSION 2.6 )
set( BIN final )
project( ${BIN} )
# libev
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
# set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set( SOURCE_ROOT ./src )
aux_source_directory( ${SOURCE_ROOT} SOURCES )
#add_library(libEV SHARED IMPORTED)
find_package(Libev REQUIRED)
include_directories(${LIBEV_INCLUDE_DIR})

set( CMAKE_CXX_FLAGS "-O0 -Wall -pedantic -std=c++11" )
# SET( CMAKE_EXE_LINKER_FLAGS  "-lev" )
# set( CMAKE_CXX_FLAGS "-std=c++11" )

add_executable( ${BIN} ${SOURCES} )
target_link_libraries(${BIN} ${LIBEV_LIBRARY} )
