echo "You need to have copied the bin, etc, lib and share directory of"
echo "your gtk + gtkmm installation into this folder"
echo "InnoSetup's iscc needs to be in your PATH"

echo "Cleanup..."
if test -e installer; then
  rm installer -Rf || exit;
fi

mkdir -p installer || exit

echo "Copying DLLs..."
mkdir -p installer/bin
cp bin/iconv.dll installer/bin || exit
cp bin/intl.dll installer/bin || exit
cp bin/jpeg62.dll installer/bin || exit
cp bin/libatk-1.0-0.dll installer/bin || exit
cp bin/libatkmm-1.6-1.dll installer/bin || exit
cp bin/libcairo-2.dll installer/bin || exit
cp bin/libcairomm-1.0-1.dll installer/bin || exit
cp bin/libgdk-win32-2.0-0.dll installer/bin || exit
cp bin/libgdkmm-2.4-1.dll installer/bin || exit
cp bin/libgdk_pixbuf-2.0-0.dll installer/bin || exit
cp bin/libgio-2.0-0.dll installer/bin || exit
cp bin/libglademm-2.4-1.dll installer/bin || exit
cp bin/libglib-2.0-0.dll installer/bin || exit
cp bin/libglibmm-2.4-1.dll installer/bin || exit
cp bin/libgmodule-2.0-0.dll installer/bin || exit
cp bin/libgobject-2.0-0.dll installer/bin || exit
cp bin/libgthread-2.0-0.dll installer/bin || exit
cp bin/libgtk-win32-2.0-0.dll installer/bin || exit
cp bin/libgtkmm-2.4-1.dll installer/bin || exit
cp bin/libgtksourceview-2.0-0.dll installer/bin || exit
cp bin/libpango-1.0-0.dll installer/bin || exit
cp bin/libpangocairo-1.0-0.dll installer/bin || exit
cp bin/libpangoft2-1.0-0.dll installer/bin || exit
cp bin/libpangomm-1.4-1.dll installer/bin || exit
cp bin/libpangowin32-1.0-0.dll installer/bin || exit
cp bin/libpng12-0.dll installer/bin || exit
cp bin/libsigc-2.0-0.dll installer/bin || exit
cp bin/libtiff3.dll installer/bin || exit
cp bin/libxml2.dll installer/bin || exit
cp bin/zlib1.dll installer/bin || exit
cp bin/libgiomm-2.4-1.dll installer/bin || exit
cp bin/libgdkglextmm-win32-1.2-0.dll installer/bin || exit
cp bin/libgtkglextmm-win32-1.2-0.dll installer/bin || exit
cp bin/libgtkglext-win32-1.0-0.dll installer/bin || exit
cp bin/libgdkglext-win32-1.0-0.dll installer/bin || exit

echo "Copying modules..."
mkdir -p installer/lib/gtk-2.0/2.10.0/engines || exit
cp -R lib/gtk-2.0/2.10.0/engines installer/lib/gtk-2.0/2.10.0 || exit

mkdir -p installer/lib/gtk-2.0/2.10.0/immodules || exit
# cp -R lib/gtk-2.0/2.10.0/immodules installer/lib/gtk-2.0/2.10.0 || exit
cp bin/gtk-query-immodules-2.0.exe installer/bin || exit

mkdir -p installer/lib/gtk-2.0/2.10.0/loaders || exit
cp -R lib/gtk-2.0/2.10.0/loaders installer/lib/gtk-2.0/2.10.0 || exit
cp bin/gdk-pixbuf-query-loaders.exe installer/bin || exit

echo "Copying themes..." || exit
mkdir -p installer/share/themes || exit
cp -R share/themes/MS-Windows installer/share/themes || exit
mkdir -p installer/etc/gtk-2.0 || exit
echo "gtk-theme-name = \"MS-Windows\"" > installer/etc/gtk-2.0/gtkrc || exit

echo "Copying gtksourceview..." || exit
mkdir -p installer/share/gtksourceview-2.0/language-specs || exit
cp share/gtksourceview-2.0/language-specs/*.rng installer/share/gtksourceview-2.0/language-specs || exit
cp share/gtksourceview-2.0/language-specs/def.lang installer/share/gtksourceview-2.0/language-specs || exit
cp share/gtksourceview-2.0/language-specs/javascript.lang installer/share/gtksourceview-2.0/language-specs || exit
mkdir -p installer/share/gtksourceview-2.0/styles || exit
cp -R share/gtksourceview-2.0/styles/ installer/share/gtksourceview-2.0 || exit

echo "Copying Gonstruct distribution files..."
cp ../dist/gonstruct.exe installer/bin || exit
cp ../dist/readme.html installer || exit
cp ../dist/changelog.txt installer || exit
cp gonstruct.iss installer || exit
cp querymodules.bat installer || exit

echo "Stripping DLLs.."
strip installer/bin/*.dll || exits  
strip installer/bin/*.exe || exit
strip installer/lib/gtk-2.0/2.10.0/engines/*.dll || exit
#strip installer/lib/gtk-2.0/2.10.0/immodules/*.dll || exit
strip installer/lib/gtk-2.0/2.10.0/loaders/*.dll || exit
# copy unstrippable DLLs again...
cp bin/libxml2.dll installer/bin || exit
cp bin/intl.dll installer/bin || exit
cp bin/iconv.dll installer/bin || exit
cp bin/zlib1.dll installer/bin || exit

echo "Creating installer..."
iscc installer/gonstruct.iss || exit
echo