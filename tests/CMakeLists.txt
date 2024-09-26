project(UnitTests)


file(GLOB_RECURSE SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

file(GLOB_RECURSE HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hxx
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
	GTest::GTest
	GTest::Main
	Grafkit::Grafkit
	Grafkit::GrafkitLoader
)

add_test(
	NAME ${PROJECT_NAME}
	COMMAND ${PROJECT_NAME}
)

file(GLOB_RECURSE data_files
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*"
)

foreach(data_file ${data_files})
    file(RELATIVE_PATH relative_file "${CMAKE_CURRENT_SOURCE_DIR}/data" "${data_file}")
    configure_file("${data_file}" "${CMAKE_CURRENT_BINARY_DIR}/${relative_file}" COPYONLY)
	message(STATUS "Copying test data file: ${data_file} to ${CMAKE_CURRENT_BINARY_DIR}/${relative_file}")
endforeach()

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
