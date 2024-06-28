# ----------------------------------------------------------------------------------------
# --- Grafkit Core Library
# ----------------------------------------------------------------------------------------

project(
  Grafkit
  VERSION 3.0.1.0
  LANGUAGES C CXX)

include(ProjectWarnings)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE GRAFKIT_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hxx
)

file(GLOB_RECURSE GRAFKIT_HEADER_FILES
  ${CMAKE_SOURCE_DIR}/include/grafkit/*.h
  ${CMAKE_SOURCE_DIR}/include/grafkit/*.hpp
  ${CMAKE_SOURCE_DIR}/include/grafkit/*.hxx
)

set(CMAKE_INCLUDE_CURRENT_DIR on)

# Disable all warnings and erros for single header library implementations
set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/core/vma_impl.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/core/stb_impl.cpp
   PROPERTIES COMPILE_FLAGS -w)

add_library(${PROJECT_NAME} STATIC
  ${GRAFKIT_HEADER_FILES}
  ${GRAFKIT_SOURCE_FILES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE GRAFKIT_LIBRARY_TYPE STATIC)

add_library(Grafkit::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    glfw
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    glm::glm
)

# target_precompile_headers(${PROJECT_NAME} PRIVATE
#   "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
# )

set_project_warnings(${PROJECT_NAME})
set_project_clang_format(${PROJECT_NAME})
set_project_clang_tidy(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

include(GNUInstallDirs)
install(
  TARGETS ${PROJECT_NAME} glm glm-header-only
  EXPORT ${PROJECT_NAME}-target
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/grafkit
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT ${PROJECT_NAME}-target
  FILE GrafkitTargets.cmake
  NAMESPACE Grafkit::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grafkit)
