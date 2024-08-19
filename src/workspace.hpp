#pragma once

#include "file_tree.h"
#include "wgrd_files.h"

#include "toml.hpp"

using namespace wgrd_files;

class Workspaces;

class Workspace {
private:
  std::atomic_bool m_is_parsed = false;
  std::atomic_bool m_is_parsing = false;

  std::optional<std::promise<bool>> m_parsed_promise = std::nullopt;
  std::optional<std::future<bool>> m_parsed_future = std::nullopt;

private:
  FileTree file_tree;
  Files files;

  WorkspaceConfig m_config;

  friend class Workspaces;
  // checks and creates directories
  bool check_directories(fs::path fs_path, fs::path dat_path, fs::path bin_path,
                         fs::path xml_path, fs::path tmp_path);

public:
  explicit Workspace() : files(m_config) {}
  std::string workspace_name;
  static std::optional<std::unique_ptr<Workspace>>
  render_init_workspace(bool *show_workspace);
  bool init(fs::path fs_path, fs::path out_path);
  bool init(fs::path fs_path, fs::path dat_path, fs::path bin_path,
            fs::path xml_path, fs::path tmp_path);
  bool init(const WorkspaceConfig &config);
  bool init_from_file(fs::path file_path, fs::path out_path);
  bool init_from_file(fs::path file_path, fs::path dat_path, fs::path bin_path,
                      fs::path xml_path, fs::path tmp_path);
  void render_window();
  void render_extra();
  // argument determines whether to save to the given dat_path or to save to the
  // input folder
  void save_changes_to_dat(bool save_to_fs_path);
  bool is_changed() { return files.is_changed(); }
  void check_parsing() {
    // if no parse ever started, the future/promise may not exist yet
    if (m_is_parsed) {
      return;
    }
    if (!m_is_parsing) {
      return;
    }
    // but if it started they have to exist
    if (!m_parsed_promise.has_value()) {
      throw std::runtime_error("m_parsed_promise not set");
    }
    if (!m_parsed_future.has_value()) {
      throw std::runtime_error("m_parsed_future not set");
    }
    // check if thread is already done
    if (m_parsed_future.value().wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready) {
      m_is_parsed = m_parsed_future.value().get();
      if (!m_is_parsed) {
        spdlog::error("Failed to parse workspace: {}", workspace_name);
      }
      m_is_parsing = false;
    }
  }
  bool is_parsed() { return m_is_parsed; }
  bool is_parsing() { return m_is_parsing; }
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
