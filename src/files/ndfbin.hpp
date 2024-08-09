#pragma once

#include "workspace.hpp"

#include "ndf.hpp"
#include "ndftransactions.hpp"

namespace wgrd_files {

class NdfBin : public File {
private:
  // object count is cached, since it iterates all objects in ndfbin
  // gets set to true if the object count changed after removing / adding / cloning objects
  bool object_count_changed = true;
  std::vector<std::string> object_list;
  std::string object_filter = "";
  std::string class_filter = "";
  int item_current_idx = -1;
  
  int property_item_current_idx = -1;

  // used to detect, whether object changed, to update property_temp array with cached values
  bool object_changed = true;
  std::vector<std::any> property_temp;

  std::map<std::string, bool> open_object_windows;

  NdfBinFile ndfbin;
  std::string render_object_list();
std::optional<std::unique_ptr<NdfTransaction>> render_object_info(std::string object_name);
  void render_property_list(std::string object_name);
  void render_property(std::string object_name, std::string property_name);
  std::optional<std::unique_ptr<NdfTransactionChangeProperty>> render_ndf_type(std::unique_ptr<NDFProperty>& property);
public:
  explicit NdfBin(FileMeta meta, fs::path out_path);
  bool render() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
  bool load_xml(fs::path path) override;
  bool save_xml(fs::path path) override;
  bool save_bin(fs::path path) override;
};

}

