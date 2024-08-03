#pragma once

#include "logger.h"
#include <argparse/argparse.hpp>

#include "workspace.hpp"

#include <pybind11/embed.h>
namespace py = pybind11;

#include "helpers.hpp"

class maingui {
private:
  bool python_works = false;

  argparse::ArgumentParser program;

  std::shared_ptr<mod_manager::logger_mt> imgui_sink;

  bool show_add_workspace = false;
  bool show_style_editor = false;
  std::vector<Workspace> workspaces;
  void render_menu_bar();
public:
  maingui();
  ~maingui() {
    imgui_sink->deinit();
  }
  bool init(int argc, char *argv[]);
  /*
   * renders the main gui, returns true if the user wants to exit the program
   * */
  bool render();
};
