# ----------------------------------------------------------------------------------------
# --- Grafkit Core Library
# ----------------------------------------------------------------------------------------

project(
  Grafkit
  VERSION 3.0.1.0
  LANGUAGES C CXX)

include(ProjectWarnings)

find_package(Vulkan REQUIRED)
# find_package(VulkanMemoryAllocator REQUIRED)

file(GLOB_RECURSE GRAFKIT_SOURCE_FILES *.cpp *.c)

file(GLOB_RECURSE GRAFKIT_HEADER_FILES ${CMAKE_SOURCE_DIR}/include/grafkit/*.h
     ${CMAKE_SOURCE_DIR}/include/grafkit/*.hpp
     ${CMAKE_SOURCE_DIR}/include/grafkit/*.hxx)

# TODO: Setup Git SHA if needed
# https://stackoverflow.com/questions/1435953/how-can-i-pass-git-sha1-to-compiler-as-definition-using-cmake
# configure_file("config.h.in"
# "${CMAKE_BINARY_DIR}/configured_files/internal_use_only/include/grafkit/internal/config.h"
# ESCAPE_QUOTES) list(APPEND GRAFKIT_HEADER_FILES
# "${CMAKE_BINARY_DIR}/configured_files/internal_use_only/include/grafkit/internal/config.h")

add_library(${PROJECT_NAME} STATIC ${GRAFKIT_SOURCE_FILES}
                                   ${GRAFKIT_HEADER_FILES})

# add_target_precompiled_header(${PROJECT_NAME} "grafkit/pch.h"
# "grafkit/pch.cpp")
target_compile_definitions(${PROJECT_NAME} PRIVATE GRAFKIT_LIBRARY_TYPE STATIC)

add_library(Grafkit::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
    # $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/configured_files/internal_use_only/include/>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(
  ${PROJECT_NAME} 
  # PRIVATE GPUOpen::VulkanMemoryAllocator
  PUBLIC Vulkan::Vulkan glfw glm::glm GPUOpen::VulkanMemoryAllocator
)

set_project_warnings(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

# TODO: https://youtu.be/bsXLMQ6WgIk?t=2256

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
