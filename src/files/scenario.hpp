#pragma once

#include "file.hpp"
#include "workspace.hpp"

namespace wgrd_files {

class Scenario : public File {
public:
  explicit Scenario(const Files *files, FileMeta meta)
      : File(files, std::move(meta)) {}
  FileType get_type() override { return FileType::SCENARIO; }
  void render_window() override;
  static bool is_file(const FileMeta &meta);
};

} // namespace wgrd_files
