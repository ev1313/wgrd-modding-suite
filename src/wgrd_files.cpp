#include "wgrd_files.h"

#include <algorithm>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <utility>

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

wgrd_files::File::File(FileMeta meta) : meta(std::move(meta)) {}

void wgrd_files::File::render_shortcuts() {
  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_R)) {
    start_parsing();
  }
  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S)) {
    save_xml(xml_path);
  }
}

void wgrd_files::File::render_menu() {
  if (ImGui::BeginMenu(gettext("File"))) {
    if (ImGui::MenuItem(gettext("Rebuild"), gettext("Ctrl+R"))) {
      start_parsing();
    }
    if (ImGui::MenuItem(gettext("Save XML"), gettext("Ctrl+S"))) {
      save_xml(xml_path);
    }
    if (ImGui::MenuItem(gettext("Save Bin"))) {
      save_bin(bin_path);
    }
    if (ImGui::MenuItem(gettext("Save XML to..."))) {
    }
    if (ImGui::MenuItem(gettext("Save Bin to..."))) {
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(gettext("Edit"))) {
    if (ImGui::MenuItem(gettext("Undo"), gettext("Ctrl+Z"))) {
    }
    if (ImGui::MenuItem(gettext("Redo"), gettext("Ctrl+Y"))) {
    }
    if (ImGui::MenuItem(gettext("Transaction Log"))) {
    }
    ImGui::EndMenu();
  }
}

void wgrd_files::File::render_window() {
  ImGui::Text("File: %s", meta.vfs_path.c_str());
  if (ImGui::Button("save file")) {
    std::filesystem::path vfs(meta.vfs_path);
    auto filename = vfs.filename().string();
    auto extension = ".*," + vfs.extension().string();
    IGFD::FileDialogConfig config;
    config.path = filename.c_str();
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File",
                                            extension.c_str(), config);
  }
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

      copy_to_file(filePath);
    }

    ImGuiFileDialog::Instance()->Close();
  }
}

std::vector<char> File::get_data() {
  std::vector<char> ret;
  ret.resize(meta.size);

  size_t old = meta.stream->tellg();
  meta.stream->seekg(meta.offset);

  meta.stream->read(ret.data(), meta.size);

  meta.stream->seekg(old);
  return std::move(ret);
}

void File::start_parsing(bool try_xml) {
  if (is_parsing()) {
    spdlog::error("Already parsing {}", meta.vfs_path);
    return;
  }
  m_is_parsing = true;
  m_is_parsed = false;

  m_parsed_promise = std::promise<bool>();
  m_parsed_future = m_parsed_promise->get_future();

  std::thread([this, try_xml]() {
    bool ret = false;
    if (try_xml) {
      ret = load_xml(xml_path);
    }
    if (!ret) {
      ret = load_stream();
      if (ret) {
        save_xml(xml_path);
      }
    }
    m_parsed_promise->set_value_at_thread_exit(ret);
  }).detach();
}

bool File::copy_to_file(std::filesystem::path path) {
  fs::create_directories(path.parent_path());

  std::ofstream of;
  of.open(path, std::ios::out | std::ios::binary | std::ios::trunc);

  if (!of.is_open()) {
    return false;
  }

  size_t old = meta.stream->tellg();
  meta.stream->seekg(meta.offset);
  size_t end = meta.offset + meta.size;

  while (!meta.stream->eof() && meta.stream->tellg() < end) {
    char buffer[1024];
    size_t count = std::min(sizeof(buffer), end - meta.stream->tellg());
    meta.stream->read(buffer, count);
    of.write(buffer, count);
  }

  if (meta.stream->tellg() < end) {
    meta.stream->seekg(old);
    return false;
  }

  meta.stream->seekg(old);
  return true;
}

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
      file = std::make_unique<Dic>(std::move(meta));
    } else if (EDat::is_file(meta)) {
      file = std::make_unique<EDat>(std::move(meta));
    } else if (Ess::is_file(meta)) {
      file = std::make_unique<Ess>(std::move(meta));
    } else if (SFormat::is_file(meta)) {
      file = std::make_unique<SFormat>(std::move(meta));
    } else if (TGV::is_file(meta)) {
      file = std::make_unique<TGV>(std::move(meta));
    } else if (PPK::is_file(meta)) {
      file = std::make_unique<PPK>(std::move(meta));
    } else if (Scenario::is_file(meta)) {
      file = std::make_unique<Scenario>(std::move(meta));
    } else if (NdfBin::is_file(meta)) {
      file = std::make_unique<NdfBin>(std::move(meta));
    } else {
      file = std::make_unique<File>(std::move(meta));
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
