function backticks(...)
  local cmd = string.format(unpack(arg))
  local pipe = io.popen(cmd)
  local output = pipe:read("*a")
  pipe:close()

  return output
end

function split(str, results)
  if not results then
    results = {}
  end
  for word in string.gfind(str, "%S+") do
    table.insert(results, word)
  end

  return results
end

function pkg_config_libs(package, ...)
  for _, lib in ipairs(arg) do
    pkg_config_once(package, lib, "libs", package.linkoptions)
  end
end

function pkg_config_cflags(package, ...)
  for _, lib in ipairs(arg) do
    pkg_config_once(package, lib, "cflags", package.buildoptions)
  end
end

function pkg_config_once(package, libname, config_option, flags)
  if options.target ~= "gnu" then
    error("windows lol")
  end

  if os.execute("pkg-config --exists "..libname) == 0 then
    split(backticks("pkg-config --%s %s", config_option, libname), flags)
  else
    error("pkg-config for " .. libname .. " failed")
  end
end

local annoying_boost_suffixes = { "", "-gcc43" }

function boost_links(package, ...)
  package.links = package.links or {}
  for _, lib in ipairs(arg) do
    local okay = false
    for _, suffix in ipairs(annoying_boost_suffixes) do
      local my_lib = string.format("boost_%s%s-mt", lib, suffix)
      local path = findlib(my_lib)
      if path then
        table.insert(package.links, my_lib)
        okay = true
        break
      end
    end
    if not okay then
      error("did not manage to find boost library " .. lib)
    end
  end
end

project.name = "Gonstruct"
project.bindir = "dist"
project.libdir = "dist"

local my_flags = {
  "optimize"
}

local my_cxxflags = {
  "-pedantic", "-ansi", "-Wall", "-Wextra", "-Wfloat-equal", "-Wwrite-strings", "-Wpointer-arith", "-Wcast-qual", "-Wctor-dtor-privacy", "-Woverloaded-virtual", "-Wno-unused-parameter", "-Wtrigraphs", "-Wsign-promo"
}

-- core
core = newpackage()
core.name = "core"
core.kind = "lib"
core.language = "c++"
core.buildflags = my_flags
core.buildoptions = my_cxxflags
core.files = {
  "src/core/csvparser.cpp",
  "src/core/csvparser.h",
  "src/core/helper.cpp",
  "src/core/helper.h",
  "src/core/preferences.cpp",
  "src/core/preferences.h",
}

-- editor
editor = newpackage()
editor.name = "level-editor"
editor.kind = "lib"
editor.language = "c++"
editor.buildflags = my_flags
editor.buildoptions = my_cxxflags
editor.includepaths = { "src" }
pkg_config_cflags(editor, "gtkmm-2.4", "gtksourceview-2.0")
editor.files = {
  "src/level_editor/copy_cache.cpp",
  "src/level_editor/copy_cache.hpp",
  "src/level_editor/default_tile_display.cpp",
  "src/level_editor/default_tile_display.hpp",
  "src/level_editor/filesystem.cpp",
  "src/level_editor/filesystem.hpp",
  "src/level_editor/helper.cpp",
  "src/level_editor/helper.hpp",
  "src/level_editor/image_cache.cpp",
  "src/level_editor/image_cache.hpp",
  "src/level_editor/image_data.cpp",
  "src/level_editor/image_data.hpp",
  "src/level_editor/level.cpp",
  "src/level_editor/level_display.cpp",
  "src/level_editor/level_display.hpp",
  "src/level_editor/level.hpp",
  "src/level_editor/link_list.cpp",
  "src/level_editor/link_list.hpp",
  "src/level_editor/main.cpp",
  "src/level_editor/npc_list.cpp",
  "src/level_editor/npc_list.hpp",
  "src/level_editor/_precompiled.cpp",
  "src/level_editor/_precompiled.hpp",
  "src/level_editor/preferences.cpp",
  "src/level_editor/preferences_display.cpp",
  "src/level_editor/preferences_display.hpp",
  "src/level_editor/preferences.hpp",
  "src/level_editor/sign_list.cpp",
  "src/level_editor/sign_list.hpp",
  "src/level_editor/tileset.cpp",
  "src/level_editor/tileset_display.cpp",
  "src/level_editor/tileset_display.hpp",
  "src/level_editor/tileset.hpp",
  "src/level_editor/tileset_list.cpp",
  "src/level_editor/tileset_list.hpp",
  "src/level_editor/undo_buffer.cpp",
  "src/level_editor/undo_buffer.hpp",
  "src/level_editor/undo_diffs.cpp",
  "src/level_editor/undo_diffs.hpp",
  "src/level_editor/window.cpp",
  "src/level_editor/window.hpp",
}

gonstruct = newpackage()
gonstruct.name = "gonstruct"
gonstruct.kind = "winexe"
gonstruct.language = "c++"
gonstruct.buildflags = my_flags
gonstruct.buildoptions = my_cxxflags
gonstruct.links = { "core", "level-editor" }
pkg_config_libs(gonstruct, "gtkmm-2.4", "gtksourceview-2.0")
boost_links(gonstruct, "filesystem", "system")
