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
    set(COMPILE_OPTS 
        /W4 
        /wd4100 
        /wd4201 
        /wd4505 
        /Zc:preprocessor
    )
    set(LINK_OPTS /SUBSYSTEM:WINDOWS)
else()
    set(COMPILE_OPTS 
        -Wall -Wextra 
        -Wno-unused-parameter 
        -Wno-ignored-qualifiers 
        -Wno-unused-function
        -Wno-gnu-anonymous-struct 
        -Wno-nested-anon-types
    )
    set(LINK_OPTS)
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
    set(PLATFORM_LIBS "dwmapi.lib" "ole32.lib" "shell32.lib")
else()
    get_filename_component(VULKAN_LIB_PATH "${VULKAN_LIBRARY_DIR}/libvulkan.so" ABSOLUTE)
    get_filename_component(SPIRV_CROSS_LIB_PATH "${VULKAN_LIBRARY_DIR}/libspirv-cross-core.a" ABSOLUTE)
    get_filename_component(SPIRV_CROSS_GLSL_LIB_PATH "${VULKAN_LIBRARY_DIR}/libspirv-cross-glsl.a" ABSOLUTE)
    get_filename_component(SHADER_C_LIB_PATH "${VULKAN_LIBRARY_DIR}/libshaderc_shared.so" ABSOLUTE)
    set(PLATFORM_LIBS "")
endif()

# Setup ImGui

    
set(IMGUI_DIR "Vendor/ImGui/imgui")
set(IMGUI_INCLUDE_DIR "Vendor/ImGui")
set(IMGUI_HEADERS
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui.cpp
)

set(IMGUI_BACKENDS
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.h
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

set(IMGUI_RESOURCES)

add_library(ImGui STATIC ${IMGUI_HEADERS} ${IMGUI_SOURCES} ${IMGUI_BACKENDS})
target_include_directories(ImGui PUBLIC ${IMGUI_DIR} ${IMGUI_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLM_INCLUDE_DIR} "Source/Public")
target_compile_definitions(ImGui PUBLIC IMGUI_USER_CONFIG="Platform/imgui/imconfig.h")
target_compile_options(ImGui PUBLIC ${COMPILE_OPTS})
set_target_properties(ImGui PROPERTIES FOLDER "Dependencies/ImGui")

# Setup CEF

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CEF_OS "windows")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CEF_OS "linux")
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CEF_ARCH)
if (CEF_ARCH MATCHES "^(amd64|x86_64)$")
    set(CEF_ARCH "x86_64")
elseif (CEF_ARCH MATCHES "^(aarch64|arm64)$")
    set(CEF_ARCH "arm64")
else()
    message(FATAL_ERROR "Unsupported arch: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(CEF_PLATFORM "${CEF_OS}-${CEF_ARCH}")
set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/cef/${CEF_PLATFORM}")
set(CEF_INCLUDE_DIR ${CEF_ROOT}/include)
set(CEF_INTERNAL_INCLUDE_DIR ${CEF_ROOT})
set(CEF_LIB_DIR ${CEF_ROOT}/${CMAKE_BUILD_TYPE})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH})
set_target_properties(libcef_dll_wrapper PROPERTIES FOLDER "Dependencies/cef")
if (MSVC)
    target_compile_options(libcef_dll_wrapper PRIVATE /MDd)
    target_link_options(libcef_dll_wrapper PRIVATE /MDd)
endif()
# List Sources

set(ENGINE_CPP_SOURCES
    Source/Private/Browser/Browser.cpp
    Source/Private/Core/App.cpp
    Source/Private/Core/EntryPoint.cpp
    Source/Private/Core/Event.cpp
    Source/Private/Core/Log.cpp
    Source/Private/Core/Object.cpp
    Source/Private/Core/Resource.cpp
    Source/Private/Core/Time.cpp
    Source/Private/Core/Plugin.cpp
    Source/Private/Platform/Platform.cpp
    Source/Private/Platform/Platform.cpp
    Source/Private/Platform/Process.cpp
    Source/Private/Platform/SharedLibrary.cpp
    Source/Private/Platform/imgui/imconsole.cpp
    Source/Private/Platform/imgui/imtheme.cpp
    Source/Private/Platform/imgui/imwidget.cpp
    Source/Private/Platform/posix/PlatformPosix.cpp
    Source/Private/Platform/posix/ProcessPosix.cpp
    Source/Private/Platform/posix/WindowPosix.cpp
    # Source/Private/Platform/win32/SharedLibraryPosix.cpp
    Source/Private/Platform/win32/PlatformWin32.cpp
    Source/Private/Platform/win32/ProcessWin32.cpp
    Source/Private/Platform/win32/WindowWin32.cpp
    Source/Private/Platform/win32/SharedLibraryWin32.cpp
    Source/Private/Platform/vk/VkAllocator.cpp
    Source/Private/Platform/vk/VkBuffer.cpp
    Source/Private/Platform/vk/VkCmdBuff.cpp
    Source/Private/Platform/vk/VkCmdPool.cpp
    Source/Private/Platform/vk/VkCommon.cpp
    Source/Private/Platform/vk/VkContext.cpp
    Source/Private/Platform/vk/VkDebugger.cpp
    Source/Private/Platform/vk/VkDescriptorPool.cpp
    Source/Private/Platform/vk/VkDeviceManager.cpp
    Source/Private/Platform/vk/VkInstance.cpp
    Source/Private/Platform/vk/VkPipeline.cpp
    Source/Private/Platform/vk/VkRenderModule.cpp
    Source/Private/Platform/vk/VkRenderer.cpp
    Source/Private/Platform/vk/VkShader.cpp
    Source/Private/Platform/vk/VkShaderCompiler.cpp
    Source/Private/Platform/vk/VkShaderModule.cpp
    Source/Private/Platform/vk/VkShaderStructs.cpp
    Source/Private/Platform/vk/VkSurface.cpp
    Source/Private/Platform/vk/VkSwapchain.cpp
    Source/Private/Platform/vk/VkTexture.cpp
    Source/Private/Rendering/Camera.cpp
    Source/Private/Rendering/Context.cpp
    Source/Private/Rendering/Font.cpp
    Source/Private/Rendering/Renderer.cpp
    Source/Private/Rendering/Shader.cpp
    Source/Private/Rendering/Texture.cpp
    Source/Private/Rendering/Vertex.cpp
    Source/Private/Rendering/Window.cpp
    Source/Private/Utility/CursorString.cpp
    Source/Private/Utility/File.cpp
    Source/Private/Utility/Inserter.cpp
    Source/Private/Utility/Random.cpp
    Source/Private/Utility/TagParser.cpp
    Source/Private/Utility/Profiler.cpp
    Source/Private/Utility/Serialize.cpp
    Source/Private/Utility/Thread.cpp
    ${STB_IMPL}
)

set(ENGINE_CPP_HEADERS
    Source/Public/Browser/Browser.h
    Source/Public/Core/App.h
    Source/Public/Core/Common.h
    Source/Public/Core/EntryPoint.h
    Source/Public/Core/Event.h
    Source/Public/Core/Log.h
    Source/Public/Core/Object.h
    Source/Public/Core/Resource.h
    Source/Public/Core/Time.h
    Source/Public/Core/Plugin.h
    Source/Public/Platform/imgui/imconfig.h
    Source/Public/Platform/imgui/imconsole.h
    Source/Public/Platform/imgui/imtheme.h
    Source/Public/Platform/imgui/imwidget.h
    Source/Public/Platform/Platform.h
    Source/Public/Platform/Process.h
    Source/Public/Platform/SharedLibrary.h
    Source/Public/Platform/posix/PlatformPosix.h
    Source/Public/Platform/posix/ProcessPosix.h
    Source/Public/Platform/posix/WindowPosix.h
  #  Source/Public/Platform/posix/SharedLibraryPosix.h
    Source/Public/Platform/win32/PlatformWin32.h
    Source/Public/Platform/win32/ProcessWin32.h
    Source/Public/Platform/win32/WindowWin32.h
    Source/Public/Platform/win32/SharedLibraryWin32.h
    Source/Public/Platform/vk/VkAllocator.h
    Source/Public/Platform/vk/VkBuffer.h
    Source/Public/Platform/vk/VkCmdBuff.h
    Source/Public/Platform/vk/VkCmdPool.h
    Source/Public/Platform/vk/VkCommon.h
    Source/Public/Platform/vk/VkContext.h
    Source/Public/Platform/vk/VkDebugger.h
    Source/Public/Platform/vk/VkDescriptorPool.h
    Source/Public/Platform/vk/VkDeviceManager.h
    Source/Public/Platform/vk/VkInstance.h
    Source/Public/Platform/vk/VkPipeline.h
    Source/Public/Platform/vk/VkRenderModule.h
    Source/Public/Platform/vk/VkRenderer.h
    Source/Public/Platform/vk/VkShader.h
    Source/Public/Platform/vk/VkShaderCompiler.h
    Source/Public/Platform/vk/VkShaderModule.h
    Source/Public/Platform/vk/VkShaderStructs.h
    Source/Public/Platform/vk/VkSurface.h
    Source/Public/Platform/vk/VkSwapchain.h
    Source/Public/Platform/vk/VkTexture.h
    Source/Public/Rendering/Camera.h
    Source/Public/Rendering/Context.h
    Source/Public/Rendering/Font.h
    Source/Public/Rendering/Renderer.h
    Source/Public/Rendering/Shader.h
    Source/Public/Rendering/Texture.h
    Source/Public/Rendering/Vertex.h
    Source/Public/Rendering/Window.h
    Source/Public/Utility/CursorString.h
    Source/Public/Utility/Delegate.h
    Source/Public/Utility/File.h
    Source/Public/Utility/Inserter.h
    Source/Public/Utility/Random.h
    Source/Public/Utility/TagParser.h
    Source/Public/Utility/Profiler.h
    Source/Public/Utility/Serialize.h
    Source/Public/Utility/Thread.h
)

set(EDITOR_CPP_SOURCES
    Source/Private/Editor/Editor.cpp
)
set(EDITOR_CPP_HEADERS
    Source/Public/Editor/Editor.h
)

# Setup virtual folders (Functions.cmake)
make_source_groups(
    Browser
    Core
    Platform
    Platform/posix
    Platform/win32
    Platform/vk
    Platform/imgui
    Rendering
    Widget
    Utility
)

source_group("Private" FILES "Source/main.cpp")
source_group("Private/Platform/stb" FILES "Vendor/stb/stb.cpp")
set(COMMON_INCLUDE_DIRS "Source/Public" ${GLFW_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${IMGUI_INCLUDE_DIR} ${IMGUI_DIR} ${ABY_FT_PP_INCLUDE_DIR})
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

# Setup engine/editor
## Engine
add_library(${ENGINE} ${ENGINE_CPP_SOURCES} ${ENGINE_CPP_HEADERS} ${RESOURCES})
target_compile_options(${ENGINE} PRIVATE ${COMPILE_OPTS})
target_compile_definitions(${ENGINE} PRIVATE ${GLM_DEFINITIONS} _CRT_SECURE_NO_WARNINGS IMGUI_USER_CONFIG="Platform/imgui/imconfig.h")
target_include_directories(${ENGINE} PUBLIC 
    ${VULKAN_INCLUDE_DIR} 
    ${COMMON_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS} 
    ${GCH_SMALL_VEC_INCLUDE_DIR} 
    ${ABYSS_FREETYPE_INCLUDE_DIR}
    ${CEF_INCLUDE_DIR}
    ${CEF_INTERNAL_INCLUDE_DIR}
)
set_target_properties(${ENGINE} PROPERTIES FOLDER "Abyss")
target_link_libraries(${ENGINE} PRIVATE
    ${VULKAN_LIB_PATH}
    ${SHADER_C_LIB_PATH}
    ${SPIRV_CROSS_LIB_PATH}
    ${SPIRV_CROSS_GLSL_LIB_PATH}
    ${PLATFORM_LIBS}
    AbyssFTLib
    glfw
    ImGui
    ${CEF_LIB_DIR}/libcef.lib
    libcef_dll_wrapper
)
add_dependencies(${ENGINE} AbyssFTLib glfw tool)

## Editor
add_executable(${EDITOR} ${EDITOR_CPP_SOURCES} ${EDITOR_CPP_HEADERS} ${RESOURCES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EDITOR})
target_link_options(${EDITOR} PRIVATE ${LINK_OPTS})
target_compile_options(${EDITOR} PRIVATE ${COMPILE_OPTS})
target_compile_definitions(${EDITOR} PRIVATE ${GLM_DEFINITIONS})
target_include_directories(${EDITOR} PUBLIC Source/Public ${COMMON_INCLUDE_DIRS} )
set_target_properties(${EDITOR} PROPERTIES FOLDER "Abyss")
target_link_libraries(${EDITOR} PRIVATE ${ENGINE})
add_dependencies(${EDITOR} ${ENGINE})
source_group("Public" FILES ${EDITOR_CPP_HEADERS})
source_group("Private" FILES ${EDITOR_CPP_SOURCES})

## Plugin Test
add_library(PlugTest SHARED "Source/Plugins/Plug.cpp")
target_link_libraries(PlugTest PUBLIC ${ENGINE})
target_include_directories(PlugTest PUBLIC Source/Public ${COMMON_INCLUDE_DIRS})
target_link_options(PlugTest PRIVATE ${LINK_OPTS})
target_compile_options(PlugTest PRIVATE ${COMPILE_OPTS})

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
    
    file(GLOB CEF_DLLS "${CEF_LIB_DIR}/*.dll")
    file(GLOB CEF_PDBS "${CEF_LIB_DIR}/*.pdb")
    file(GLOB CEF_DATA "${CEF_LIB_DIR}/*.bin")
    file(GLOB CEF_JSON "${CEF_LIB_DIR}/*.json")

    copy_files_post_build(${EDITOR} CEF_DLLS)
    copy_files_post_build(${EDITOR} CEF_PDBS)
    copy_files_post_build(${EDITOR} CEF_DATA)
    copy_files_post_build(${EDITOR} CEF_JSON)

    add_custom_command(TARGET ${EDITOR} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CEF_ROOT}/Resources ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
        COMMENT "Copy CEF resources"
    )

    add_custom_command(TARGET ${EDITOR} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Checking and copying DLLs..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADERC_DLL_IN} ${SHADERC_DLL_OUT}
        COMMENT "Copy necessary DLLs"
    )
endif()

add_custom_target(run_post_build ALL DEPENDS ${EDITOR})
set_target_properties(run_post_build PROPERTIES FOLDER "Abyss/Tools")

