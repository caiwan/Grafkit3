project(Thirdparties)

# Disable all compiler errors and warnings for thirdparty libraries
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /w")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wno-error -Wno-nullability-extension")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -w -Wno-error -Wno-nullability-extension")
endif()

add_subdirectory(glfw)

# option(GLM_ENABLE_CXX_20 "Enable C++ 20" ON)
add_subdirectory(glm)

add_subdirectory(VulkanMemoryAllocator)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS VulkanMemoryAllocator EXPORT VulkanMemoryAllocatorConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT VulkanMemoryAllocatorConfig NAMESPACE "GPUOpen::" DESTINATION "share/cmake/VulkanMemoryAllocator")

if(GK_BUILD_TESTS)
  add_subdirectory(googletest)

  add_library(GTest::GTest ALIAS gtest)
  add_library(GTest::Main ALIAS gtest_main)
endif()
