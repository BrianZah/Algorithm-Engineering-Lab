cmake_minimum_required(VERSION 3.15)
project(pi LANGUAGES CXX)

find_package(OpenMP)

add_executable(pi_monte_carlo pi_monte_carlo.cpp)

if(OpenMP_CXX_FOUND)
    target_link_libraries(pi_monte_carlo PUBLIC OpenMP::OpenMP_CXX)
endif()
