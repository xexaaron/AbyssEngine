
cmake_minimum_required(VERSION 3.28.3)
project(tool)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

add_executable(${PROJECT_NAME} Source/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../Vendor/CmdLine/Source/Public")
