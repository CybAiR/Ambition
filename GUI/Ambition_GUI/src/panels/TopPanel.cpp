#include "TopPanel.h"
#include "imgui.h"
#include <../fonts/IconsFontAwesome6.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

TopPanel::TopPanel(float width, ImGuiWindowFlags top_panel_flags)
{
    this->width = width;
    this->top_panel_flags = top_panel_flags;
}

void TopPanel::generate_view_text(std::string text) const
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, this->bg_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->bg_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->bg_color);
    ImGui::PushStyleColor(ImGuiCol_Border, this->border_color);

    ImGui::Button("##ViewMain", ImVec2(this->view_btn_width, this->view_btn_height));

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);

    ImVec2 p_min = ImGui::GetItemRectMin();

    ImVec2 text1_size = ImGui::CalcTextSize("VIEW ");
    ImVec2 text2_size = ImGui::CalcTextSize(text.c_str());
    float total_width = text1_size.x + text2_size.x;

    float base_x = p_min.x + (this->view_btn_width - total_width) * 0.5f;
    float base_y = p_min.y + (this->view_btn_height - text1_size.y) * 0.5f;

    float visual_y_offset = 3.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddText(ImVec2(base_x, base_y + visual_y_offset),
                       ImGui::GetColorU32(this->lighter_text_color), "VIEW ");
    draw_list->AddText(ImVec2(base_x + text1_size.x, base_y + visual_y_offset),
                       ImGui::GetColorU32(this->text_color), text.c_str());

    ImGui::SameLine();
}

void TopPanel::generate_e_stop(const ImGuiViewport* viewport) const
{
    float absolute_center_x = (viewport->WorkSize.x - this->e_stop_width) * 0.5f;
    float cursor_x = absolute_center_x - (this->start_x);
    ImGui::SetCursorPosX(cursor_x);

    float cursor_y = (ImGui::GetWindowHeight() - this->e_stop_height) * 0.5f;
    ImGui::SetCursorPosY(cursor_y);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, this->e_stop_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->e_stop_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->e_stop_color);
    ImGui::PushStyleColor(ImGuiCol_Border, this->e_stop_border_color);
    ImGui::PushStyleColor(ImGuiCol_Text, this->e_stop_text_color);

    ImGui::Button(ICON_FA_TRIANGLE_EXCLAMATION "  E-STOP",
                  ImVec2(this->e_stop_width, this->e_stop_height));

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);

    ImGui::SameLine();
}

void TopPanel::generate_speed_view(const ImGuiViewport* viewport) const
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    float cursor_x = viewport->WorkSize.x - this->start_x - 300.0f;
    ImGui::SetCursorPosX(cursor_x);
    ImGui::SetCursorPosY(0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, this->bg_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->bg_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->bg_color);
    ImGui::PushStyleColor(ImGuiCol_Border, this->border_color);
    ImGui::PushStyleColor(ImGuiCol_Text, this->speed_btn_color);

    ImVec2 start_pos = ImGui::GetCursorPos();

    ImGui::Button("2.51 m/s", ImVec2(this->speed_btn_width, this->speed_btn_height));

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

void TopPanel::generate_drive_btn(const ImGuiViewport* viewport) const
{

    ImGui::PushStyleColor(ImGuiCol_Button, this->drive_btn_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->drive_btn_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->drive_btn_hover_color);
    ImGui::PushStyleColor(ImGuiCol_Text, this->drive_text_color);

    ImGui::Button(ICON_FA_GAMEPAD "DRIVE", ImVec2(drive_btn_width, drive_btn_height));

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
}

void TopPanel::render(const ImGuiViewport* viewport) const
{
    ImVec2 top_panel_pos(viewport->WorkPos.x + this->start_x, viewport->WorkPos.y);
    ImVec2 top_panel_size(this->width, this->height);

    ImGui::SetNextWindowPos(top_panel_pos);
    ImGui::SetNextWindowSize(top_panel_size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("TopPanel", nullptr, this->top_panel_flags);



    TopPanel::generate_view_text("Maintenance");

    TopPanel::generate_e_stop(viewport);

    TopPanel::generate_speed_view(viewport);

    TopPanel::generate_drive_btn(viewport);

    ImGui::End();

    ImGui::PopStyleVar();
}