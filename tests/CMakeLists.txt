project(UnitTests)

set(TEST_SOURCES
  test_one.cpp
  test_two.cpp
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME} GTest::GTest GTest::Main)

add_test(NAME UnitTests COMMAND ${PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
