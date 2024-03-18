# Allows run custom code generation using prototypes

# Bootsreap

include(CMakeParseArguments)
include (PyEnv)

if(DEFINED ENV{CLANG_PATH})
  set(CLANG_PATH "$ENV{CLANG_PATH}")
else()
  if(NOT CLANG_PATH)
    message(FATAL_ERROR "Variable CLANG_PATH is not set")
  endif()
endif()

# ---
#
# ---

function(initialize_code_generator PACKAGE_PATH)
  create_python_venv("venv" PYTHON_VENV_EXECUTABLE)

  if (NOT _PYTHON_CLANG_BINDING_OK)

      message("${PYTHON_VENV_EXECUTABLE} -m pip install ${PACKAGE_PATH}")
      execute_process(
          COMMAND ${PYTHON_VENV_EXECUTABLE} -m pip install ${PACKAGE_PATH}
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          RESULT_VARIABLE _error_code
          OUTPUT_VARIABLE _result
      )

    #   message("${PYTHON_VENV_EXECUTABLE} ${PROJECT_SOURCE_DIR}/cmake/test_clang.py ${CLANG_PATH}")

    #   execute_process(
    #     COMMAND ${PYTHON_VENV_EXECUTABLE} ${PROJECT_SOURCE_DIR}/cmake/test_clang.py ${CLANG_PATH}
    #     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    #     RESULT_VARIABLE _error_code
    #     OUTPUT_VARIABLE _result
    # )

    #   string(REGEX REPLACE "\n$" "" _result "${_result}")

    #   if (_error_code MATCHES "0")
    #       message("Python Clang binding OK")
    #   else ()
    #       message(FATAL_ERROR "Python not installed or requirements not properly set up.\n Exit code: ${_error_code}\n Message: ${_result}")
    #   endif ()

    #   # Setup cli script paths
    #   get_filename_component(_PY_VENV_LOCATION ${PYTHON_VENV_EXECUTABLE} DIRECTORY)

    #   #TODO: If win32 ...
    #   set(_COLLECT_SCRIPT ${_PY_VENV_LOCATION}/source_index.exe)

    #   set(_PYTHON_CLANG_BINDING_OK True)

  endif ()

endfunction()

# ---
# Usage:
# TARGET target to add to
# PRE_BUILD, POST_BUILD or PRE_LINK (PRE_BUILD default)
# SCRIPT script path
#

function(_add_execute_script_target)
    if (NOT _PYTHON_CLANG_BINDING_OK)
        message(FATAL_ERROR "Python clang script is not installed or not properly set up.")
    endif ()

    cmake_parse_arguments(
        ARGS # prefix
        "" # flags
        "TARGET;SCRIPT;SUFFIX" # single-values
        "OPTIONS"   # lists
        ${ARGN}
    )

    if (NOT ARGS_TARGET AND NOT TARGET ${ARGS_TARGET})
        message(FATAL_ERROR "You must provide a target in parameter TARGET")
    endif ()

    if (NOT ARGS_SCRIPT)
        message(FATAL_ERROR "You must provide parameter SCRIPT")
    elseif (NOT EXISTS "${ARGS_SCRIPT}")
        message(FATAL_ERROR "Script ${ARGS_SCRIPT} does not exist")
    endif ()

    set(_command ${ARGS_SCRIPT} ${ARGS_OPTIONS})

    add_custom_command(
        TARGET ${ARGS_TARGET}
        PRE_BUILD
        COMMAND ${_command}
        COMMAND_EXPAND_LISTS
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "${_command}"
    )

endfunction(_add_execute_script_target)


# ---

# Usage:
#
# TARGET target which generation will attached to
# OUT_DIR where collected translations are collected
# SOURCES sources to be scanned
# INPUT_DIR directory where sources are

function(add_collect_translations_target)

    if (NOT _PYTHON_CLANG_BINDING_OK)
        message(FATAL_ERROR "Clang Python binding Sctipt is not installed or not properly set up.")
    endif ()

    cmake_parse_arguments(
        ARGS                 # prefix
        ""                   # flags
        "TARGET;INPUT_DIRS;OUT_DIR"     # single-values
        "SOURCES;INCLUDE_DIRS" # lists
        ${ARGN}
    )

    if (NOT ARGS_TARGET AND NOT TARGET ${ARGS_TARGET})
        message(FATAL_ERROR "You must provide a valid target")
    endif ()

    set(_input_dirs ${ARGS_INPUT_DIRS})
    if (NOT ARGS_INPUT_DIRS)
        message(WARNING "No input directories were given")
        set(_input_dirs ${CMAKE_CURRENT_SOURCE_DIR})
    endif ()

    if (NOT TARGET ${ARGS_TARGET}_collect)
        add_custom_target(${ARGS_TARGET}_collect)
    endif ()

    set(_include_dirs)
    if (ARGS_INCLUDE_DIRS)
        set(_include_dirs "-I" ${ARGS_INCLUDE_DIRS})
    endif (ARGS_INCLUDE_DIRS)

    _add_execute_script_target(
        TARGET ${ARGS_TARGET}_collect PRE_BUILD
        SCRIPT ${_COLLECT_SCRIPT}
        OPTIONS "-i" ${ARGS_SOURCES} "-d" ${_input_dirs} "-o" ${ARGS_OUT_DIR} "-t" ${ARGS_TARGET} "-c" ${CLANG_PATH} ${_include_dirs})
    add_dependencies(${ARGS_TARGET} ${ARGS_TARGET}_collect)

endfunction(add_collect_translations_target)
