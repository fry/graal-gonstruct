m4_define([gonstruct_version], [0.2.1])

AC_INIT([gonstruct], [gonstruct_version], [fry@londeroth.org])
AC_CONFIG_SRCDIR([src/level_editor/main.cpp])
AM_INIT_AUTOMAKE([
  -Wall foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_CONFIG_HEADERS([config.h])

GONSTRUCT_VERSION=gonstruct_version
AC_SUBST(GONSTRUCT_VERSION)
AC_CACHE_SAVE

PKG_CHECK_MODULES([gonstruct], [gtkmm-2.4 gtksourceview-2.0 gtkgl-2.0])
AC_CACHE_SAVE

#AC_CHECK_HEADER([boost/filesystem.hpp], [], AC_MSG_ERROR(["gonstruct requires boost_filesystem"]))
#AC_CHECK_HEADER([boost/shared_ptr.hpp], [], AC_MSG_ERROR(["gonstruct requires boost_system"]))

# Platform check (stolen from gobby)
AC_MSG_CHECKING([whether to enable WIN32 specific flags])
case "$host_os" in
*mingw*)
	platform='win32'
	AC_MSG_RESULT([yes])
	AC_CHECK_TOOL(WINDRES, windres)
	;;
*)
	platform='unix'
	AC_MSG_RESULT([no])
	;;
esac

AM_CONDITIONAL(WIN32, test $platform = 'win32')
AC_CACHE_SAVE

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/core/Makefile
  src/level_editor/Makefile
	win/Makefile
])
AC_OUTPUT

