# GrafkitVE - Grafkit Vulkan Edition Copyright (C) 2020-2021 by Caiwan
#
# Licensed under MIT License See LICENSE for details

cmake_minimum_required(VERSION 3.16)

project(
  GrafkitVE
  VERSION 0.0.0.0
  LANGUAGES C CXX)

# --- Options
option(GK_BUILD_TESTS "Build all tests" ON)
option(GK_BUILD_EDITOR "Build Editor" ON)
option(GK_BUILD_PLAYER "Build Player" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include (PyEnv)
create_python_venv("venv" PYTHON_VENV_EXECUTABLE)

# --- Cache options
if(NOT DEFINED CMAKE_C_COMPILER_LAUNCHER AND NOT DEFINED
                                             CMAKE_CXX_COMPILER_LAUNCHER)
  find_program(CCACHE_FOUND ccache)
  find_program(SCCACHE_FOUND sccache)
  if(CCACHE_FOUND AND NOT SCCACHE_FOUND)
    message("setting CCACHE to ${CCACHE_FOUND}")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})
  elseif(SCCACHE_FOUND AND NOT CCACHE_FOUND)
    message("setting CCACHE to ${CCACHE_FOUND}")
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_FOUND})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_FOUND})
  endif(CCACHE_FOUND AND NOT SCCACHE_FOUND)
endif()

# --- Project and build options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --- Compiler options
if(DEFINED CMAKE_C_COMPILER_LAUNCHER)
  if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG
                     "${CMAKE_CXX_FLAGS_DEBUG}")
      string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
      string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELEASE
                     "${CMAKE_CXX_FLAGS_RELEASE}")
      string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELEASE
                     "${CMAKE_C_FLAGS_RELEASE}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO
                     "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
      string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO
                     "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    endif()
  endif()
endif()

# --- Module and prefix path
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX
    "d"
    CACHE STRING "")

add_subdirectory(thirdparties)
add_subdirectory(src)

if(GK_BUILD_TESTS)
  add_subdirectory(tests)
endif()
