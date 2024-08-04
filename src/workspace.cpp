#include "workspace.hpp"

#include <filesystem>
#include <imgui.h>
#include "imgui_stdlib.h"
#include <ImGuiFileDialog.h>

#include <toml.hpp>

#include <libintl.h>

std::optional<std::string> show_file_dialog(std::string title,
                                            std::string previous_path,
                                            std::string dialogkey) {
  std::optional<std::string> ret = std::nullopt;
  std::string button_text = gettext("Open File Dialog##") + dialogkey;
  if (ImGui::Button(button_text.c_str())) {
    IGFD::FileDialogConfig config;
    config.path = previous_path;
    ImGuiFileDialog::Instance()->OpenDialog(dialogkey, title, nullptr, config);
  }

  if (ImGuiFileDialog::Instance()->Display(
    dialogkey, ImGuiWindowFlags_NoCollapse, ImVec2(800, 400))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      ret = ImGuiFileDialog::Instance()->GetCurrentPath();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  return ret;
}

std::optional<Workspace> Workspace::render_init_workspace(bool* show_workspace) {
  std::optional<Workspace> ret = std::nullopt;
  ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
  ImGui::Begin("Add a workspace", show_workspace);

  ImGui::Text(gettext("Workspace name: "));
  ImGui::SameLine();
  static std::string name = "Workspace";
  ImGui::InputText("##WorkspaceName", &name);

  ImGui::Text(gettext("Path to dat files: "));
  ImGui::SameLine();
  static std::string dat_path = ".";
  ImGui::InputText("##DatFilePath", &dat_path);
  ImGui::SameLine();
  auto dat_path_ret = show_file_dialog(gettext("Path to dat files"), "", "dat_path_dialog");
  dat_path = dat_path_ret.value_or(dat_path);
  
  ImGui::Text(gettext("Path to output folder: "));
  ImGui::SameLine();
  static std::string out_path = "out/";
  ImGui::InputText("##OutFilePath", &out_path);
  ImGui::SameLine();
  auto out_path_ret = show_file_dialog(gettext("Path to output directory"), "", "out_path_dialog");
  out_path = out_path_ret.value_or(out_path);

  if(ImGui::Button(gettext("Load workspace"))) {
    Workspace w;
    w.workspace_name = name;
    if(w.init(dat_path, out_path)) {
      ret = std::move(w);
    }
  }

  ImGui::End();

  return ret;
}

bool Workspace::init(fs::path dat_path, fs::path out_path) {
  if(!fs::exists(dat_path)) {
    spdlog::warn("dat_path does not exist {}", dat_path.string());
    return false;
  }
  fs::create_directories(out_path / "xml");
  if(!fs::is_directory(out_path / "xml")) {
    spdlog::warn("could not create directories {}", (out_path / "xml").string());
    return false;
  }
  fs::create_directories(out_path / "bin");
  if(!fs::is_directory(out_path / "bin")) {
    spdlog::warn("could not create directories {}", (out_path / "bin").string());
    return false;
  }
  fs::create_directories(out_path / "dat");
  if(!fs::is_directory(out_path / "dat")) {
    spdlog::warn("could not create directories {}", (out_path / "dat").string());
    return false;
  }
  
  file_tree.init_from_dat_path(dat_path);
  workspace_dat_path = dat_path;
  workspace_out_path = out_path;
  return true;
}

void Workspace::render() {
  auto meta = file_tree.render();
  if(meta) {
    files.add_file(workspace_out_path, meta.value());
  }
  ImGui::SameLine();
  files.imgui_call(meta);
}

toml::table Workspace::to_toml() {
  toml::table table;
  table["name"] = workspace_name;
  table["dat_path"] = workspace_dat_path.string();
  table["out_path"] = workspace_out_path.string();
  return table;
}

void Workspaces::render() {
  size_t idx = 0;
  for(auto& workspace : workspaces) {
    std::string name = std::format("{}##WP_{}", workspace.workspace_name, idx++);
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(name.c_str(), nullptr)) {
      workspace.render();
      ImGui::End();
    } 
  }
  if(show_add_workspace) {
    auto workspace = Workspace::render_init_workspace(&show_add_workspace);
    if(workspace) {
      workspaces.push_back(std::move(workspace.value()));
      show_add_workspace = false;
    }
  }
}

void Workspaces::save_workspaces(fs::path path) {
  toml::array arr;
  for(auto& workspace : workspaces) {
    auto data = workspace.to_toml();
    arr.push_back(data);
  }
  toml::table table;
  table["workspaces"] = arr;
  std::fstream out(path, std::ios::out);
  out << toml::format(toml::value(table)) << std::endl;
}

void Workspaces::load_workspaces(fs::path path) {
  if(fs::exists(path)) {
    auto data = toml::parse(path.string());
    auto workspaces = data["workspaces"].as_array();
    for(auto& workspace : workspaces) {
      Workspace w;
      w.workspace_name = workspace["name"].as_string();
      w.init(workspace["dat_path"].as_string(), workspace["out_path"].as_string());
      this->workspaces.push_back(std::move(w));
    }
  }
}
