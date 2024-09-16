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
		"EDITABLE" # flags
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


# Function to generate headers from YAML files
function(generate_code_from_yaml_files)
	cmake_parse_arguments(
		ARGS # prefix
		"FLATTEN_DIR_STRUCTURE;GENETATE_SINGLE_FILE" # flags
		"TEMPLATE;GENERATED_FILES_ARG;TARGET_DIR;TARGET_FILE;PREFIX;SUFFIX;" # single-values
		"SOURCES" # multi-values
		${ARGN}
	)

	if (NOT ARGS_TEMPLATE)
		message(FATAL_ERROR "You must provide a template file")
	endif ()

	if (NOT ARGS_GENERATED_FILES_ARG)
		message(FATAL_ERROR "You must provide a variable to store generated headers")
	endif ()

	if (NOT ARGS_SOURCES)
		message(FATAL_ERROR "You must provide a list of source files")
	endif ()

	foreach(SOURCE_FILE ${ARGS_SOURCES})
		if (NOT EXISTS ${SOURCE_FILE})
			message(FATAL_ERROR "Source file ${SOURCE_FILE} does not exist")
		endif ()
	endforeach(SOURCE_FILE)

	if (NOT ARGS_GENETATE_SINGLE_FILE) # Render each file separately
		foreach(SOURCE_FILE ${ARGS_SOURCES})
			if (NOT ARGS_FLATTEN_DIR_STRUCTURE)
				file(RELATIVE_PATH GEN_REL_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE_FILE})
			else()
				get_filename_component(GEN_REL_FILE ${SOURCE_FILE} NAME_WE)
			endif()

			string(REPLACE ".gen.yaml" "" GEN_REL_FILE ${GEN_REL_FILE})

			set(GENERATED_FILE ${ARGS_TARGET_DIR}/${ARGS_PREFIX}${GEN_REL_FILE}${ARGS_SUFFIX}${GENERATED_SUFFIX})
			file(TO_CMAKE_PATH ${GENERATED_FILE} GENERATED_FILE)

			file(MAKE_DIRECTORY ${ARGS_TARGET_DIR})

			set(CODEGEN_COMMAND ${PYTHON_VENV_EXECUTABLE} -m grafkit_tools.codegen --template ${ARGS_TEMPLATE} --output ${GENERATED_FILE} --input-files ${SOURCE_FILE})

			message(STATUS "Generating ${GENERATED_FILE}")

			execute_process(
				COMMAND ${CODEGEN_COMMAND}
			)

			add_custom_command(
				OUTPUT ${GENERATED_FILE}
				COMMAND ${CODEGEN_COMMAND}
				DEPENDS ${SOURCE_FILE}
				COMMENT "Generating ${GENERATED_FILE}"
				VERBATIM
			)

			set(${ARGS_GENERATED_FILES_ARG} "${${ARGS_GENERATED_FILES_ARG}}" ${GENERATED_FILE} PARENT_SCOPE)
		endforeach(SOURCE_FILE)

	else() # Render all files in one

		if (NOT ARGS_TARGET_FILE)
			message(FATAL_ERROR "You must provide a target file")
		endif()

		set(GENERATED_FILE ${ARGS_TARGET_DIR}/${ARGS_PREFIX}${ARGS_TARGET_FILE}${ARGS_SUFFIX}${GENERATED_SUFFIX})
		file(TO_CMAKE_PATH ${GENERATED_FILE} GENERATED_FILE)
		file(MAKE_DIRECTORY ${ARGS_TARGET_DIR})

		set(CODEGEN_COMMAND ${PYTHON_VENV_EXECUTABLE} -m grafkit_tools.codegen --template ${ARGS_TEMPLATE} --output ${GENERATED_FILE} --input-files ${ARGS_SOURCES})

		message(STATUS "Generating ${GENERATED_FILE}")

		execute_process(
			COMMAND ${CODEGEN_COMMAND}
		)

		add_custom_command(
			OUTPUT ${GENERATED_FILE}
			COMMAND ${CODEGEN_COMMAND}
			DEPENDS ${SOURCE_FILE}
			COMMENT "Generating ${GENERATED_FILE}"
			VERBATIM
		)

		set(${ARGS_GENERATED_FILES_ARG} "${${ARGS_GENERATED_FILES_ARG}}" ${GENERATED_FILE} PARENT_SCOPE)

	endif()
endfunction()
