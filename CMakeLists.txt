cmake_minimum_required (VERSION 2.8)
project (gonstruct)

set(GONSTRUCT_VERSION 0.2.3)

configure_file(
  "${PROJECT_SOURCE_DIR}/gonstruct_config.h.in"
  "${PROJECT_BINARY_DIR}/gonstruct_config.h"
  )

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-2.4 gtksourceview-2.0 gtkgl-2.0)

find_program(RUBY ruby REQUIRED)

find_package(Boost COMPONENTS system filesystem REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

if(WIN32)
  find_program(WINDRES windres)
  
  if(NOT WINDRES)
    message("Could not find windres to build gonstruct.rc")
  endif(NOT WINDRES)

  # Build InnoSetup file for windows
  configure_file(
    "${PROJECT_SOURCE_DIR}/win/gonstruct.iss.in"
    "${PROJECT_BINARY_DIR}/win/gonstruct.iss"
    )

  add_subdirectory(win)

  # Hide console on windows in Release mode
  if(NOT CMAKE_BUILD_TYPE EQUAL Debug)
  	set(CMAKE_CREATE_WIN32_EXE "-mwindows")
  endif(NOT CMAKE_BUILD_TYPE EQUAL Debug)
endif(WIN32)

include_directories("${PROJECT_BINARY_DIR}")
add_subdirectory(src)

