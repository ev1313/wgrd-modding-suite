#pragma once

#include <optional>
#include <pybind11/pybind11.h>
#include <string>

#include "tsl/ordered_map.h"
#include <map>

#include <libintl.h>

namespace py = pybind11;

#include <filesystem>
namespace fs = std::filesystem;

struct FileMeta {
  std::string vfs_path;
  std::string fs_path;
  size_t offset;
  size_t size;
  size_t idx;
};

class __attribute__((visibility("default"))) FileTree {
private:
  std::string m_search = "";
  std::string m_search_lower = "";
  bool m_tree_view = true;

  // full vfs path -> FileMeta
  std::map<std::string, FileMeta> vfs_files;
  // the vfs_files get filtered into this
  tsl::ordered_map<std::string, uint32_t> vfs_string_table;
  std::map<std::vector<uint32_t>, std::string> vfs_indexed_files;
  std::map<std::vector<uint32_t>, std::string> vfs_filtered_files;

  std::string selected_vfs_path = "";
  void create_filetree(fs::path path, bool is_file = false);
  void fill_filetree(py::dict files);
  void filter_filetree();
  std::optional<FileMeta> render_file_list();
  std::optional<FileMeta> render_file_tree();

public:
  bool init_from_wgrd_path(fs::path wgrd_path);
  bool init_from_dat_path(fs::path path);
  bool init_from_path(fs::path path);
  bool init_from_stream(std::ifstream &stream);
  std::optional<FileMeta> render();
};
