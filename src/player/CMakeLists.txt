project(Player)

include(ProjectWarnings)

include (PyEnv)
create_python_venv("venv" PYTHON_VENV_EXECUTABLE)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

file(GLOB_RECURSE HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.hxx
)

file(GLOB_RECURSE GLSL_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.vert
	${CMAKE_CURRENT_SOURCE_DIR}/*.frag
	${CMAKE_CURRENT_SOURCE_DIR}/*.comp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

set_project_warnings(${PROJECT_NAME})

set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/)

set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(SHADER_INCLUDE_DIR ${GENERATED_INCLUDE_DIR}/shaders)

set(CMAKE_INCLUDE_CURRENT_DIR on)

foreach(GLSL ${GLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${GLSL} NAME)
	set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
		DEPENDS ${GLSL}
	)
	set(SPIRV_H "${SHADER_INCLUDE_DIR}/${FILE_NAME}.h")

	add_custom_command(
		OUTPUT ${SPIRV_H}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_INCLUDE_DIR}
		COMMAND ${PYTHON_VENV_EXECUTABLE} -m grafkit_tools.hexdump -n ${FILE_NAME} -i ${SPIRV} -o ${SPIRV_H} -t ${CMAKE_CURRENT_SOURCE_DIR}/file.j2
		DEPENDS ${SPIRV}
		VERBATIM
	)

	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
	list(APPEND SPIRV_HEADER_FILES ${SPIRV_H})
endforeach(GLSL)

add_custom_target(${PROJECT_NAME}_Shaders DEPENDS ${SPIRV_BINARY_FILES} ${SPIRV_HEADER_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_Shaders)

target_link_libraries(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${GENERATED_INCLUDE_DIR})
target_link_libraries(
	${PROJECT_NAME}
	Grafkit::Grafkit
	Grafkit::GrafkitLoader
)

# Install?
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
