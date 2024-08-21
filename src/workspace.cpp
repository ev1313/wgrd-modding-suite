#include "workspace.hpp"

#include "files/file.hpp"
#include "helpers.hpp"
#include "threadpool.hpp"
#include <algorithm>
#include <filesystem>
#include <imgui.h>

#include "imgui_helpers.hpp"
#include "imgui_stdlib.h"

#include <memory>
#include <toml.hpp>

#include <ranges>

#include <libintl.h>

std::optional<std::unique_ptr<Workspace>>
Workspace::render_init_workspace(bool *show_workspace) {
  std::optional<std::unique_ptr<Workspace>> ret = std::nullopt;
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
  auto dat_path_ret = show_file_dialog_input(gettext("Path to dat files"), "",
                                             "dat_path_dialog");
  dat_path = dat_path_ret.value_or(dat_path);

  ImGui::Text(gettext("Path to output folder: "));
  ImGui::SameLine();
  static std::string out_path = "out/";
  ImGui::InputText("##OutFilePath", &out_path);
  ImGui::SameLine();
  auto out_path_ret = show_file_dialog_input(
      gettext("Path to output directory"), "", "out_path_dialog");
  out_path = out_path_ret.value_or(out_path);

  if (ImGui::Button(gettext("Load workspace"))) {
    std::unique_ptr<Workspace> w = std::make_unique<Workspace>();
    w->workspace_name = name;
    if (w->init(dat_path, out_path)) {
      ret = std::move(w);
    }
  }

  ImGui::End();

  return ret;
}

bool Workspace::check_directories(fs::path fs_path, fs::path dat_path,
                                  fs::path bin_path, fs::path xml_path,
                                  fs::path tmp_path) {
  if (!fs::exists(fs_path)) {
    spdlog::warn("fs_path does not exist {}", fs_path.string());
    return false;
  }
  fs::create_directories(dat_path);
  if (!fs::is_directory(dat_path)) {
    spdlog::warn("could not create directories {}", (dat_path).string());
    return false;
  }
  fs::create_directories(bin_path);
  if (!fs::is_directory(bin_path)) {
    spdlog::warn("could not create directories {}", (bin_path).string());
    return false;
  }
  fs::create_directories(xml_path);
  if (!fs::is_directory(xml_path)) {
    spdlog::warn("could not create directories {}", (xml_path).string());
    return false;
  }
  fs::create_directories(tmp_path);
  if (!fs::is_directory(tmp_path)) {
    spdlog::warn("could not create directories {}", (tmp_path).string());
    return false;
  }
  m_config.fs_path = fs_path;
  m_config.dat_path = dat_path;
  m_config.bin_path = bin_path;
  m_config.xml_path = xml_path;
  m_config.tmp_path = tmp_path;
  return true;
}

bool Workspace::init(fs::path fs_path, fs::path out_path) {
  return init(fs_path, fs_path, out_path / "bin", out_path / "xml",
              out_path / "tmp");
}

bool Workspace::init(const WorkspaceConfig &config) {
  return init(config.fs_path, config.dat_path, config.bin_path, config.xml_path,
              config.tmp_path);
}

bool Workspace::init(fs::path fs_path, fs::path dat_path, fs::path bin_path,
                     fs::path xml_path, fs::path tmp_path) {
  if (!check_directories(fs_path, dat_path, bin_path, xml_path, tmp_path)) {
    return false;
  }

  m_is_parsing = true;
  m_parsed_promise = std::promise<bool>();
  m_parsed_future = m_parsed_promise->get_future();

  ThreadPoolSingleton::get_instance().submit([this, dat_path]() {
    try {
      file_tree.init_from_dat_path(dat_path);
      m_parsed_promise->set_value(true);
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse workspace: {}", e.what());
      m_parsed_promise->set_value(false);
    }
  });

  m_config.name = workspace_name;
  assert(!workspace_name.empty());

  return true;
}

bool Workspace::init_from_file(fs::path file_path, fs::path out_path) {
  return init_from_file(file_path, out_path / "dat", out_path / "bin",
                        out_path / "xml", out_path / "tmp");
}

bool Workspace::init_from_file(const WorkspaceConfig &config) {
  return init_from_file(config.fs_path, config.dat_path, config.bin_path,
                        config.xml_path, config.tmp_path);
}

bool Workspace::init_from_file(fs::path file_path, fs::path dat_path,
                               fs::path bin_path, fs::path xml_path,
                               fs::path tmp_path) {
  spdlog::info("Loading workspace from file {}", file_path.string());
  if (!check_directories(file_path, dat_path, bin_path, xml_path, tmp_path)) {
    return false;
  }

  m_is_parsing = true;
  m_parsed_promise = std::promise<bool>();
  m_parsed_future = m_parsed_promise->get_future();

  ThreadPoolSingleton::get_instance().submit([this, file_path]() {
    try {
      bool ret = file_tree.init_from_path(file_path);
      m_parsed_promise->set_value(ret);
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse workspace: {}", e.what());
      m_parsed_promise->set_value(false);
    }
  });
  return true;
}

void Workspace::render_window() {
  auto file_metas = file_tree.render();
  if (file_metas) {
    if (!file_metas->size()) {
      spdlog::error("file_tree returned empty file_meta list!");
      return;
    }
    std::string vfs_path = file_metas.value()[0].vfs_path;
    files.add_file(std::move(file_metas.value()));
    files.open_window(vfs_path);
  }
}

void Workspace::render_extra() { files.render(); }

void Workspace::save_changes_to_dat(bool save_to_fs_path) {
  files.save_changes_to_dat(save_to_fs_path);
}

bool Workspace::is_changed() { return files.is_changed(); }

void Workspace::check_parsing() {
  // if no parse ever started, the future/promise may not exist yet
  if (m_is_parsed) {
    return;
  }
  if (!m_is_parsing) {
    return;
  }
  // but if it started they have to exist
  if (!m_parsed_promise.has_value()) {
    throw std::runtime_error("m_parsed_promise not set");
  }
  if (!m_parsed_future.has_value()) {
    throw std::runtime_error("m_parsed_future not set");
  }
  // check if thread is already done
  if (m_parsed_future.value().wait_for(std::chrono::seconds(0)) ==
      std::future_status::ready) {
    m_is_parsed = m_parsed_future.value().get();
    if (!m_is_parsed) {
      spdlog::error("Failed to parse workspace: {}", workspace_name);
    }
    m_is_parsing = false;
  }
}

bool Workspace::is_parsed() { return m_is_parsed; }

bool Workspace::is_parsing() { return m_is_parsing; }

void Workspaces::render() {
  for (auto &[workspace_name, p_open] : open_workspace_windows) {
    if (!p_open) {
      continue;
    }
    if (!workspaces.contains(workspace_name)) {
      spdlog::error("Workspace {} does not exist", workspace_name);
      continue;
    }
    auto &workspace = workspaces[workspace_name];
    ImGuiWindowFlags wndflags = ImGuiWindowFlags_None;
    if (workspace->is_changed()) {
      wndflags |= ImGuiWindowFlags_UnsavedDocument;
    }
    ImGui::SetNextWindowSize(ImVec2(800, 800), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(workspace->workspace_name.c_str(), &p_open)) {
      workspace->check_parsing();
      if (!workspace->is_parsed()) {
        if (!workspace->is_parsing()) {
          spdlog::error(
              "Workspace is not loaded and probably failed loading! {}",
              workspace->workspace_name);
        } else {
          ImGui::Text("Parsing %s", workspace->workspace_name.c_str());
        }
      } else {
        workspace->render_window();
      }
    }
    ImGui::End();
    if (workspace->is_parsed()) {
      workspace->render_extra();
    }
  }
  if (show_add_workspace) {
    auto workspace = Workspace::render_init_workspace(&show_add_workspace);
    if (workspace) {
      add_workspace(std::move(workspace.value()));
      show_add_workspace = false;
    }
  }
}

void Workspaces::render_menu() {
  if (ImGui::BeginMenu(gettext("Workspaces"))) {
    if (open_workspace_windows.empty()) {
      ImGui::MenuItem(gettext("No workspaces loaded"), nullptr, nullptr, false);
    }
    assert(open_workspace_windows.size() == workspaces.size());
    for (auto &[workspace_name, p_open] : open_workspace_windows) {
      ImGui::MenuItem(workspace_name.c_str(), nullptr, &p_open);
    }
    ImGui::EndMenu();
  }
}

void Workspaces::add_workspace(std::unique_ptr<Workspace> w) {
  spdlog::info("Loading workspace {} from {} xml_path {}", w->workspace_name,
               w->m_config.fs_path.string(), w->m_config.xml_path.string());
  open_workspace_windows.insert({w->workspace_name, true});
  this->workspaces.insert({w->workspace_name, std::move(w)});
}

void Workspaces::save_project_file(fs::path path) {
  toml::array arr;
  for (auto &[_, workspace] : workspaces) {
    auto data = workspace->m_config.to_toml();
    arr.push_back(data);
  }
  toml::table table;
  table["workspaces"] = arr;
  std::fstream out(path, std::ios::out);
  out << toml::format(toml::value(table)) << std::endl;
}

void Workspaces::load_project_file(fs::path path) {
  if (fs::exists(path)) {
    auto data = toml::parse(path.string());
    auto workspaces = data["workspaces"].as_array();
    for (auto &workspace : workspaces) {
      toml::table t = workspace.as_table();
      std::unique_ptr<Workspace> w = std::make_unique<Workspace>();
      WorkspaceConfig config;
      if (!config.from_toml(t)) {
        spdlog::error("Failed to load workspace config from toml!");
        return;
      }
      w->workspace_name = config.name;
      w->init(config);
      add_workspace(std::move(w));
    }
  } else {
    spdlog::info("Could not load project file from {}", path.string());
  }
}

void Workspaces::save_workspaces(bool save_to_fs_path) {
  for (auto &[_, workspace] : workspaces) {
    if (!workspace->is_parsed() || workspace->is_parsing()) {
      continue;
    }
    workspace->save_changes_to_dat(save_to_fs_path);
  }
}
