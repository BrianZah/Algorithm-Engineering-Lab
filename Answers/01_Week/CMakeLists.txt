cmake_minimum_required(VERSION 3.15)
project(pi_monte_carlo LANGUAGES CXX)

find_package(OpenMP REQUIRED)

add_executable(pi_monte_carlo pi_monte_carlo.cpp)
target_link_libraries(pi_monte_carlo -fopenmp)
