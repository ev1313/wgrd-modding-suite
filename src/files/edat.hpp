#pragma once

#include "../workspace.hpp"

namespace wgrd_files {

class __attribute__((visibility("default"))) EDat : public File {
private:
  py::object vfs_dict;
  Workspace workspace;
public:
  explicit EDat(FileMeta meta, fs::path out_path);
  bool render() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}

