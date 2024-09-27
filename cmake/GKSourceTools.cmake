# Allows run custom code generation using prototypes
# Bootsrap

include(CMakeParseArguments)
include (PyEnv)
find_package(Vulkan REQUIRED)

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


			if (NOT EXISTS ${GENERATED_FILE} OR ${SOURCE_FILE} IS_NEWER_THAN ${GENERATED_FILE})
				message(STATUS "Generating ${GENERATED_FILE}")
				execute_process(
					COMMAND ${CODEGEN_COMMAND}
				)
			endif()

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

		set(TO_GENERATE NOT EXISTS ${GENERATED_FILE})
		foreach(SOURCE_FILE ${ARGS_SOURCES})
			if (${SOURCE_FILE} IS_NEWER_THAN ${GENERATED_FILE})
				set(TO_GENERATE TRUE)
			endif()
		endforeach(SOURCE_FILE)

		if (${TO_GENERATE})
			message(STATUS "Generating ${GENERATED_FILE}")
			execute_process(
				COMMAND ${CODEGEN_COMMAND}
			)
		endif()

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

function(spirv_compile_shaders)
	cmake_parse_arguments(
		ARGS # prefix
		"" # flags
		"SHADER_BINARY_DIR;SHADER_INCLUDE_DIR;GENERATED_FILES_ARG;TEMPLATE" # single-values
		"SOURCES" # multi-values
		${ARGN}
	)

	if (NOT ARGS_SOURCES)
		message(FATAL_ERROR "You must provide a list of source files")
	endif ()

	if (NOT ARGS_SHADER_BINARY_DIR)
		message(FATAL_ERROR "You must provide a binary directory")
	endif ()

	if (NOT ARGS_SHADER_INCLUDE_DIR)
		message(FATAL_ERROR "You must provide an include directory")
	endif ()

	set(SPIRV_GENERATED_SOURCE_FILES "")

	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGS_SHADER_INCLUDE_DIR})
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGS_SHADER_BINARY_DIR})

	foreach(SOURCE ${ARGS_SOURCES})
		get_filename_component(FILE_NAME ${SOURCE} NAME)

		set(SPIRV "${ARGS_SHADER_BINARY_DIR}/${FILE_NAME}.spv")
		set(SPIRV_COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SOURCE} -o ${SPIRV})

		if (NOT EXISTS ${SPIRV} OR ${SOURCE} IS_NEWER_THAN ${SPIRV})
			message(STATUS "Generating ${GENERATED_FILE}")
			execute_process(COMMAND ${SPIRV_COMMAND})
		endif()

		add_custom_command(
			OUTPUT ${SPIRV}
			COMMAND ${SPIRV_COMMAND}
			COMMENT "Compiling ${GLSL} to ${SPIRV}"
			DEPENDS ${GLSL}
		)

		set(SPIRV_H "${ARGS_SHADER_INCLUDE_DIR}/${FILE_NAME}.h")
		set(SPIRV_C "${ARGS_SHADER_BINARY_DIR}/${FILE_NAME}.c")

		set(TEMPLATE "")
		if (ARGS_TEMPLATE)
			set(TEMPLATE -t ${ARGS_TEMPLATE})
		endif()

		set(PACK_H_COMMAND ${PYTHON_VENV_EXECUTABLE} -m grafkit_tools.hexdump -x -n ${FILE_NAME} -i ${SPIRV} -o ${SPIRV_H} ${TEMPLATE})
		set(PACK_C_COMMAND ${PYTHON_VENV_EXECUTABLE} -m grafkit_tools.hexdump -n ${FILE_NAME} -i ${SPIRV} -o ${SPIRV_C} ${TEMPLATE})

		if (NOT EXISTS ${SPIRV_H} OR ${SPIRV} IS_NEWER_THAN ${SPIRV_H})
			message(STATUS "Generating ${SPIRV_H}")
			execute_process(COMMAND ${PACK_C_COMMAND})
			execute_process(COMMAND ${PACK_H_COMMAND})
		endif()

		add_custom_command(
			OUTPUT ${SPIRV_H}
			COMMAND ${PACK_H_COMMAND}
			COMMAND ${PACK_C_COMMAND}
			DEPENDS ${SPIRV}
			COMMENT "Generating source file ${SPIRV_C}"
			VERBATIM
		)

		list(APPEND SPIRV_GENERATED_SOURCE_FILES ${SPIRV_H} ${SPIRV_C})
	endforeach()

	set(${ARGS_GENERATED_FILES_ARG} "${SPIRV_GENERATED_SOURCE_FILES}" PARENT_SCOPE)

endfunction()
