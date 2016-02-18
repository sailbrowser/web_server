cmake_minimum_required( VERSION 2.6 )
set( BIN final )
project( ${BIN} )

# set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set( SOURCE_ROOT ./src )
aux_source_directory( ${SOURCE_ROOT} SOURCES )
add_library(libEV SHARED IMPORTED)

set( CMAKE_CXX_FLAGS "-O0 -Wall -pedantic -std=c++11 -lev" )
# SET( CMAKE_EXE_LINKER_FLAGS  "-lev" )
# set( CMAKE_CXX_FLAGS "-std=c++11" )

add_executable( ${BIN} ${SOURCES} )
target_link_libraries(${BIN} "ev" )
