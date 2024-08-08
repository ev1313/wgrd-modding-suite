#include "maingui.hpp"

#include "imgui.h"
#include "imgui_helpers.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "ImGuiFileDialog.h"

#include <libintl.h>

maingui::maingui() : program(gettext("WG: RD Modding Suite")) {
  program.add_argument("-p").help(gettext("Load a project file")).default_value(std::string{"project.toml"});
  program.add_argument("-v", "--verbose").help(gettext("Shows debug logs")).default_value(false).implicit_value(true);
}

bool maingui::init(int argc, char *argv[]) {
  program.parse_args(argc, argv);

  if(program.get<bool>("-v")) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  spdlog::debug("checking python");

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
    spdlog::error(gettext("Python does not work, exiting"));
    return false;
  }
  py::gil_scoped_release release;

  // configure spdlog
  auto logpattern = "[%H:%M:%S] [%^%l%$] %v";
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern(logpattern);

  auto max_size = 1048576 * 5;
  auto max_files = 10;
  std::string filename = "logs/modding_suite_log.txt";
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      filename, max_size, max_files);
  file_sink->set_pattern(logpattern);

  imgui_sink = std::make_shared<mod_manager::logger_mt>();
  imgui_sink->set_pattern(logpattern);

  spdlog::logger test(gettext("Mod Manager"), {console_sink, file_sink, imgui_sink});
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

  py::gil_scoped_release release2;
  
  workspaces.load_project_file(program.get("-p"));

  return true;
}

bool maingui::render_menu_bar() {
  bool ret = false;
  if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O)) {
    workspaces.show_add_workspace = true;
  }
  if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S)) {
    workspaces.save_workspaces(save_to_fs_path);
  }
  if(ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_F4)) {
    ret = true;
  }

  if(ImGui::BeginMenuBar()) {
    if(ImGui::BeginMenu(gettext("File"))) {
      if(ImGui::MenuItem(gettext("Open new workspace"), "Ctrl+O")) {
        workspaces.show_add_workspace = true;
      }
      if(ImGui::MenuItem(gettext("Save all workspaces"), "Ctrl+Shift+S")) {
        workspaces.save_workspaces(save_to_fs_path);
      }
      ImGui::Checkbox(gettext("Save dat files to input path"), &save_to_fs_path);
      ImGui::Separator();
      if(ImGui::MenuItem(gettext("Load project file"))) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        
        ImGuiFileDialog::Instance()->OpenDialog("Load project file", gettext("Load project file"), ".toml", config);
      }
      if(ImGui::MenuItem(gettext("Save project file"))) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("Save project file", gettext("Save project file"), ".toml", config);
      }
      ImGui::Separator();
      if(ImGui::MenuItem(gettext("Exit"), "Alt+F4")) {
        ret = true;
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

  if (ImGuiFileDialog::Instance()->Display(
    "Load project file", ImGuiWindowFlags_NoCollapse, ImVec2(800, 800))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      fs::path ret = ImGuiFileDialog::Instance()->GetFilePathName();
      spdlog::info("Load project file {}", ret.string());
      workspaces.load_project_file(ret);
    }
    ImGuiFileDialog::Instance()->Close();
  }
  if (ImGuiFileDialog::Instance()->Display(
    "Save project file", ImGuiWindowFlags_NoCollapse, ImVec2(800, 800))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      fs::path ret = ImGuiFileDialog::Instance()->GetFilePathName();
      spdlog::info("Save project file {}", ret.string());
      workspaces.save_project_file(ret);
    }
    ImGuiFileDialog::Instance()->Close();
  }

  return ret;
}

bool maingui::render() {
  if(!python_works) {
    ImGui::Begin(gettext("Error"), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text(gettext("Python does not work, see the logfile, exiting."));
    ImGui::Text(gettext("If you are on Windows, you need to start the start_modding_suite.bat instead of directly the modding_suite.exe file!"));
    ImGui::Text(gettext("Also check you installed Python 3.11 into your PATH."));
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
  ImGui::PopStyleVar(3);
        
  ImGuiID dockspace_id = ImGui::GetID("ModdingSuiteDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
  bool exit_now = render_menu_bar();

  ImGui::End();

  workspaces.render();

  if(show_style_editor) {
    ImGui::ShowStyleEditor(&show_style_editor, nullptr);
  }

  if(ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_L)) {
    imgui_sink->open_log = true;
  }
  imgui_sink->render_log();

  return exit_now;
}
