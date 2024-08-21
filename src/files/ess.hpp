#pragma once

#include "file.hpp"
#include "workspace.hpp"

namespace wgrd_files {

class Ess : public File {
private:
  uint32_t loop_start = 0;
  uint32_t loop_end = 0;

public:
  explicit Ess(const Files *files, FileMeta meta)
      : File(files, std::move(meta)) {}
  FileType get_type() override { return FileType::ESS; }
  void render_window() override;
  static bool is_file(const FileMeta &meta);
  bool load_bin();
};

} // namespace wgrd_files
