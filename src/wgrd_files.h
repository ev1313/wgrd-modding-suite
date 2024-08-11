#pragma once

#include <any>

#include <fstream>
#include <filesystem>
#include <pybind11/embed.h>
namespace py = pybind11;

#include <future>

#include <set>
#include <map>
#include "spdlog/spdlog.h"

#include "pugixml.hpp"

#include "file_tree.h"

namespace wgrd_files {

class Files;

class File {
protected:
  std::string vfs_path;
  std::ifstream file;
  size_t offset;
  size_t size;
  fs::path out_path;
  fs::path xml_path;
  bool m_is_changed = false;
  fs::path fs_path;

  friend class Files;
public:
  explicit File(FileMeta meta, fs::path out_path);
  void render();
  // this function is called *inside* the window
  virtual void render_window();
  // this function is called *outside* the window (e.g. ndfbin uses this to spawn new object windows)
  virtual void render_extra() {};
  // FIXME: for what is this function
  std::vector<char> get_file();
  // this function returns the bytes stored in the given filestream
  std::vector<char> get_data();
  // this function just plainly copies from the given filestream to the given path
  bool copy_to_file(fs::path path);

  virtual bool load_xml(fs::path path) {
    spdlog::error("cannot load xml file {} into {}", vfs_path, path.string());
    return false;
  }
  virtual bool save_xml(fs::path path) {
    spdlog::error("cannot save xml file {} into {}", vfs_path, path.string());
    return false;
  }
  virtual bool load_bin(fs::path path) {
    spdlog::error("cannot save bin file {} into {}", vfs_path, path.string());
    return false;
  }
  virtual bool save_bin(fs::path path) {
    spdlog::error("cannot save bin file {} into {}", vfs_path, path.string());
    return false;
  }

  virtual bool undo() {
    spdlog::error("cannot undo {}", vfs_path);
    return false;
  }
  virtual bool redo() {
    spdlog::error("cannot redo {}", vfs_path);
    return false;
  }

  bool is_changed() {
    return m_is_changed;
  }
};

class Files {
private:
  std::unordered_map<size_t, std::unique_ptr<File>> files;
  std::unordered_map<size_t, bool> open_file_windows;
public:
  void render();
  void add_file(fs::path out_path, FileMeta meta, size_t offset = 0);
  void open_window(FileMeta meta);
  void copy_bin_changes(fs::path dat_path, fs::path out_folder_path);
  bool is_changed() {
    for (auto& [_, file] : files) {
      if (file->is_changed()) {
        return true;
      }
    }
    return false;
  }
};

}
