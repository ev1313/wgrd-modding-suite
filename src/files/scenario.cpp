#include "scenario.hpp"

#include <imgui.h>

wgrd_files::Scenario::Scenario(FileMeta meta, fs::path out_path) : File(meta, out_path) {
}

void wgrd_files::Scenario::render_window() {
  ImGui::Text("Scenario: %s", vfs_path.c_str());
}

bool wgrd_files::Scenario::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[8];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if(!strcmp(magic, "SCENARIO")) {
    return true;
  }
  return false;
}

