#pragma once

#include "file_tree.h"
#include "wgrd_files.h"

class Workspace {
private:
  FileTree file_tree;
  wgrd_files::Files files;
  fs::path workspace_dat_path;
  fs::path workspace_out_path;
public:
  std::string workspace_name;
  static std::optional<Workspace> render_init_workspace(bool* show_workspace);
  bool init(fs::path dat_path, fs::path out_path);
  void render();
};

class Workspaces {
private:
  std::vector<Workspace> workspaces;

public:
  bool show_add_workspace = false;
  void render();
};
