project(Sample)

include(ProjectWarnings)
include(GKSourceTools)

find_package(Vulkan REQUIRED)

option(GRAFKIT_ENABLE_STRIP_DEBUG_SYMBOLS "Strip debug symbols from executables" OFF)

# MARK: Shaders

file(GLOB_RECURSE GLSL_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp
)

set(SHADER_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/)
set(SPIRV_EMBED_SOURCE_FILES "")

spirv_compile_shaders(
	SOURCES ${GLSL_SOURCE_FILES}
	SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders
	SHADER_INCLUDE_DIR ${SHADER_INCLUDE_DIR}/shaders
	GENERATED_FILES_ARG SPIRV_EMBED_SOURCE_FILES
)
set(SHADER_LIBRARY_NAME ${PROJECT_NAME}_Shaders)

add_library(${SHADER_LIBRARY_NAME} STATIC ${GLSL_SOURCE_FILES} ${SPIRV_EMBED_SOURCE_FILES})
target_include_directories(${SHADER_LIBRARY_NAME} PUBLIC ${SHADER_INCLUDE_DIR})

set_target_properties(${SHADER_LIBRARY_NAME} PROPERTIES FOLDER "Samples")
set_target_properties(${SHADER_LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE C)

include(GNUInstallDirs)

# MARK: Samples

macro(add_sample name sources)
	set(EXECUTABLE_NAME ${PROJECT_NAME}_${name})
	add_executable(${EXECUTABLE_NAME} ${sources})

	set_project_warnings(${EXECUTABLE_NAME})
	set_project_clang_format(${EXECUTABLE_NAME})
	set_project_clang_tidy(${EXECUTABLE_NAME})
	target_link_libraries(${EXECUTABLE_NAME}
		${SHADER_LIBRARY_NAME}
		Grafkit::Grafkit
		Grafkit::GrafkitLoader
	)

	if(GRAFKIT_ENABLE_STRIP_DEBUG_SYMBOLS AND UNIX AND NOT APPLE)
		add_custom_target(${EXECUTABLE_NAME}_strip_debug_symbols ALL
			COMMAND ${CMAKE_STRIP} --only-keep-debug $<TARGET_FILE:${EXECUTABLE_NAME}> -o $<TARGET_FILE:${EXECUTABLE_NAME}>.debug
			COMMAND ${CMAKE_STRIP} --strip-debug --strip-unneeded $<TARGET_FILE:${EXECUTABLE_NAME}>
			COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE:${EXECUTABLE_NAME}>.debug $<TARGET_FILE:${EXECUTABLE_NAME}>
			DEPENDS ${EXECUTABLE_NAME}
		)
		set_target_properties(${EXECUTABLE_NAME}_strip_debug_symbols  PROPERTIES FOLDER "Utilities")
		add_dependencies(${EXECUTABLE_NAME}_strip_debug_symbols ${EXECUTABLE_NAME})
	endif()

	set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "Samples")
	install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

add_sample(cubes
	${CMAKE_CURRENT_SOURCE_DIR}/cubes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cube_mesh.h
)

add_sample(bloom
	${CMAKE_CURRENT_SOURCE_DIR}/bloom.cpp
)
