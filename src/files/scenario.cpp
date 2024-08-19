#include "scenario.hpp"

#include <imgui.h>

void wgrd_files::Scenario::render_window() {
  ImGui::Text("Scenario: %s", meta.vfs_path.c_str());
}

bool wgrd_files::Scenario::is_file(const FileMeta &meta) {
  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  char magic[8];
  meta.stream->read(magic, sizeof(magic));

  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  if (!strcmp(magic, "SCENARIO")) {
    return true;
  }
  return false;
}
