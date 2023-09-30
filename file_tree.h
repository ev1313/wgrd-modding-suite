#pragma once

#include <map>
#include <string>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct FileMeta {
    std::string vfs_path;
    std::string fs_path;
    size_t offset;
    size_t size;
};

std::vector<FileMeta> imgui_file_tree(const std::string& name, py::dict files);