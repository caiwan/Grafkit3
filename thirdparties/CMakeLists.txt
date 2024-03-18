project(Thirdparties)
add_subdirectory(glfw)
add_subdirectory(glm)

add_subdirectory(VulkanMemoryAllocator)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS VulkanMemoryAllocator EXPORT VulkanMemoryAllocatorConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT VulkanMemoryAllocatorConfig NAMESPACE "GPUOpen::" DESTINATION "share/cmake/VulkanMemoryAllocator")
