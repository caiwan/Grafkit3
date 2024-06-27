# Helps create and manage venv inside cmake infrastructure without pollute any existing one
find_package(Python3 COMPONENTS Interpreter REQUIRED)

function(create_python_venv PY_ENVIRONMENT_NAME _OUT_PY_EXECUTABLE_PATH)

  set(_PY_DIRECTORY "${CMAKE_BINARY_DIR}/${PY_ENVIRONMENT_NAME}")

  if(WIN32)
    set(_PY_EXECUTABLE_PATH "${_PY_DIRECTORY}/Scripts/python.exe")
  elseif(APPLE)
    set(_PY_EXECUTABLE_PATH "${_PY_DIRECTORY}/bin/python")
  else()
    set(_PY_EXECUTABLE_PATH "${_PY_DIRECTORY}/bin/python")
  endif()

  if (NOT EXISTS "${_PY_EXECUTABLE_PATH}")
    message("Creating pyenv: ${PY_ENVIRONMENT_NAME}")

    execute_process(
      COMMAND ${Python3_EXECUTABLE} -m venv ${_PY_DIRECTORY}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE _error_code
      OUTPUT_VARIABLE _result
    )

    string(REGEX REPLACE "\n$" "" _result "${_result}")

    if (_error_code MATCHES "0")
        message("Python OK")
        set(${_OUT_PY_EXECUTABLE_PATH} "${__OUT_PY_EXECUTABLE_PATH}" PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "Python not installed or requirements not properly set up.\n Exit code: ${_error_code}\n Message: ${_result}")
        unset(${_OUT_PY_EXECUTABLE_PATH})
    endif ()

  else ()
    message("pyenv found: ${PY_ENVIRONMENT_NAME} - path: ${_PY_EXECUTABLE_PATH}")
    set(${_OUT_PY_EXECUTABLE_PATH} "${_PY_EXECUTABLE_PATH}" PARENT_SCOPE)
  endif()

endfunction()
