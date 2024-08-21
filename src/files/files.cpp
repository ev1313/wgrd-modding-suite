#include "files.hpp"

#include <algorithm>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <utility>

#include "ImGuiFileDialog.h"

#include <libintl.h>

#include "helpers.hpp"

#include <filesystem>

#include "file.hpp"
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

void wgrd_files::Files::render_menu(const std::unique_ptr<File> &file) {
  if (ImGui::BeginMenuBar()) {
    file->render_menu();
    ImGui::EndMenuBar();
  }
}

void wgrd_files::Files::render() {
  for (auto &[vfs_path, p_open] : open_file_windows) {
    if (!p_open) {
      continue;
    }
    if (!files.contains(vfs_path)) {
      spdlog::error("Trying to render not existing file {}", vfs_path);
      continue;
    }
    assert(files.contains(vfs_path));
    auto &[file_list, idx] = files[vfs_path];
    assert(idx <= file_list.size());
    auto &file = file_list[idx];
    if (ImGui::Begin(file->meta.vfs_path.c_str(), &p_open,
                     ImGuiWindowFlags_MenuBar)) {
      file->render_shortcuts();
      render_menu(file);
      file->check_parsing();
      if (!file->is_parsed()) {
        if (!file->is_parsing()) {
          ImGui::Text("Failed to parse %s", file->meta.vfs_path.c_str());
        } else {
          ImGui::Text("Parsing %s", file->meta.vfs_path.c_str());
        }
      } else {
        file->render_window();
      }
    }
    ImGui::End();

    if (file->is_parsed()) {
      file->render_extra();
    }
  }
}

void wgrd_files::Files::open_window(std::string vfs_path) {
  open_file_windows[vfs_path] = true;
}

void wgrd_files::Files::add_file(FileMetaList file_metas) {
  if (file_metas.size() == 0) {
    spdlog::error("Files::add_file called with empty file_metas");
  }
  std::string vfs_path = file_metas[0].vfs_path;
  for (auto &meta : file_metas) {
    assert(meta.vfs_path == vfs_path);
  }
  if (files.contains(vfs_path)) {
    spdlog::error("Files::add_file vfs_path {} already exists", vfs_path);
    return;
  }

  FileList file_list;
  for (auto &meta : file_metas) {
    meta.stream = std::make_unique<std::ifstream>(
        meta.fs_path, std::ios::in | std::ios::binary);

    std::unique_ptr<File> file;
    fs::path vfs_path = remove_dollar(meta.vfs_path);
    if (Dic::is_file(meta)) {
      file = std::make_unique<Dic>(this, std::move(meta));
    } else if (EDat::is_file(meta)) {
      file = std::make_unique<EDat>(this, std::move(meta));
    } else if (Ess::is_file(meta)) {
      file = std::make_unique<Ess>(this, std::move(meta));
    } else if (SFormat::is_file(meta)) {
      file = std::make_unique<SFormat>(this, std::move(meta));
    } else if (TGV::is_file(meta)) {
      file = std::make_unique<TGV>(this, std::move(meta));
    } else if (PPK::is_file(meta)) {
      file = std::make_unique<PPK>(this, std::move(meta));
    } else if (Scenario::is_file(meta)) {
      file = std::make_unique<Scenario>(this, std::move(meta));
    } else if (NdfBin::is_file(meta)) {
      file = std::make_unique<NdfBin>(this, std::move(meta));
    } else {
      file = std::make_unique<File>(this, std::move(meta));
    }
    file->bin_path = m_config.bin_path / vfs_path;
    file->tmp_path = m_config.tmp_path / vfs_path;
    std::string new_ext = vfs_path.extension().string() + ".xml";
    file->xml_path = m_config.xml_path / vfs_path.replace_extension(new_ext);
    file_list.push_back(std::move(file));
  }
  file_list.back()->start_parsing();
  size_t len = file_list.size() - 1;
  files.insert({vfs_path, {std::move(file_list), len}});
}

void wgrd_files::Files::copy_bin_changes(fs::path fs_path,
                                         fs::path out_folder_path) {
  for (auto &[vfs_path, files_idx] : files) {
    auto &[file_list, idx] = files_idx;
    auto &file = file_list[idx];
    if (file->is_changed()) {
      if (file->meta.fs_path == fs_path) {
        // vfs_path contains $/ at the beginning, drop it
        fs::path bin_path =
            out_folder_path / remove_dollar(file->meta.vfs_path);
        spdlog::info("Saving binary for {} to {}", file->meta.vfs_path,
                     bin_path.string());
        file->save_bin(bin_path);
      }
    }
  }
}

void wgrd_files::Files::save_changes_to_dat(bool save_to_fs_path) {
  fs::path parent_out_path = m_config.dat_path;
  if (save_to_fs_path) {
    parent_out_path = m_config.fs_path.parent_path();
  }

  spdlog::info("Saving changes to dat files with tmp_path {}",
               m_config.tmp_path.string());
  // get all changed dat files
  std::unordered_set<std::string> changed_files_paths;
  for (const auto &[_, files_idx] : files) {
    const auto &[file_list, idx] = files_idx;
    const auto &file = file_list[idx];
    if (file->is_changed()) {
      changed_files_paths.insert(file->meta.fs_path.string());
    }
  }
  // iterate all changed dat files in this workspace
  for (const auto &fs_path_str : changed_files_paths) {
    fs::path fs_path = fs_path_str;
    fs::path part_path = fs::relative(fs_path, m_config.fs_path);
    fs::path out_path = parent_out_path / part_path.parent_path();
    if (save_to_fs_path) {
      assert(out_path == fs_path.parent_path());
    }
    fs::create_directories(out_path);

    // unpack dat file to tmp directory
    fs::create_directories(m_config.tmp_path);

    spdlog::info("Saving changes in {} to {}", m_config.fs_path.string(),
                 out_path.string());
    {
      py::gil_scoped_acquire acquire;
      try {
        py::object edat = py::module::import("wgrd_cons_parsers")
                              .attr("edat")
                              .attr("EdatMain")();
        // as the EdatMain uses the . instead of [] for accessing, we need the
        // dingsda Container
        py::object container =
            py::module::import("dingsda.lib.containers").attr("Container");
        py::dict args = py::dict();
        args["no_alignment"] = true;
        args["disable_checksums"] = true;
        edat.attr("args") = container(args);
        py::object data = edat.attr("get_data")(m_config.fs_path.string());
        edat.attr("unpack")(m_config.fs_path.string(),
                            m_config.tmp_path.string(), data);
      } catch (const py::error_already_set &e) {
        spdlog::error(e.what());
      }
    }

    copy_bin_changes(m_config.fs_path.string(), m_config.tmp_path / "out");

    // since now all changed binary files are in the directory, rebuild the dat
    // file
    {
      py::gil_scoped_acquire acquire;
      try {
        py::object edat = py::module::import("wgrd_cons_parsers")
                              .attr("edat")
                              .attr("EdatMain")();
        py::object container =
            py::module::import("dingsda.lib.containers").attr("Container");
        py::dict args = py::dict();
        args["no_alignment"] = true;
        args["disable_checksums"] = true;
        edat.attr("args") = container(args);
        fs::path gen_xml_path =
            m_config.tmp_path /
            m_config.fs_path.filename().replace_extension(".dat.xml");
        py::object data = edat.attr("get_data")(gen_xml_path.string());
        edat.attr("pack")(gen_xml_path.string(), out_path.string(), data);
      } catch (const py::error_already_set &e) {
        spdlog::error(e.what());
      }
    }

    // remove the tmp directory
    fs::remove_all(m_config.tmp_path);
  }
}

File *Files::get_file(std::string vfs_path) {
  auto file_it = files.find(vfs_path);
  if (file_it == files.end()) {
    return nullptr;
  }
  auto &[file_list, _] = file_it->second;
  assert(file_list.size() > 0);
  return file_list.back().get();
}

std::vector<std::string> Files::get_files_of_type(FileType type) {
  std::vector<std::string> ret;
  for (const auto &[vfs_path, file_idx] : files) {
    const auto &[files, _] = file_idx;
    auto &file = files.back();

    if (file->get_type() == type) {
      ret.push_back(vfs_path);
    }
  }
  return ret;
}

bool Files::is_changed() {
  for (const auto &[_, file_idx] : files) {
    const auto &[files, _] = file_idx;
    for (const auto &file : files) {
      if (file->is_changed()) {
        return true;
      }
    }
  }
  return false;
}
