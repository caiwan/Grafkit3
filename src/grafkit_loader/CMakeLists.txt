# Loader systyem for Grafkit engine

project(
  GrafkitLoader
  VERSION 3.0.1.0
  LANGUAGES C CXX)

include(ProjectWarnings)

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

# TODO: Add Generated files

set(CMAKE_INCLUDE_CURRENT_DIR on)

add_library(${PROJECT_NAME} STATIC
  ${HEADER_FILES}
  ${SOURCE_FILES}
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
	Grafkit::Grafkit
	nlohmann_json::nlohmann_json
)

# target_precompile_headers(${PROJECT_NAME} PRIVATE
#   "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
# )

add_library(Grafkit::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_project_warnings(${PROJECT_NAME})
set_project_clang_format(${PROJECT_NAME})
set_project_clang_tidy(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
