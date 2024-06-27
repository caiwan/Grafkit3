add_subdirectory(grafkit)
add_subdirectory(player)

if (GK_BUILD_EDITOR)
  add_subdirectory(editor)
endif()
