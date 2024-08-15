#include "tgv.hpp"

#include <imgui.h>

wgrd_files::TGV::TGV(FileMeta meta, fs::path out_path) : File(meta, out_path) {}

void wgrd_files::TGV::render_window() {
  ImGui::Text("TGV: %s", vfs_path.c_str());
}

bool wgrd_files::TGV::is_file(std::string vfs_path, std::ifstream &f,
                              size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if (vfs_path.ends_with(".tgv") && magic[0] == 0x02 && magic[1] == 0x00 &&
      magic[2] == 0x00 && magic[3] == 0x00) {
    return true;
  }
  return false;
}
