#pragma once

#include "file_tree.h"
#include "wgrd_files.h"

struct Workspace {
private:
  FileTree file_tree;
  wgrd_files::Files files;
public:
  std::string workspace_name;
  static std::optional<Workspace> render_init_workspace();
  bool init(fs::path dat_path, fs::path out_path);
  void render();
};
