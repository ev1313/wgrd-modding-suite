#include "workspace.hpp"

#include <algorithm>
#include <filesystem>
#include <imgui.h>
#include "helpers.hpp"
#include "imgui_stdlib.h"

#include "imgui_helpers.hpp"

#include <memory>
#include <toml.hpp>

#include <ranges>

#include <libintl.h>

std::optional<std::unique_ptr<Workspace>> Workspace::render_init_workspace(bool* show_workspace) {
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
  auto dat_path_ret = show_file_dialog_input(gettext("Path to dat files"), "", "dat_path_dialog");
  dat_path = dat_path_ret.value_or(dat_path);

  ImGui::Text(gettext("Path to output folder: "));
  ImGui::SameLine();
  static std::string out_path = "out/";
  ImGui::InputText("##OutFilePath", &out_path);
  ImGui::SameLine();
  auto out_path_ret = show_file_dialog_input(gettext("Path to output directory"), "", "out_path_dialog");
  out_path = out_path_ret.value_or(out_path);

  if(ImGui::Button(gettext("Load workspace"))) {
    std::unique_ptr<Workspace> w = std::make_unique<Workspace>();
    w->workspace_name = name;
    if(w->init(dat_path, out_path)) {
      ret = std::move(w);
    }
  }

  ImGui::End();

  return ret;
}

bool Workspace::check_directories(fs::path dat_path, fs::path out_path) {
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
  return true;
}

bool Workspace::init(fs::path dat_path, fs::path out_path) {
  if(!check_directories(dat_path, out_path)) {
    return false;
  }
  workspace_dat_path = dat_path;
  workspace_out_path = out_path;
  
  m_is_parsing = true;
  m_parsed_promise = std::promise<bool>();
  m_parsed_future = m_parsed_promise->get_future();

  std::thread([this, dat_path]() {
    try{
      file_tree.init_from_dat_path(dat_path);
      m_parsed_promise->set_value_at_thread_exit(true);
    } catch (const std::exception& e) {
      spdlog::error("Failed to parse workspace: {}", e.what());
      m_parsed_promise->set_value_at_thread_exit(false);
    }
  }).detach();
  return true;
}

bool Workspace::init_from_file(fs::path file_path, fs::path out_path) {
  spdlog::info("Loading workspace from file {} {}", file_path.string(), out_path.string());
  if(!check_directories(file_path, out_path)) {
    return false;
  }

  workspace_dat_path = file_path.parent_path();
  workspace_out_path = out_path;

  m_is_parsing = true;
  m_parsed_promise = std::promise<bool>();
  m_parsed_future = m_parsed_promise->get_future();

  std::thread([this, file_path]() {
    try{
      bool ret = file_tree.init_from_path(file_path);
      m_parsed_promise->set_value_at_thread_exit(ret);
    } catch (const std::exception& e) {
      spdlog::error("Failed to parse workspace: {}", e.what());
      m_parsed_promise->set_value_at_thread_exit(false);
    }
  }).detach();
  return true;
}

void Workspace::render_window() {
  auto meta = file_tree.render();
  if(meta) {
    files.add_file(workspace_out_path, meta.value());
    loaded_file_paths.insert(meta.value().fs_path);
    files.open_window(meta.value());
  }
}

void Workspace::render_extra() {
  files.render();
}

toml::table Workspace::to_toml() {
  toml::table table;
  table["name"] = workspace_name;
  table["dat_path"] = workspace_dat_path.string();
  table["out_path"] = workspace_out_path.string();
  return table;
}

void Workspace::save_changes_to_dat(bool save_to_fs_path) {
  fs::path out_path = workspace_out_path / "dat";
  if(save_to_fs_path) {
    out_path = workspace_dat_path.parent_path();
  }
  // iterate all ever opened dat files in this workspace
  for(fs::path dat_path : loaded_file_paths) {
    fs::path part_path = fs::relative(dat_path, workspace_dat_path);
    out_path = out_path / part_path.parent_path();
    if(save_to_fs_path) {
      assert(out_path == dat_path.parent_path());
    }
    fs::create_directories(out_path);

    spdlog::info("Saving changes in {} to {}", dat_path.string(), out_path.string());

    // unpack dat file to tmp directory
    fs::path tmp_dir = workspace_out_path / "tmp";
    fs::create_directories(tmp_dir);
    {
      py::gil_scoped_acquire acquire;
      try {
        py::object edat = py::module::import("wgrd_cons_parsers").attr("edat").attr("EdatMain")();
        // as the EdatMain uses the . instead of [] for accessing, we need the dingsda Container
        py::object container = py::module::import("dingsda.lib.containers").attr("Container");
        py::dict args = py::dict();
        args["no_alignment"] = true;
        args["disable_checksums"] = true;
        edat.attr("args") = container(args);
        py::object data = edat.attr("get_data")(dat_path.string());
        edat.attr("unpack")(dat_path.string(), tmp_dir.string(), data);
      } catch(const py::error_already_set& e) {
        spdlog::error(e.what());
      }
    }

    files.copy_bin_changes(dat_path.string(), tmp_dir / "out");

    // since now all changed binary files are in the directory, rebuild the dat file
    {
      py::gil_scoped_acquire acquire;
      try {
        py::object edat = py::module::import("wgrd_cons_parsers").attr("edat").attr("EdatMain")();
        py::object container = py::module::import("dingsda.lib.containers").attr("Container");
        py::dict args = py::dict();
        args["no_alignment"] = true;
        args["disable_checksums"] = true;
        edat.attr("args") = container(args);
        fs::path gen_xml_path = tmp_dir / dat_path.filename().replace_extension(".dat.xml");
        py::object data = edat.attr("get_data")(gen_xml_path.string());
        edat.attr("pack")(gen_xml_path.string(), out_path.string(), data);
      } catch(const py::error_already_set& e) {
        spdlog::error(e.what());
      }
    }
    // remove the tmp directory
    fs::remove_all(tmp_dir);
  }
}

void Workspaces::render() {
  for(auto& [workspace_name, p_open] : open_workspace_windows) {
    if(!p_open) {
      continue;
    }
    if(!workspaces.contains(workspace_name)) {
      spdlog::error("Workspace {} does not exist", workspace_name);
      continue;
    }
    auto& workspace = workspaces[workspace_name];
    ImGuiWindowFlags wndflags = ImGuiWindowFlags_None;
    if(workspace->is_changed()) {
      wndflags |= ImGuiWindowFlags_UnsavedDocument;
    }
    ImGui::SetNextWindowSize(ImVec2(800, 800), ImGuiCond_FirstUseEver);
    if(ImGui::Begin(workspace->workspace_name.c_str(), &p_open)) {
      workspace->check_parsing();
      if(!workspace->is_parsed()) {
        if(!workspace->is_parsing()) {
          spdlog::error("Workspace is not loaded and probably failed loading! {}", workspace->workspace_name);
        } else {
          ImGui::Text("Parsing %s", workspace->workspace_name.c_str());
        }
      } else {
        workspace->render_window();
      }
    }
    ImGui::End();
    if(workspace->is_parsed()) {
      workspace->render_extra();
    }
  }
  if(show_add_workspace) {
    auto workspace = Workspace::render_init_workspace(&show_add_workspace);
    if(workspace) {
      add_workspace(std::move(workspace.value()));
      show_add_workspace = false;
    }
  }
}

void Workspaces::render_menu() {
  if(ImGui::BeginMenu(gettext("Workspaces"))) {
    if(open_workspace_windows.empty()) {
      ImGui::MenuItem(gettext("No workspaces loaded"), nullptr, nullptr, false);
    }
    assert(open_workspace_windows.size() == workspaces.size());
    for(auto& [workspace_name, p_open] : open_workspace_windows) {
      ImGui::MenuItem(workspace_name.c_str(), nullptr, &p_open);
    }
    ImGui::EndMenu();
  }
}

void Workspaces::add_workspace(std::unique_ptr<Workspace> w) {
  spdlog::info("Loading workspace {} {} {}", w->workspace_name, w->workspace_dat_path.string(), w->workspace_out_path.string());
  open_workspace_windows.insert({w->workspace_name, true});
  this->workspaces.insert({w->workspace_name, std::move(w)});
}

void Workspaces::save_project_file(fs::path path) {
  toml::array arr;
  for(auto& [_, workspace] : workspaces) {
    auto data = workspace->to_toml();
    arr.push_back(data);
  }
  toml::table table;
  table["workspaces"] = arr;
  std::fstream out(path, std::ios::out);
  out << toml::format(toml::value(table)) << std::endl;
}

void Workspaces::load_project_file(fs::path path) {
  if(fs::exists(path)) {
    auto data = toml::parse(path.string());
    auto workspaces = data["workspaces"].as_array();
    for(auto& workspace : workspaces) {
      std::unique_ptr<Workspace> w = std::make_unique<Workspace>();
      w->workspace_name = workspace["name"].as_string();
      w->init(workspace["dat_path"].as_string(), workspace["out_path"].as_string());
      add_workspace(std::move(w));
    }
  } else {
    spdlog::info("Could not load project file from {}", path.string());
  }
}

void Workspaces::save_workspaces(bool save_to_fs_path) {
  for(auto& [_, workspace] : workspaces) {
    if(!workspace->is_parsed() || workspace->is_parsing()) {
      continue;
    }
    workspace->save_changes_to_dat(save_to_fs_path);
  }
}
