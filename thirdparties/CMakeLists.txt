project(Thirdparties)

# MARK: Set up compiler flags
# Disable all compiler errors and warnings for thirdparty libraries
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /w")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wno-error -Wno-nullability-extension")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -w -Wno-error -Wno-nullability-extension")
endif()

# MARK: GLFW
add_subdirectory(glfw)

# MARK: GLM
# option(GLM_ENABLE_CXX_20 "Enable C++ 20" ON)
add_subdirectory(glm)

# MARK: VulkanMemoryAllocator
add_subdirectory(VulkanMemoryAllocator)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS VulkanMemoryAllocator EXPORT VulkanMemoryAllocatorConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT VulkanMemoryAllocatorConfig NAMESPACE "GPUOpen::" DESTINATION "share/cmake/VulkanMemoryAllocator")

# MARK: Assimp
add_subdirectory(assimp)

# MARK: GoogleTest
if(GK_BUILD_TESTS)
  add_subdirectory(googletest)

  add_library(GTest::GTest ALIAS gtest)
  add_library(GTest::Main ALIAS gtest_main)
endif()

# MARK: nlohmann_json
add_subdirectory(nlohmann_json)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS nlohmann_json EXPORT nlohmann_jsonConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT nlohmann_jsonConfig NAMESPACE "nlohmann_json::" DESTINATION "share/cmake/nlohmann_json")
