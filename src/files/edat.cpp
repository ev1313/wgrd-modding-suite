#include "edat.hpp"

#include <imgui.h>

wgrd_files::EDat::EDat(FileMeta meta, fs::path out_path) : File(meta, out_path) {
}

void wgrd_files::EDat::render_window() {
  if(!workspace->is_parsed()) {
    if(!workspace->is_parsing()) {
      spdlog::error("Workspace is not loaded and probably failed loading! {}", workspace->workspace_name);
    } else {
      ImGui::Text("Parsing %s", workspace->workspace_name.c_str());
    }
  } else {
    workspace->render_window();
  }
}

void wgrd_files::EDat::render_extra() {
  if(workspace->is_parsed()) {
    workspace->render_extra();
  }
}

bool wgrd_files::EDat::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if(!strcmp(magic, "edat")) {
    return true;
  }
  return false;
}

bool wgrd_files::EDat::load_bin(fs::path path) {
  workspace = std::make_unique<Workspace>();
  workspace->workspace_name = vfs_path;
  return workspace->init_from_file(path, out_path);
}
