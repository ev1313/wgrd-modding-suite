#include "maingui.hpp"

#include "imgui.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <libintl.h>

maingui::maingui() : program(gettext("WG: RD Modding Suite")) {
}

bool maingui::init(int argc, char *argv[]) {
  program.parse_args(argc, argv);

  // configure spdlog
  auto logpattern = "[%H:%M:%S] [%^%l%$] %v";
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern(logpattern);

  auto max_size = 1048576 * 5;
  auto max_files = 10;
  std::string filename = "logs/mod_manager_log.txt";
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      filename, max_size, max_files);
  file_sink->set_pattern(logpattern);

  imgui_sink = std::make_shared<mod_manager::logger_mt>();
  imgui_sink->set_pattern(logpattern);

  spdlog::logger test("Mod Manager", {console_sink, file_sink, imgui_sink});
  auto logger = std::make_shared<spdlog::logger>(test);
  spdlog::set_default_logger(logger);

  py::module sys = py::module::import("sys");
  py::print(sys.attr("path"));
  py::print("python version: ", sys.attr("version_info"));

  //file_tree.init_from_wgrd_path(program.get("wgrd_dir"));

  py::gil_scoped_release release;

  return true;
}

void maingui::render_menu_bar() {
  if(ImGui::BeginMenuBar()) {
    if(ImGui::BeginMenu(gettext("File"))) {
      if(ImGui::MenuItem(gettext("Open workspace"), "Ctrl+O")) {
        show_add_workspace = true;
      }
      if(ImGui::MenuItem(gettext("Save all workspaces"), "Ctrl+Shift+S")) {
        spdlog::info("Save file");
      }
      if(ImGui::MenuItem(gettext("Exit"), "Alt+F4")) {
        spdlog::info("Exit");
      }
      ImGui::EndMenu();
    }
    if(ImGui::BeginMenu(gettext("Edit"))) {
      if(ImGui::MenuItem(gettext("Undo"), "Ctrl+Z")) {
        spdlog::info("Undo");
      }
      if(ImGui::MenuItem(gettext("Redo"), "Ctrl+Y")) {
        spdlog::info("Redo");
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

bool maingui::render() {
  // render main window
#ifdef IMGUI_HAS_VIEWPORT
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->GetWorkPos());
  ImGui::SetNextWindowSize(viewport->GetWorkSize());
  ImGui::SetNextWindowViewport(viewport->ID);
#else 
  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNav);
  render_menu_bar();

  ImGui::BeginTabBar("Workspace Tabs", ImGuiTabBarFlags_Reorderable);
  size_t idx = 0;
  for(auto& workspace : workspaces) {
    std::string name = std::format("{}##WP_{}", workspace.workspace_name, idx++);
    if (ImGui::BeginTabItem(name.c_str())) {
      workspace.render();
      ImGui::EndTabItem();
    } 
  }
  ImGui::EndTabBar();

  if(workspaces.empty()) {
    if(ImGui::Button("Add workspace", ImGui::GetContentRegionAvail())) {
      show_add_workspace = true;
    }
  }
  if(show_add_workspace) {
    auto workspace = Workspace::render_init_workspace();
    if(workspace) {
      workspaces.push_back(std::move(workspace.value()));
      show_add_workspace = false;
    }
  }

  ImGui::PopStyleVar();
  ImGui::End();

  // FIXME: add something to exit the program?
  return false;
}
