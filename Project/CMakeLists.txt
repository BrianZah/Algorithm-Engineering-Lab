cmake_minimum_required( VERSION 3.15 )
project( ppartquick LANGUAGES CXX)

if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native" )
elseif ( CMAKE_CXX_COMPILER_ID STREQUAL "INTEL" )
  #set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native" )
  # For profiling with intel oneAPI
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-ffast-math -march=native -O2 -fopenmp -g -qopt-report=5 -qopt-report-phase=vec" )
endif ()

message ("Flags: ${CMAKE_CXX_FLAGS}")

add_library( ppartquick INTERFACE )
target_include_directories( ppartquick INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}" )

find_package( OpenMP REQUIRED )

add_executable( test_vs_gnu.exe TEST_vs_gnu_parallel_and_correctness.cc )
target_link_libraries( test_vs_gnu.exe PRIVATE ppartquick )
target_link_libraries( test_vs_gnu.exe PUBLIC OpenMP::OpenMP_CXX )

add_executable( test_vs_time.exe TEST_vs_time.cc )
target_link_libraries( test_vs_time.exe PRIVATE ppartquick )
target_link_libraries( test_vs_time.exe PUBLIC OpenMP::OpenMP_CXX )
