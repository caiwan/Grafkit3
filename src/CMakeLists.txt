add_subdirectory(grafkit)
add_subdirectory(grafkit_loader)
add_subdirectory(player)

if (GK_BUILD_SAMPLES)
	add_subdirectory(samples)
endif()

if (GK_BUILD_EDITOR)
	add_subdirectory(editor)
endif()
