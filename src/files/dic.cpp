#include "dic.hpp"

#include <imgui.h>

wgrd_files::Dic::Dic(FileMeta meta, fs::path out_path) : File(meta, out_path) {
}

bool wgrd_files::Dic::imgui_call() {
  ImGui::Text("Dic: %s", vfs_path.c_str());
  return true;
}

bool wgrd_files::Dic::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if(!strcmp(magic, "TRAD")) {
    return true;
  }
  return false;
}

