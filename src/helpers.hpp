#pragma once

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <ranges>
#include "spdlog/spdlog.h"

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
    _stdout_buffer = stringio();  // Other filelike object can be used here as well, such as objects created by pybind11
    _stderr_buffer = stringio();
    sysm.attr("stdout") = _stdout_buffer;
    sysm.attr("stderr") = _stderr_buffer;
    py::gil_scoped_release release;
  }
  std::string stdoutString() {
    py::gil_scoped_acquire acquire;
    _stdout_buffer.attr("seek")(last_offset_stdout);
    std::string ret = py::str(_stdout_buffer.attr("read")());
    last_offset_stdout = py::int_(_stdout_buffer.attr("tell")());
    py::gil_scoped_release release;
    return ret;
  }
  std::string stderrString() {
    py::gil_scoped_acquire acquire;
    _stderr_buffer.attr("seek")(last_offset_stderr);
    std::string ret = py::str(_stderr_buffer.attr("read")());
    last_offset_stderr = py::int_(_stderr_buffer.attr("tell")());
    py::gil_scoped_release release;
    return ret;
  }
  void update_log() {
    for(const auto line : stdoutString() | std::views::split('\n')) {
      spdlog::info(std::string_view(line));
    }
    for(const auto line : stderrString() | std::views::split('\n')) {
      spdlog::error(std::string_view(line));
    }
  }
  ~PyStdErrOutStreamRedirect() {
    py::gil_scoped_acquire acquire;
    auto sysm = py::module::import("sys");
    sysm.attr("stdout") = _stdout;
    sysm.attr("stderr") = _stderr;
    py::gil_scoped_release release;
  }
};

// used for vfs paths, as the contain $/ at the beginning, which is omitted in dat files
inline std::string remove_dollar(const std::string& str) {
  if(str.size() >= 2 && str[0] == '$' && str[1] == '/') {
    return str.substr(2);
  }
  return str;
}

inline std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); } // correct
                  );
    return s;
}
