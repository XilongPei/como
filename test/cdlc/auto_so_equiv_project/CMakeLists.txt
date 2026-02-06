
cmake_minimum_required(VERSION 3.10)
project(auto_so_equiv)

set(CMAKE_CXX_STANDARD 17)

add_executable(auto_so_equiv auto_so_equiv.cpp)
target_link_libraries(auto_so_equiv dl)
