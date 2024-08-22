#include "scenario.hpp"

#include <imgui.h>

void wgrd_files::Scenario::render_window() {
  ImGui::Text("Scenario: %s", meta.vfs_path.c_str());
}

bool wgrd_files::Scenario::is_file(const FileMeta &meta) {
  auto stream_opt = open_file(meta.fs_path);
  if (!stream_opt) {
    return false;
  }
  auto &stream = stream_opt.value();
  stream.seekg(meta.offset);

  char magic[8];
  stream.read(magic, sizeof(magic));

  if (!strcmp(magic, "SCENARIO")) {
    return true;
  }
  return false;
}
