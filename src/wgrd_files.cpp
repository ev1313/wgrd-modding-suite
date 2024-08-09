#include "wgrd_files.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <iostream>
#include <imgui.h>

#include "file_tree.h"

#include "ImGuiFileDialog.h"

#include <libintl.h>

#include "helpers.hpp"

#include <filesystem>

#include "files/dic.hpp"
#include "files/edat.hpp"
#include "files/ess.hpp"
#include "files/ndfbin.hpp"
#include "files/ppk.hpp"
#include "files/scenario.hpp"
#include "files/sformat.hpp"
#include "files/tgv.hpp"

namespace fs = std::filesystem;
using namespace wgrd_files;

bool wgrd_files::File::render() {
  ImGui::Text("File: %s", vfs_path.c_str());
  if(ImGui::Button("save file")) {
    std::filesystem::path vfs(vfs_path);
    auto filename = vfs.filename().string();
    auto extension = ".*," + vfs.extension().string();
    IGFD::FileDialogConfig config;
    config.path = filename.c_str();
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", extension.c_str(), config);
  }
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
  {
    if (ImGuiFileDialog::Instance()->IsOk())
    {
      std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

      copy_to_file(filePath);
    }

    ImGuiFileDialog::Instance()->Close();
  }
  return true;
}

wgrd_files::File::File(FileMeta meta, fs::path out_path)
: out_path(out_path) {
  vfs_path = meta.vfs_path;
  file.open(meta.fs_path, std::ios::in | std::ios::binary);
  offset = meta.offset;
  size = meta.size;
  fs_path = meta.fs_path;
}

std::vector<char> File::get_file() {
  size_t old = file.tellg();
  file.seekg(offset);
  std::vector<char> ret;
  ret.resize(size);
  file.read(reinterpret_cast<char*>(ret.data()), size);
  file.seekg(old);
  return ret;
}

std::vector<char> File::get_data() {
  std::vector<char> ret;
  ret.resize(size);

  size_t old = file.tellg();
  file.seekg(offset);

  file.read(ret.data(), size);

  file.seekg(old);
  return std::move(ret);
}

bool File::copy_to_file(std::filesystem::path path) {
  fs::create_directories(path.parent_path());

  std::ofstream of;
  of.open(path, std::ios::out | std::ios::binary | std::ios::trunc);

  if(!of.is_open()) {
    return false;
  }

  size_t old = file.tellg();
  file.seekg(offset);
  size_t end = offset+size;

  while(!file.eof() && file.tellg() < end) {
    char buffer[1024];
    size_t count = std::min(sizeof(buffer), end-file.tellg());
    file.read(buffer, count);
    of.write(buffer, count);
  }

  if(file.tellg() < offset+size) {
    file.seekg(old);
    return false;
  }

  file.seekg(old);
  return true;
}

void wgrd_files::Files::render() {
  for(auto &[idx, file] : files) {
    if(file->window_opened) {
      file->render();
    }
  }
}

void wgrd_files::Files::open_window(FileMeta meta) {
  auto file = files.find(meta.idx);
  if(file == files.end()) {
    spdlog::error("Trying to open window for not loaded file");
    return;
  }
  file->second->window_opened = true;
}

void wgrd_files::Files::add_file(fs::path out_path, FileMeta meta, size_t file_offset) {
  if(files.find(meta.idx) != files.end()) {
    return;
  }
  std::ifstream f(meta.fs_path, std::ios::in | std::ios::binary);
  meta.offset += file_offset;

  if(Dic::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<Dic>(meta, out_path);
  } else
  if(EDat::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<EDat>(meta, out_path);
  } else
  if(Ess::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<Ess>(meta, out_path);
  } else
  if(SFormat::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<SFormat>(meta, out_path);
  } else
  if(TGV::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<TGV>(meta, out_path);
  } else
  if(PPK::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<PPK>(meta, out_path);
  } else
  if(Scenario::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<Scenario>(meta, out_path);
  } else
  if(NdfBin::is_file(meta.vfs_path, f, meta.offset)) {
    files[meta.idx] = std::make_unique<NdfBin>(meta, out_path);
  } else {
    files[meta.idx] = std::make_unique<File>(meta, out_path);
  }
  // if a xml file already exists, load the file from it instead of the dat file
  std::string new_ext = fs::path(meta.vfs_path).extension();
  new_ext += ".xml";
  fs::path xml_path = out_path / "xml" / fs::path(meta.vfs_path).replace_extension(new_ext);
  files[meta.idx]->xml_path = xml_path;
  if(fs::exists(xml_path)) {
    files[meta.idx]->load_xml(xml_path);
  } else {
    spdlog::info("No ndf xml file found at {}", xml_path.string());
  }
}

void wgrd_files::Files::copy_bin_changes(fs::path dat_path, fs::path out_folder_path) {
  for(auto &[idx, file] : files) {
    if(file->is_changed()) {
      if(file->fs_path == dat_path) {
        // vfs_path contains $/ at the beginning, drop it
        fs::path bin_path = out_folder_path / remove_dollar(file->vfs_path);
        spdlog::info("Saving binary for {} to {}", file->vfs_path, bin_path.string());
        file->save_bin(bin_path);
      }
    }
  }
}
