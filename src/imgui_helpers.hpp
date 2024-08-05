#pragma once

#include "imgui.h"

#include <optional>
#include <string>

std::optional<std::string> show_file_dialog_input(std::string title,
                                                  std::string previous_path,
                                                  std::string dialogkey);

namespace ImGui {
void ShowFontSelector(const char* label);
bool ShowStyleSelector(const char* label);
void ShowStyleEditor(bool* show_window, ImGuiStyle* ref);
}
