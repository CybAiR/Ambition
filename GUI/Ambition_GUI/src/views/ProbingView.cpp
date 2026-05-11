#include "ProbingView.h"

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

const ImVec4 K_PANEL_BG = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
const ImVec4 K_LOG_BASE = ImVec4(0.12f, 0.22f, 0.15f, 1.0f);
const ImVec4 K_LOG_HOVER = ImVec4(0.18f, 0.28f, 0.20f, 1.0f);
const ImVec4 K_LOG_TEXT = ImVec4(0.50f, 0.85f, 0.60f, 1.0f);

const ImVec4 K_RESET_BASE = ImVec4(0.25f, 0.12f, 0.12f, 1.0f);
const ImVec4 K_RESET_HOVER = ImVec4(0.35f, 0.18f, 0.18f, 1.0f);
const ImVec4 K_RESET_TEXT = ImVec4(0.85f, 0.40f, 0.40f, 1.0f);
} // namespace

bool ProbingView::renderColoredButton(const char* label, const ImVec2& size,
                                      const ImVec4& base_color, const ImVec4& hover_color) const
{
    ImGui::PushStyleColor(ImGuiCol_Button, base_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, base_color);

    const bool clicked = ImGui::Button(label, size);

    ImGui::PopStyleColor(3);

    return clicked;
}

void ProbingView::innerSeparator() const
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

void ProbingView::render()
{
    ImGui::PushID(this);

    const ImVec2 avail = ImGui::GetContentRegionAvail();

    if (is_camera_fullscreen_)
    {
        renderLeftColumn(avail.x);
    }
    else
    {
        const float gap_x = ImGui::GetStyle().ItemSpacing.x;

        float right_width = K_RIGHT_PANEL_WIDTH;

        if (avail.x < K_MIN_LEFT_WIDTH + right_width + gap_x)
            right_width = avail.x - K_MIN_LEFT_WIDTH - gap_x;

        right_width = std::clamp(right_width, K_MIN_RIGHT_WIDTH, K_RIGHT_PANEL_WIDTH);

        if (avail.x < K_MIN_LEFT_WIDTH + K_MIN_RIGHT_WIDTH + gap_x)
            right_width = std::max(avail.x * 0.35f, 1.0f);

        float left_width = std::max(avail.x - right_width - gap_x, 1.0f);

        renderLeftColumn(left_width);

        ImGui::SameLine(0.0f, gap_x);
        renderRightColumn(right_width);
    }

    ImGui::PopID();
}

void ProbingView::renderLeftColumn(float width)
{
    if (ImGui::BeginChild("LeftColumn", ImVec2(width, 0.0f), false, kNoScrollFlags))
    {
        const float gap_y = ImGui::GetStyle().ItemSpacing.y;
        const ImVec2 avail = ImGui::GetContentRegionAvail();

        const bool was_fullscreen = is_camera_fullscreen_;
        float camera_height = avail.y;
        camera_height = std::max(camera_height, 1.0f);

        const bool is_fullscreen_toggled = RenderCameraContainer(
            camera_height, "MAIN CAMERA (PROBING)", false, true, was_fullscreen);

        if (is_fullscreen_toggled)
        {
            is_camera_fullscreen_ = !is_camera_fullscreen_;
        }

        if (!was_fullscreen && !is_fullscreen_toggled)
        {
        }
    }

    ImGui::EndChild();
}

void ProbingView::renderRightColumn(float width) const
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, K_PANEL_BG);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(K_OUTER_PAD_X, K_OUTER_PAD_Y));

    if (ImGui::BeginChild("RightColumn", ImVec2(width, 0.0f), true, kNoScrollFlags))
    {
        renderMissionProgressCard(state_.mission);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        renderArmTelemetryCard(state_.arm, state_.gripper);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void ProbingView::renderArmTelemetryCard(const ArmTelemetry& arm,
                                         const GripperTelemetry& gripper) const
{
    if (BeginCard("ArmCard", ImVec2(0.0f, 400.0f)))
    {
        DrawHeader("ARM TELEMETRY");

        struct floatRow_S
        {
            const char* label;
            float value;
        };

        const floatRow_S joints[] = {{"BASE:", arm.base_deg},
                                     {"SHOULDER:", arm.shoulder_deg},
                                     {"ELBOW:", arm.elbow_deg},
                                     {"WRIST PITCH:", arm.wrist_pitch_deg},
                                     {"WRIST ROLL:", arm.wrist_roll_deg}};

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "JOINT ANGLES");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const floatRow_S& row : joints)
        {
            ValueRow(kLabelX, kValueX, kBlue, row.label, "%.0f°", row.value);
        }

        InnerSeparator();

        const floatRow_S position[] = {{"X:", arm.ee_x_m}, {"Y:", arm.ee_y_m}, {"Z:", arm.ee_z_m}};

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "END-EFFECTOR POSITION");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const floatRow_S& row : position)
        {
            ValueRow(kLabelX, kValueX, kPurple, row.label, "%.2f m", row.value);
        }

        InnerSeparator();

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "GRIPPER STATE");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ValueRow(kLabelX, kValueX, kGreen, "STATE:", "%s", toString(gripper.state));

        ValueRow(kLabelX, kValueX, kOrange, "FORCE:", "%.0f N", gripper.force_n);
    }

    EndCard();
}

void ProbingView::renderMissionProgressCard(const missionProgress_S& mission) const
{
    if (BeginCard("MissionCard", ImVec2(0.0f, 160.0f)))
    {
        DrawHeader("MISSION PROGRESS");

        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        float avail_width = ImGui::GetContentRegionAvail().x - kCardPadX - 12.0f;
        ImGui::SetCursorPosX(kCardPadX);

        ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (ImGui::BeginChild("ProgressBox", ImVec2(avail_width, 60.0f), true,
                              ImGuiWindowFlags_NoScrollbar))
        {

            ImGui::SetCursorPos(ImVec2(10.0f, 10.0f));
            ImGui::TextColored(kMutedText, "PROBES COLLECTED");

            ImGui::SetCursorPos(ImVec2(10.0f, 35.0f));
            ImGui::TextColored(kGreen, "TARGET: %d", mission.target);

            float old_scale = ImGui::GetFont()->Scale;
            ImGui::SetWindowFontScale(3.0f);

            char num_buf[16];
            snprintf(num_buf, sizeof(num_buf), "%d", mission.probes_collected);
            ImVec2 text_size = ImGui::CalcTextSize(num_buf);

            float text_x = ImGui::GetWindowWidth() - text_size.x - 15.0f;
            float text_y = (ImGui::GetWindowHeight() - text_size.y) * 0.5f;

            ImGui::SetCursorPos(ImVec2(text_x, text_y));
            ImGui::TextColored(kGreen, "%s", num_buf);

            ImGui::SetWindowFontScale(old_scale);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        ImGui::SetCursorPosX(kCardPadX);
        float gap = ImGui::GetStyle().ItemSpacing.x;
        float btn_width = (avail_width - gap) * 0.5f;

        ImGui::PushStyleColor(ImGuiCol_Text, K_LOG_TEXT);
        if (renderColoredButton("+ LOG PROBE", ImVec2(btn_width, 32.0f), K_LOG_BASE, K_LOG_HOVER))
        {
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, K_RESET_TEXT);
        if (renderColoredButton("RESET", ImVec2(btn_width, 32.0f), K_RESET_BASE, K_RESET_HOVER))
        {
        }
        ImGui::PopStyleColor();
    }

    EndCard();
}

const char* ProbingView::toString(GripperState state)
{
    switch (state)
    {
    case GripperState::Open:
        return "OPEN";
    case GripperState::Closed:
        return "CLOSED";
    case GripperState::Holding:
        return "HOLDING";
    case GripperState::Error:
        return "ERROR";
    }

    return "UNKNOWN";
}