#include "ess.hpp"

#include <imgui.h>

wgrd_files::Ess::Ess(FileMeta meta, fs::path out_path) : File(meta, out_path) {
}

bool wgrd_files::Ess::imgui_call() {
  ImGui::Text("Ess: %s", vfs_path.c_str());
  return true;
}

bool wgrd_files::Ess::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if(magic[0] == 0x01 && magic[1] == 0x00 && magic[2] == 0x02 && magic[3] == 0x02) {
    return true;
  }
  return false;
}
