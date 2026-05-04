#include "MaintenanceView.h"

#include <cstdarg>
#include <imgui.h>

namespace
{
constexpr float kRightPanelWidth = 400.0f;
constexpr float kMinLeftWidth = 240.0f;
constexpr float kMinRightWidth = 180.0f;

constexpr float kOuterPadX = 10.0f;
constexpr float kOuterPadY = 10.0f;

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
} // namespace

// 1. Added the missing constructor definition
MaintenanceView::MaintenanceView(ImGuiWindowFlags flags) : View(flags)
{
}

// 2. Added the missing InnerSeparator definition
void MaintenanceView::InnerSeparator() const
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

// 3. Added the pure virtual override required by the base class
void MaintenanceView::Render()
{
    Render(state_);
}

void MaintenanceView::Render(const State& state) const
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

    // 4. Fixed the SameLine() crash by rendering an empty dummy column first
    ImGui::BeginChild("LeftColumnPlaceholder", ImVec2(left_width, 0.0f), false, kNoScrollFlags);
    ImGui::EndChild();

    ImGui::SameLine(0.0f, gap_x);
    RenderRightColumn(state, right_width);

    ImGui::PopID();
}

void MaintenanceView::RenderRightColumn(const State& state, float width) const
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kOuterPadX, kOuterPadY));

    if (ImGui::BeginChild("RightColumn", ImVec2(width, 0.0f), true, kNoScrollFlags))
    {
        ImGui::TextColored(kTitleText, "ARM CONTROL & DIAGNOSTICS");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        RenderArmTelemetryCard(state.arm, state.gripper);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        RenderJointDiagnostics(state.joints);
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void MaintenanceView::RenderArmTelemetryCard(const ArmTelemetry& arm,
                                             const GripperTelemetry& gripper) const
{
    if (BeginCard("ArmCard", ImVec2(0.0f, 400.0f)))
    {
        DrawHeader("ARM TELEMETRY");

        struct FloatRow
        {
            const char* label;
            float value;
        };

        const FloatRow joints[] = {{"BASE:", arm.base_deg},
                                   {"SHOULDER:", arm.shoulder_deg},
                                   {"ELBOW:", arm.elbow_deg},
                                   {"WRIST PITCH:", arm.wrist_pitch_deg},
                                   {"WRIST ROLL:", arm.wrist_roll_deg}};

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "JOINT ANGLES");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const FloatRow& row : joints)
        {
            ValueRow(kLabelX, kValueX, kBlue, row.label, "%.0f°", row.value);
        }

        InnerSeparator();

        const FloatRow position[] = {{"X:", arm.ee_x_m}, {"Y:", arm.ee_y_m}, {"Z:", arm.ee_z_m}};

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "END-EFFECTOR POSITION");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const FloatRow& row : position)
        {
            ValueRow(kLabelX, kValueX, kPurple, row.label, "%.2f m", row.value);
        }

        InnerSeparator();

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "GRIPPER STATE");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ValueRow(kLabelX, kValueX, kGreen, "STATE:", "%s", ToString(gripper.state));

        ValueRow(kLabelX, kValueX, kOrange, "FORCE:", "%.0f N", gripper.force_n);
    }

    EndCard();
}

void MaintenanceView::RenderJointDiagnostics(const JointDiagnostics& joints) const
{
    // The height is hardcoded to 140.0f here, but you can adjust it or use 0.0f to auto-fit
    if (BeginCard("JointsCard", ImVec2(0.0f, 140.0f)))
    {
        DrawHeader("JOINT DIAGNOSTICS");

        // Layout constants for right-aligning the values
        const float temp_x = ImGui::GetWindowWidth() - 140.0f;
        const float current_x = ImGui::GetWindowWidth() - 65.0f;

        // Colors specific to this card
        const ImVec4 kWarningRed = ImVec4(0.90f, 0.35f, 0.35f, 1.0f);
        const ImVec4 kCurrentYellow = ImVec4(0.95f, 0.85f, 0.15f, 1.0f);

        struct JointRow
        {
            const char* label;
            float temp;
            float current;
        };

        const JointRow rows[] = {{"Base", joints.base_deg_cel, joints.base_current_a},
                                 {"Shoulder", joints.shoulder_deg_cel, joints.shoulder_current_a},
                                 {"Elbow", joints.elbow_deg_cel, joints.elbow_current_a}};

        ImGui::Dummy(ImVec2(0.0f, 4.0f)); // Top padding below the header

        for (const auto& row : rows)
        {
            // 1. Joint Name
            ImGui::SetCursorPosX(kCardPadX);
            ImGui::TextColored(kTitleText, "%s", row.label);

            // 2. Temperature (Turns red if 65 degrees or hotter)
            ImGui::SameLine(temp_x);
            const ImVec4 temp_color = (row.temp >= 65.0f) ? kWarningRed : kMutedText;

            // Note: Using standard UTF-8 symbols for icons.
            ImGui::TextColored(temp_color, u8"%.0f°C", row.temp);

            // 3. Current
            ImGui::SameLine(current_x);
            ImGui::TextColored(kCurrentYellow, u8"%.1fA", row.current);

            ImGui::Dummy(ImVec2(0.0f, 6.0f)); // Spacing between rows
        }
    }

    EndCard();
}

const char* MaintenanceView::ToString(GripperState state) noexcept
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