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
    print "Building image_data"
    build_image_data("dist/images/*", "src/level_editor/image_data")
  end
}

-- Embeds image data in a source file and generates a header
function build_image_data(source, dest)
  print("  creating " .. dest .. ".cpp")
  out_cpp = io.open(dest .. ".cpp", "w+b")
  out_cpp:write([[
namespace Graal {
  namespace level_editor {
    namespace image_data {
]])

  images = {}
  for _, image in pairs(os.matchfiles(source)) do
    data = escape_file(image)
    var_name = path.getbasename(image):gsub("[^%w]", "_")
    table.insert(images, var_name)
    
    out_cpp:write(string.format("      const char %s[] =\n", var_name))
    out_cpp:write(string.format("\"%s\";\n", data))
  end
  
  out_cpp:write("      const char* images[] = {\n")
  for _, image in pairs(images) do
    out_cpp:write(
      string.format(
        "        \"internal/%s.png\", %s, %s + sizeof(%s) - 1,\n",
        image, image, image, image))
  end
  
  out_cpp:write([[
        0, 0, 0
      };
    }
  }
}
]])
  out_cpp:close()
  
  print("  creating " .. dest .. ".hpp")
  out_hpp = io.open(dest .. ".hpp", "w+b")
  out_hpp:write([[
#pragma once
namespace Graal {
  namespace level_editor {
    namespace image_data {
]])

  for _, image in pairs(images) do
    out_hpp:write(string.format("      extern const char %s[];\n", image))
  end
  
  out_hpp:write("      extern const char* images[];\n")
  out_hpp:write([[
    }
  }
}
]])

  out_hpp:close()
end
