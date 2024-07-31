ls -hal /mingw64/lib/cmake/glfw3/
cmake -DCMAKE_MODULE_PATH="/mingw64/lib/cmake/glfw3/ $(pwd)/modules/" -B build/
cmake --build build/ -j8
