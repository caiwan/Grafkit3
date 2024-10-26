# Warning settings Based on Jason Turners recommendations
# https://github.com/lefticus/cpp_starter_project/blob/master/CMakeLists.txt

# ---

# Function: set_project_warnings
#
# Sets the warning flags for the specified target.
#
# Parameters:
#	 - TARGET: The target for which the warning flags will be set.
#
function(set_project_warnings TARGET)
	option(GK_ENABLE_STRICT_COMPILE "Enable strict compilation mode" ON)

	if(MSVC)
	if (GK_ENABLE_STRICT_COMPILE)
		target_compile_options(${TARGET} INTERFACE
		/WX # treat warnings as errors
		)
	endif()

	target_compile_options(${TARGET} INTERFACE
		/W4 # Baseline reasonable warnings
		/w14242 # 'identfier': conversion from 'type1' to 'type1',
				# possible loss of data
		/w14254 # 'operator': conversion from 'type1:field_bits' to
				# 'type2:field_bits', possible loss of data
		/w14263 # 'function': member function does not override any base
				# class virtual member function
		/w14265 # 'classname': class has virtual functions, but
				# destructor is not virtual instances of this class may
				# not be destructed correctly
		/w14287 # 'operator': unsigned/negative constant mismatch
		/we4289 # nonstandard extension used: 'variable': loop control
				# variable declared in the for-loop is used outside the
				# for-loop scope
		/w14296 # 'operator': expression is always 'boolean_value'
		/w14311 # 'variable': pointer truncation from 'type1' to 'type2'
		/w14545 # expression before comma evaluates to a function which
				# is missing an argument list
		/w14546 # function call before comma missing argument list
		/w14547 # 'operator': operator before comma has no effect;
				# expected operator with side-effect
		/w14549 # 'operator': operator before comma has no effect; did
				# you intend 'operator'?
		/w14555 # expression has no effect; expected expression with
				# side- effect
		/w14619 # pragma warning: there is no warning number 'number'
		/w14640 # Enable warning on thread un-safe static member
				# initialization
		/w14826 # Conversion from 'type1' to 'type_2' is sign-extended.
				# This may cause unexpected runtime behavior.
		/w14905 # wide string literal cast to 'LPSTR'
		/w14906 # string literal cast to 'LPWSTR'
		/w14928 # illegal copy-initialization; more than one
				# user-defined conversion has been implicitly applied
		)

	else()
	if (GK_ENABLE_STRICT_COMPILE)
		target_compile_options( ${TARGET} INTERFACE
		-Werror # treat warnings as errors
		)
	endif()
	target_compile_options(
		${TARGET}
		INTERFACE
		-Wall
		-Wextra # reasonable and standard
		-Wno-nullability-extension # suppress nullability warnings
		-Wnullability-completeness # warn about incomplete nullability
		-Wno-unknown-pragmas	# allow unknown pragmas for GCC such as
								# clang-tidy
		-Wshadow	# warn the user if a variable declaration shadows one
					# from a parent context
		-Wnon-virtual-dtor	#	warn the user if a class with virtual
							# functions has a
		# non-virtual destructor. This helps catch hard to track down
		# memory errors
		-Wold-style-cast # warn for c-style casts
		-Wcast-align # warn for potential performance problem casts
		-Wunused # warn on anything being unused
		-Woverloaded-virtual # warn if you overload (not override) a
							# virtual
		# function
		-Wpedantic # warn if non-standard C++ is used
		-Wconversion # warn on type conversions that may lose data
		-Wsign-conversion # warn on sign conversions
		-Wmisleading-indentation	# warn if identation implies blocks
								# where blocks
		# do not exist
		# -Wduplicated-cond # warn if if / else chain has duplicated
		# conditions
		# -Wduplicated-branches # warn if if / else branches have
		# duplicated code
		# -Wlogical-op # warn about logical operations being used where
		# bitwise were
		# probably wanted
		-Wnull-dereference # warn if a null dereference is detected
		# -Wuseless-cast # warn if you perform a cast to the same type
		-Wdouble-promotion # warn if float is implicit promoted to
							# double
		-Wformat=2 # warn on security issues around functions that
					# format output
		# (ie printf)
	)
	endif()
endfunction()

# Function: set_project_clang_format
# Sets up clang-format for the specified target.
#
# Parameters:
#	 - TARGET_NAME: The target for which clang-format will be set up.
#
function(set_project_clang_format TARGET_NAME)
	option (GK_ENABLE_CLANG_FORMAT "Enable clang-format" ON)

	find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
	if(NOT CLANG_FORMAT_EXECUTABLE)
		message(WARNING "clang-format not found! Skipping formatting for ${TARGET_NAME}.")
		return()
	endif()


	if (GK_ENABLE_CLANG_FORMAT)
		message(STATUS "Adding clang-format for ${TARGET_NAME}")

		get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
		list(REMOVE_DUPLICATES TARGET_SOURCES)

		foreach(SOURCE ${TARGET_SOURCES})

			# Create a flag file for each source file to avoid reformatting all files each time
			file(RELATIVE_PATH RELATIVE_SOURCE_FILENAME "${CMAKE_SOURCE_DIR}" "${SOURCE}")
			file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${RELATIVE_SOURCE_FILENAME}.formatted" FORMATTED_SOURCE_FLAG)

			if (WIN32)
				set(TOUCH_COMMAND "copy;/y;NUL;${FORMATTED_SOURCE_FLAG};>;NUL")
			else()
				set(TOUCH_COMMAND "touch;${FORMATTED_SOURCE_FLAG}")
			endif()

			add_custom_command(
				OUTPUT "${FORMATTED_SOURCE_FLAG}"
				COMMAND ${CLANG_FORMAT_EXECUTABLE} -style=file -assume-filename=${CMAKE_SOURCE_DIR}/.clang-format -i ${SOURCE}
				COMMAND ${TOUCH_COMMAND}
				DEPENDS "${SOURCE}"
				COMMENT "Running clang-format on ${SOURCE}"
				COMMAND_EXPAND_LISTS
				VERBATIM
			)
			list(APPEND FORMATTED_SOURCES ${FORMATTED_SOURCE_FLAG})
		endforeach()

		add_custom_target(
			${TARGET_NAME}_format
			DEPENDS ${FORMATTED_SOURCES}
			COMMENT "Checking if all source files are formatted."
		)
		set_target_properties(${TARGET_NAME}_format PROPERTIES FOLDER "Utilities")
		add_dependencies(${TARGET_NAME} ${TARGET_NAME}_format)
	endif()
endfunction()

# ---

# Function to recursively collect include directories
function(_collect_include_directories TARGET OUT_VAR)
	set(LOCAL_INCLUDE_DIRS "")
	get_target_property(CURRENT_INCLUDE_DIRS ${TARGET} INCLUDE_DIRECTORIES)
	if(CURRENT_INCLUDE_DIRS)
		list(APPEND LOCAL_INCLUDE_DIRS ${CURRENT_INCLUDE_DIRS})
	endif()
	get_target_property(LINK_LIBRARIES ${TARGET} LINK_LIBRARIES)
	if(LINK_LIBRARIES)
		foreach(LIBRARY in ${LINK_LIBRARIES})
			if(TARGET ${LIBRARY})
				_collect_include_directories(${LIBRARY} RECURSIVE_INCLUDES)
				list(APPEND LOCAL_INCLUDE_DIRS ${RECURSIVE_INCLUDES})
			endif()
		endforeach()
	endif()
	list(REMOVE_DUPLICATES LOCAL_INCLUDE_DIRS)
	set(${OUT_VAR} ${LOCAL_INCLUDE_DIRS} PARENT_SCOPE)
endfunction()

# Function: set_project_clang_tidy
# Sets up clang-tidy for the specified target.
#
# Parameters:
#	 - TARGET_NAME: The target for which clang-tidy will be set up.
#
function(set_project_clang_tidy TARGET_NAME)
	option(GK_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
	option(GK_ENABLE_CLANG_TIDY_FIX "Enable clang-tidy fix" OFF)

	if (GK_ENABLE_CLANG_TIDY)
		find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
		if(NOT CLANG_TIDY_EXECUTABLE)
			message(WARNING "clang-tidy not found! Skipping clang-tidy for ${TARGET_NAME}.")
		return()
	endif()

	message(STATUS "Adding clang-tidy for ${TARGET_NAME}")

	# -- Set clang-tidy command
	file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/.clang-tidy" CLANG_TIDY_CONFIG_FILE)
	set(CLANG_TIDY_CMD "-format-style=file;-config-file=${CLANG_TIDY_CONFIG_FILE};-header-filter='^./include/.*'")
	if(GK_ENABLE_CLANG_TIDY_FIX)
		set(CLANG_TIDY_CMD "${CLANG_TIDY_CMD};-fix-errors")
	endif()
	set_target_properties(${TARGET_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};${CLANG_TIDY_CMD}")

	# -- Tidy all header files separately
	get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
	list(REMOVE_DUPLICATES TARGET_SOURCES)

	list(FILTER TARGET_SOURCES INCLUDE REGEX "\\.(h|hxx)$")
	list(FILTER TARGET_SOURCES EXCLUDE REGEX "_generated\\.(h|hxx)$")

	_collect_include_directories(${TARGET_NAME} ALL_INCLUDE_DIRECTORIES)
	set(INCLUDE_FLAGS "")
	foreach(INCLUDE_DIR ${ALL_INCLUDE_DIRECTORIES})
		list(APPEND INCLUDE_FLAGS "-I${INCLUDE_DIR}")
	endforeach()
	string(REPLACE ";" " " INCLUDE_FLAGS "${INCLUDE_FLAGS}")

	foreach(SOURCE ${TARGET_SOURCES})
		# Create a flag file for each source file to avoid reformatting all files each time
		file(RELATIVE_PATH RELATIVE_SOURCE_FILENAME "${CMAKE_SOURCE_DIR}" "${SOURCE}")
		file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${RELATIVE_SOURCE_FILENAME}.tidy" TIDIED_SOURCE_FLAG)

		if (WIN32)
			set(TOUCH_COMMAND "copy;/y;NUL;${TIDIED_SOURCE_FLAG};>;NUL")
		else()
			set(TOUCH_COMMAND "touch;${TIDIED_SOURCE_FLAG}")
		endif()

		add_custom_command(
			OUTPUT ${TIDIED_SOURCE_FLAG}
			COMMAND ${CLANG_TIDY_EXECUTABLE} ${SOURCE} ${CLANG_TIDY_CMD} --extra-arg=${INCLUDE_FLAGS} -p ${CMAKE_BINARY_DIR}
			COMMAND ${TOUCH_COMMAND}
			DEPENDS "${SOURCE}"
			COMMENT "Running clang-tidy with fix-ups on ${SOURCE}"
			VERBATIM
			COMMAND_EXPAND_LISTS
		)
		list(APPEND TIDIED_SOURCES ${TIDIED_SOURCE_FLAG})
	endforeach()

	add_custom_target(
		${TARGET_NAME}_tidy
		DEPENDS ${TIDIED_SOURCES}
		COMMENT "Checking source files with clang-tidy."
	)
	set_target_properties(${TARGET_NAME}_tidy PROPERTIES FOLDER "Utilities")
	add_dependencies(${TARGET_NAME} ${TARGET_NAME}_tidy)

	endif()
endfunction()
