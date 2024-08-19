#include "ppk.hpp"

#include <imgui.h>

void wgrd_files::PPK::render_window() {
  ImGui::Text("PPK: %s", meta.vfs_path.c_str());
}

bool wgrd_files::PPK::is_file(const FileMeta &meta) {
  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  char magic[8];
  meta.stream->read(magic, sizeof(magic));

  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  if (meta.vfs_path.ends_with(".ppk") && !strcmp(magic, "PRXYPCPC")) {
    return true;
  }
  return false;
}
