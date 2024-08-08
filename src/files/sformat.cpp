#include "sformat.hpp"

#include <imgui.h>

wgrd_files::SFormat::SFormat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
}

bool wgrd_files::SFormat::imgui_call() {
  ImGui::Text("SFormat: %s", vfs_path.c_str());
  return true;
}

bool wgrd_files::SFormat::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  (void)f;
  (void)offset;
  if(vfs_path.ends_with(".sformat")) {
    return true;
  }

  return false;
}

