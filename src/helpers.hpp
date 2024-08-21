#pragma once

#include <pybind11/pybind11.h>
#include <pystate.h>
#include <pytypedefs.h>
namespace py = pybind11;

#include <any>

#include "spdlog/spdlog.h"
#include <ranges>

#include <iostream>

#include "ThreadPool.h"

#include <filesystem>
namespace fs = std::filesystem;

extern ThreadPool thread_pool;

// Based on:
// https://stackoverflow.com/questions/58758429/pybind11-redirect-python-sys-stdout-to-c-from-print
class __attribute__((visibility("default"))) PyStdErrOutStreamRedirect {
  py::object _stdout;
  py::object _stderr;
  py::object _stdout_buffer;
  py::object _stderr_buffer;
  size_t last_offset_stdout = 0;
  size_t last_offset_stderr = 0;

public:
  PyStdErrOutStreamRedirect() {
    py::gil_scoped_acquire acquire;
    auto sysm = py::module::import("sys");
    _stdout = sysm.attr("stdout");
    _stderr = sysm.attr("stderr");
    auto stringio = py::module::import("io").attr("StringIO");
    _stdout_buffer = stringio(); // Other filelike object can be used here as
                                 // well, such as objects created by pybind11
    _stderr_buffer = stringio();
    sysm.attr("stdout") = _stdout_buffer;
    sysm.attr("stderr") = _stderr_buffer;
  }
  std::string stdoutString() {
    py::gil_scoped_acquire acquire;
    _stdout_buffer.attr("seek")(last_offset_stdout);
    std::string ret = py::str(_stdout_buffer.attr("read")());
    last_offset_stdout = py::int_(_stdout_buffer.attr("tell")());
    return ret;
  }
  std::string stderrString() {
    py::gil_scoped_acquire acquire;
    _stderr_buffer.attr("seek")(last_offset_stderr);
    std::string ret = py::str(_stderr_buffer.attr("read")());
    last_offset_stderr = py::int_(_stderr_buffer.attr("tell")());
    return ret;
  }
  void update_log() {
    return;
    for (const auto line : stdoutString() | std::views::split('\n')) {
      spdlog::info(std::string_view(line));
    }
    for (const auto line : stderrString() | std::views::split('\n')) {
      spdlog::error(std::string_view(line));
    }
  }
  ~PyStdErrOutStreamRedirect() {
    auto sysm = py::module::import("sys");
    sysm.attr("stdout") = _stdout;
    sysm.attr("stderr") = _stderr;
  }
};

// used for vfs paths, as the contain $/ at the beginning, which is omitted in
// dat files
inline std::string remove_dollar(const std::string &str) {
  if (str.size() >= 2 && str[0] == '$' && str[1] == '/') {
    return str.substr(2);
  }
  return str;
}

inline std::string str_tolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); } // correct
  );
  return s;
}

inline fs::path append_ext(fs::path path, std::string ext) {
  std::string new_ext = path.extension().string() + ext;
  return path.replace_extension(new_ext);
}
