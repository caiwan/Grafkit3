project(Sample)

include(ProjectWarnings)
include(GKSourceTools)

find_package(Vulkan REQUIRED)

# Mark: Shaders

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

add_library(${PROJECT_NAME}-Shaders STATIC ${GLSL_SOURCE_FILES} ${SPIRV_EMBED_SOURCE_FILES})
target_include_directories(${PROJECT_NAME}-Shaders PUBLIC ${SHADER_INCLUDE_DIR})
set_target_properties(${PROJECT_NAME}-Shaders PROPERTIES FOLDER "Samples")
set_target_properties(${PROJECT_NAME}-Shaders PROPERTIES LINKER_LANGUAGE C)

include(GNUInstallDirs)

macro(add_sample name sources)
	add_executable(${PROJECT_NAME}-${name} ${sources})
	set_project_warnings(${PROJECT_NAME}-${name})
	set_project_clang_format(${PROJECT_NAME}-${name})
	set_project_clang_tidy(${PROJECT_NAME}-${name})
	target_link_libraries(${PROJECT_NAME}-${name}
		${PROJECT_NAME}-Shaders
		Grafkit::Grafkit
		Grafkit::GrafkitLoader
	)
	set_target_properties(${PROJECT_NAME}-${name} PROPERTIES FOLDER "Samples")
	install(TARGETS ${PROJECT_NAME}-${name} DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

add_sample(01_Cubes
	${CMAKE_CURRENT_SOURCE_DIR}/cubes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cube_mesh.h
)

add_sample(02_Bloom
	${CMAKE_CURRENT_SOURCE_DIR}/bloom.cpp
)
