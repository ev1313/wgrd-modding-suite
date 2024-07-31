#pragma once

#include <optional>
#include <string>
#include <pybind11/pybind11.h>

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
  bool m_rebuild_tree = true;
  std::string m_search = "";
  bool m_tree_view = true;

  int selected_file = -1;
private:
  py::dict vfs_files;
  py::dict vfs_tree;
  std::optional<FileMeta> file_list(py::dict files, const std::string& vfs_path = "$");
  std::optional<FileMeta> file_tree(py::dict files, const std::string& vfs_path = "$", bool open_all = false);
public:
  bool init_from_wgrd_path(fs::path wgrd_path);
  bool init_from_dat_path(fs::path path);
  bool init_from_path(fs::path path);
  bool init_from_stream(std::ifstream& stream);
  std::optional<FileMeta> render();
};
