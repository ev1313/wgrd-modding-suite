#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class Scenario : public File {
public:
  explicit Scenario(FileMeta meta, fs::path out_path);
  void render_window() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
