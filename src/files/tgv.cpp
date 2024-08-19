#include "tgv.hpp"

#include <imgui.h>

void wgrd_files::TGV::render_window() {
  ImGui::Text("TGV: %s", meta.vfs_path.c_str());
}

bool wgrd_files::TGV::is_file(const FileMeta &meta) {
  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  char magic[4];
  meta.stream->read(magic, sizeof(magic));

  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  if (meta.vfs_path.ends_with(".tgv") && magic[0] == 0x02 && magic[1] == 0x00 &&
      magic[2] == 0x00 && magic[3] == 0x00) {
    return true;
  }
  return false;
}
