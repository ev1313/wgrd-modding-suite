#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class Ess : public File {
public:
  explicit Ess(FileMeta meta, fs::path out_path);
  bool render() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
