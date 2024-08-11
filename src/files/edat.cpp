#include "edat.hpp"

#include <imgui.h>

wgrd_files::EDat::EDat(FileMeta meta, fs::path out_path) : File(meta, out_path) {
  workspace.workspace_name = vfs_path;
  copy_to_file(out_path / "bin" / vfs_path);
  workspace.init_from_file(out_path / "bin" / vfs_path, out_path);
}

void wgrd_files::EDat::render_window() {
  workspace.render_window();
}

void wgrd_files::EDat::render_extra() {
  workspace.render_extra();
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
