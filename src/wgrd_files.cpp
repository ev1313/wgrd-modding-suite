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

bool wgrd_files::File::imgui_call() {
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

wgrd_files::File::File(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path)
: vfs_path(std::move(vfs_path)), file(std::move(f)), offset(offset), size(size), out_path(out_path) {
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

void wgrd_files::Files::imgui_call() {
  for(auto &[idx, file] : files) {
    if(file->window_opened) {
      file->imgui_call();
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
  auto vfs_path = meta.vfs_path;
  size_t offset = file_offset + meta.offset;
  auto size = meta.size;

  if(EDat::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<EDat>(vfs_path, f, offset, size, out_path);
  } else
  if(Ess::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<Ess>(vfs_path, f, offset, size, out_path);
  } else
  if(SFormat::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<SFormat>(vfs_path, f, offset, size, out_path);
  } else
  if(TGV::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<TGV>(vfs_path, f, offset, size, out_path);
  } else
  if(PPK::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<PPK>(vfs_path, f, offset, size, out_path);
  } else
  if(Scenario::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<Scenario>(vfs_path, f, offset, size, out_path);
  } else
  if(NdfBin::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<NdfBin>(vfs_path, f, offset, size, out_path);
  } else {
    files[meta.idx] = std::make_unique<File>(vfs_path, f, offset, size, out_path);
  }
  // FIXME: just change all constructors to take FileMeta instead at this point
  files[meta.idx]->fs_path = meta.fs_path;
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
