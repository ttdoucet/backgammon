cmake_minimum_required(VERSION 3.21.3)

project(bearoff LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-Ofast -ffast-math --std=c++17")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. cmdline)

add_executable(beargen  beargen.cpp bearoff.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../move.cpp)

add_custom_command(
        OUTPUT bearoff.dat
        COMMAND ./beargen > ${CMAKE_CURRENT_SOURCE_DIR}/bearoff.dat
        DEPENDS beargen
        VERBATIM)

add_library(bearoff  bdata.cpp bearoff.cpp bearoff.dat)
