#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class TGV : public File {
public:
  explicit TGV(FileMeta meta) : File(std::move(meta)) {};
  FileType get_type() override { return FileType::TGV; }
  void render_window() override;
  static bool is_file(const FileMeta &meta);
};

} // namespace wgrd_files
