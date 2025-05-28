
cmake_minimum_required(VERSION 3.28.3)
project(tests)

set(CMAKE_CXX_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CPP_SOURCES 
    Source/main.cpp
)
set(CPP_HEADERS 
    Source/Public/Framework.h
)

source_group("Public" FILES
   Source/Public/Framework.h
)
source_group("Private" FILES 
    Source/main.cpp
)

add_executable(${PROJECT_NAME} ${CPP_SOURCES} ${CPP_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC Source/Public)
