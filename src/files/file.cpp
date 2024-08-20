#include "file.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "ImGuiFileDialog.h"

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
