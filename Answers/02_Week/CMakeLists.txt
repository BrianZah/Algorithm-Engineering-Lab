cmake_minimum_required(VERSION 3.15)
project(pi LANGUAGES CXX)

find_package(OpenMP)

add_executable(pi_openmp_v1 pi_openmp_v1.cpp)

if(OpenMP_CXX_FOUND)
    target_link_libraries(pi_openmp_v1 PUBLIC OpenMP::OpenMP_CXX)
endif()
