premake4 gmake || exit
cd build || exit
make gonstruct -j 3 || exit
cd .. || exit
