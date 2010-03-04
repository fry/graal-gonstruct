premake4 embed || exit
premake4 gmake || exit
cd build || exit
make gonstruct config=release -j 3 || exit
cd .. || exit
premake4 dist || exit
