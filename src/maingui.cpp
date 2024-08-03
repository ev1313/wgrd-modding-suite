#include "maingui.hpp"

#include "imgui.h"
#include "imgui_helpers.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <format>

#include <libintl.h>

maingui::maingui() : program(gettext("WG: RD Modding Suite")) {
}

bool maingui::init(int argc, char *argv[]) {
  program.parse_args(argc, argv);

  // this checks the most basic things about whether python works at all
  py::gil_scoped_acquire acquire;
  try {
    py::str py_exec = (py::module::import("sys").attr("executable"));
    spdlog::info(std::string(py_exec));
    py::str py_path = (py::module::import("sys").attr("path"));
    spdlog::info(std::string(py_path));
    python_works = true;
  } catch(const py::error_already_set& e) {
    spdlog::error(e.what());
    python_works = false;
  }

  if(!python_works) {
    spdlog::error("Python does not work, exiting");
    return false;
  }
  py::gil_scoped_release release;

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

  spdlog::flush_on(spdlog::level::warn);
  spdlog::flush_every(std::chrono::seconds(3));

  imgui_sink->open_log = false;

  py::gil_scoped_acquire acquire2;
  try {
    py::str wgrd_cons_parsers = py::module::import("wgrd_cons_parsers");
    spdlog::info(std::string(wgrd_cons_parsers));
    py::str wgrd_cons_tools = py::module::import("wgrd_cons_tools");
    spdlog::info(std::string(wgrd_cons_tools));

    python_works = true;
  } catch(const py::error_already_set& e) {
    spdlog::error(e.what());
    python_works = false;
  }

  //file_tree.init_from_wgrd_path(program.get("wgrd_dir"));

  py::gil_scoped_release release2;

  return true;
}

void maingui::render_menu_bar() {
  if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O)) {
    show_add_workspace = true;
  }

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
    if(ImGui::BeginMenu(gettext("Settings"))) {
      if(ImGui::MenuItem(gettext("Style Editor"))) {
        show_style_editor = true;
      }
      if(ImGui::MenuItem(gettext("Log"), "Ctrl+L")) {
        imgui_sink->open_log = true;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

bool maingui::render() {
  if(!python_works) {
    ImGui::Begin("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Python does not work, see the logfile, exiting.");
    ImGui::Text("If you are on Windows, you need to start the start_modding_suite.bat instead of directly the modding_suite.exe file!");
    ImGui::Text("Also check you installed Python 3.11 into your PATH.");
    ImGui::End();
    imgui_sink->open_log = true;
    imgui_sink->render_log();
    return false;
  }

  // render main window
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);
        
  ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
  render_menu_bar();

  ImGui::End();

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
    auto workspace = Workspace::render_init_workspace();
    if(workspace) {
      workspaces.push_back(std::move(workspace.value()));
      show_add_workspace = false;
    }
  }

  if(show_style_editor) {
    ImGui::ShowStyleEditor();
  }

  imgui_sink->render_log();

  // FIXME: add something to exit the program?
  return false;
}
