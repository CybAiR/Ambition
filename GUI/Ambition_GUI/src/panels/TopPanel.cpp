#include "TopPanel.h"
#include "imgui.h"
#include <../fonts/IconsFontAwesome6.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

TopPanel::TopPanel(float width, ImGuiWindowFlags top_panel_flags)
{
    width_ = width;
    top_panel_flags_ = top_panel_flags;
}

void TopPanel::generateViewText(std::string text) const
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, bg_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bg_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, bg_color_);
    ImGui::PushStyleColor(ImGuiCol_Border, border_color_);

    ImGui::Button("##ViewMain", ImVec2(view_btn_width_, view_btn_height_));

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);

    ImVec2 p_min = ImGui::GetItemRectMin();

    ImVec2 text1_size = ImGui::CalcTextSize("VIEW ");
    ImVec2 text2_size = ImGui::CalcTextSize(text.c_str());
    float total_width = text1_size.x + text2_size.x;

    float base_x = p_min.x + (view_btn_width_ - total_width) * 0.5f;
    float base_y = p_min.y + (view_btn_height_ - text1_size.y) * 0.5f;

    float visual_y_offset = 3.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddText(ImVec2(base_x, base_y + visual_y_offset),
                       ImGui::GetColorU32(lighter_text_color_), "VIEW ");
    draw_list->AddText(ImVec2(base_x + text1_size.x, base_y + visual_y_offset),
                       ImGui::GetColorU32(text_color_), text.c_str());

    ImGui::SameLine();
}

void TopPanel::generateEStop(const ImGuiViewport* viewport) const
{
    float absolute_center_x = (viewport->WorkSize.x - e_stop_width_) * 0.5f;
    float cursor_x = absolute_center_x - (start_x_);
    ImGui::SetCursorPosX(cursor_x);

    float cursor_y = (ImGui::GetWindowHeight() - e_stop_height_) * 0.5f;
    ImGui::SetCursorPosY(cursor_y);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, e_stop_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, e_stop_hover_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, e_stop_color_);
    ImGui::PushStyleColor(ImGuiCol_Border, e_stop_border_color_);
    ImGui::PushStyleColor(ImGuiCol_Text, e_stop_text_color_);

    ImGui::Button(ICON_FA_TRIANGLE_EXCLAMATION "  E-STOP", ImVec2(e_stop_width_, e_stop_height_));

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);

    ImGui::SameLine();
}

void TopPanel::generateSpeedView(const ImGuiViewport* viewport) const
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    float cursor_x = viewport->WorkSize.x - start_x_ - 300.0f;
    ImGui::SetCursorPosX(cursor_x);
    ImGui::SetCursorPosY(0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, bg_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bg_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, bg_color_);
    ImGui::PushStyleColor(ImGuiCol_Border, border_color_);
    ImGui::PushStyleColor(ImGuiCol_Text, speed_btn_color_);

    ImVec2 start_pos = ImGui::GetCursorPos();

    ImGui::Button("2.51 m/s", ImVec2(speed_btn_width_, speed_btn_height_));

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(1);

    ImGui::SameLine();

    ImVec2 next_button_pos = ImGui::GetCursorPos();

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::SetCursorPos(
        ImVec2(start_pos.x + style.FramePadding.x + 40.0f, start_pos.y + style.FramePadding.y));

    ImGui::Text("SPEED");
    ImGui::SameLine();

    ImGui::SetCursorPosX(next_button_pos.x);
    ImGui::SetCursorPosY(0.0f);
}

void TopPanel::generateDriveBtn(const ImGuiViewport* viewport) const
{

    ImGui::PushStyleColor(ImGuiCol_Button, drive_btn_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, drive_btn_hover_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, drive_btn_hover_color_);
    ImGui::PushStyleColor(ImGuiCol_Text, drive_text_color_);

    ImGui::Button(ICON_FA_GAMEPAD "DRIVE", ImVec2(drive_btn_width_, drive_btn_height_));

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
}

void TopPanel::render(const ImGuiViewport* viewport) const
{
    ImVec2 top_panel_pos(viewport->WorkPos.x + start_x_, viewport->WorkPos.y);
    ImVec2 top_panel_size(width_, height_);

    ImGui::SetNextWindowPos(top_panel_pos);
    ImGui::SetNextWindowSize(top_panel_size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("TopPanel", nullptr, top_panel_flags_);

    ImGui::SetWindowFontScale(font_scale_);

    TopPanel::generateViewText("Maintenance");

    TopPanel::generateEStop(viewport);

    TopPanel::generateSpeedView(viewport);

    TopPanel::generateDriveBtn(viewport);

    ImGui::End();

    ImGui::PopStyleVar();
}
