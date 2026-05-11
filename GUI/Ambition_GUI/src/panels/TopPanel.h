#pragma once
#include "imgui.h"
#include <string>

class TopPanel
{
  public:
    TopPanel(float width, ImGuiWindowFlags top_panel_flags);

    void render(const ImGuiViewport* viewport) const;

  private:
    float width_;
    float height_ = 75.0f;
    float start_x_ = 250.0f;
    float start_y_ = 0.0f;
    ImVec4 bg_color_ = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    ImVec4 border_color_ = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    ImVec4 text_color_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 lighter_text_color_ = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    ImVec4 e_stop_text_color_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 e_stop_color_ = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);
    ImVec4 e_stop_hover_color_ = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
    ImVec4 e_stop_border_color_ = ImVec4(0.94f, 0.33f, 0.31f, 1.00f);
    ImVec4 speed_btn_color_ = ImVec4(1.00f, 0.79f, 0.16f, 1.00f);
    ImVec4 drive_btn_color_ = ImVec4(0.11f, 0.15f, 0.29f, 1.00f);
    ImVec4 drive_btn_hover_color_ = ImVec4(0.16f, 0.22f, 0.40f, 1.00f);
    ImVec4 drive_btn_border_color_ = ImVec4(0.39f, 0.71f, 0.96f, 1.00f);
    ImVec4 drive_text_color_ = ImVec4(0.39f, 0.71f, 0.96f, 1.00f);
    ImGuiWindowFlags top_panel_flags_;
    float font_scale_ = 1.5f;
    float view_btn_width_ = 250.0f;
    float view_btn_height_ = height_;
    float e_stop_width_ = 150.0f;
    float e_stop_height_ = 50.0f;
    float speed_btn_width_ = 150.0f;
    float speed_btn_height_ = height_;
    float drive_btn_width_ = 150.0f;
    float drive_btn_height_ = height_;

    void generateViewText(std::string text) const;
    void generateEStop(const ImGuiViewport* viewport) const;
    void generateSpeedView(const ImGuiViewport* viewport) const;
    void generateDriveBtn(const ImGuiViewport* viewport) const;
};