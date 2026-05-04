#include "NavigationView.h"

#include <cstdarg>
#include <cstdio>
#include <imgui.h>

namespace
{
constexpr float kRightPanelWidth = 400.0f;
constexpr float kMinLeftWidth = 240.0f;
constexpr float kMinRightWidth = 180.0f;

constexpr float kOuterPadX = 10.0f;
constexpr float kOuterPadY = 10.0f;

// --- MANUAL / ORANGE THEME ---
const ImVec4 kThemeBrown = ImVec4(0.227f, 0.114f, 0.063f, 1.0f);
const ImVec4 kThemeOrange = ImVec4(1.000f, 0.678f, 0.420f, 1.0f);
const ImVec4 kThemeRedOrange = ImVec4(0.929f, 0.294f, 0.114f, 1.0f);

// --- AUTO / BLUE THEME ---
const ImVec4 kThemeDarkBlue = ImVec4(0.063f, 0.114f, 0.227f, 1.0f);
const ImVec4 kThemeBlue = ImVec4(0.420f, 0.678f, 1.000f, 1.0f);
const ImVec4 kThemeIntenseBlue = ImVec4(0.220f, 0.380f, 0.839f, 1.0f);

// --- GLOBAL COLORS ---
const ImVec4 kThemeCyan = ImVec4(0.267f, 0.816f, 0.910f, 1.0f);
const ImVec4 kThemeLightBlue = ImVec4(0.631f, 0.749f, 0.929f, 1.0f);
const ImVec4 kDarkestBg = ImVec4(0.067f, 0.067f, 0.067f, 1.0f);

const ImVec4 kYellow = ImVec4(0.95f, 0.85f, 0.15f, 1.0f);
const ImVec4 kButtonBg = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
const ImVec4 kButtonHover = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
const ImVec4 kPanelBg = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);

float Max(float a, float b) noexcept
{
    return a > b ? a : b;
}
float Clamp(float value, float min_value, float max_value) noexcept
{
    if (value < min_value)
        return min_value;
    if (value > max_value)
        return max_value;
    return value;
}

bool ColoredButton(const char* label, const ImVec2& size, const ImVec4& base, const ImVec4& hover)
{
    ImGui::PushStyleColor(ImGuiCol_Button, base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, base);
    const bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return clicked;
}
} // namespace

NavigationView::NavigationView(ImGuiWindowFlags flags) : View(flags)
{
}

void NavigationView::InnerSeparator() const
{
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    const float y = ImGui::GetCursorScreenPos().y;

    draw_list->AddLine(ImVec2(pos.x + kCardPadX, y), ImVec2(pos.x + size.x - kCardPadX, y),
                       ImGui::GetColorU32(kBorder));
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

void NavigationView::Render()
{
    Render(state_);
}

void NavigationView::Render(State& state)
{
    ImGui::PushID(this);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float gap_x = ImGui::GetStyle().ItemSpacing.x;

    float right_width = kRightPanelWidth;
    if (avail.x < kMinLeftWidth + right_width + gap_x)
    {
        right_width = avail.x - kMinLeftWidth - gap_x;
    }
    right_width = Clamp(right_width, kMinRightWidth, kRightPanelWidth);
    if (avail.x < kMinLeftWidth + kMinRightWidth + gap_x)
    {
        right_width = Max(avail.x * 0.35f, 1.0f);
    }

    float left_width = avail.x - right_width - gap_x;
    left_width = Max(left_width, 1.0f);

    ImGui::BeginChild("LeftColumnPlaceholder", ImVec2(left_width, 0.0f), false, kNoScrollFlags);
    ImGui::EndChild();

    ImGui::SameLine(0.0f, gap_x);
    RenderRightColumn(state, right_width);

    ImGui::PopID();
}

void NavigationView::RenderRightColumn(State& state, float width)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kOuterPadX, kOuterPadY));

    if (ImGui::BeginChild("RightColumn", ImVec2(width, 0.0f), true, kNoScrollFlags))
    {
        RenderStatus(state.status);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        RenderEstimatedKinematics(state.kinematics);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        RenderWaypointEditor(state.editor);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void NavigationView::RenderStatus(Status& status)
{
    bool is_auto = (status.type == StatusState::AUTO);

    ImVec4 current_bg = is_auto ? kThemeDarkBlue : kThemeBrown;
    ImVec4 current_text_border = is_auto ? kThemeBlue : kThemeOrange;
    ImVec4 current_active_btn = is_auto ? kThemeIntenseBlue : kThemeRedOrange;

    ImGui::PushStyleColor(ImGuiCol_Border, current_text_border);

    if (BeginCard("StatusCard", ImVec2(0.0f, 95.0f)))
    {

        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                                                  ImGui::GetColorU32(current_bg),
                                                  ImGui::GetStyle().ChildRounding);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        const char* state_str = ToString(status.type);
        const char* icon = is_auto ? u8"" : u8"";

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - 60.0f);

        ImGui::TextColored(current_text_border, "%s STATUS:", icon);
        ImGui::SameLine();
        ImGui::TextColored(current_text_border, "%s", state_str);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        float btn_w = (ImGui::GetWindowWidth() - (kCardPadX * 2.0f)) * 0.5f;
        ImGui::SetCursorPosX(kCardPadX);

        ImGui::PushStyleColor(ImGuiCol_Text, current_text_border);

        ImVec4 auto_color = is_auto ? current_active_btn : kButtonBg;
        if (ColoredButton("AUTO", ImVec2(btn_w, 35.0f), auto_color, auto_color))
        {
            status.type = StatusState::AUTO;
        }

        ImGui::SameLine(0.0f, 0.0f);

        ImVec4 manual_color = (!is_auto) ? current_active_btn : kButtonBg;
        if (ColoredButton("MANUAL", ImVec2(btn_w, 35.0f), manual_color, manual_color))
        {
            status.type = StatusState::MANUAL;
        }

        ImGui::PopStyleColor();
    }
    EndCard();
    ImGui::PopStyleColor();
}

void NavigationView::RenderEstimatedKinematics(const EstimatedKinematics& kinematics) const
{
    if (BeginCard("KinematicsCard", ImVec2(0.0f, 240.0f)))
    {
        DrawHeader("ESTIMATED KINEMATICS");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        float avail_w = ImGui::GetWindowWidth() - (kCardPadX * 2.0f);
        float box_w = (avail_w - 10.0f) * 0.5f;

        ImGui::SetCursorPosX(kCardPadX);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, kDarkestBg);

        if (ImGui::BeginChild("SpeedBox", ImVec2(box_w, 75.0f), true, kNoScrollFlags))
        {
            ImVec2 label_size = ImGui::CalcTextSize("SPEED");
            ImGui::SetCursorPos(ImVec2((box_w - label_size.x) * 0.5f, 10.0f));
            ImGui::TextColored(kMutedText, "SPEED");

            float old_scale = ImGui::GetFont()->Scale;
            ImGui::SetWindowFontScale(2.0f);

            char val_buf[16];
            snprintf(val_buf, sizeof(val_buf), "%.1f", kinematics.speed);
            ImVec2 val_size = ImGui::CalcTextSize(val_buf);

            ImGui::SetCursorPos(ImVec2((box_w - val_size.x) * 0.5f - 10.0f, 32.0f));
            ImGui::TextColored(kYellow, "%s", val_buf);

            ImGui::SetWindowFontScale(old_scale);

            ImGui::SameLine(0.0f, 2.0f);
            ImGui::SetCursorPosY(44.0f);
            ImGui::TextColored(kYellow, "m/s");
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
            snprintf(head_buf, sizeof(head_buf), "%d", kinematics.heading);
            ImVec2 head_size = ImGui::CalcTextSize(head_buf);

            ImGui::SetCursorPos(ImVec2((box_w - head_size.x) * 0.5f - 5.0f, 32.0f));
            ImGui::TextColored(kBlue, "%s", head_buf);

            ImGui::SetWindowFontScale(old_scale);

            ImGui::SameLine(0.0f, 2.0f);
            ImGui::SetCursorPosY(36.0f);

            ImGui::TextColored(kThemeCyan, u8"°");
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        InnerSeparator();

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kTitleText, "GPS / ODOMETRY");
        ImGui::Dummy(ImVec2(0.0f, 6.0f));

        ValueRow(kLabelX, kValueX, kGreen, "LAT:", "%.4f N", kinematics.gps.lat);
        ValueRow(kLabelX, kValueX, kGreen, "LON:", "%.4f W", kinematics.gps.lon);
        ValueRow(kLabelX, kValueX, kGreen, "ALT:", "%.1f m", kinematics.gps.alt_m);
    }
    EndCard();
}

void NavigationView::RenderWaypointEditor(WaypointEditor& editor)
{
    if (BeginCard("WaypointCard", ImVec2(0.0f, 0.0f)))
    {
        DrawHeader("WAYPOINT EDITOR");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        float avail_w = ImGui::GetWindowWidth() - (kCardPadX * 2.0f);
        float btn_w = (avail_w - 20.0f) / 3.0f;

        ImGui::SetCursorPosX(kCardPadX);

        ColoredButton("ADD", ImVec2(btn_w, 35.0f), kButtonBg, kButtonHover);
        ImGui::SameLine(0.0f, 10.0f);
        ColoredButton(u8"CLR", ImVec2(btn_w, 35.0f), kButtonBg, kButtonHover);
        ImGui::SameLine(0.0f, 10.0f);
        ColoredButton(u8"SYNC", ImVec2(btn_w, 35.0f), kButtonBg, kButtonHover);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        for (const auto& pair : editor.waypoints)
        {
            int id = pair.first;
            const Waypoint& wp = pair.second;

            bool is_active = (wp.state == WaypointState::ACTIVE);

            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                                  is_active ? kThemeBrown : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Border,
                                  is_active ? kThemeOrange : ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

            ImGui::SetCursorPosX(kCardPadX);
            ImGui::PushID(id);

            if (ImGui::BeginChild("wp_row", ImVec2(avail_w, 36.0f), true, kNoScrollFlags))
            {

                ImGui::SetCursorPos(ImVec2(10.0f, 10.0f));

                ImGui::TextColored(is_active ? kThemeOrange : kMutedText, "#%d ", id);
                ImGui::SameLine(0.0f, 0.0f);

                ImGui::TextColored(kThemeLightBlue, "X:%.1f Y:%.1f", wp.x, wp.y);

                const char* state_str = ToString(wp.state);
                ImVec2 text_size = ImGui::CalcTextSize(state_str);

                if (is_active)
                {
                    // --- CHANGED HERE: Lowered further to 11.0f ---
                    ImVec2 cur_pos = ImVec2(avail_w - text_size.x - 16.0f, 10.5f);
                    ImGui::SetCursorPos(cur_pos);
                    ImVec2 screen_pos = ImGui::GetCursorScreenPos();

                    ImGui::GetWindowDrawList()->AddRectFilled(
                        ImVec2(screen_pos.x - 6.0f, screen_pos.y - 4.0f),
                        ImVec2(screen_pos.x + text_size.x + 6.0f,
                               screen_pos.y + text_size.y + 4.0f),
                        ImGui::GetColorU32(kThemeRedOrange), 2.0f);

                    ImGui::TextColored(kThemeOrange, "%s", state_str);
                }
                else
                {
                    ImGui::SetCursorPos(ImVec2(avail_w - text_size.x - 10.0f, 10.0f));
                    ImVec4 state_color = (wp.state == WaypointState::DONE) ? kGreen : kMutedText;
                    ImGui::TextColored(state_color, "%s", state_str);
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

const char* NavigationView::ToString(StatusState state) noexcept
{
    return state == StatusState::AUTO ? "AUTO" : "MANUAL";
}

const char* NavigationView::ToString(WaypointState state) noexcept
{
    if (state == WaypointState::DONE)
        return "DONE";
    if (state == WaypointState::ACTIVE)
        return "ACTIVE";
    return "PENDING";
}