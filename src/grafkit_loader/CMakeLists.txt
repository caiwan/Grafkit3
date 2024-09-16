# Loader systyem for Grafkit engine

project(
  GrafkitLoader
  VERSION 3.0.1.0
  LANGUAGES C CXX)

include(ProjectWarnings)
include(GKSourceTools)

# --- Generated code

set(GENERATED_SOURCES "")

file(GLOB_RECURSE GEN_SERIALIZER_SOURCES ${CMAKE_SOURCE_DIR}/src/grafkit/descriptors/*.gen.yaml)
generate_code_from_yaml_files(
	GENETATE_SINGLE_FILE
	SOURCES ${GEN_SERIALIZER_SOURCES}
	TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/json/json_template.j2
	GENERATED_FILES_ARG "GENERATED_SOURCES"
	TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/json/generated
	TARGET_FILE "json_serializers"
	SUFFIX ".cpp"
)

generate_code_from_yaml_files(
	GENETATE_SINGLE_FILE
	SOURCES ${GEN_SERIALIZER_SOURCES}
	TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/json/json_header.j2
	GENERATED_FILES_ARG "GENERATED_SOURCES"
	TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/json/generated
	TARGET_FILE "json_serializers"
	SUFFIX ".h"
)

add_custom_target(${PROJECT_NAME}_Generated DEPENDS ${GRAFKIT_GENERATED_HEADERS})

# -- Loader library
file(GLOB_RECURSE SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.hxx
)

file(GLOB_RECURSE HEADER_FILES
	${CMAKE_SOURCE_DIR}/include/grafkit_loader/*.h
	${CMAKE_SOURCE_DIR}/include/grafkit_loader/*.hpp
	${CMAKE_SOURCE_DIR}/include/grafkit_loader/*.hxx
)

set(CMAKE_INCLUDE_CURRENT_DIR on)

set (ALL_SOURCE_FILES
	${HEADER_FILES}
	${SOURCE_FILES}
	${GENERATED_SOURCES}
)

list(REMOVE_DUPLICATES SOURCE_FILES)
add_library(${PROJECT_NAME} STATIC ${ALL_SOURCE_FILES} )

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
		$<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Grafkit::Grafkit
		glm::glm
		nlohmann_json::nlohmann_json
		assimp
)

# target_precompile_headers(${PROJECT_NAME} PRIVATE
#   "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
# )

add_library(Grafkit::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_project_warnings(${PROJECT_NAME})
set_project_clang_format(${PROJECT_NAME})
set_project_clang_tidy(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
