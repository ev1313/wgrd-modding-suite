#pragma once

#include "logger.h"
#include <argparse/argparse.hpp>

#include "ThreadPool.h"

#include "workspace.hpp"

class maingui {
private:
  bool python_works = false;

  argparse::ArgumentParser program;

  std::shared_ptr<mod_manager::logger_mt> imgui_sink;

  Workspaces workspaces;
  bool show_style_editor = false;
  bool save_to_fs_path = false;
  bool render_menu_bar();

public:
  maingui();
  ~maingui();
  bool init(int argc, char *argv[]);
  /*
   * renders the main gui, returns true if the user wants to exit the program
   * */
  bool render();
};
