cmake -DPython3_ROOT_DIR="${Python3_ROOT_DIR}" -DCMAKE_MODULE_PATH="$(pwd)/modules/" -DWIN32=ON -B build/
cmake --build build/ -j8
cp -r fonts/ build/
cp /ucrt64/bin/libgcc_s_seh-1.dll build/
cp /ucrt64/bin/libwinpthread-1.dll build/
cp /ucrt64/bin/libstdc++-6.dll build/
cp /ucrt64/bin/libintl-8.dll build/
cp /ucrt64/bin/libiconv-2.dll build/
cp /ucrt64/bin/libepoxy-0.dll build/
cp /ucrt64/bin/glfw3.dll build/
cp start_modding_suite.bat build/
