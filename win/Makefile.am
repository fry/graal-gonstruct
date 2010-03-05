if WIN32
gonstruct.res: gonstruct.rc
	$(WINDRES) -i gonstruct.rc --input-format=rc -o gonstruct.res -O coff

BUILT_SOURCES = gonstruct.res
endif

EXTRA_DIST = \
	gonstruct.ico \
	gonstruct.rc \
	gonstruct.iss.in \
	querymodules.bat \
	build_installer.sh
