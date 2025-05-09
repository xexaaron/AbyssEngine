cmake_minimum_required(VERSION 3.28.3)
project(Abyss)

include(${CMAKE_SOURCE_DIR}/tools/cmake/Functions.cmake)

# Setup Build Options

set(CMAKE_DISABLE_PREDEFINED_TARGETS TRUE)
set(CMAKE_CXX_STANDARD 23)
set(ENGINE ${PROJECT_NAME}Engine)
set(EDITOR ${PROJECT_NAME}Editor)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_SUFFIX "d")
else()
    set(DEBUG_SUFFIX "")
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_SUFFIX ".exe")
else()
    set(PLATFORM_SUFFIX "")
endif()

if (MSVC)
    # W4                    warn level 4
    # wd4100                unreferenced formal parameter
    # Zc:preprocessor       standard conformant preprocssor (allow __VA_OPT__)
    set(COMPILE_OPTS /W4 /wd4100 /Zc:preprocessor)
else()
    set(COMPILE_OPTS -Wall -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-unused-function)
endif()

# Setup GLFW

set(GLFW_DIR "Vendor/glfw")
set(GLFW_INCLUDE_DIR "${GLFW_DIR}/include")
set(GLFW_LIBRARY_DIR "${CMAKE_BINARY_DIR}/Vendor/glfw/src/${CMAKE_BUILD_TYPE}")

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW docs")
if(CMAKE_HOST_SYSTEM_VERSION MATCHES "microsoft-standard-WSL2")
set(GLFW_BUILD_WAYLAND 0)
set(GLFW_BUILD_X11 1)
endif()
add_subdirectory(${GLFW_DIR})
set_vendor_properties(glfw "Dependencies/glfw")

# Setup STB

set(STB_INCLUDE_DIR "Vendor/stb")
set(STB_IMPL "${STB_INCLUDE_DIR}/stb.cpp")

# Setup GLM

set(GLM_DIR "$ENV{VULKAN_SDK}")
set(GLM_INCLUDE_DIR ${GLM_DIR})
set(GLM_DEFINITIONS GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE  GLM_FORCE_LEFT_HANDED)

# Setup Freetype
set(FREETYPE_DIR "Vendor/freetype")
set(FREETYPE_INCLUDE_DIR "${FREETYPE_DIR}/include")
set(FREETYPE_DEVEL_DIR "${FREETYPE_DIR}/devel")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
set(FREETYPE_LIBRARY_DIR "${FREETYPE_DIR}/objs")
add_subdirectory(${FREETYPE_DIR})
set_vendor_properties(freetype "Dependencies")
target_compile_definitions(freetype PRIVATE SKIP_INSTALL_ALL)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR} ${FREETYPE_DEVEL_DIR})


# Setup Vulkan

set(VULKAN_DIR $ENV{VULKAN_SDK} CACHE STRING "" FORCE)
set(VULKAN_INCLUDE_DIR "${VULKAN_DIR}/include")
set(VULKAN_LIBRARY_DIR "${VULKAN_DIR}/lib")
set(VULKAN_BIN_DIR "${VULKAN_DIR}/bin")
set(SHADER_C_INCLUDE_DIR "${VULKAN_INCLUDE_DIR}/shaderc")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    get_filename_component(VULKAN_LIB_PATH "${VULKAN_LIBRARY_DIR}/vulkan-1.lib" ABSOLUTE)
    get_filename_component(SPIRV_CROSS_LIB_PATH "${VULKAN_LIBRARY_DIR}/spirv-cross-core${DEBUG_SUFFIX}.lib" ABSOLUTE)
    get_filename_component(SPIRV_CROSS_GLSL_LIB_PATH "${VULKAN_LIBRARY_DIR}/spirv-cross-glsl${DEBUG_SUFFIX}.lib" ABSOLUTE)
    get_filename_component(SHADER_C_LIB_PATH "${VULKAN_LIBRARY_DIR}/shaderc_shared${DEBUG_SUFFIX}.lib" ABSOLUTE)
    set(PLATFORM_LIBS "dwmapi.lib")
else()
    get_filename_component(VULKAN_LIB_PATH "${VULKAN_LIBRARY_DIR}/libvulkan.so" ABSOLUTE)
    get_filename_component(SPIRV_CROSS_LIB_PATH "${VULKAN_LIBRARY_DIR}/libspirv-cross-core.a" ABSOLUTE)
    get_filename_component(SPIRV_CROSS_GLSL_LIB_PATH "${VULKAN_LIBRARY_DIR}/libspirv-cross-glsl.a" ABSOLUTE)
    get_filename_component(SHADER_C_LIB_PATH "${VULKAN_LIBRARY_DIR}/libshaderc_shared.so" ABSOLUTE)
    set(PLATFORM_LIBS "")
endif()

# List Sources

set(CPP_SOURCES
    Source/Private/Core/Log.cpp
    Source/Private/Core/App.cpp
    Source/Private/Core/Window.cpp
    Source/Private/Core/Event.cpp
    Source/Private/Core/Serialize.cpp
    Source/Private/Core/Object.cpp
    Source/Private/Core/Resource.cpp
    Source/Private/Core/Time.cpp
    Source/Private/Core/Thread.cpp
    Source/Private/Utility/Inserter.cpp
    Source/Private/Utility/TagParser.cpp
    Source/Private/Utility/Random.cpp
    Source/Private/Rendering/Vertex.cpp
    Source/Private/Rendering/Font.cpp
    Source/Private/Rendering/Context.cpp
    Source/Private/Rendering/Renderer.cpp
    Source/Private/Rendering/Camera.cpp
    Source/Private/Rendering/Shader.cpp
    Source/Private/Rendering/Texture.cpp
    Source/Private/Rendering/UI/Widget.cpp
    Source/Private/Rendering/UI/UICommon.cpp
    Source/Private/Rendering/UI/Textbox.cpp
    Source/Private/Rendering/UI/ParentWidget.cpp
    Source/Private/Rendering/UI/Panel.cpp
    Source/Private/Rendering/UI/LayoutContainer.cpp
    Source/Private/Rendering/UI/Image.cpp
    Source/Private/Rendering/UI/Canvas.cpp
    Source/Private/Rendering/UI/Button.cpp
    Source/Private/Rendering/UI/InputTextbox.cpp
    Source/Private/Rendering/UI/Console.cpp
    Source/Private/Rendering/UI/Dropdown.cpp
    Source/Private/vk/VkRenderer.cpp
    Source/Private/vk/VkBuffer.cpp
    Source/Private/vk/VkContext.cpp
    Source/Private/vk/VkCommon.cpp
    Source/Private/vk/VkCmdPool.cpp
    Source/Private/vk/VkCmdBuff.cpp
    Source/Private/vk/VkDeviceManager.cpp
    Source/Private/vk/VkInstance.cpp
    Source/Private/vk/VkDebugger.cpp
    Source/Private/vk/VkSurface.cpp
    Source/Private/vk/VkSwapchain.cpp
    Source/Private/vk/VkAllocator.cpp
    Source/Private/vk/VkShader.cpp
    Source/Private/vk/VkDescriptorPool.cpp
    Source/Private/vk/VkPipeline.cpp
    Source/Private/vk/VkRenderModule.cpp
    Source/Private/vk/VkTexture.cpp
    Source/Private/Platform/Platform.cpp
    Source/Private/Platform/win32/PlatformWin32.cpp
    Source/Private/Platform/posix/PlatformPosix.cpp
    Source/Private/Platform/Platform.cpp
    Source/Private/Platform/win32/ProcessWin32.cpp
    Source/Private/Platform/posix/ProcessPosix.cpp
    Source/Private/Platform/Process.cpp
    ${STB_IMPL}
)

set(HEADER_FILES
    Source/Public/Core/Log.h
    Source/Public/Core/Common.h
    Source/Public/Core/Event.h
    Source/Public/Core/App.h
    Source/Public/Core/Window.h
    Source/Public/Core/Object.h
    Source/Public/Core/Serialize.h
    Source/Public/Core/Resource.h
    Source/Public/Core/Time.h
    Source/Public/Core/Thread.h
    Source/Public/Utility/Inserter.h
    Source/Public/Utility/TagParser.h
    Source/Public/Utility/Random.h
    Source/Public/Rendering/Font.h
    Source/Public/Rendering/Context.h
    Source/Public/Rendering/Renderer.h
    Source/Public/Rendering/Camera.h
    Source/Public/Rendering/Shader.h
    Source/Public/Rendering/Texture.h
    Source/Public/Rendering/Vertex.h
    Source/Public/Rendering/UI/UI.h
    Source/Public/Rendering/UI/Widget.h
    Source/Public/Rendering/UI/UICommon.h
    Source/Public/Rendering/UI/Textbox.h
    Source/Public/Rendering/UI/ParentWidget.h
    Source/Public/Rendering/UI/Panel.h
    Source/Public/Rendering/UI/LayoutContainer.h
    Source/Public/Rendering/UI/Image.h
    Source/Public/Rendering/UI/Canvas.h
    Source/Public/Rendering/UI/Button.h
    Source/Public/Rendering/UI/InputTextbox.h
    Source/Public/Rendering/UI/Console.h
    Source/Public/Rendering/UI/Dropdown.h
    Source/Public/vk/VkRenderer.h
    Source/Public/vk/VkBuffer.h
    Source/Public/vk/VkContext.h
    Source/Public/vk/VkCommon.h
    Source/Public/vk/VkCmdPool.h
    Source/Public/vk/VkCmdBuff.h
    Source/Public/vk/VkDeviceManager.h
    Source/Public/vk/VkInstance.h
    Source/Public/vk/VkDebugger.h
    Source/Public/vk/VkSurface.h
    Source/Public/vk/VkSwapchain.h
    Source/Public/vk/VkAllocator.h
    Source/Public/vk/VkShader.h
    Source/Public/vk/VkDescriptorPool.h
    Source/Public/vk/VkPipeline.h
    Source/Public/vk/VkRenderModule.h
    Source/Public/vk/VkTexture.h
    Source/Public/Platform/win32/PlatformWin32.h
    Source/Public/Platform/posix/PlatformPosix.h
    Source/Public/Platform/Platform.h
    Source/Public/Platform/win32/ProcessWin32.h
    Source/Public/Platform/posix/ProcessPosix.h
    Source/Public/Platform/Process.h
)

# Setup virtual folders (Functions.cmake)
make_source_groups(
    Core
    Platform
    Platform/posix
    Platform/win32
    Rendering
    Rendering/UI
    Utility
    vk
)
source_group("Private" FILES ${STB_IMPL} "Source/main.cpp")
set(COMMON_INCLUDE_DIRS "Source/Public" ${GLFW_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
set(RESOURCES ${SHADER_SOURCES} ${TEXTURE_SOURCES} ${FONT_SOURCES})
source_group("Resources" FILES ${RESOURCES})
# Subprojects and compile defintions

add_subproject("localize")
add_subproject("tests")
add_subproject("watchdog")
add_subproject("tool")

add_library(${ENGINE} ${CPP_SOURCES} ${HEADER_FILES} ${RESOURCES})
add_executable(${EDITOR} Source/main.cpp ${RESOURCES})
target_compile_options(${ENGINE} PRIVATE ${COMPILE_OPTS})
target_compile_options(${EDITOR} PRIVATE ${COMPILE_OPTS})
target_compile_definitions(${ENGINE} PRIVATE ${GLM_DEFINITIONS} ABY_BUFFERED_LOGGING)
target_compile_definitions(${EDITOR} PRIVATE ${GLM_DEFINITIONS} ABY_BUFFERED_LOGGING)
target_include_directories(${ENGINE} PUBLIC ${VULKAN_INCLUDE_DIR} ${COMMON_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
target_include_directories(${EDITOR} PUBLIC Source/Public ${COMMON_INCLUDE_DIRS})
set_target_properties(${ENGINE} PROPERTIES FOLDER "Abyss")
set_target_properties(${EDITOR} PROPERTIES FOLDER "Abyss")

# Build Projects

target_link_libraries(${ENGINE} PRIVATE
    ${VULKAN_LIB_PATH}
    ${SHADER_C_LIB_PATH}
    ${SPIRV_CROSS_LIB_PATH}
    ${SPIRV_CROSS_GLSL_LIB_PATH}
    ${PLATFORM_LIBS}
    freetype
    glfw
)

target_link_libraries(${EDITOR} PRIVATE ${ENGINE})
add_dependencies(${ENGINE} freetype glfw)
add_dependencies(${EDITOR} ${ENGINE})

# Copy Resources
add_custom_command(
    TARGET ${EDITOR} POST_BUILD
    DEPENDS tool 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Source/Textures"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Textures"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Source/Fonts"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Source/Shaders"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/tool${PLATFORM_SUFFIX}"
            "${CMAKE_CURRENT_LIST_DIR}/tool${PLATFORM_SUFFIX}"
    COMMENT "Copy resources and tools"
)

# Copy DLLs 

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(FREETYPE_DLL_IN  "${CMAKE_BINARY_DIR}/Vendor/freetype/${CMAKE_BUILD_TYPE}/freetype${DEBUG_SUFFIX}.dll")
    set(FREETYPE_DLL_OUT "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/freetype${DEBUG_SUFFIX}.dll")
    set(SHADERC_DLL_IN   "${VULKAN_BIN_DIR}/shaderc_shared${DEBUG_SUFFIX}.dll")
    set(SHADERC_DLL_OUT  "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaderc_shared${DEBUG_SUFFIX}.dll")

    add_custom_command(TARGET ${EDITOR} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Checking and copying DLLs..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREETYPE_DLL_IN} ${FREETYPE_DLL_OUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADERC_DLL_IN} ${SHADERC_DLL_OUT}
        COMMENT "Copy necessary DLLs"
    )
endif()

# Ensure post build commands run even when not expliclty stating ${EDITOR} as the build target.
add_custom_target(run_post_build ALL DEPENDS ${EDITOR})
set_target_properties(run_post_build PROPERTIES FOLDER "Abyss/Tools")

