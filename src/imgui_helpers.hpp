#pragma once

#include "imgui.h"

namespace ImGui {
void ShowFontSelector(const char* label);
bool ShowStyleSelector(const char* label);
void ShowStyleEditor(ImGuiStyle* ref);
}
