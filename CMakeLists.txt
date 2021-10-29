cmake_minimum_required(VERSION 3.21.4)

project(bg LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-Ofast -ffast-math")

list(APPEND common_src  hits.cpp bgnet.cpp move.cpp ttydisp.cpp human.cpp)

macro(make targ)
  add_executable(${targ} ${targ}.cpp ${common_src})
  target_include_directories(${targ} PUBLIC cmdline matrix stopwatch)
  target_compile_features(${targ} PUBLIC cxx_std_17)
  target_link_libraries(${targ} libbearoff.a)
  target_link_libraries(${targ} -L${CMAKE_CURRENT_SOURCE_DIR}/bearoff)
 endmacro()

make(playoff)
make(train)



