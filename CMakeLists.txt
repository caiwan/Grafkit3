# GrafkitVE - Grafkit Vulkan Edition Copyright (C) 2020-2021 by Caiwan
#
# Licensed under MIT License See LICENSE for details

cmake_minimum_required(VERSION 3.16)

project(
  GrafkitVE
  VERSION 3.0.1.0
  DESCRIPTION "Grafkit Engine Vulkan Edition"
  LANGUAGES C CXX)

# --- Options
option(GK_BUILD_TESTS "Build all tests" ON)
option(GK_BUILD_EDITOR "Build Editor" OFF)
option(GK_BUILD_PLAYER "Build Player" ON)
option(GK_USE_CPACK "Pack project with CPack" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(PyEnv)
include(GKSourceTools)
create_python_venv("venv" PYTHON_VENV_EXECUTABLE)
initialize_source_tools(EDITABLE PACKAGE "${CMAKE_SOURCE_DIR}/tools")

# --- Project and build options
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --- Module and prefix path
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX
    "d"
    CACHE STRING ""
)

if(NOT CMAKE_CONFIGURATION_TYPES)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(_DEBUG)
    endif()
else()
    add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
endif()

add_subdirectory(thirdparties)
add_subdirectory(src)

if(GK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(GK_USE_CPACK)
  set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
  set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.MD")
  set(CPACK_PACKAGE_CONTACT "${PROJECT_HOMEPAGE_URL}")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.MD")
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}")
  include(CPack)
endif()
