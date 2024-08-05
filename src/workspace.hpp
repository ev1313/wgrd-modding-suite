#pragma once

#include "file_tree.h"
#include "wgrd_files.h"

#include "toml.hpp"

class Workspace {
private:
  FileTree file_tree;
  wgrd_files::Files files;
  fs::path workspace_dat_path;
  fs::path workspace_out_path;
public:
  bool is_open = true;
  std::string workspace_name;
  static std::optional<Workspace> render_init_workspace(bool* show_workspace);
  bool init(fs::path dat_path, fs::path out_path);
  void render();
  toml::table to_toml();
};

class Workspaces {
private:
  std::vector<Workspace> workspaces;

public:
  bool show_add_workspace = false;
  void render();
  void save_workspaces(fs::path path);
  void load_workspaces(fs::path path);
};
