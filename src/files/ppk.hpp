#pragma once

#include "workspace.hpp"

namespace wgrd_files {

/*
 * Note: most ppk files are actually just EDat files, but with the same
 * extension.
 *
 * This handles edat like ppk files.
 * */
class PPK : public File {
public:
  explicit PPK(FileMeta meta) : File(std::move(meta)) {}
  void render_window() override;
  static bool is_file(const FileMeta &meta);
};

} // namespace wgrd_files
