#pragma once

#include "file_tree.h"
#include "wgrd_files.h"

#include "toml.hpp"

class Workspace {
private:
  FileTree file_tree;
  wgrd_files::Files files;
  std::set<std::string> loaded_file_paths;
  fs::path workspace_dat_path;
  fs::path workspace_out_path;
public:
  bool is_open = true;
  std::string workspace_name;
  static std::optional<Workspace> render_init_workspace(bool* show_workspace);
  bool init(fs::path dat_path, fs::path out_path);
  void render();
  toml::table to_toml();
  void save_changes_to_dat(bool save_to_fs_path);
};

class Workspaces {
private:
  std::vector<Workspace> workspaces;
  
public:
  bool show_add_workspace = false;
  void render();
  void save_workspaces(bool save_to_fs_path = false);
  void save_project_file(fs::path path);
  void load_project_file(fs::path path);
};
