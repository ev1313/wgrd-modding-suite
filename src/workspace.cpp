#include "workspace.hpp"

#include <filesystem>
#include <imgui.h>
#include "imgui_stdlib.h"
#include <ImGuiFileDialog.h>

#include <libintl.h>

std::optional<std::string> show_file_dialog(std::string title,
                                            std::string previous_path,
                                            std::string dialogkey) {
  std::optional<std::string> ret = std::nullopt;
  if (ImGui::Button(gettext("Open File Dialog"))) {
    IGFD::FileDialogConfig config;
    config.path = previous_path;
    ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", title, nullptr, config);
  }

  if (ImGuiFileDialog::Instance()->Display(
    "ChooseDirDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(800, 400))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      ret = ImGuiFileDialog::Instance()->GetCurrentPath();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  return ret;
}

std::optional<Workspace> Workspace::render_init_workspace() {
  std::optional<Workspace> ret = std::nullopt;
  ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
  ImGui::Begin("Add a workspace", nullptr);

  ImGui::Text(gettext("Workspace name: "));
  ImGui::SameLine();
  static std::string name = "Workspace";
  ImGui::InputText("##WorkspaceName", &name);

  static std::string dat_path = ".";
  ImGui::InputText("##DatFilePath", &dat_path);
  ImGui::SameLine();
  auto dat_path_ret = show_file_dialog(gettext("Path to dat files"), "", "dat_path_dialog");
  dat_path = dat_path_ret.value_or(dat_path);
  
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
