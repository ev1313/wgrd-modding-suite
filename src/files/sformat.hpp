#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class SFormat : public File {
public:
  explicit SFormat(FileMeta meta, fs::path out_path);
  void render_window() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

} // namespace wgrd_files
