#pragma once

#include "workspace.hpp"

namespace wgrd_files {

class Ess : public File {
private:
  uint32_t loop_start = 0;
  uint32_t loop_end = 0;
  bool is_parsed = false;
public:
  explicit Ess(FileMeta meta, fs::path out_path);
  bool render() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
  bool load_bin();
};

}
