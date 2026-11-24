#include "SidePanel.h"
#include "imgui.h"
#include <../fonts/IconsFontAwesome6.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

SidePanel::SidePanel(ImGuiWindowFlags side_panel_flags)
{
    this->side_panel_flags_ = side_panel_flags;
}

void SidePanel::generateLogo() const
{
    ImGui::Spacing();
    ImFont* logo_font = ImGui::GetIO().Fonts->Fonts[2];
    ImGui::PushFont(logo_font);
    ImVec2 text_size = ImGui::CalcTextSize("Ambition GUI");
    ImGui::SetCursorPosX((this->width_ - text_size.x) * 0.5f);
    ImGui::TextColored(this->logo_color_, "Ambition GUI");
    ImGui::PopFont();
    ImGui::Spacing();
}

void SidePanel::generateSideButtons(const ImGuiViewport* viewport, Views& active_view)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.1f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_Border, this->border_color_);
    ImGui::PushStyleColor(ImGuiCol_Button, this->side_btn_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->side_btn_hover_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->side_btn_active_color_);

    ImVec2 side_btn_size(this->width_, this->side_btn_height_);

    if (ImGui::Button("Science", side_btn_size))
    {
        active_view = Views::Science;
    }

    if (ImGui::Button("Navigation", side_btn_size))
    {
        active_view = Views::Navigation;
    }

    if (ImGui::Button("Maintance", side_btn_size))
    {
        active_view = Views::Maintenance;
    }

    if (ImGui::Button("Probing", side_btn_size))
    {
        active_view = Views::Probing;
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);
}

void SidePanel::generateLink(const ImGuiViewport* viewport) const
{
    ImGui::SetCursorPosY(viewport->WorkSize.y - this->side_btn_height_);

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_Border, this->border_color_);
    ImGui::PushStyleColor(ImGuiCol_Button, this->link_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, this->link_color_);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, this->link_color_);
    ImGui::PushStyleColor(ImGuiCol_Text, this->link_text_color_);

    ImVec2 side_btn_size(this->width_, this->side_btn_height_);

    ImGui::Button(ICON_FA_SIGNAL "LINK OK", side_btn_size);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);
}

void SidePanel::render(const ImGuiViewport* viewport, Views& active_view)
{
    ImVec2 side_panel_pos(viewport->WorkPos.x, viewport->WorkPos.y);
    ImVec2 side_panel_size(this->width_, viewport->WorkSize.y);

    ImGui::SetNextWindowPos(side_panel_pos);
    ImGui::SetNextWindowSize(side_panel_size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("SidePanel", nullptr, side_panel_flags_);

    generateLogo();
    generateSideButtons(viewport, active_view);
    generateLink(viewport);

    ImGui::PopStyleVar(1);
    ImGui::End();
}