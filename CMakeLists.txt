cmake_minimum_required(VERSION 3.21.4)
project(bg LANGUAGES CXX)

set(CMAKE_INCLUDE_FLAG_CXX "-iquote")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -Ofast -ffast-math --std=c++17")

include_directories(cmdline matrix stopwatch)

add_subdirectory(bearoff)

add_library(common  hits.cpp move.cpp)
target_link_libraries(common PUBLIC bearoff)

add_executable(playoff playoff.cpp)
target_link_libraries(playoff common)

add_executable(train train.cpp)
target_link_libraries(train common)

add_executable(convert convert.cpp)
target_link_libraries(convert common)

add_executable(etest etest.cpp)

