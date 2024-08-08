#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class TGV : public File {
public:
  explicit TGV(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
