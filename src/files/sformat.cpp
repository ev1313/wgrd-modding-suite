#include "sformat.hpp"

#include <imgui.h>

void wgrd_files::SFormat::render_window() {
  ImGui::Text("SFormat: %s", meta.vfs_path.c_str());
}

bool wgrd_files::SFormat::is_file(const FileMeta &meta) {
  if (meta.vfs_path.ends_with(".sformat")) {
    return true;
  }

  return false;
}
