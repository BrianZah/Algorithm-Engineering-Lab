cmake_minimum_required( VERSION 3.15 )
project( ppartquick LANGUAGES CXX)

message( "Profiling = ${PROFILING}" )

# PROFILING enables profiling with intel oneAPI Tools
if( NOT DEFINED PROFILING )
  set( PROFILING off )
elseif( PROFILING STREQUAL "on" )
  set( CMAKE_CXX_COMPILER icpc )
endif()

# set compiler flags depending on compiler
if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-O2 -march=native" )
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Intel" )
  if( PROFILING STREQUAL "on" )
    # required for profiling with intel oneAPI Tools
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-std=c++17 -O2 -march=native -ffast-math -g -qopt-report=5 -qopt-report-phase=vec" )
  else()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-O2 -march=native -ffast-math" )
  endif()
endif()

message( "Compiler call: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS}" )

# add the library
file( GLOB LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib" )
file( GLOB TEST "${CMAKE_CURRENT_SOURCE_DIR}/test" )

add_library( ppartquick INTERFACE )
target_include_directories( ppartquick INTERFACE "${LIB}" )

# add openMP
find_package( OpenMP REQUIRED )

# create the add_executable depending on the state of the PROFILING variable
if( PROFILING STREQUAL "on" )
  add_executable( profiling.exe ${TEST}/profiling.cc )
  target_link_libraries( profiling.exe PRIVATE ppartquick )
  target_link_libraries( profiling.exe PUBLIC OpenMP::OpenMP_CXX )
else()
  add_executable( test_with_gnu.exe ${TEST}/test_with_gnu_parallel.cc )
  target_link_libraries( test_with_gnu.exe PRIVATE ppartquick )
  target_link_libraries( test_with_gnu.exe PUBLIC OpenMP::OpenMP_CXX )
endif()
