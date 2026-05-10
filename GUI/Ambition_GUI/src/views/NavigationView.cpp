#include "NavigationView.h"

#include <../fonts/IconsFontAwesome6.h>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <imgui.h>

namespace
{
constexpr float K_RIGHT_PANEL_WIDTH = 400.0f;
constexpr float K_MIN_LEFT_WIDTH = 240.0f;
constexpr float K_MIN_RIGHT_WIDTH = 180.0f;

constexpr float K_OUTER_PAD_X = 10.0f;
constexpr float K_OUTER_PAD_Y = 10.0f;

// Status - Manual
const ImVec4 K_THEME_BROWN = ImVec4(0.227f, 0.114f, 0.063f, 1.0f);
const ImVec4 K_THEME_ORANGE = ImVec4(1.000f, 0.678f, 0.420f, 1.0f);
const ImVec4 K_THEME_RED_ORANGE = ImVec4(0.929f, 0.294f, 0.114f, 1.0f);

// Status - Auto
const ImVec4 K_THEME_DARK_BLUE = ImVec4(0.063f, 0.114f, 0.227f, 1.0f);
const ImVec4 K_THEME_BLUE = ImVec4(0.420f, 0.678f, 1.000f, 1.0f);
const ImVec4 K_THEME_INTENSE_BLUE = ImVec4(0.220f, 0.380f, 0.839f, 1.0f);

const ImVec4 K_THEME_CYAN = ImVec4(0.267f, 0.816f, 0.910f, 1.0f);
const ImVec4 K_THEME_LIGHT_BLUE = ImVec4(0.631f, 0.749f, 0.929f, 1.0f);
const ImVec4 K_DARKEST_BG = ImVec4(0.067f, 0.067f, 0.067f, 1.0f);

const ImVec4 K_YELLOW = ImVec4(0.95f, 0.85f, 0.15f, 1.0f);
const ImVec4 K_BUTTON_BG = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
const ImVec4 K_BUTTON_HOVER = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
const ImVec4 K_PANEL_BG = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
} // namespace

bool NavigationView::renderColoredButton(const char* label, const ImVec2& size,
                                         const ImVec4& base_color, const ImVec4& hover_color)
{
    ImGui::PushStyleColor(ImGuiCol_Button, base_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, base_color);
    const bool is_clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return is_clicked;
}

void NavigationView::InnerSeparator() const
{
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImDrawList* pDraw_list = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    const float y = ImGui::GetCursorScreenPos().y;

    pDraw_list->AddLine(ImVec2(pos.x + kCardPadX, y), ImVec2(pos.x + size.x - kCardPadX, y),
                        ImGui::GetColorU32(kBorder));
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

void NavigationView::render()
{
    ImGui::PushID(this);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float gap_x = ImGui::GetStyle().ItemSpacing.x;

    float right_width = K_RIGHT_PANEL_WIDTH;
    if (avail.x < K_MIN_LEFT_WIDTH + right_width + gap_x)
    {
        right_width = avail.x - K_MIN_LEFT_WIDTH - gap_x;
    }
    right_width = std::clamp(right_width, K_MIN_RIGHT_WIDTH, K_RIGHT_PANEL_WIDTH);
    if (avail.x < K_MIN_LEFT_WIDTH + K_MIN_RIGHT_WIDTH + gap_x)
    {
        std::max(avail.x * 0.35f, 1.0f);
    }

    float left_width = avail.x - right_width - gap_x;
    left_width = std::max(left_width, 1.0f);

    renderLeftColumn(left_width);

    ImGui::SameLine(0.0f, gap_x);
    renderRightColumn(right_width);

    ImGui::PopID();
}

void NavigationView::renderLeftColumn(float width) const
{
    if (ImGui::BeginChild("LeftColumn", ImVec2(width, 0.0f), false, kNoScrollFlags))
    {
        const float gap_y = ImGui::GetStyle().ItemSpacing.y;
        const ImVec2 avail = ImGui::GetContentRegionAvail();

        float map_height = avail.y;
        map_height = std::max(map_height, 1.0f);

        NavigationView::renderMapContainer(map_height);
    }

    ImGui::EndChild();
}

void NavigationView::renderRightColumn(float width)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, K_PANEL_BG);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(K_OUTER_PAD_X, K_OUTER_PAD_Y));

    if (ImGui::BeginChild("RightColumn", ImVec2(width, 0.0f), true, kNoScrollFlags))
    {
        renderStatus();
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        renderEstimatedKinematics();
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        renderWaypointEditor();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void NavigationView::renderMapContainer(float height) const
{
    if (ImGui::BeginChild("MapContainer", ImVec2(0.0f, height), true, kNoScrollFlags))
    {
    }

    ImGui::EndChild();
}

void NavigationView::renderStatus()
{
    bool is_auto = (state_.status_type == statusState_E::AUTO);

    ImVec4 current_bg;
    ImVec4 current_text_border;
    ImVec4 current_active_btn;

    if (is_auto)
    {
        current_bg = K_THEME_DARK_BLUE;
        current_text_border = K_THEME_BLUE;
        current_active_btn = K_THEME_INTENSE_BLUE;
    }
    else
    {
        current_bg = K_THEME_BROWN;
        current_text_border = K_THEME_ORANGE;
        current_active_btn = K_THEME_RED_ORANGE;
    }

    ImGui::PushStyleColor(ImGuiCol_Border, current_text_border);

    if (BeginCard("StatusCard", ImVec2(0.0f, 95.0f)))
    {

        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                                                  ImGui::GetColorU32(current_bg),
                                                  ImGui::GetStyle().ChildRounding);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        const char* pState_str = toString(state_.status_type);

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - 60.0f);

        ImGui::TextColored(current_text_border, ICON_FA_HAND "STATUS:");
        ImGui::SameLine();
        ImGui::TextColored(current_text_border, "%s", pState_str);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        float btn_w = (ImGui::GetWindowWidth() - (kCardPadX * 2.0f)) * 0.5f;
        ImGui::SetCursorPosX(kCardPadX);

        ImGui::PushStyleColor(ImGuiCol_Text, current_text_border);

        ImVec4 auto_color = is_auto ? current_active_btn : K_BUTTON_BG;
        if (renderColoredButton("AUTO", ImVec2(btn_w, 35.0f), auto_color, auto_color))
        {
            state_.status_type = statusState_E::AUTO;
        }

        ImGui::SameLine(0.0f, 0.0f);

        ImVec4 manual_color = (!is_auto) ? current_active_btn : K_BUTTON_BG;
        if (renderColoredButton("MANUAL", ImVec2(btn_w, 35.0f), manual_color, manual_color))
        {
            state_.status_type = statusState_E::MANUAL;
        }

        ImGui::PopStyleColor();
    }
    EndCard();
    ImGui::PopStyleColor();
}

void NavigationView::renderEstimatedKinematics() const
{
    if (BeginCard("KinematicsCard", ImVec2(0.0f, 240.0f)))
    {
        DrawHeader("ESTIMATED KINEMATICS");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        float avail_w = ImGui::GetWindowWidth() - (kCardPadX * 2.0f);
        float box_w = (avail_w - 10.0f) * 0.5f;

        ImGui::SetCursorPosX(kCardPadX);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, K_DARKEST_BG);

        if (ImGui::BeginChild("SpeedBox", ImVec2(box_w, 75.0f), true, kNoScrollFlags))
        {
            ImVec2 label_size = ImGui::CalcTextSize("SPEED");
            ImGui::SetCursorPos(ImVec2((box_w - label_size.x) * 0.5f, 10.0f));
            ImGui::TextColored(kMutedText, "SPEED");

            float old_scale = ImGui::GetFont()->Scale;
            ImGui::SetWindowFontScale(2.0f);

            char val_buf[16];
            snprintf(val_buf, sizeof(val_buf), "%.1f", state_.kinematics.speed);
            ImVec2 val_size = ImGui::CalcTextSize(val_buf);

            ImGui::SetCursorPos(ImVec2((box_w - val_size.x) * 0.5f - 10.0f, 32.0f));
            ImGui::TextColored(K_YELLOW, "%s", val_buf);

            ImGui::SetWindowFontScale(old_scale);

            ImGui::SameLine(0.0f, 2.0f);
            ImGui::SetCursorPosY(44.0f);
            ImGui::TextColored(K_YELLOW, "m/s");
        }
        ImGui::EndChild();

        ImGui::SameLine(0.0f, 10.0f);

        if (ImGui::BeginChild("HeadingBox", ImVec2(box_w, 75.0f), true, kNoScrollFlags))
        {
            ImVec2 label_size = ImGui::CalcTextSize("HEADING");
            ImGui::SetCursorPos(ImVec2((box_w - label_size.x) * 0.5f, 10.0f));
            ImGui::TextColored(kMutedText, "HEADING");

            float old_scale = ImGui::GetFont()->Scale;
            ImGui::SetWindowFontScale(2.0f);

            char head_buf[16];
            snprintf(head_buf, sizeof(head_buf), "%d", state_.kinematics.heading);
            ImVec2 head_size = ImGui::CalcTextSize(head_buf);

            ImGui::SetCursorPos(ImVec2((box_w - head_size.x) * 0.5f - 5.0f, 32.0f));
            ImGui::TextColored(kBlue, "%s", head_buf);

            ImGui::SetWindowFontScale(old_scale);

            ImGui::SameLine(0.0f, 2.0f);
            ImGui::SetCursorPosY(36.0f);

            ImGui::TextColored(K_THEME_CYAN, u8"°");
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        InnerSeparator();

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kTitleText, "GPS / ODOMETRY");
        ImGui::Dummy(ImVec2(0.0f, 6.0f));

        ValueRow(kLabelX, kValueX, kGreen, "LAT:", "%.4f N", state_.kinematics.gps.lat);
        ValueRow(kLabelX, kValueX, kGreen, "LON:", "%.4f W", state_.kinematics.gps.lon);
        ValueRow(kLabelX, kValueX, kGreen, "ALT:", "%.1f m", state_.kinematics.gps.alt_m);
    }
    EndCard();
}

void NavigationView::renderWaypointEditor()
{
    if (BeginCard("WaypointCard", ImVec2(0.0f, 0.0f)))
    {
        DrawHeader("WAYPOINT EDITOR");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        float avail_w = ImGui::GetWindowWidth() - (kCardPadX * 2.0f);
        float btn_w = (avail_w - 20.0f) / 3.0f;

        ImGui::SetCursorPosX(kCardPadX);

        renderColoredButton("ADD", ImVec2(btn_w, 35.0f), K_BUTTON_BG, K_BUTTON_HOVER);
        ImGui::SameLine(0.0f, 10.0f);
        renderColoredButton(u8"CLR", ImVec2(btn_w, 35.0f), K_BUTTON_BG, K_BUTTON_HOVER);
        ImGui::SameLine(0.0f, 10.0f);
        renderColoredButton(u8"SYNC", ImVec2(btn_w, 35.0f), K_BUTTON_BG, K_BUTTON_HOVER);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        for (const auto& pair : state_.editor.waypoints)
        {
            int id = pair.first;
            const waypoint_S& wp = pair.second;

            bool is_active = (wp.state == waypointState_E::ACTIVE);

            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                                  is_active ? K_THEME_BROWN : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Border,
                                  is_active ? K_THEME_ORANGE : ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

            ImGui::SetCursorPosX(kCardPadX);
            ImGui::PushID(id);

            if (ImGui::BeginChild("wp_row", ImVec2(avail_w, 36.0f), true, kNoScrollFlags))
            {

                ImGui::SetCursorPos(ImVec2(10.0f, 10.0f));

                ImGui::TextColored(is_active ? K_THEME_ORANGE : kMutedText, "#%d ", id);
                ImGui::SameLine(0.0f, 0.0f);

                ImGui::TextColored(K_THEME_LIGHT_BLUE, "X:%.1f Y:%.1f", wp.x, wp.y);

                const char* pState_str = toString(wp.state);
                ImVec2 text_size = ImGui::CalcTextSize(pState_str);

                if (is_active)
                {
                    ImVec2 cur_pos = ImVec2(avail_w - text_size.x - 16.0f, 10.5f);
                    ImGui::SetCursorPos(cur_pos);
                    ImVec2 screen_pos = ImGui::GetCursorScreenPos();

                    ImGui::GetWindowDrawList()->AddRectFilled(
                        ImVec2(screen_pos.x - 6.0f, screen_pos.y - 4.0f),
                        ImVec2(screen_pos.x + text_size.x + 6.0f,
                               screen_pos.y + text_size.y + 4.0f),
                        ImGui::GetColorU32(K_THEME_RED_ORANGE), 2.0f);

                    ImGui::TextColored(K_THEME_ORANGE, "%s", pState_str);
                }
                else
                {
                    ImGui::SetCursorPos(ImVec2(avail_w - text_size.x - 10.0f, 10.0f));
                    ImVec4 state_color = (wp.state == waypointState_E::DONE) ? kGreen : kMutedText;
                    ImGui::TextColored(state_color, "%s", pState_str);
                }
            }
            ImGui::EndChild();

            ImGui::PopID();
            ImGui::PopStyleColor(2);

            ImGui::Dummy(ImVec2(0.0f, 2.0f));
        }
    }
    EndCard();
}

const char* NavigationView::toString(statusState_E state)
{
    if (state == statusState_E::AUTO)
        return "AUTO";
    return "MANUAL";
}

const char* NavigationView::toString(waypointState_E state)
{
    if (state == waypointState_E::DONE)
        return "DONE";
    if (state == waypointState_E::ACTIVE)
        return "ACTIVE";
    return "PENDING";
}