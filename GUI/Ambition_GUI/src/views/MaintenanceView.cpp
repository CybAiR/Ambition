#include "MaintenanceView.h"

#include <algorithm>
#include <cstdarg>
#include <imgui.h>

namespace
{
constexpr float K_RIGHT_PANEL_WIDTH = 400.0f;
constexpr float K_MIN_LEFT_WIDTH = 240.0f;
constexpr float K_MIN_RIGHT_WIDTH = 180.0f;

constexpr float K_OUTER_PAD_X = 10.0f;
constexpr float K_OUTER_PAD_Y = 10.0f;

const ImVec4 K_PANEL_BG = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
const ImVec4 K_WARNING_RED = ImVec4(0.90f, 0.35f, 0.35f, 1.0f);
const ImVec4 K_CURRENT_YELLOW = ImVec4(0.95f, 0.85f, 0.15f, 1.0f);

} // namespace

void MaintenanceView::innerSeparator() const
{
    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    const float y = ImGui::GetCursorScreenPos().y;

    draw_list->AddLine(ImVec2(pos.x + kCardPadX, y), ImVec2(pos.x + size.x - kCardPadX, y),
                       ImGui::GetColorU32(kBorder) // kBorder comes from View.h
    );

    ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

void MaintenanceView::render()
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

void MaintenanceView::renderLeftColumn(float width)
{
    if (ImGui::BeginChild("LeftColumn", ImVec2(width, 0.0f), false, kNoScrollFlags))
    {
        const float gap_y = ImGui::GetStyle().ItemSpacing.y;
        const ImVec2 avail = ImGui::GetContentRegionAvail();

        const bool was_fullscreen = is_camera_fullscreen_;
        float camera_height = avail.y;
        camera_height = std::max(camera_height, 1.0f);

        const bool is_fullscreen_toggled = RenderCameraContainer(
            camera_height, "MAIN CAMERA (MAINTENANCE)", false, true, was_fullscreen);

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

void MaintenanceView::renderRightColumn(float width) const
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, K_PANEL_BG);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(K_OUTER_PAD_X, K_OUTER_PAD_Y));

    if (ImGui::BeginChild("RightColumn", ImVec2(width, 0.0f), true, kNoScrollFlags))
    {
        ImGui::TextColored(kTitleText, "ARM CONTROL & DIAGNOSTICS");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        renderArmTelemetryCard(state_.arm, state_.gripper);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        renderJointDiagnostics(state_.joints);
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void MaintenanceView::renderArmTelemetryCard(const ArmTelemetry& arm,
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

void MaintenanceView::renderJointDiagnostics(const jointDiagnostics_S& joints) const
{
    if (BeginCard("JointsCard", ImVec2(0.0f, 140.0f)))
    {
        DrawHeader("JOINT DIAGNOSTICS");

        const float temp_x = ImGui::GetWindowWidth() - 140.0f;
        const float current_x = ImGui::GetWindowWidth() - 65.0f;

        struct jointRow_S
        {
            const char* label;
            float temp;
            float current;
        };

        const jointRow_S rows[] = {{"Base", joints.base_deg_cel, joints.base_current_a},
                                   {"Shoulder", joints.shoulder_deg_cel, joints.shoulder_current_a},
                                   {"Elbow", joints.elbow_deg_cel, joints.elbow_current_a}};

        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const auto& row : rows)
        {
            ImGui::SetCursorPosX(kCardPadX);
            ImGui::TextColored(kTitleText, "%s", row.label);

            ImGui::SameLine(temp_x);
            const ImVec4 temp_color = (row.temp >= 65.0f) ? K_WARNING_RED : kMutedText;

            ImGui::TextColored(temp_color, u8"%.0f°C", row.temp);

            ImGui::SameLine(current_x);
            ImGui::TextColored(K_CURRENT_YELLOW, u8"%.1fA", row.current);

            ImGui::Dummy(ImVec2(0.0f, 6.0f));
        }
    }

    EndCard();
}

const char* MaintenanceView::toString(GripperState state)
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