cmake -DCMAKE_PREFIX_PATH="/mingw64/" -DCMAKE_MODULE_PATH="$(pwd)/modules/" -DWIN32=ON -B build/
cmake --build build/ -j8
cp /mingw64/bin/libepoxy-0.dll build/
cp /mingw64/bin/glfw3.dll build/
cp /mingw64/bin/libpython3.*.dll build/
