cmake_minimum_required(VERSION 3.21)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_PARALLEL_LEVEL 6)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_GENERATOR_PLATFORM x64)

project(VulkanPlayground)

file(GLOB H_SOURCES "src/*.h")
file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB C_SOURCES "src/*.c")
file(GLOB SUBFOLDER_H_SOURCES "src/*/*.h")
file(GLOB SUBFOLDER_CPP_SOURCES "src/*/*.cpp")
file(GLOB SUBFOLDER_C_SOURCES "src/*/*.c")
file(GLOB RAW_SHADERS "shaders/*.vert" "shaders/*.frag")

# ImGUI
include_directories(libraries/imgui/)
include_directories(libraries/imgui/backends/)
set(IMGUI_IMPL_SRC libraries/imgui/backends/imgui_impl_glfw.cpp 
                   libraries/imgui/backends/imgui_impl_opengl3.cpp)
file(GLOB IMGUI_SRC "libraries/imgui/*.cpp")

add_executable(VulkanPlayground ${IMGUI_IMPL_SRC} ${IMGUI_SRC} ${C_SOURCES} ${CPP_SOURCES} ${H_SOURCES} ${SUBFOLDER_H_SOURCES} ${SUBFOLDER_C_SOURCES} ${SUBFOLDER_CPP_SOURCES})
set_target_properties(VulkanPlayground PROPERTIES OUTPUT_NAME "VulkanPlayground")

# Shader compiling
set(SHADERS_COMPILED_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(MAKE_DIRECTORY ${SHADERS_COMPILED_DIR})
foreach(RAW_SHADER_DIR ${RAW_SHADERS})
	get_filename_component(SHADER_NAME ${RAW_SHADER_DIR} NAME)
	set(SHADER_OUT_NAME "${SHADERS_COMPILED_DIR}/${SHADER_NAME}.spv")
	list(APPEND SHADER_OUT_NAMES ${SHADER_OUT_NAME})
	add_custom_command(
		MAIN_DEPENDENCY ${RAW_SHADER_DIR}
		OUTPUT ${SHADER_OUT_NAME}
		COMMAND "glslc"  ${RAW_SHADER_DIR} "-o" ${SHADER_OUT_NAME}
		VERBATIM)
endforeach()

add_custom_target(build_shaders DEPENDS ${SHADER_OUT_NAMES})

add_dependencies(VulkanPlayground build_shaders)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/glfw)

add_subdirectory(libraries/glm)

add_subdirectory(libraries/vk-bootstrap)

add_subdirectory(libraries/spdlog)

add_subdirectory(libraries/VMA)

find_package(Vulkan REQUIRED)

target_link_libraries(VulkanPlayground glm glfw vk-bootstrap spdlog::spdlog GPUOpen::VulkanMemoryAllocator ${Vulkan_LIBRARIES})

include_directories(${Vulkan_INCLUDE_DIR} ${C_SOURCES} ${CPP_SOURCES} ${H_SOURCES} ${SUBFOLDER_H_SOURCES} ${SUBFOLDER_C_SOURCES} ${SUBFOLDER_CPP_SOURCES})

add_custom_command(TARGET VulkanPlayground POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:VulkanPlayground>/resources)