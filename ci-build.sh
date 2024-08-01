mkdir build
python3 -m venv build/venv/
source build/venv/bin/activate
pip install wgrd-cons-parsers wgrd-cons-tools

cmake -DCMAKE_PREFIX_PATH="/mingw64/" -DCMAKE_MODULE_PATH="$(pwd)/modules/" -DWIN32=ON -B build/
cmake --build build/ -j8
cp -r /ucrt64/lib/python3.11/* build/venv/lib/python3.11/
cp /ucrt64/bin/libgcc_s_seh-1.dll build/
cp /ucrt64/bin/libwinpthread-1.dll build/
cp /ucrt64/bin/libepoxy-0.dll build/
cp /ucrt64/bin/glfw3.dll build/
cp /ucrt64/bin/libpython3*.dll build/
