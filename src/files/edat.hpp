#pragma once

#include "../workspace.hpp"

namespace wgrd_files {

class __attribute__((visibility("default"))) EDat : public File {
private:
  std::unique_ptr<Workspace> workspace;
public:
  explicit EDat(FileMeta meta, fs::path out_path);
  void render_window() override;
  void render_extra() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
  bool is_parsed() override {
    if(!workspace) {
      return false;
    }
    return workspace->is_parsed();
  }
  bool is_parsing() override {
    if(!workspace) {
      return false;
    }
    return workspace->is_parsing();
  }
  void check_parsing() override {
    if(!workspace) {
      return;
    }
    workspace->check_parsing();
  }
  bool load_bin(fs::path path) override;
};

}

