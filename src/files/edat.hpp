#pragma once

#include "../workspace.hpp"

namespace wgrd_files {

class __attribute__((visibility("default"))) EDat : public File {
private:
  py::object vfs_dict;
  Workspace workspace;
public:
  explicit EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}

