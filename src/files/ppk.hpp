#pragma once

#include "workspace.hpp"

namespace wgrd_files {

/*
 * Note: most ppk files are actually just EDat files, but with the same extension.
 *
 * This handles edat like ppk files.
 * */
class PPK : public File {
public:
  explicit PPK(FileMeta meta, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
