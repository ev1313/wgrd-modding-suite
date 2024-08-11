#pragma once

#include "file_tree.h"
#include "wgrd_files.h"

#include "toml.hpp"

class Workspaces;

class Workspace {
private:
  FileTree file_tree;
  wgrd_files::Files files;
  std::set<std::string> loaded_file_paths;
  fs::path workspace_dat_path;
  fs::path workspace_out_path;

  friend class Workspaces;
public:
  std::string workspace_name;
  static std::optional<Workspace> render_init_workspace(bool* show_workspace);
  bool init(fs::path dat_path, fs::path out_path);
  bool init_from_file(fs::path file_path, fs::path out_path);
  void render_window();
  void render_extra();
  toml::table to_toml();
  void save_changes_to_dat(bool save_to_fs_path);
  bool is_changed() {
    return files.is_changed();
  }
};

class Workspaces {
private:
  std::unordered_map<std::string, Workspace> workspaces;

  std::unordered_map<std::string, bool> open_workspace_windows;
public:
  bool show_add_workspace = false;
  void render();
  void render_menu();
  void add_workspace(Workspace workspace);
  void save_workspaces(bool save_to_fs_path = false);
  void save_project_file(fs::path path);
  void load_project_file(fs::path path);
};
