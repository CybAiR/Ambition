#include "SidePanel.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

SidePanel::SidePanel(ImGuiWindowFlags side_panel_flags)
{
    this->side_panel_flags = side_panel_flags;
}

void SidePanel::generate_logo() const
{
    ImGui::Spacing();
    ImGui::SetWindowFontScale(2.0f);
    ImVec2 text_size = ImGui::CalcTextSize("Ambition GUI");
    ImGui::SetCursorPosX((this->width - text_size.x) * 0.5f);
    ImGui::TextColored(this->logo_color, "Ambition GUI");
    ImGui::Spacing();
    ImGui::SetWindowFontScale(1.5f);
}

void SidePanel::generate_side_buttons(const ImGuiViewport* viewport) const
{
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.1f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_Border, this->border_color);
    ImGui::PushStyleColor(ImGuiCol_Button, this->side_btn_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->side_btn_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->side_btn_active_color);

    ImVec2 side_btn_size(this->width, this->side_btn_height);

    ImGui::Button("Science", side_btn_size);
    ImGui::Button("Navigation", side_btn_size);
    ImGui::Button("Maintenance", side_btn_size);
    ImGui::Button("Probing", side_btn_size);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);

}

void SidePanel::generate_link(const ImGuiViewport* viewport) const
{
    ImGui::SetCursorPosY(viewport->WorkSize.y - this->side_btn_height);

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_Border, this->border_color);
    ImGui::PushStyleColor(ImGuiCol_Button, this->link_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->link_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->link_color);
    ImGui::PushStyleColor(ImGuiCol_Text, this->link_text_color);

    ImVec2 side_btn_size(this->width, this->side_btn_height);

    
    ImGui::Button("Link OK", side_btn_size);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);
}

void SidePanel::render(const ImGuiViewport* viewport) const 
{
    ImVec2 side_panel_pos(
        viewport->WorkPos.x,
        viewport->WorkPos.y
    );
    ImVec2 side_panel_size(
        this->width, 
        viewport->WorkSize.y
    );

    ImGui::SetNextWindowPos(side_panel_pos);
    ImGui::SetNextWindowSize(side_panel_size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("SidePanel", nullptr, side_panel_flags);

    generate_logo();
    generate_side_buttons(viewport);
    generate_link(viewport);
    
    ImGui::PopStyleVar(1);
    ImGui::End();
}