#pragma once

#include "toml11/value.hpp"
#include <any>

#include <filesystem>
#include <fstream>
#include <pybind11/embed.h>
namespace py = pybind11;

#include <atomic>
#include <future>

#include "spdlog/spdlog.h"
#include <map>
#include <set>

#include "pugixml.hpp"

#include "configs.hpp"

#include "file_tree.hpp"
#include "files/file.hpp"

#include "toml.hpp"

namespace wgrd_files {

class File;
typedef std::vector<std::unique_ptr<File>> FileList;

class Files {
private:
  // maps vfs_path to files / selected_file
  std::unordered_map<std::string, std::pair<FileList, size_t>> files;
  // used for tracking which windows are opened in imgui
  std::unordered_map<std::string, bool> open_file_windows;

  const WorkspaceConfig &m_config;

public:
  explicit Files(const WorkspaceConfig &config) : m_config(config) {}
  void render_menu(const std::unique_ptr<File> &file);
  void render();
  void add_file(FileMetaList file_metas);
  void open_window(std::string vfs_path);
  void copy_bin_changes(fs::path dat_path, fs::path out_folder_path);
  void save_changes_to_dat(bool save_to_fs_path);
  File *get_file(std::string vfs_path);
  std::vector<std::string> get_files_of_type(FileType type);
  bool is_changed();
};

} // namespace wgrd_files
