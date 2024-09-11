project(Thirdparties)

# MARK: Set up compiler flags
# Disable all compiler errors and warnings for thirdparty libraries
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /w")
else ()
  set (CXX_EXTRA_FLAGS
    "-Wno-error -Wno-nullability-extension -Wno-implicit-function-declaration"
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w ${CXX_EXTRA_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w ${CXX_EXTRA_FLAGS}")
endif()

# MARK: GLFW
set(GLFW_BUILD_DOCS OFF)
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
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
set(ASSIMP_BUILD_TESTS OFF)
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
