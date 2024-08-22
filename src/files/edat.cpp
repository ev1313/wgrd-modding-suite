#include "edat.hpp"
#include "files.hpp"

#include <imgui.h>

#include "helpers.hpp"

wgrd_files::EDat::EDat(const Files *files, FileMeta meta)
    : File(files, std::move(meta)) {
  // xml_path = out_path / "edat" / vfs_path;
}

void wgrd_files::EDat::render_window() { workspace->render_window(); }

void wgrd_files::EDat::render_extra() { workspace->render_extra(); }

bool wgrd_files::EDat::is_file(const FileMeta &meta) {
  auto stream_opt = open_file(meta.fs_path);
  if (!stream_opt) {
    return false;
  }
  auto &stream = stream_opt.value();
  stream.seekg(meta.offset);

  char magic[4];
  stream.read(magic, sizeof(magic));

  if (!strcmp(magic, "edat")) {
    return true;
  }
  return false;
}

bool wgrd_files::EDat::load_bin(fs::path path) {
  workspace = std::make_unique<Workspace>();
  WorkspaceConfig config;
  config.name = meta.vfs_path;
  config.fs_path = path;
  config.dat_path = path.parent_path();
  config.bin_path = append_ext(bin_path, ".bin");
  config.xml_path = xml_path.parent_path() / fs::path(meta.vfs_path).filename();
  config.tmp_path = tmp_path;
  spdlog::info("loading edat with config: name {} fs_path {} dat_path {} "
               "bin_path {} xml_path {} tmp_path {}",
               config.name, config.fs_path.string(), config.dat_path.string(),
               config.bin_path.string(), config.xml_path.string(),
               config.tmp_path.string());
  return workspace->init_from_file(config);
}
