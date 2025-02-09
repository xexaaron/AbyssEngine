cmake_minimum_required(VERSION 3.28.3)
project(Abyss)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

set(VULKAN_DIR "Vendor/VulkanSDK")
set(VULKAN_INCLUDE_DIR "${VULKAN_DIR}/include")
set(VULKAN_LIBRARY_DIR "${VULKAN_DIR}/lib")
set(VULKAN_BIN_DIR "${VULKAN_DIR}/bin")
set(SHADER_C_INCLUDE_DIR "${VULKAN_INCLUDE_DIR}/shaderc")

set(GLFW_DIR "Vendor/glfw")
set(GLFW_INCLUDE_DIR "${GLFW_DIR}/include")
set(GLFW_LIBRARY_DIR "${CMAKE_BINARY_DIR}/Vendor/glfw/src/${CMAKE_BUILD_TYPE}")
get_filename_component(GLFW_LIB_PATH "${GLFW_LIBRARY_DIR}/glfw3.lib" ABSOLUTE)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW docs")
add_subdirectory(${GLFW_DIR})

set(STB_INCLUDE_DIR "Vendor/stb")
set(STB_IMAGE_IMPL "${STB_INCLUDE_DIR}/stb_image/stb_image.cpp")

set(PACKAGER_DIR "package")
add_subdirectory(${PACKAGER_DIR})

set(GLM_DIR "Vendor/glm")
set(GLM_INCLUDE_DIR ${GLM_DIR})
add_subdirectory(${GLM_DIR})

set(CPP_SOURCES
    Source/main.cpp
    Source/Private/Core/App.cpp
    Source/Private/Core/Window.cpp
    Source/Private/Core/Event.cpp
    Source/Private/Core/Serialize.cpp
    Source/Private/Core/Object.cpp
    Source/Private/Core/Resource.cpp
    Source/Private/Rendering/Context.cpp
    Source/Private/Rendering/Renderer.cpp
    Source/Private/Rendering/Camera.cpp
    Source/Private/Rendering/Viewport.cpp
    Source/Private/Rendering/Shader.cpp
    Source/Private/Rendering/Texture.cpp
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
    ${STB_IMAGE_IMPL}
)

# Add header files and group them in the "Include" folder
set(HEADER_FILES
    Source/Public/Core/Log.h
    Source/Public/Core/Common.h
    Source/Public/Core/Event.h
    Source/Public/Core/App.h
    Source/Public/Core/Window.h
    Source/Public/Core/Object.h
    Source/Public/Core/Serialize.h
    Source/Public/Core/Resource.h
    Source/Public/Rendering/Context.h
    Source/Public/Rendering/Renderer.h
    Source/Public/Rendering/Camera.h
    Source/Public/Rendering/Viewport.h
    Source/Public/Rendering/Shader.h
    Source/Public/Rendering/Texture.h
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
)

set(SHADER_SOURCES 
    Source/Shaders/Default2D.vert
    Source/Shaders/Default2D.frag
    Source/Shaders/Default3D.vert
    Source/Shaders/Default3D.frag
)

source_group("Public/Core" FILES
    Source/Public/Core/App.h
    Source/Public/Core/Common.h
    Source/Public/Core/Log.h
    Source/Public/Core/Renderer.h
    Source/Public/Core/Window.h
    Source/Public/Core/Event.h
    Source/Public/Core/Serialize.h
    Source/Public/Core/Object.h
    Source/Public/Core/Resource.h
) 

source_group("Public/Rendering" FILES
    Source/Public/Rendering/Context.h
    Source/Public/Rendering/Renderer.h
    Source/Public/Rendering/Camera.h
    Source/Public/Rendering/Viewport.h
    Source/Public/Rendering/Shader.h
    Source/Public/Rendering/Texture.h
)

source_group("Public/vk" FILES
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
    Source/Public/vk/VkRenderer.h
    Source/Public/vk/VkBuffer.h
    Source/Public/vk/VkShader.h
    Source/Public/vk/VkDescriptorPool.h
    Source/Public/vk/VkPipeline.h
    Source/Public/vk/VkRenderModule.h
    Source/Public/vk/VkTexture.h
)

source_group("Shaders" FILES ${SHADER_SOURCES})


source_group("Private" FILES
    Source/main.cpp
    ${STB_IMAGE_IMPL}
)

source_group("Private/Core" FILES
    Source/Private/Core/App.cpp
    Source/Private/Core/Window.cpp
    Source/Private/Core/Event.cpp
    Source/Private/Core/Serialize.cpp
    Source/Private/Core/Object.cpp
    Source/Private/Core/Resource.cpp
)
 
source_group("Private/Rendering" FILES
    Source/Private/Rendering/Context.cpp
    Source/Private/Rendering/Renderer.cpp
    Source/Private/Rendering/Camera.cpp
    Source/Private/Rendering/Viewport.cpp
    Source/Private/Rendering/Shader.cpp
    Source/Private/Rendering/Texture.cpp
)

source_group("Private/vk" FILES
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
    Source/Private/vk/VkRenderer.cpp
    Source/Private/vk/VkBuffer.cpp
    Source/Private/vk/VkShader.cpp
    Source/Private/vk/VkDescriptorPool.cpp
    Source/Private/vk/VkPipeline.cpp
    Source/Private/vk/VkRenderModule.cpp
    Source/Private/vk/VkTexture.cpp
)

add_executable(${PROJECT_NAME} ${CPP_SOURCES} ${HEADER_FILES} ${SHADER_SOURCES})
add_dependencies(${PROJECT_NAME} Package)

target_include_directories(${PROJECT_NAME} PUBLIC
    Source/Public
    ${VULKAN_INCLUDE_DIR} 
    ${GLFW_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_SUFFIX "d")
else()
    set(DEBUG_SUFFIX "")
endif()
   
get_filename_component(VULKAN_LIB_PATH "${VULKAN_LIBRARY_DIR}/vulkan-1.lib" ABSOLUTE)
get_filename_component(SHADER_C_LIB_PATH "${VULKAN_LIBRARY_DIR}/shaderc_shared${DEBUG_SUFFIX}.lib" ABSOLUTE)
get_filename_component(SPIRV_CROSS_LIB_PATH "${VULKAN_LIBRARY_DIR}/spirv-cross-core${DEBUG_SUFFIX}.lib" ABSOLUTE)
get_filename_component(SPIRV_CROSS_GLSL_LIB_PATH "${VULKAN_LIBRARY_DIR}/spirv-cross-glsl${DEBUG_SUFFIX}.lib" ABSOLUTE)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${VULKAN_LIB_PATH}
    ${SHADER_C_LIB_PATH}
    ${SPIRV_CROSS_LIB_PATH}
    ${SPIRV_CROSS_GLSL_LIB_PATH}
)

target_link_libraries(${PROJECT_NAME} PRIVATE "${GLFW_LIB_PATH}")

