#include "ppk.hpp"

#include <imgui.h>

void wgrd_files::PPK::render_window() {
  ImGui::Text("PPK: %s", meta.vfs_path.c_str());
}

bool wgrd_files::PPK::is_file(const FileMeta &meta) {
  auto stream_opt = open_file(meta.fs_path);
  if (!stream_opt) {
    return false;
  }
  auto &stream = stream_opt.value();
  stream.seekg(meta.offset);

  char magic[8];
  stream.read(magic, sizeof(magic));

  stream.clear();
  stream.seekg(meta.offset);

  if (meta.vfs_path.ends_with(".ppk") && !strcmp(magic, "PRXYPCPC")) {
    return true;
  }
  return false;
}
