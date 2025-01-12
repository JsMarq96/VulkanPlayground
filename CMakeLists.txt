cmake_minimum_required(VERSION 3.21)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_PARALLEL_LEVEL 6)
set(CMAKE_CXX_STANDARD 23)

option(CLANG_TIME_TRACE "Enable clang profiling." ON)

project(VulkanPlayground)

set(SRC_FILE_DIR "src/")

macro(ADD_SOURCE)
	file(GLOB H_FILES_APPEND "${ARGV0}/*.h")
	file(GLOB CPP_FILES_APPEND "${ARGV0}/*.cpp")
	file(GLOB C_FILES_APPEND "${ARGV0}/*.c")

	list(APPEND HEADER_FILES ${H_FILES_APPEND})
	list(APPEND SOURCE_FILES ${CPP_FILES_APPEND})
	list(APPEND SOURCE_FILES ${C_FILES_APPEND})
endmacro()

file(GLOB RAW_SHADERS "shaders/*.vert" "shaders/*.frag" "shaders/*.comp")

ADD_SOURCE("src/")
ADD_SOURCE("src/parsers")
ADD_SOURCE("src/render")
ADD_SOURCE("src/render/stages")
ADD_SOURCE("src/render/resources")
ADD_SOURCE("src/utils")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CLANG_TIME_TRACE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
endif()

# ImGUI
include_directories(libraries/imgui/)
include_directories(libraries/imgui/backends/)
set(IMGUI_IMPL_SRC libraries/imgui/backends/imgui_impl_glfw.cpp 
                   libraries/imgui/backends/imgui_impl_opengl3.cpp)
file(GLOB IMGUI_SRC "libraries/imgui/*.cpp")

add_executable(VulkanPlayground ${IMGUI_IMPL_SRC} ${IMGUI_SRC} ${HEADER_FILES} ${SOURCE_FILES})
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

add_subdirectory(libraries/fastgltf)

find_package(Vulkan REQUIRED)

include_directories(libraries/stb)

target_link_libraries(VulkanPlayground glm fastgltf glfw vk-bootstrap spdlog::spdlog GPUOpen::VulkanMemoryAllocator ${Vulkan_LIBRARIES})

include_directories(${Vulkan_INCLUDE_DIR} ${HEADER_FILES} ${SOURCE_FILES})

# Custom commands

# Copy Shaders & resources to folder to binary path
add_custom_command(	TARGET VulkanPlayground POST_BUILD
                   	COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_command(	TARGET VulkanPlayground POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory
				 ${CMAKE_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources)


# Parallel build
add_custom_target(my_parallel_build 
                      COMMAND ${CMAKE_COMMAND} --build -j ${CMAKE_BUILD_PARALLEL_LEVEL}
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})