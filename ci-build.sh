pacman -S mingw-w64-x86_64-glfw cmake ninja --noconfirm --needed
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja
