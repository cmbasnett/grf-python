cmake_minimum_required(VERSION 3.16)
project(grf_python)

find_package(Python COMPONENTS Development REQUIRED)
include_directories(${Python_INCLUDE_DIRS})
link_libraries(${Python_LIBRARIES})

add_subdirectory(libgrf)
include_directories(libgrf/include)

set(CMAKE_CXX_STANDARD 14)

add_library(grf_python SHARED grf.cpp)
target_link_libraries(grf_python grf)
