cmake_minimum_required(VERSION 3.16)
project(grf_python)

find_package(Python COMPONENTS Development REQUIRED)

message("${Python_INCLUDE_DIRS}")

include_directories(${Python_INCLUDE_DIRS})
include_directories(/home/colin/libgrf/include)
link_directories(/home/colin/libgrf/cmake-build-debug)

set(CMAKE_CXX_STANDARD 14)

add_library(grf_python SHARED grf.cpp)
target_link_libraries(grf_python libgrf.a)
