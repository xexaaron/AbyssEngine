cmake_minimum_required(VERSION 3.28.3)
project(tool)

set(CMAKE_CXX_STANDARD 23)

# Set the build type if not already set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Get the final component of the build directory (e.g., "build" or "out")
get_filename_component(EXECUTABLE_FOLDER_NAME ${CMAKE_BINARY_DIR} NAME)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(EXECUTABLE_SUFFIX ".exe")
else()
    set(EXECUTABLE_SUFFIX "")
endif()

# Check if EDITOR is defined in the parent scope (top-level project)
if (NOT DEFINED EDITOR)
    message(FATAL_ERROR "tools/tool/CMakeLists.txt was not built using top-level CMakeLists.txt. EDITOR variable not set.")
endif()

# Pass the build folder name as a preprocessor definition
add_definitions(
    -DEXECUTABLE_FOLDER_NAME="${EXECUTABLE_FOLDER_NAME}"
    -DEDITOR_EXECUTABLE_NAME="${EDITOR}${EXECUTABLE_SUFFIX}"  # Inherited from top-level CMakeLists.txt
    -DEXECUTABLE_FOLDER="${CMAKE_BUILD_TYPE}"
    -DEXECUTABLE_SUFFIX="${EXECUTABLE_SUFFIX}"  # Fixed typo here
)

add_executable(${PROJECT_NAME} Source/main.cpp)

# Include the CmdLine header files
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../Vendor/CmdLine/Source/Public")