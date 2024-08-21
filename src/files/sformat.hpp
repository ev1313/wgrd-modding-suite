#pragma once

#include "file.hpp"
#include "workspace.hpp"

namespace wgrd_files {

class SFormat : public File {
public:
  explicit SFormat(const Files *files, FileMeta meta)
      : File(files, std::move(meta)) {}
  FileType get_type() override { return FileType::SFORMAT; }
  void render_window() override;
  static bool is_file(const FileMeta &meta);
};

} // namespace wgrd_files
