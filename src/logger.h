#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"
#include <mutex>

#include <sstream>
#include <imgui.h>

#include <libintl.h>

namespace mod_manager {
    template<typename Mutex>
    class logger : public spdlog::sinks::base_sink<Mutex> {
    private:
        std::stringstream log;
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
            //log << std::flush;
        }

    public:
        void render_log() {
            ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
            ImGui::Begin(gettext("Log"));
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
    };

    using logger_mt = logger<std::mutex>;
    using logger_st = logger<spdlog::details::null_mutex>;
}