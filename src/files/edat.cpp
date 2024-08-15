#include "edat.hpp"

#include <imgui.h>

wgrd_files::EDat::EDat(FileMeta meta, fs::path out_path)
    : File(meta, out_path) {
  xml_path = out_path / "edat" / vfs_path;
}

void wgrd_files::EDat::render_window() { workspace->render_window(); }

void wgrd_files::EDat::render_extra() { workspace->render_extra(); }

bool wgrd_files::EDat::is_file(std::string vfs_path, std::ifstream &f,
                               size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if (!strcmp(magic, "edat")) {
    return true;
  }
  return false;
}

bool wgrd_files::EDat::load_bin(fs::path path) {
  workspace = std::make_unique<Workspace>();
  workspace->workspace_name = vfs_path;
  fs::path edat_out_path = out_path / "edat" / vfs_path;
  fs::create_directories(edat_out_path);
  return workspace->init_from_file(path, edat_out_path);
}
