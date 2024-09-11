# Helps create and manage venv inside cmake infrastructure without pollute any existing one
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Creates a python environment
#
# Parameters:
# - PY_ENVIRONMENT_NAME: The name of the environment
# - _OUT_PY_EXECUTABLE_PATH: The output variable that will store the path to the python executable
function(create_python_venv PY_ENVIRONMENT_NAME _OUT_PY_EXECUTABLE_PATH)

  set(_PY_DIRECTORY "${CMAKE_BINARY_DIR}/${PY_ENVIRONMENT_NAME}")

  if(WIN32)
	set(_PY_EXECUTABLE_PATH "${_PY_DIRECTORY}/Scripts/python.exe")
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

	# _execute_pyton(MODULE "pip" OPTIONS install --upgrade setuptools setuptools_scm pip wheel)
	# _execute_pyton(MODULE "pip" OPTIONS install "cython<3.0.0")

	string(REGEX REPLACE "\n$" "" _result "${_result}")

	if (_error_code MATCHES "0")
		message("Python OK")
		set(${_OUT_PY_EXECUTABLE_PATH} "${_PY_EXECUTABLE_PATH}" PARENT_SCOPE)

		execute_process(
	  		COMMAND ${_PY_EXECUTABLE_PATH} -m pip install --upgrade pip wheel setuptools setuptools_scm cython
	  		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	  		RESULT_VARIABLE _error_code
	  		OUTPUT_VARIABLE _result
		)


	else ()
		message(FATAL_ERROR "Python not installed or requirements not properly set up.\n Exit code: ${_error_code}\n Message: ${_result}")
		unset(${_OUT_PY_EXECUTABLE_PATH})
	endif ()

  else ()
	message("pyenv found: ${PY_ENVIRONMENT_NAME} - path: ${_PY_EXECUTABLE_PATH}")
	set(${_OUT_PY_EXECUTABLE_PATH} "${_PY_EXECUTABLE_PATH}" PARENT_SCOPE)
  endif()

endfunction()

# Executes a python command and returns the result
#
# Parameters:
# - SCRIPT: The python script to execute
# - MODULE: The python module to execute
# - RESULT: The output variable that will store the result
# - RETURNCODE: The output variable that will store the return code
# - OPTIONS: The options to pass to the script or module
function(execute_pyton)
	if (NOT PYTHON_VENV_EXECUTABLE)
		message(FATAL_ERROR "Python executable is not set")
	endif ()

	cmake_parse_arguments(
		ARGS # prefix
		""
		"SCRIPT;MODULE;RESULT,RETURNCODE" # single-values
		"OPTIONS"	 # lists
		${ARGN}
	)

	if (ARGS_MODULE)
		set(_command ${PYTHON_VENV_EXECUTABLE} "-m" ${ARGS_MODULE} ${ARGS_OPTIONS})
	elseif (ARGS_SCRIPT)
		set(_command ${PYTHON_VENV_EXECUTABLE} ${ARGS_SCRIPT} ${ARGS_OPTIONS})
	else ()
		message(FATAL_ERROR "You must provide a valid script or module")
	endif ()

	if (ARGS_RESULT)
		set(_result ${ARGS_RESULT})
	else ()
		set(_result "_result")
	endif ()

	if (ARGS_RETURNCODE)
		set(_error_code ${ARGS_RETURNCODE})
	else ()
		set(_error_code "_error_code")
	endif ()

	execute_process(
		COMMAND ${_command}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		RESULT_VARIABLE ${_error_code}
		OUTPUT_VARIABLE ${_result}
		COMMAND_ECHO STDOUT
	)

endfunction(execute_pyton)
