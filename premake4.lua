-- Captures output of a command
function os.capture(cmd, raw)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  if raw then return s end
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  return s
end

function pkg_config(packages)
  packages = table.concat(packages, ' ')

  configuration "vs*"
    buildoptions { os.capture("pkg-config --cflags --msvc-syntax " .. packages) }
    linkoptions { os.capture("pkg-config --libs --msvc-syntax " .. packages) }

  configuration { "gmake" }
    buildoptions { os.capture("pkg-config --cflags " .. packages) }
    linkoptions { os.capture("pkg-config --libs " .. packages) }
end

solution "Gonstruct"
  configurations { "Debug", "Release" }

  configuration "Debug"
    defines { "DEBUG" }
    flags { "Symbols" }
    targetdir "bin/debug"
  
  configuration "Release"
    defines { "NDEBUG" }
    flags { "Optimize", "OptimizeSpeed" }
    targetdir "bin/release"
  
  location "build"

  project "core"
    kind "StaticLib"
    language "C++"
    files { "src/core/*.hpp", "src/core/*.cpp" }

  project "Gonstruct"
    kind "WindowedApp"
    language "C++"
    files { "src/level_editor/*.hpp", "src/level_editor/*.cpp" }
    includedirs { "src" }
    links { "core", "boost_filesystem-mt", "boost_system-mt" }
    
    -- GTKmm through pkg-config
    pkg_config { "gtkmm-2.4", "gtksourceview-2.0" }
    
    -- Disable excessive GTKmm warnings
    configuration { "vs2008" }
      buildoptions { "/wd4250" }

if _ACTION == "clean" then
  os.rmdir("bin")
  os.rmdir("build")
end
