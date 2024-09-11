# Allows run custom code generation using prototypes
# Bootsrap

include(CMakeParseArguments)
include (PyEnv)

# ---
# Initialize code generator
# Usage:
# PACKAGE_PATH path to the package to install
#
# This function will install the package and check if the clang is available
# ---
function(initialize_source_tools)
	create_python_venv("venv" PYTHON_VENV_EXECUTABLE)
	cmake_parse_arguments(
		ARGS # prefix
		"EDITABLE"
		"PACKAGE" # single-values
		""	 # lists
		${ARGN}
	)

	if (NOT ARGS_PACKAGE)
		message(FATAL_ERROR "You must provide a package name")
	endif ()

	if (NOT _GK_PYTHON_TOOLS_OK)
		if (ARGS_EDITABLE)
			_execute_pyton(MODULE "pip" OPTIONS install --editable ${ARGS_PACKAGE})
		else ()
			_execute_pyton(MODULE "pip" OPTIONS install ${ARGS_PACKAGE})
		endif ()

		if (WIN32)
			get_filename_component(PYTHON_VENV_SCRIPT_PATH ${PYTHON_VENV_EXECUTABLE} DIRECTORY)
			set(CLANG_CHECK_PATH ${PYTHON_VENV_SCRIPT_PATH}/check-clang.exe)
			set(_GENERATE_CODE_PATH ${PYTHON_VENV_SCRIPT_PATH}/generate-code.exe CACHE STRING "Generate code script path" FORCE)
		else ()
			get_filename_component(PYTHON_VENV_SCRIPT_PATH ${PYTHON_VENV_EXECUTABLE} DIRECTORY)
			set(CLANG_CHECK_PATH ${PYTHON_VENV_SCRIPT_PATH}/check-clang)
			set(_GENERATE_CODE_PATH ${PYTHON_VENV_SCRIPT_PATH}/generate-code CACHE STRING "Generate code script path" FORCE)
		endif ()

		set(_GK_PYTHON_TOOLS_OK True PARENT_SCOPE CACHE BOOL "Python clang binding OK" FORCE)
	endif ()
endfunction()

# ---
# Add code generation target
#
# Usage:
# TARGET target which generation will attached to
# OUT_DIR where collected translations are collected
# SOURCES sources to be scanned
# INPUT_DIR directory where sources are
#
# Allows run custom code generation using prototypes
# ---
function(add_code_generation_target)
	if(NOT _GK_PYTHON_TOOLS_OK)
		message(FATAL_ERROR "Clang Python binding Sctipt is not installed or not properly set up.")
	endif()

	cmake_parse_arguments(
		ARGS # prefix
		"" # flags
		"TARGET;OUT_DIR;CONFIG_FILE;PREFIX;SUFFIX" # single-values
		"SOURCES;INCLUDE_DIRS" # lists
		${ARGN}
	)

	if(NOT ARGS_TARGET AND NOT TARGET ${ARGS_TARGET})
		message(FATAL_ERROR "You must provide a valid target")
	endif()

	if(NOT ARGS_CONFIG_FILE)
		message(FATAL_ERROR "You must provide a valid output directory")
	endif()

	set(GEN_TARGET "${ARGS_TARGET}")

	if(NOT TARGET ${GEN_TARGET})
		add_custom_target(${GEN_TARGET})
	endif()

	set(_include_dirs)
	if(ARGS_INCLUDE_DIRS)
		set(_include_dirs "-I" ${ARGS_INCLUDE_DIRS})
	endif(ARGS_INCLUDE_DIRS)

	foreach (SOURCE ${ARGS_SOURCES})
		get_filename_component(SOURCE_FILENAME ${SOURCE} NAME_WE)
		set(_GENERATED_SOURCE_FILE ${ARGS_OUT_DIR}/${ARGS_PREFIX}${SOURCE_FILENAME}${ARGS_SUFFIX})

		message("Generating code for ${_GENERATE_CODE_PATH} --config ${ARGS_CONFIG_FILE} ${_include_dirs} --input ${SOURCE} --dir ${ARGS_OUT_DIR}")

		add_custom_command(
			OUTPUT ${_GENERATED_SOURCE_FILE}
			COMMAND ${_GENERATE_CODE_PATH} --config ${ARGS_CONFIG_FILE} ${_include_dirs} --input ${SOURCE} --dir ${ARGS_OUT_DIR}
			DEPENDS "${SOURCE}"
			COMMENT "Generating code for ${SOURCE}"
			VERBATIM
			COMMAND_EXPAND_LISTS
		)
		list(APPEND FORMATTED_SOURCES ${_GENERATED_SOURCE_FILE})
	endforeach()

endfunction(add_code_generation_target)
