#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class Ess : public File {
private:
  uint32_t loop_start = 0;
  uint32_t loop_end = 0;

public:
  explicit Ess(FileMeta meta) : File(std::move(meta)) {}
  void render_window() override;
  static bool is_file(const FileMeta &meta);
  bool load_bin();
};

} // namespace wgrd_files
