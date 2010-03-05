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

PKG_CHECK_MODULES([gonstruct], [gtkmm-2.4 gtksourceview-2.0 gtkgl-2.0])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/core/Makefile
  src/level_editor/Makefile
])
AC_OUTPUT

