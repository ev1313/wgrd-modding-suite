#include "scenario.hpp"

#include <imgui.h>

wgrd_files::Scenario::Scenario(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
}

bool wgrd_files::Scenario::imgui_call() {
  ImGui::Text("Scenario: %s", vfs_path.c_str());
  return true;
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

