# WG: RD Modding Suite

This modding suite uses the new tools found at [wgrd-tools](https://github.com/ev1313/wgrd-tools), [wgrd-cons-tools](https://github.com/ev1313/wgrd-cons-tools), [wgrd-cons-parsers](https://github.com/ev1313/wgrd-cons-parsers).

## Installation

Windows binaries can be found in the artifacts.

## Building

### Linux

```bash
git clone --recursive https://github.com/ev1313/wgrd-modding-suite
mkdir build
cd build
cmake ..
make -j
python3 -m venv venv/
source venv/bin/activate
pip install wgrd-cons-tools
cp -r ../fonts/ .
./modding_suite
```

### Windows

On Windows you can use [MSYS2](https://www.msys2.org/) to build the project. You will also need to install [Python 3.11](https://www.python.org/downloads/release/python-3119/) into the PATH (a checkbox when installing allows this).

When installed, start the UCRT64 shell and run the following commands:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-libepoxy mingw-w64-ucrt-x86_64-pybind11 mingw-w64-ucrt-x86_64-libiconv
git clone --recursive https://github.com/ev1313/wgrd-modding-suite
cmake -DPython3_ROOT_DIR="${Python3_ROOT_DIR}" -DCMAKE_MODULE_PATH="$(pwd)/modules/" -DWIN32=ON -B build/
cmake --build build/ -j8
cp -r fonts/ build/
```
