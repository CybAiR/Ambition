#pragma once
#include "Views.h"
#include "imgui.h"

class SidePanel
{
  public:
    SidePanel(ImGuiWindowFlags side_panel_flags);

    [[nodiscard]] float getWidth() const
    {
        return width_;
    }
    void render(const ImGuiViewport* viewport, Views& active_view);

  private:
    float width_ = 250.0f;
    ImVec4 bg_color_ = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    ImVec4 border_color_ = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    ImVec4 text_color_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 link_text_color_ = ImVec4(0.00f, 0.90f, 0.46f, 1.00f);
    ImVec4 link_color_ = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    ImVec4 logo_color_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 side_btn_color_ = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    ImVec4 side_btn_hover_color_ = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    ImVec4 side_btn_active_color_ = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    ImVec4 side_btn_border_color_ = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    ImGuiWindowFlags side_panel_flags_;
    float font_scale_ = 1.5f;
    float side_btn_height_ = 75.0f;

    void generateLogo() const;
    void generateSideButtons(const ImGuiViewport* viewport, Views& active_view);
    void generateLink(const ImGuiViewport* viewport) const;
};