#pragma once

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
  std::vector<std::unique_ptr<File>> files;
public:
  void imgui_call();
  bool add_file(FileMeta meta, std::ifstream& f, size_t f_offset = 0);
};

class File {
protected:
  std::string vfs_path;
  std::ifstream file;
  size_t offset;
  size_t size;
public:
  explicit File(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  virtual bool imgui_call();
  std::vector<char> get_file();
  bool copy_to_file(std::filesystem::path path);
};

class EDat : public File {
private:
  py::object vfs_dict;
  FileTree file_tree;
  Files files;
public:
  explicit EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Ess : public File {
public:
  explicit Ess(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class SFormat : public File {
public:
  explicit SFormat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class TGV : public File {
public:
  explicit TGV(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class PPK : public File {
public:
  explicit PPK(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Scenario : public File {
public:
  explicit Scenario(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Dic : public File {
public:
  explicit Dic(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class NdfBin : public File {
private:
  NdfBinFile ndfbin;
  int render_object_list();
  int render_property_list(int object_idx);
  void render_property(int object_idx, int property_idx);
public:
  explicit NdfBin(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
  bool imgui_call() override;
  static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
