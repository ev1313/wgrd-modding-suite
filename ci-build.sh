cmake -DCMAKE_PREFIX_PATH="/mingw64/" -DCMAKE_MODULE_PATH="$(pwd)/modules/" -B build/
cmake --build build/ -j8
