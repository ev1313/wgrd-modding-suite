#pragma once

#include <memory>
#include <optional>
#include <pybind11/pybind11.h>
#include <string>

#include "tsl/ordered_map.h"
#include <map>

#include <libintl.h>

namespace py = pybind11;

#include <filesystem>
namespace fs = std::filesystem;

#include <fstream>
#include <spdlog/spdlog.h>

namespace wgrd_files {

class FileTree;

struct FileMeta {
  // path in the vfs of the game
  std::string vfs_path;
  // path to the dat file
  fs::path fs_path;
  // offset inside the dat file
  size_t offset;
  // size of the dat file
  size_t size;
  // unique index of the dat file for the workspace
  // FIXME: technically useless now
  size_t idx;
  // used by Files to access the file
  std::unique_ptr<std::ifstream> stream;

private:
  // we do not want any other class then FileTree to be able to copy the
  // FileMeta, since it should only be handled by exactly one File instance
  FileMeta get_copy() const {
    FileMeta ret;
    ret.vfs_path = vfs_path;
    ret.fs_path = fs_path;
    ret.offset = offset;
    ret.size = size;
    ret.idx = idx;
    return std::move(ret);
  }
  friend class FileTree;
};

typedef std::vector<FileMeta> FileMetaList;

class FileTree {
private:
  std::string m_search = "";
  std::string m_search_lower = "";
  bool m_tree_view = true;

  // full vfs path -> FileMeta
  std::map<std::string, FileMetaList> vfs_files;
  // the vfs_files get filtered into this
  tsl::ordered_map<std::string, uint32_t> vfs_string_table;
  std::map<std::vector<uint32_t>, std::string> vfs_indexed_files;
  std::map<std::vector<uint32_t>, std::string> vfs_filtered_files;

  std::string selected_vfs_path = "";
  void create_filetree(fs::path path, bool is_file = false);
  void fill_filetree(py::dict files);
  void filter_filetree();
  std::optional<std::string> render_file_list();
  std::optional<std::string> render_file_tree();

public:
  bool init_from_wgrd_path(fs::path wgrd_path);
  bool init_from_dat_path(fs::path path);
  bool init_from_path(fs::path path);
  bool init_from_stream(std::ifstream &stream);
  std::optional<FileMetaList> render();
  std::vector<FileMetaList> get_all_files() {
    std::vector<FileMetaList> ret;
    for (auto &[_, metas] : vfs_files) {
      ret.push_back({});
      for (auto &meta : metas) {
        if (meta.vfs_path.ends_with(".ndfbin")) {
          spdlog::info("all files contains {}", meta.vfs_path);
          ret.back().push_back(meta.get_copy());
        }
      }
    }
    return ret;
  }
};

} // namespace wgrd_files
