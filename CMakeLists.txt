cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 17)

project(wlog)

find_package(CLI11 CONFIG REQUIRED)

add_executable(wlog src/main.cpp)

target_link_libraries(wlog PRIVATE CLI11::CLI11)