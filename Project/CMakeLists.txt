cmake_minimum_required( VERSION 3.15 )
project( quicksort LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native")


add_library( partition INTERFACE )
target_include_directories( partition INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}" )

find_package(OpenMP REQUIRED)

add_executable( test.exe test.cc )
target_link_libraries( test.exe PRIVATE partition)
target_link_libraries( test.exe PUBLIC OpenMP::OpenMP_CXX)
