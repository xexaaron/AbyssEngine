cmake_minimum_required(VERSION 3.28.3)
project(tool)

set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

get_filename_component(EXECUTABLE_FOLDER_NAME ${CMAKE_BINARY_DIR} NAME)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(EXECUTABLE_SUFFIX ".exe")
else()
    set(EXECUTABLE_SUFFIX "")
endif()

if (NOT DEFINED EDITOR)
    message(FATAL_ERROR "tools/tool/CMakeLists.txt was not built using top-level CMakeLists.txt. EDITOR variable not set.")
endif()

add_definitions(
    -DEXECUTABLE_FOLDER_NAME="${EXECUTABLE_FOLDER_NAME}"
    -DEDITOR_EXECUTABLE_NAME="${EDITOR}${EXECUTABLE_SUFFIX}"  # Inherited from top-level CMakeLists.txt
    -DEXECUTABLE_FOLDER="${CMAKE_BUILD_TYPE}"
    -DEXECUTABLE_SUFFIX="${EXECUTABLE_SUFFIX}"
)

set(CMDLINE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Vendor/AbyssFreetype/Vendor/CmdLine")

add_executable(${PROJECT_NAME} 
    Source/main.cpp 
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMDLINE_DIR}/Source/Public" "${CMDLINE_DIR}/Vendor/AbyssPrettyPrint/Source/Public")
target_link_libraries(${PROJECT_NAME} CmdLine)