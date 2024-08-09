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

class File;

class Files {
private:
  std::map<size_t, std::unique_ptr<File>> files;
public:
  void render();
  void add_file(fs::path out_path, FileMeta meta, size_t offset = 0);
  void open_window(FileMeta meta);
  void copy_bin_changes(fs::path dat_path, fs::path out_folder_path);
};

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
  bool window_opened = true;
  explicit File(FileMeta meta, fs::path out_path);
  virtual bool render();
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
  virtual bool save_bin(fs::path path) {
    spdlog::error("cannot save bin file {} into {}", vfs_path, path.string());
    return false;
  }
  bool is_changed() {
    return m_is_changed;
  }
};


}
