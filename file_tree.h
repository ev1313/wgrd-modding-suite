#pragma once

#include <map>
#include <string>
#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <filesystem>
namespace fs = std::filesystem;

struct FileMeta {
    std::string vfs_path;
    std::string fs_path;
    size_t offset;
    size_t size;
};

class FileTree {
private:
  py::dict vfs_files;
  py::dict vfs_tree;
  std::vector<FileMeta> file_tree(py::dict files, const std::string& vfs_path = "$", bool open_all = false);
public:
  bool init_from_wgrd_path(fs::path wgrd_path);
  bool init_from_path(fs::path path);
  std::vector<FileMeta> render(const std::string& name);
};
