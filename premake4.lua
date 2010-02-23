dofile "utility.lua"

VERSION = "0.1.8"

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

newaction {
  trigger = "clean",
  description = "Removes all build-related folders and files",
  execute = function()
    os.rmdir("bin")
    os.rmdir("build")
  end
}

newaction {
  trigger = "embed",
  description = "Serializes config data (such as version number) and embeds image data",
  execute = function()
    print "Building config.cpp"
    config_file("src/level_editor/config.cpp.in", "src/level_editor/config.cpp", {
      version_string = VERSION
    })
  end
}
