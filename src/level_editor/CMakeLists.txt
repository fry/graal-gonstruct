link_directories(${GTKMM_LIBRARY_DIRS})

add_definitions(-DGLEW_STATIC)

include_directories(SYSTEM
  ${GTKMM_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/glew
)

include_directories(
  ${PROJECT_SOURCE_DIR}/src/level_editor
  ${PROJECT_SOURCE_DIR}/src/core
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/image_data.cpp
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/level_editor
  COMMAND ${RUBY} make_images.rb "${PROJECT_SOURCE_DIR}/icons/png/" "${CMAKE_CURRENT_BINARY_DIR}/image_data.cpp")

add_executable(gonstruct WIN32
	GLArea.cpp
	basic_tiles_display.cpp
	copy_cache.cpp
	default_tile_display.cpp
	filesystem.cpp
	helper.cpp
	image_cache.cpp
	${CMAKE_CURRENT_BINARY_DIR}/image_data.cpp
	layers_control.cpp
	level.cpp
	level_display.cpp
	link_list.cpp
	main.cpp
	npc_list.cpp
	edit_npc.cpp
	ogl_texture_cache.cpp
	ogl_tiles_display.cpp
	preferences.cpp
	preferences_display.cpp
	sign_list.cpp
	tile_objects_display.cpp
	tileset.cpp
	tileset_display.cpp
	tileset_list.cpp
	toolbar_tools_display.cpp
	undo_buffer.cpp
	undo_diffs.cpp
	window.cpp
	${PROJECT_SOURCE_DIR}/src/glew/glew.c
	window/header.cpp
	window/file_commands.cpp
	window/edit_commands.cpp
	window/level_commands.cpp
	level_map.cpp
	gtkmarshalers.c
  )

target_link_libraries(gonstruct ${GTKMM_LIBRARIES} core ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

if(WINDRES)
  add_dependencies(gonstruct generate_resource)
  target_link_libraries(gonstruct ${PROJECT_SOURCE_DIR}/win/gonstruct.res)
endif(WINDRES)

