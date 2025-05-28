
cmake_minimum_required(VERSION 3.28.3)
project(localize)

set(CMAKE_CXX_STANDARD 23)
if (NOT CMAKE_BUILD_TYE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(${PROJECT_NAME} Source/main.cpp)