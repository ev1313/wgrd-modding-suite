#include "edat.hpp"

#include <imgui.h>

wgrd_files::EDat::EDat(FileMeta meta) : File(std::move(meta)) {
  // xml_path = out_path / "edat" / vfs_path;
}

void wgrd_files::EDat::render_window() { workspace->render_window(); }

void wgrd_files::EDat::render_extra() { workspace->render_extra(); }

bool wgrd_files::EDat::is_file(const FileMeta &meta) {
  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  char magic[4];
  meta.stream->read(magic, sizeof(magic));

  meta.stream->clear();
  meta.stream->seekg(meta.offset);

  if (!strcmp(magic, "edat")) {
    return true;
  }
  return false;
}

bool wgrd_files::EDat::load_bin(fs::path path) {
  throw std::runtime_error("Not implemented");
  workspace = std::make_unique<Workspace>();
  workspace->workspace_name = meta.vfs_path;
  // fs::path edat_out_path = out_path / "edat" / vfs_path;
  // fs::create_directories(edat_out_path);
  // return workspace->init_from_file(path, edat_out_path);
}
