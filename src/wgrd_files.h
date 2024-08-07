#pragma once

#include <any>

#include <fstream>
#include <filesystem>
#include <pybind11/embed.h>
namespace py = pybind11;

#include <future>

#include "pugixml.hpp"

#include "file_tree.h"

#include "ndftransactions.hpp"

namespace wgrd_files {

class File;

class Files {
private:
  std::map<size_t, std::unique_ptr<File>> files;
public:
  void imgui_call();
  void add_file(fs::path out_path, FileMeta meta, size_t offset = 0);
  void open_window(FileMeta meta);
};

class File {
protected:
  std::string vfs_path;
  std::ifstream file;
  size_t offset;
  size_t size;
  fs::path out_path;
public:
  bool window_opened = true;
  explicit File(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  virtual bool imgui_call();
  std::vector<char> get_file();
  std::vector<char> get_data();
  bool copy_to_file(std::filesystem::path path);
};

class __attribute__((visibility("default"))) EDat : public File {
private:
  py::object vfs_dict;
  FileTree file_tree;
  Files files;
public:
  explicit EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Ess : public File {
public:
  explicit Ess(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class SFormat : public File {
public:
  explicit SFormat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class TGV : public File {
public:
  explicit TGV(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class PPK : public File {
public:
  explicit PPK(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Scenario : public File {
public:
  explicit Scenario(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Dic : public File {
public:
  explicit Dic(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

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

  fs::path xml_path;

  NdfBinFile ndfbin;
  std::string render_object_list();
  void render_property_list(std::string object_name);
  void render_property(std::string object_name, std::string property_name);
  std::optional<std::unique_ptr<NdfTransactionChangeProperty>> render_ndf_type(std::unique_ptr<NDFProperty>& property);
public:
  explicit NdfBin(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
