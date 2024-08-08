#include "edat.hpp"

#include <imgui.h>

wgrd_files::EDat::EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {

}

bool wgrd_files::EDat::imgui_call() {


  ImGui::Text("EDat: %s", vfs_path.c_str());
  return true;
}

bool wgrd_files::EDat::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if(!strcmp(magic, "edat")) {
    return true;
  }
  return false;
}
