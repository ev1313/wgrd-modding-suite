#include "ppk.hpp"

#include <imgui.h>

wgrd_files::PPK::PPK(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
}

bool wgrd_files::PPK::imgui_call() {
  ImGui::Text("PPK: %s", vfs_path.c_str());
  return true;
}

bool wgrd_files::PPK::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[8];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if(vfs_path.ends_with(".ppk") && !strcmp(magic, "PRXYPCPC")) {
    return true;
  }
  return false;
}
