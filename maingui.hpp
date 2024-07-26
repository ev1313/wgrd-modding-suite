#pragma once

#include "logger.h"
#include <argparse/argparse.hpp>

#include "file_tree.h"
#include "wgrd_files.h"

#include <pybind11/embed.h>
namespace py = pybind11;


class maingui {
private:
  argparse::ArgumentParser program;
  std::shared_ptr<mod_manager::logger_mt> imgui_sink;

  FileTree file_tree;
  wgrd_files::Files files;
public:
  maingui();
  bool init(int argc, char *argv[]);
  /*
   * renders the main gui, returns true if the user wants to exit the program
   * */
  bool render();
};
