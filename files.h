#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include "wgrd_files.h"
#include "file_tree.h"

#include <pybind11/embed.h>
namespace py = pybind11;

namespace wgrd_files {

class Files {
private:
    std::vector<std::unique_ptr<File>> files;
public:
    void imgui_call();
    bool add_file(FileMeta meta);
};

}