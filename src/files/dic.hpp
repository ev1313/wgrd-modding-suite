#pragma once

#include "workspace.hpp"

namespace wgrd_files {

struct DicTransaction {
  virtual ~DicTransaction() = default;
  virtual void apply(std::map<std::string, std::string>& data) = 0;
  virtual void undo(std::map<std::string, std::string>& data) = 0;
};

struct DicTransaction_ChangeHash : DicTransaction {
  // both need to be set
  std::string previous_hash;
  std::string new_hash;
  void apply(std::map<std::string, std::string>& data) override {
    std::string value = data[previous_hash];
    data.erase(previous_hash);
    data[new_hash] = value;
  }
  void undo(std::map<std::string, std::string>& data) override {
    std::string value = data[new_hash];
    data.erase(new_hash);
    data[previous_hash] = value;
  }
};

struct DicTransaction_ChangeValue : DicTransaction {
  std::string hash;
  std::string new_value;
  // gets set when applied
  std::string previous_value;
  void apply(std::map<std::string, std::string>& data) override {
    previous_value = data[hash];
    data[hash] = new_value;
  }
  void undo(std::map<std::string, std::string>& data) override {
    data[hash] = previous_value;
  }
};

class Dic : public File {
private:
  bool is_parsed = false;
  std::map<std::string, std::string> dic_data;
  std::vector<std::unique_ptr<DicTransaction>> transactions;
public:
  explicit Dic(FileMeta meta, fs::path out_path);
  bool parse_file();
  bool load_xml(fs::path path) override;
  bool save_xml(fs::path path) override;
  bool save_bin(fs::path path) override;
  bool render() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
