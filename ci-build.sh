cmake -DCMAKE_MODULE_PATH="$(pwd)/modules/" -B build/
cmake --build build/ -j8
