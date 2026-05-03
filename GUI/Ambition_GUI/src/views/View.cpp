#include "View.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

View::View(ImGuiWindowFlags view_flags)
{
    this->view_flags = view_flags;
}

bool View::BeginCard(const char* id, const ImVec2& size) const
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kCardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    return ImGui::BeginChild(id, size, true, kNoScrollFlags);
}

void View::InnerSeparator() const
{
    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    const float y = ImGui::GetCursorScreenPos().y;

    draw_list->AddLine(
        ImVec2(pos.x + kCardPadX, y),
        ImVec2(pos.x + size.x - kCardPadX, y),
        ImGui::GetColorU32(kBorder)
    );

    ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

void View::RenderCameraContainer(float height) const
{
    if (ImGui::BeginChild(
        "CameraContainer",
        ImVec2(0.0f, height),
        true,
        kNoScrollFlags
    )) {
        // Tutaj można narysować tło, teksturę lub podgląd kamery.
    }

    ImGui::EndChild();
}

void View::DrawHeader(const char* title, float text_y) const
{
    const ImVec4& text_color = this->kTitleText;
    float text_x = this->kCardPadX;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();

    draw_list->AddRectFilled(
        pos,
        ImVec2(pos.x + size.x, pos.y + kHeaderHeight),
        ImGui::GetColorU32(kHeaderBg)
    );

    draw_list->AddLine(
        ImVec2(pos.x, pos.y + kHeaderHeight),
        ImVec2(pos.x + size.x, pos.y + kHeaderHeight),
        ImGui::GetColorU32(kBorder)
    );

    draw_list->AddRect(
        pos,
        ImVec2(pos.x + size.x, pos.y + size.y),
        ImGui::GetColorU32(kBorder)
    );

    ImGui::SetCursorPos(ImVec2(text_x, text_y));
    ImGui::TextColored(text_color, "%s", title);

    ImGui::SetCursorPos(ImVec2(kCardPadX, kHeaderHeight + 10.0f));
}

void View::ValueRow(float label_x, float value_x, const ImVec4& value_color, const char* label, const char* fmt, ...) const
{
    ImGui::SetCursorPosX(label_x);
    ImGui::TextColored(this->kMutedText, "%s", label);

    ImGui::SameLine(value_x);

    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(value_color, fmt, args);
    va_end(args);
}

void View::EndCard() const
{
    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void View::RenderGpsCard(const GpsOdometry& gps) const
{
    if (BeginCard("GpsCard", ImVec2(0.0f, kGpsCardHeight))) {
        DrawHeader("GPS / ODOMETRY");

        ValueRow(kLabelX, kValueX, kGreen, "LAT:", "%.4f N", gps.lat);
        ValueRow(kLabelX, kValueX, kGreen, "LON:", "%.4f W", gps.lon);
        ValueRow(kLabelX, kValueX, kGreen, "ALT:", "%.1f m", gps.alt_m);
    }

    EndCard();
}