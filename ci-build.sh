mkdir build
python3 -m venv --copies build/venv/
source build/venv/bin/activate
pip install wgrd-cons-parsers wgrd-cons-tools

cmake -DCMAKE_PREFIX_PATH="/mingw64/" -DCMAKE_MODULE_PATH="$(pwd)/modules/" -DWIN32=ON -B build/
cmake --build build/ -j8
cp -r /ucrt64/lib/python3.11/ build/venv/lib/
find build/venv/lib/ -name "*.pyc" -delete
cp -r fonts/ build/
cp /ucrt64/bin/libgcc_s_seh-1.dll build/
cp /ucrt64/bin/libwinpthread-1.dll build/
cp /ucrt64/bin/libstdc++-6.dll build/
cp /ucrt64/bin/libintl-8.dll build/
cp /ucrt64/bin/libiconv-2.dll build/
cp /ucrt64/bin/libepoxy-0.dll build/
cp /ucrt64/bin/glfw3.dll build/
cp /ucrt64/bin/libpython3*.dll build/
cp start_modding_suite.bat build/
