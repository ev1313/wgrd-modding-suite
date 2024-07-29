#include "maingui.hpp"

#include "imgui.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

maingui::maingui() : program(gettext("WG: RD Modding Suite")) {
  program.add_argument("--wgrd_dir").help("Path to Wargame directory.").metavar("wgrd_dir");
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

  file_tree.init_from_wgrd_path(program.get("wgrd_dir"));

  py::gil_scoped_release release;

  return true;
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
  ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav);
  auto meta = file_tree.render("Workspace 1");
  if(meta) {
    files.add_file(meta.value());
  }
  ImGui::SameLine();
    files.imgui_call(meta);
  ImGui::End();

  // FIXME: add something to exit the program?
  return false;
}
