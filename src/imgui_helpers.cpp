#include "imgui_helpers.hpp"

#include <libintl.h>

#include "math.h"

#include <ImGuiFileDialog.h>

std::optional<std::string> show_file_dialog_input(std::string title,
                                                  std::string previous_path,
                                                  std::string dialogkey) {
  std::optional<std::string> ret = std::nullopt;
  std::string button_text = gettext("Open File Dialog##") + dialogkey;
  if (ImGui::Button(button_text.c_str())) {
    IGFD::FileDialogConfig config;
    config.path = previous_path;
    ImGuiFileDialog::Instance()->OpenDialog(dialogkey, title, nullptr, config);
  }

  if (ImGuiFileDialog::Instance()->Display(
          dialogkey, ImGuiWindowFlags_NoCollapse, ImVec2(800, 800))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      ret = ImGuiFileDialog::Instance()->GetCurrentPath();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  return ret;
}

// Forward declare ShowFontAtlas() which isn't worth putting in public API yet
namespace ImGui {
IMGUI_API void ShowFontAtlas(ImFontAtlas *atlas);
}

// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is the more
// flexible one.
void ImGui::ShowFontSelector(const char *label) {
  ImGuiIO &io = ImGui::GetIO();
  ImFont *font_current = ImGui::GetFont();
  if (ImGui::BeginCombo(label, font_current->GetDebugName())) {
    for (ImFont *font : io.Fonts->Fonts) {
      ImGui::PushID((void *)font);
      if (ImGui::Selectable(font->GetDebugName(), font == font_current))
        io.FontDefault = font;
      ImGui::PopID();
    }
    ImGui::EndCombo();
  }
  ImGui::SameLine();
}

// Demo helper function to select among default colors. See ShowStyleEditor()
// for more advanced options. Here we use the simplified Combo() api that packs
// items into a single literal string. Useful for quick combo boxes where the
// choices are known locally.
bool ImGui::ShowStyleSelector(const char *label) {
  static int style_idx = -1;
  if (ImGui::Combo(label, &style_idx, "Dark\0Light\0Classic\0")) {
    switch (style_idx) {
    case 0:
      ImGui::StyleColorsDark();
      break;
    case 1:
      ImGui::StyleColorsLight();
      break;
    case 2:
      ImGui::StyleColorsClassic();
      break;
    }
    return true;
  }
  return false;
}

void ImGui::ShowStyleEditor(bool *show_window, ImGuiStyle *ref) {
  ImGui::Begin(gettext("ImGui Style Editor"), show_window);
  // You can pass in a reference ImGuiStyle structure to compare to, revert to
  // and save to (without a reference style pointer, we will use one compared
  // locally as a reference)
  ImGuiStyle &style = ImGui::GetStyle();
  static ImGuiStyle ref_saved_style;

  // Default to using internal storage as reference
  static bool init = true;
  if (init && ref == NULL)
    ref_saved_style = style;
  init = false;
  if (ref == NULL)
    ref = &ref_saved_style;

  ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

  if (ImGui::ShowStyleSelector("Colors##Selector"))
    ref_saved_style = style;
  ImGui::ShowFontSelector("Fonts##Selector");

  // Simplified Settings (expose floating-pointer border sizes as boolean
  // representing 0.0f or 1.0f)
  if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f,
                         "%.0f"))
    style.GrabRounding = style.FrameRounding; // Make GrabRounding always the
                                              // same value as FrameRounding
  {
    bool border = (style.WindowBorderSize > 0.0f);
    if (ImGui::Checkbox("WindowBorder", &border)) {
      style.WindowBorderSize = border ? 1.0f : 0.0f;
    }
  }
  ImGui::SameLine();
  {
    bool border = (style.FrameBorderSize > 0.0f);
    if (ImGui::Checkbox("FrameBorder", &border)) {
      style.FrameBorderSize = border ? 1.0f : 0.0f;
    }
  }
  ImGui::SameLine();
  {
    bool border = (style.PopupBorderSize > 0.0f);
    if (ImGui::Checkbox("PopupBorder", &border)) {
      style.PopupBorderSize = border ? 1.0f : 0.0f;
    }
  }

  // Save/Revert button
  // if (ImGui::Button("Save Ref"))
  //  *ref = ref_saved_style = style;
  // ImGui::SameLine();
  // if (ImGui::Button("Revert Ref"))
  //  style = *ref;

  ImGui::Separator();

  if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Sizes")) {
      ImGui::SeparatorText("Main");
      ImGui::SliderFloat2("WindowPadding", (float *)&style.WindowPadding, 0.0f,
                          20.0f, "%.0f");
      ImGui::SliderFloat2("FramePadding", (float *)&style.FramePadding, 0.0f,
                          20.0f, "%.0f");
      ImGui::SliderFloat2("ItemSpacing", (float *)&style.ItemSpacing, 0.0f,
                          20.0f, "%.0f");
      ImGui::SliderFloat2("ItemInnerSpacing", (float *)&style.ItemInnerSpacing,
                          0.0f, 20.0f, "%.0f");
      ImGui::SliderFloat2("TouchExtraPadding",
                          (float *)&style.TouchExtraPadding, 0.0f, 10.0f,
                          "%.0f");
      ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f,
                         "%.0f");
      ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f,
                         "%.0f");
      ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f,
                         "%.0f");

      ImGui::SeparatorText("Borders");
      ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f,
                         1.0f, "%.0f");
      ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f,
                         "%.0f");
      ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f,
                         "%.0f");
      ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f,
                         "%.0f");
      ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f,
                         "%.0f");
      ImGui::SliderFloat("TabBarBorderSize", &style.TabBarBorderSize, 0.0f,
                         2.0f, "%.0f");
      ImGui::SliderFloat("TabBarOverlineSize", &style.TabBarOverlineSize, 0.0f,
                         2.0f, "%.0f");

      ImGui::SeparatorText("Rounding");
      ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f,
                         "%.0f");
      ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f,
                         "%.0f");
      ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f,
                         "%.0f");
      ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f,
                         "%.0f");
      ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f,
                         12.0f, "%.0f");
      ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f,
                         "%.0f");
      ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f,
                         "%.0f");

      ImGui::SeparatorText("Tables");
      ImGui::SliderFloat2("CellPadding", (float *)&style.CellPadding, 0.0f,
                          20.0f, "%.0f");
      ImGui::SliderAngle("TableAngledHeadersAngle",
                         &style.TableAngledHeadersAngle, -50.0f, +50.0f);
      ImGui::SliderFloat2("TableAngledHeadersTextAlign",
                          (float *)&style.TableAngledHeadersTextAlign, 0.0f,
                          1.0f, "%.2f");

      ImGui::SeparatorText("Widgets");
      ImGui::SliderFloat2("WindowTitleAlign", (float *)&style.WindowTitleAlign,
                          0.0f, 1.0f, "%.2f");
      int window_menu_button_position = style.WindowMenuButtonPosition + 1;
      if (ImGui::Combo("WindowMenuButtonPosition",
                       (int *)&window_menu_button_position,
                       "None\0Left\0Right\0"))
        style.WindowMenuButtonPosition =
            (ImGuiDir)(window_menu_button_position - 1);
      ImGui::Combo("ColorButtonPosition", (int *)&style.ColorButtonPosition,
                   "Left\0Right\0");
      ImGui::SliderFloat2("ButtonTextAlign", (float *)&style.ButtonTextAlign,
                          0.0f, 1.0f, "%.2f");
      ImGui::SliderFloat2("SelectableTextAlign",
                          (float *)&style.SelectableTextAlign, 0.0f, 1.0f,
                          "%.2f");
      ImGui::SliderFloat("SeparatorTextBorderSize",
                         &style.SeparatorTextBorderSize, 0.0f, 10.0f, "%.0f");
      ImGui::SliderFloat2("SeparatorTextAlign",
                          (float *)&style.SeparatorTextAlign, 0.0f, 1.0f,
                          "%.2f");
      ImGui::SliderFloat2("SeparatorTextPadding",
                          (float *)&style.SeparatorTextPadding, 0.0f, 40.0f,
                          "%.0f");
      ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f,
                         12.0f, "%.0f");

      ImGui::SeparatorText("Docking");
      ImGui::SliderFloat("DockingSplitterSize", &style.DockingSeparatorSize,
                         0.0f, 12.0f, "%.0f");

      ImGui::SeparatorText("Tooltips");
      for (int n = 0; n < 2; n++)
        if (ImGui::TreeNodeEx(n == 0 ? "HoverFlagsForTooltipMouse"
                                     : "HoverFlagsForTooltipNav")) {
          ImGuiHoveredFlags *p = (n == 0) ? &style.HoverFlagsForTooltipMouse
                                          : &style.HoverFlagsForTooltipNav;
          ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayNone", p,
                               ImGuiHoveredFlags_DelayNone);
          ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayShort", p,
                               ImGuiHoveredFlags_DelayShort);
          ImGui::CheckboxFlags("ImGuiHoveredFlags_DelayNormal", p,
                               ImGuiHoveredFlags_DelayNormal);
          ImGui::CheckboxFlags("ImGuiHoveredFlags_Stationary", p,
                               ImGuiHoveredFlags_Stationary);
          ImGui::CheckboxFlags("ImGuiHoveredFlags_NoSharedDelay", p,
                               ImGuiHoveredFlags_NoSharedDelay);
          ImGui::TreePop();
        }

      ImGui::SeparatorText("Misc");
      ImGui::SliderFloat2("DisplayWindowPadding",
                          (float *)&style.DisplayWindowPadding, 0.0f, 30.0f,
                          "%.0f");
      ImGui::SliderFloat2("DisplaySafeAreaPadding",
                          (float *)&style.DisplaySafeAreaPadding, 0.0f, 30.0f,
                          "%.0f");

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Colors")) {
      static int output_dest = 0;
      static bool output_only_modified = true;
      if (ImGui::Button("Export")) {
        if (output_dest == 0)
          ImGui::LogToClipboard();
        else
          ImGui::LogToTTY();
        ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;");
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
          const ImVec4 &col = style.Colors[i];
          const char *name = ImGui::GetStyleColorName(i);
          if (!output_only_modified ||
              memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
            ImGui::LogText(
                "colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);",
                name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
        }
        ImGui::LogFinish();
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120);
      ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
      ImGui::SameLine();
      ImGui::Checkbox("Only Modified Colors", &output_only_modified);

      static ImGuiTextFilter filter;
      filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

      static ImGuiColorEditFlags alpha_flags = 0;
      if (ImGui::RadioButton("Opaque",
                             alpha_flags == ImGuiColorEditFlags_None)) {
        alpha_flags = ImGuiColorEditFlags_None;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Alpha",
                             alpha_flags == ImGuiColorEditFlags_AlphaPreview)) {
        alpha_flags = ImGuiColorEditFlags_AlphaPreview;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton(
              "Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) {
        alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf;
      }
      ImGui::SameLine();

      ImGui::SetNextWindowSizeConstraints(
          ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 10),
          ImVec2(FLT_MAX, FLT_MAX));
      ImGui::BeginChild("##colors", ImVec2(0, 0),
                        ImGuiChildFlags_Border | ImGuiChildFlags_NavFlattened,
                        ImGuiWindowFlags_AlwaysVerticalScrollbar |
                            ImGuiWindowFlags_AlwaysHorizontalScrollbar);
      ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
      for (int i = 0; i < ImGuiCol_COUNT; i++) {
        const char *name = ImGui::GetStyleColorName(i);
        if (!filter.PassFilter(name))
          continue;
        ImGui::PushID(i);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
        if (ImGui::Button("?"))
          ImGui::DebugFlashStyleColor((ImGuiCol)i);
        ImGui::SetItemTooltip(
            "Flash given color to identify places where it is used.");
        ImGui::SameLine();
#endif
        ImGui::ColorEdit4("##color", (float *)&style.Colors[i],
                          ImGuiColorEditFlags_AlphaBar | alpha_flags);
        if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0) {
          // Tips: in a real user application, you may want to merge and use an
          // icon font into the main font, so instead of "Save"/"Revert" you'd
          // use icons! Read the FAQ and docs/FONTS.md about using icon fonts.
          // It's really easy and super convenient!
          ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
          if (ImGui::Button("Save")) {
            ref->Colors[i] = style.Colors[i];
          }
          ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
          if (ImGui::Button("Revert")) {
            style.Colors[i] = ref->Colors[i];
          }
        }
        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
        ImGui::TextUnformatted(name);
        ImGui::PopID();
      }
      ImGui::PopItemWidth();
      ImGui::EndChild();

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Fonts")) {
      ImGuiIO &io = ImGui::GetIO();
      ImFontAtlas *atlas = io.Fonts;
      ImGui::ShowFontAtlas(atlas);

      // Post-baking font scaling. Note that this is NOT the nice way of scaling
      // fonts, read below. (we enforce hard clamping manually as by default
      // DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
      const float MIN_SCALE = 0.3f;
      const float MAX_SCALE = 2.0f;
      static float window_scale = 1.0f;
      ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
      if (ImGui::DragFloat(
              "window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE,
              "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
        ImGui::SetWindowFontScale(window_scale);
      ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE,
                       MAX_SCALE, "%.2f",
                       ImGuiSliderFlags_AlwaysClamp); // Scale everything
      ImGui::PopItemWidth();

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Rendering")) {
      ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
      ImGui::SameLine();

      ImGui::Checkbox("Anti-aliased lines use texture",
                      &style.AntiAliasedLinesUseTex);
      ImGui::SameLine();

      ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
      ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
      ImGui::DragFloat("Curve Tessellation Tolerance",
                       &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f,
                       "%.2f");
      if (style.CurveTessellationTol < 0.10f)
        style.CurveTessellationTol = 0.10f;

      // When editing the "Circle Segment Max Error" value, draw a preview of
      // its effect on auto-tessellated circles.
      ImGui::DragFloat("Circle Tessellation Max Error",
                       &style.CircleTessellationMaxError, 0.005f, 0.10f, 5.0f,
                       "%.2f", ImGuiSliderFlags_AlwaysClamp);
      const bool show_samples = ImGui::IsItemActive();
      if (show_samples)
        ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
      if (show_samples && ImGui::BeginTooltip()) {
        ImGui::TextUnformatted("(R = radius, N = approx number of segments)");
        ImGui::Spacing();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        const float min_widget_width = ImGui::CalcTextSize("R: MMM\nN: MMM").x;
        for (int n = 0; n < 8; n++) {
          const float RAD_MIN = 5.0f;
          const float RAD_MAX = 70.0f;
          const float rad =
              RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);

          ImGui::BeginGroup();

          // N is not always exact here due to how PathArcTo() function work
          // internally
          ImGui::Text("R: %.f\nN: %d", rad,
                      draw_list->_CalcCircleAutoSegmentCount(rad));

          const float canvas_width = std::max(min_widget_width, rad * 2.0f);
          const float offset_x = floorf(canvas_width * 0.5f);
          const float offset_y = floorf(RAD_MAX);

          const ImVec2 p1 = ImGui::GetCursorScreenPos();
          draw_list->AddCircle(ImVec2(p1.x + offset_x, p1.y + offset_y), rad,
                               ImGui::GetColorU32(ImGuiCol_Text));
          ImGui::Dummy(ImVec2(canvas_width, RAD_MAX * 2));

          /*
                    const ImVec2 p2 = ImGui::GetCursorScreenPos();
                    draw_list->AddCircleFilled(ImVec2(p2.x + offset_x, p2.y +
             offset_y), rad, ImGui::GetColorU32(ImGuiCol_Text));
                    ImGui::Dummy(ImVec2(canvas_width, RAD_MAX * 2));
                    */

          ImGui::EndGroup();
          ImGui::SameLine();
        }
        ImGui::EndTooltip();
      }
      ImGui::SameLine();

      ImGui::DragFloat(
          "Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f,
          "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero
                   // alpha clips all widgets). But application code could have
                   // a toggle to switch between zero and non-zero.
      ImGui::DragFloat("Disabled Alpha", &style.DisabledAlpha, 0.005f, 0.0f,
                       1.0f, "%.2f");
      ImGui::PopItemWidth();

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  ImGui::PopItemWidth();
  ImGui::End();
}
