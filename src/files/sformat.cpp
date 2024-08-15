#include "sformat.hpp"

#include <imgui.h>

wgrd_files::SFormat::SFormat(FileMeta meta, fs::path out_path)
    : File(meta, out_path) {}

void wgrd_files::SFormat::render_window() {
  ImGui::Text("SFormat: %s", vfs_path.c_str());
}

bool wgrd_files::SFormat::is_file(std::string vfs_path, std::ifstream &f,
                                  size_t offset) {
  (void)f;
  (void)offset;
  if (vfs_path.ends_with(".sformat")) {
    return true;
  }

  return false;
}
