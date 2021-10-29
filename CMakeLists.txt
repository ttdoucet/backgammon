cmake_minimum_required(VERSION 3.21.3)

project(bg LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-Ofast -ffast-math --std=c++17")

include_directories(cmdline matrix stopwatch)

list(APPEND common_src  hits.cpp bgnet.cpp move.cpp ttydisp.cpp human.cpp)

add_library(common ${common_src})
target_link_libraries(common libbearoff.a)
target_link_libraries(common -L${CMAKE_CURRENT_SOURCE_DIR}/bearoff)

add_executable(playoff playoff.cpp)
target_link_libraries(playoff common)

add_executable(train playoff.cpp)
target_link_libraries(train common)
