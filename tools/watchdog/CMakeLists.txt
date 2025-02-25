
cmake_minimum_required(VERSION 3.28.3)
project(watchdog)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

add_executable(${PROJECT_NAME} Source/Private/main.cpp)