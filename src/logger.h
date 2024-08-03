#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"
#include <mutex>

#include <sstream>
#include <imgui.h>

#include <libintl.h>

#include "helpers.hpp"

namespace mod_manager {
template<typename Mutex>
class logger : public spdlog::sinks::base_sink<Mutex> {
private:
  std::stringstream log;
  std::unique_ptr<PyStdErrOutStreamRedirect> py_redirect;
protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
    // msg.raw contains pre formatted log

    // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    log << fmt::to_string(formatted);
  }

  void flush_() override {
    log << py_redirect->stdoutString();
    log << py_redirect->stderrString();
  }

public:
  logger() {
    py_redirect = std::make_unique<PyStdErrOutStreamRedirect>();
  }
  bool open_log = false;
  void render_log() {
    if(!open_log) {
      return;
    }
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    if(ImGui::Begin(gettext("Log"))) {
      ImGui::TextWrapped("%s", log.str().c_str());

      ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() * 2.0f);
      ImGui::Separator();
      if(ImGui::Button(gettext("Copy to Clipboard"))) {
        ImGui::LogToClipboard();
        ImGui::LogText("%s", log.str().c_str());
        ImGui::LogFinish();
      }
      ImGui::End();
    }
  }
  void deinit() {
    py_redirect.reset();
  }
};

using logger_mt = logger<std::mutex>;
using logger_st = logger<spdlog::details::null_mutex>;
}
