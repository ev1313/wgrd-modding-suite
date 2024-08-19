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

#include "file_tree.h"

#include "toml.hpp"

namespace wgrd_files {

struct WorkspaceConfig {
  // name of the workspace
  std::string name;
  // path to the input files
  fs::path fs_path;
  // output folder for the generated dat files
  fs::path dat_path;
  // output folder for loaded and generated bin files
  // (mostly extracted from dat files or generated for dat files)
  fs::path bin_path;
  // output folder for loaded and generated xml files
  fs::path xml_path;
  // temp folder
  fs::path tmp_path;
  toml::table to_toml() {
    toml::table table;
    table["name"] = name;
    table["fs_path"] = fs_path.string();
    table["dat_path"] = dat_path.string();
    table["bin_path"] = bin_path.string();
    table["xml_path"] = xml_path.string();
    table["tmp_path"] = tmp_path.string();
    return table;
  }
  bool from_toml(toml::table table) noexcept {
    try {
      name = table["name"].as_string();
      fs_path = table["fs_path"].as_string();
      dat_path = table["dat_path"].as_string();
      bin_path = table["bin_path"].as_string();
      xml_path = table["xml_path"].as_string();
      tmp_path = table["tmp_path"].as_string();
    } catch (const toml::type_error &e) {
      spdlog::error("error while parsing workspace config: {}", e.what());
      return false;
    }
    return true;
  }
};

class Files;

class File {
private:
  std::atomic_bool m_is_parsed = false;
  std::atomic_bool m_is_parsing = false;

  std::optional<std::promise<bool>> m_parsed_promise;
  std::optional<std::future<bool>> m_parsed_future;

protected:
  // stores the actual informations regarding the dat file
  FileMeta meta;
  // path to the tmp directory of this file
  // (may be used while parsing or saving, will be deleted afterwards)
  fs::path tmp_path;
  // path to the binary output file of this file
  fs::path bin_path;
  // path to the xml file describing this file
  // (may not exist, will be created if it doesn't exist yet on parsing or
  // save_xml call)
  fs::path xml_path;
  // FIXME: maybe we should just create a transaction list here instead
  // of having different transaction lists that mark this flag themselves
  bool m_is_changed = false;

  friend class Files;

public:
  explicit File(FileMeta meta);
  void render();
  // this function should call the imgui shortcuts
  virtual void render_shortcuts();
  // this function is called *inside* the BeginMenuBar call
  virtual void render_menu();
  // this function is called *inside* the window
  virtual void render_window();
  // this function is called *outside* the window (e.g. ndfbin uses this to
  // spawn new object windows)
  virtual void render_extra() {};
  // this function returns the bytes stored in the given filestream
  std::vector<char> get_data();
  // this function just plainly copies from the given filestream to the given
  // path
  bool copy_to_file(fs::path path);

  void start_parsing(bool try_xml = true);

  // default implementation, may be overridden
  virtual bool load_stream() {
    copy_to_file(bin_path);
    return load_bin(bin_path);
  }

  virtual bool load_xml(fs::path path) {
    spdlog::error("cannot load xml file {} into {}", meta.vfs_path,
                  path.string());
    return false;
  }
  virtual bool save_xml(fs::path path) {
    spdlog::error("cannot save xml file {} into {}", meta.vfs_path,
                  path.string());
    return false;
  }
  virtual bool load_bin(fs::path path) {
    spdlog::error("cannot save bin file {} into {}", meta.vfs_path,
                  path.string());
    return false;
  }
  virtual bool save_bin(fs::path path) {
    spdlog::error("cannot save bin file {} into {}", meta.vfs_path,
                  path.string());
    return false;
  }

  virtual bool undo() {
    spdlog::error("cannot undo {}", meta.vfs_path);
    return false;
  }
  virtual bool redo() {
    spdlog::error("cannot redo {}", meta.vfs_path);
    return false;
  }

  virtual bool is_changed() { return m_is_changed; }

  // virtual since edat overwrites it to pass down to workspace
  virtual void check_parsing() {
    if (m_is_parsed) {
      return;
    }
    if (!m_is_parsing) {
      return;
    }
    if (!m_parsed_promise.has_value()) {
      throw std::runtime_error("m_ndf_parsed_promise not set");
    }
    if (!m_parsed_future.has_value()) {
      throw std::runtime_error("m_ndf_parsed_future not set");
    }
    if (m_parsed_future.value().wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready) {
      m_is_parsed = m_parsed_future.value().get();
      m_is_parsing = false;
    }
  }

  // virtual since edat overwrites it to pass down to workspace
  virtual bool is_parsed() { return m_is_parsed; }

  // virtual since edat overwrites it to pass down to workspace
  virtual bool is_parsing() { return m_is_parsing; }
};

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
  bool is_changed() {
    for (const auto &[_, file_idx] : files) {
      const auto &[files, _] = file_idx;
      for (const auto &file : files) {
        if (file->is_changed()) {
          return true;
        }
      }
    }
    return false;
  }
};

} // namespace wgrd_files
