#pragma once

#include "workspace.hpp"

#include "ndf.hpp"
#include "ndftransactions.hpp"

namespace wgrd_files {

class NdfBin : public File {
private:
  // object count is cached, since it iterates all objects in ndfbin
  // gets set to true if the object count changed after removing / adding /
  // cloning objects
  bool object_count_changed = true;
  // this is the option, whether the object and class list should be filtered
  // again
  bool filter_changed = true;
  std::vector<std::string> object_list_filtered;
  std::string object_filter = "";
  std::string object_filter_lower = "";
  std::string class_filter = "";
  std::string class_filter_lower = "";
  // only for being able to save the last clicked position for auto focus of the
  // selected object
  int item_current_idx = -1;

  int property_item_current_idx = -1;

  // used to detect, whether object changed, to update property_temp array with
  // cached values
  bool object_changed = true;
  std::vector<std::any> property_temp;

  std::map<std::string, bool> open_object_windows;

  NdfBinFile ndfbin;
  void render_object_list();

  struct Property {
    // maps the possible value to an object name
    std::map<std::string, std::unordered_set<std::string>> values;
  };

  struct Class {
    // used for rendering the object list
    std::vector<std::string> objects;
    // maps the property name to the property
    std::map<std::string, Property> properties;
  };
  // maps the class name to the class
  std::map<std::string, Class> class_list;
  // contains a mapping object_name -> objects referencing the object
  std::unordered_map<std::string, std::unordered_set<std::string>>
      object_references;
  // contains a mapping export_path -> objects names importing it
  std::unordered_map<std::string, std::unordered_set<std::string>>
      import_references;
  // used for filtering the class list
  std::vector<std::string> class_list_filtered;
  // only for being able to save the last clicked position for auto focus
  std::string selected_class = "";
  std::unordered_map<std::string, bool> open_class_windows;
  std::unordered_map<std::string, bool> open_class_bulk_rename_windows;
  void fill_class_list();
  std::string render_class_list();
  void render_classes();
  void render_class_menu(std::string class_name);

  int bulk_rename_property_count = 1;
  std::string bulk_rename_prefix = "";
  std::vector<std::string> bulk_rename_selected_properties = {""};
  // FIXME: this should be cleared every time the bulk rename window is opened
  std::map<std::string, std::pair<std::string, bool>> bulk_rename_overrides;
  void render_bulk_renames();

  std::optional<std::unique_ptr<NdfTransaction>>
  render_object_info(std::string object_name);
  void render_property_list(std::string object_name);
  void render_property(std::string object_name, std::string property_name);
  std::optional<std::unique_ptr<NdfTransactionChangeProperty>>
  render_ndf_type(std::unique_ptr<NDFProperty> &property);

  // returns list of object_names containing a reference to the given
  // export_path
  std::unordered_set<std::string>
  get_import_references(std::string export_path);
  // returns whether this object references the given export_path
  bool references_export_path(std::string export_path);

public:
  explicit NdfBin(FileMeta meta) : File(std::move(meta)) {}
  FileType get_type() override { return FileType::NDFBIN; }
  void render_window() override;
  void render_extra() override;
  static bool is_file(const FileMeta &meta);
  bool load_xml(fs::path path) override;
  bool save_xml(fs::path path) override;
  bool load_bin(fs::path path) override;
  bool save_bin(fs::path path) override;
  void open_window(std::string object_name);
  void close_window(std::string object_name);
};

} // namespace wgrd_files
