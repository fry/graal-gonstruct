SUBDIRS = win icons src

#ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gonstruct
gonstruct_LDADD = \
	src/level_editor/libgonstruct.la \
	src/core/libcore.la \
	-lboost_filesystem-mt \
	-lboost_system-mt \
	$(gonstruct_LIBS) \
	$(LIBS)

if WIN32
gonstruct_LDFLAGS = \
	-mwindows \
	win/gonstruct.res
endif

gonstruct_SOURCES = 

# Dummy C++ source to cause C++ linking, otherwise some platforms don't link
# correctly against libstdc++. See also #399. Note that this file does not
# (need to) exist in the source tree. (stolen from Gobby)
nodist_EXTRA_gonstruct_SOURCES = dummy.cpp

EXTRA_DIST = \
	dist/readme.txt \
	dist/changelog.txt
