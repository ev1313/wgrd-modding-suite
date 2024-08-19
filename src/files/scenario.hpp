#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class Scenario : public File {
public:
  explicit Scenario(FileMeta meta) : File(std::move(meta)) {}
  FileType get_type() override { return FileType::SCENARIO; }
  void render_window() override;
  static bool is_file(const FileMeta &meta);
};

} // namespace wgrd_files
