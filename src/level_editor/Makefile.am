bin_PROGRAMS = gonstruct

gonstruct_SOURCES = \
	GLArea.cpp \
	basic_tiles_display.cpp \
	copy_cache.cpp \
	default_tile_display.cpp \
	filesystem.cpp \
	helper.cpp \
	image_cache.cpp \
	image_data.cpp \
	layers_control.cpp \
	level.cpp \
	level_display.cpp \
	link_list.cpp \
	main.cpp \
	npc_list.cpp \
	ogl_texture_cache.cpp \
	ogl_tiles_display.cpp \
	preferences.cpp \
	preferences_display.cpp \
	sign_list.cpp \
	tile_objects_display.cpp \
	tileset.cpp \
	tileset_display.cpp \
	tileset_list.cpp \
	toolbar_tools_display.cpp \
	undo_buffer.cpp \
	undo_diffs.cpp \
	window.cpp \
	GLArea.hpp \
	basic_tiles_display.hpp \
	copy_cache.hpp \
	default_tile_display.hpp \
	filesystem.hpp \
	helper.hpp \
	image_cache.hpp \
	image_data.hpp \
	layers_control.hpp \
	level.hpp \
	level_display.hpp \
	link_list.hpp \
	npc_list.hpp \
	ogl_texture_cache.hpp \
	ogl_tiles_display.hpp \
	preferences.hpp \
	preferences_display.hpp \
	sign_list.hpp \
	tile_objects_display.hpp \
	tiles_display.hpp \
	tileset.hpp \
	tileset_display.hpp \
	tileset_list.hpp \
	toolbar_tools_display.hpp \
	undo_buffer.hpp \
	undo_diffs.hpp \
	window.hpp \
	../glew/glew.c

gonstruct_CPPFLAGS = \
	$(gonstruct_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/level_editor \
	-I$(top_srcdir)/src/glew \
	-DGLEW_STATIC

gonstruct_LDADD = \
	$(top_srcdir)/src/core/libcore.la \
	$(gonstruct_LIBS) \
	-lboost_filesystem-mt \
	-lboost_system-mt

EXTRA_DIST = \
	icon.svg \
	logo.svg \
	no-img.svg \
	npc-default.svg \
	toolbar-create-link.svg \
	toolbar-links.svg \
	toolbar-npcs.svg \
	toolbar-signs.svg \
	toolbar-tilesets.svg
