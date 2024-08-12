#pragma once

#include "file_tree.h"
#include "wgrd_files.h"

#include "toml.hpp"

class Workspaces;

class Workspace {
private:
  std::atomic_bool m_is_parsed = false;
  std::atomic_bool m_is_parsing = false;
  
  std::optional<std::promise<bool>> m_parsed_promise = std::nullopt;
  std::optional<std::future<bool>> m_parsed_future = std::nullopt;

private:
  FileTree file_tree;
  wgrd_files::Files files;
  std::set<std::string> loaded_file_paths;
  fs::path workspace_dat_path;
  fs::path workspace_out_path;

  friend class Workspaces;
  bool check_directories(fs::path dat_path, fs::path out_path);
public:
  std::string workspace_name;
  static std::optional<std::unique_ptr<Workspace>> render_init_workspace(bool* show_workspace);
  bool init(fs::path dat_path, fs::path out_path);
  bool init_from_file(fs::path file_path, fs::path out_path);
  void render_window();
  void render_extra();
  toml::table to_toml();
  void save_changes_to_dat(bool save_to_fs_path);
  bool is_changed() {
    return files.is_changed();
  }
  void check_parsing() {
    if(m_is_parsed) {
      return;
    }
    if(!m_is_parsing) {
      return;
    }
    if(!m_parsed_promise.has_value()) {
      throw std::runtime_error("m_parsed_promise not set");
    }
    if(!m_parsed_future.has_value()) {
      throw std::runtime_error("m_parsed_future not set");
    }
    if(m_parsed_future.value().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      m_is_parsed = m_parsed_future.value().get();
      if(!m_is_parsed) {
        spdlog::error("Failed to parse workspace: {}", workspace_name);
      }
      m_is_parsing = false;
    }
  }
  bool is_parsed() {
    return m_is_parsed;
  }
  bool is_parsing() {
    return m_is_parsing;
  }
};

class Workspaces {
private:
  std::unordered_map<std::string, std::unique_ptr<Workspace>> workspaces;

  std::unordered_map<std::string, bool> open_workspace_windows;
public:
  bool show_add_workspace = false;
  void render();
  void render_menu();
  void add_workspace(std::unique_ptr<Workspace> workspace);
  void save_workspaces(bool save_to_fs_path = false);
  void save_project_file(fs::path path);
  void load_project_file(fs::path path);
};
