#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class SFormat : public File {
public:
  explicit SFormat(FileMeta meta) : File(std::move(meta)) {}
  FileType get_type() override { return FileType::SFORMAT; }
  void render_window() override;
  static bool is_file(const FileMeta &meta);
};

} // namespace wgrd_files
