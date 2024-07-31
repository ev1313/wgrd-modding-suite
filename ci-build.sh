cmake -DCMAKE_PREFIX_PATH="/mingw64/" -DCMAKE_MODULE_PATH="$(pwd)/modules/" -DWIN32=ON -B build/
cmake --build build/ -j8
find /mingw64/ -name "libepoxy-0.dll"
find /mingw64/ -name "libglfw3.dll"
find /mingw64/ -name "libpython3.11.dll"
cp /mingw64/bin/libepoxy-0.dll build/
cp /mingw64/bin/libglfw3.dll build/
cp /mingw64/bin/libpython3.11.dll build/
