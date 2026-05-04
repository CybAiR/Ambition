#pragma once
#include "Views.h"
#include "imgui.h"
#include <string>

class SidePanel
{
  public:
    SidePanel(ImGuiWindowFlags side_panel_flags);

    [[nodiscard]] float get_width() const
    {
        return width;
    }
    void render(const ImGuiViewport* viewport, Views& active_view);

  private:
    float width = 250.0f;
    ImVec4 bg_color = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    ImVec4 border_color = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 link_text_color = ImVec4(0.00f, 0.90f, 0.46f, 1.00f);
    ImVec4 link_color = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    ImVec4 logo_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 side_btn_color = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    ImVec4 side_btn_hover_color = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    ImVec4 side_btn_active_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    ImVec4 side_btn_border_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    ImGuiWindowFlags side_panel_flags;
    float font_scale = 1.5f;
    float side_btn_height = 75.0f;

    void generate_logo() const;
    void generate_side_buttons(const ImGuiViewport* viewport, Views& active_view);
    void generate_link(const ImGuiViewport* viewport) const;
};