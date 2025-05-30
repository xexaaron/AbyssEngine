cmake_minimum_required(VERSION 3.28.3)
project(Abyss)

include(${CMAKE_SOURCE_DIR}/tools/cmake/Functions.cmake)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Setup Build Options
set(CMAKE_DISABLE_CPACK TRUE)
set(CMAKE_DISABLE_PREDEFINED_TARGETS TRUE)
set(CMAKE_CXX_STANDARD 23)
set(ENGINE ${PROJECT_NAME}Engine)
set(EDITOR ${PROJECT_NAME}Editor)
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
    # W4                    warn level 4.
    # wd4100                Disable unreferenced formal parameter.
    # wd4201                Disable nonstandard ext: nameless struct/union.
    # wd4505                Disable unreferenced function with internal linkage has been removed.
    # Zc:preprocessor       standard conformant preprocssor (allow __VA_OPT__).
    set(COMPILE_OPTS /W4 /wd4100 /wd4201 /wd4505 /Zc:preprocessor)
else()
    set(COMPILE_OPTS -Wall -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-unused-function
                     -Wno-gnu-anonymous-struct -Wno-nested-anon-types
    )
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
set(STB_SOURCES 
    "${STB_INCLUDE_DIR}/stb_image/stb_image_resize2.h"
    "${STB_INCLUDE_DIR}/stb_image/stb_image_write.h"
    "${STB_INCLUDE_DIR}/stb_image/stb_image.h"
    "${STB_INCLUDE_DIR}/stb.cpp"
)
add_library(stb INTERFACE ${STB_SOURCES})
set_target_properties(stb PROPERTIES FOLDER "Dependencies/stb")
# Setup gch::small_vector

set(GCH_SMALL_VEC_DIR "Vendor/gch")
set(GCH_SMALL_VEC_INCLUDE_DIR "${GCH_SMALL_VEC_DIR}/source/include")
set(GCH_SMALL_VEC_SOURCES "${GCH_SMALL_VEC_INCLUDE_DIR}/gch/small_vector.hpp")
add_library(small_vector INTERFACE ${GCH_SMALL_VEC_SOURCES})
set_target_properties(small_vector PROPERTIES FOLDER "Dependencies/gch")

# Setup GLM

set(GLM_DIR "Vendor/glm")
set(GLM_INCLUDE_DIR ${GLM_DIR})
set(GLM_DEFINITIONS GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_LEFT_HANDED)
add_subdirectory(${GLM_DIR})
set_target_properties(glm PROPERTIES FOLDER "Dependencies/glm")

# Setup AbyssFreetype

set(ABYSS_FREETYPE_DIR "Vendor/AbyssFreetype")
set(ABYSS_FREETYPE_INCLUDE_DIR "${ABYSS_FREETYPE_DIR}/Source/Public")
add_subdirectory(${ABYSS_FREETYPE_DIR})
set_target_properties(AbyssFT PROPERTIES FOLDER "Dependencies/AbyssFT")
set_target_properties(AbyssFTLib PROPERTIES FOLDER "Dependencies/AbyssFT")
set_target_properties(CmdLine PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(AbyssFTTest PROPERTIES FOLDER "Dependencies/AbyssFT")
    set_target_properties(CmdLineTest PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
    set_target_properties(PrettyPrint PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
    set_target_properties(PrettyPrintTest PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
endif()
set_target_properties(brotlicommon PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(brotlienc PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(brotlidec PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(brotli PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(bz2 PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(bz2_ObjLib PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(bzip2 PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(bzip2recover PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(check PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(freetype PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(zlib PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(zlibstatic PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(png_genfiles PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(png_shared PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(png_static PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(pngfix PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(png-fix-itxt PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(pngimage PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(pngstest PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(pngtest PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(pngunknown PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")
set_target_properties(pngvalid PROPERTIES FOLDER "Dependencies/AbyssFT/Dependencies")


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
    Source/Private/Utility/CursorString.cpp
    Source/Private/Utility/MaxEmptyRectFinder.cpp
    Source/Private/Rendering/Vertex.cpp
    Source/Private/Rendering/Font.cpp
    Source/Private/Rendering/Context.cpp
    Source/Private/Rendering/Renderer.cpp
    Source/Private/Rendering/Camera.cpp
    Source/Private/Rendering/Shader.cpp
    Source/Private/Rendering/Texture.cpp
    Source/Private/Widget/Widget.cpp
    Source/Private/Widget/UICommon.cpp
    Source/Private/Widget/Textbox.cpp
    Source/Private/Widget/Panel.cpp
    Source/Private/Widget/LayoutContainer.cpp
    Source/Private/Widget/Image.cpp
    Source/Private/Widget/Canvas.cpp
    Source/Private/Widget/Button.cpp
    Source/Private/Widget/InputTextbox.cpp
    Source/Private/Widget/Console.cpp
    Source/Private/Widget/Dropdown.cpp
    Source/Private/Widget/WidgetSwitcher.cpp
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
    Source/Public/Utility/CursorString.h
    Source/Public/Utility/Delegate.h
    Source/Public/Utility/MaxEmptyRectFinder.h
    Source/Public/Rendering/Font.h
    Source/Public/Rendering/Context.h
    Source/Public/Rendering/Renderer.h
    Source/Public/Rendering/Camera.h
    Source/Public/Rendering/Shader.h
    Source/Public/Rendering/Texture.h
    Source/Public/Rendering/Vertex.h
    Source/Public/Widget/UI.h
    Source/Public/Widget/Widget.h
    Source/Public/Widget/UICommon.h
    Source/Public/Widget/Textbox.h
    Source/Public/Widget/Panel.h
    Source/Public/Widget/LayoutContainer.h
    Source/Public/Widget/Image.h
    Source/Public/Widget/Canvas.h
    Source/Public/Widget/Button.h
    Source/Public/Widget/InputTextbox.h
    Source/Public/Widget/Console.h
    Source/Public/Widget/Dropdown.h
    Source/Public/Widget/WidgetSwitcher.h
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
    Widget
    Utility
    vk
)


source_group("Private" FILES ${STB_IMPL} "Source/main.cpp")
set(COMMON_INCLUDE_DIRS "Source/Public" ${GLFW_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/Source/Resources")

# Setup subprojects
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_subproject("tests")
endif()
add_subproject("localize")
add_subproject("watchdog")
add_subproject("aby_package")
add_subproject("tool")
add_dependencies(tool AbyssFTLib)

# Setup library/engine

add_library(${ENGINE} ${CPP_SOURCES} ${HEADER_FILES} ${RESOURCES})
add_executable(${EDITOR} Source/main.cpp ${RESOURCES})
target_compile_options(${ENGINE} PRIVATE ${COMPILE_OPTS})
target_compile_options(${EDITOR} PRIVATE ${COMPILE_OPTS})
target_compile_definitions(${ENGINE} PRIVATE ${GLM_DEFINITIONS} ABY_BUFFERED_LOGGING _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${EDITOR} PRIVATE ${GLM_DEFINITIONS} ABY_BUFFERED_LOGGING)
target_include_directories(${ENGINE} PUBLIC 
    ${VULKAN_INCLUDE_DIR} 
    ${COMMON_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS} 
    ${GCH_SMALL_VEC_INCLUDE_DIR} 
    ${ABYSS_FREETYPE_INCLUDE_DIR}
    ${ABY_FT_PP_INCLUDE_DIR}
)
target_include_directories(${EDITOR} PUBLIC Source/Public ${COMMON_INCLUDE_DIRS} ${ABY_FT_PP_INCLUDE_DIR})
set_target_properties(${ENGINE} PROPERTIES FOLDER "Abyss")
set_target_properties(${EDITOR} PROPERTIES FOLDER "Abyss")

# Build Projects

target_link_libraries(${ENGINE} PRIVATE
    ${VULKAN_LIB_PATH}
    ${SHADER_C_LIB_PATH}
    ${SPIRV_CROSS_LIB_PATH}
    ${SPIRV_CROSS_GLSL_LIB_PATH}
    ${PLATFORM_LIBS}
    AbyssFTLib
    glfw
)

target_link_libraries(${EDITOR} PRIVATE ${ENGINE})
add_dependencies(${ENGINE} AbyssFTLib glfw tool)
add_dependencies(${EDITOR} ${ENGINE})

# Copy Resources
add_custom_command(
    TARGET ${ENGINE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RESOURCE_DIR}/Textures"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Textures"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RESOURCE_DIR}/Fonts"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RESOURCE_DIR}/Shaders"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Shaders"
    COMMENT "Copy resources"
)

# Copy tool

add_custom_command(
    TARGET ${ENGINE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/tool${PLATFORM_SUFFIX}"
            "${CMAKE_CURRENT_LIST_DIR}/tool${PLATFORM_SUFFIX}"
    COMMENT "Copy tool executable to root directory"
)


# Copy DLLs 

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SHADERC_DLL_IN   "${VULKAN_BIN_DIR}/shaderc_shared${DEBUG_SUFFIX}.dll")
    set(SHADERC_DLL_OUT  "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaderc_shared${DEBUG_SUFFIX}.dll")
    add_custom_command(TARGET ${EDITOR} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Checking and copying DLLs..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADERC_DLL_IN} ${SHADERC_DLL_OUT}
        COMMENT "Copy necessary DLLs"
    )
endif()

add_custom_target(run_post_build ALL DEPENDS ${EDITOR})
set_target_properties(run_post_build PROPERTIES FOLDER "Abyss/Tools")

