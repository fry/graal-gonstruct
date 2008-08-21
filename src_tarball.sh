cd ..
rm -f gonstruct-src
ln -s graal gonstruct-src
tar cfz gonstruct-src.tar.gz gonstruct-src/{SConstruct,installer.py,src_tarball.sh,dist.py,dist/{changelog.txt,readme.html},src/{SConscript,core/{SConscript,*.{h,cpp}},level_editor/{*.[hc]pp{,.in},*.rb,SConscript,*.svg}}}
rm -f gonstruct-src
mv gonstruct-src.tar.gz graal
