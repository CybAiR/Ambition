#include "ProbingView.h"

#include <imgui.h>
#include <cstdio> // Added for snprintf
#include <cstdarg>

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

    // Helper added to colorize the custom buttons
    bool ColoredButton(
        const char* label,
        const ImVec2& size,
        const ImVec4& base,
        const ImVec4& hover
    )
    {
        ImGui::PushStyleColor(ImGuiCol_Button, base);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, base);

        const bool clicked = ImGui::Button(label, size);

        ImGui::PopStyleColor(3);

        return clicked;
    }
}

ProbingView::ProbingView(ImGuiWindowFlags flags) : View(flags) {}

void ProbingView::InnerSeparator() const
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

void ProbingView::Render()
{
    Render(state_);
}

void ProbingView::Render(const State& state) const
{
    ImGui::PushID(this);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float gap_x = ImGui::GetStyle().ItemSpacing.x;

    float right_width = kRightPanelWidth;

    if (avail.x < kMinLeftWidth + right_width + gap_x) {
        right_width = avail.x - kMinLeftWidth - gap_x;
    }

    right_width = Clamp(right_width, kMinRightWidth, kRightPanelWidth);

    if (avail.x < kMinLeftWidth + kMinRightWidth + gap_x) {
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

void ProbingView::RenderRightColumn(const State& state, float width) const
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kOuterPadX, kOuterPadY));

    if (ImGui::BeginChild(
        "RightColumn",
        ImVec2(width, 0.0f),
        true,
        kNoScrollFlags
    )) {
        RenderMissionProgressCard(state.mission);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        RenderArmTelemetryCard(state.arm, state.gripper);
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void ProbingView::RenderArmTelemetryCard(
    const ArmTelemetry& arm,
    const GripperTelemetry& gripper
) const
{
    if (BeginCard("ArmCard", ImVec2(0.0f, 400.0f))) {
        DrawHeader("ARM TELEMETRY");

        struct FloatRow {
            const char* label;
            float value;
        };

        const FloatRow joints[] = {
            { "BASE:", arm.base_deg },
            { "SHOULDER:", arm.shoulder_deg },
            { "ELBOW:", arm.elbow_deg },
            { "WRIST PITCH:", arm.wrist_pitch_deg },
            { "WRIST ROLL:", arm.wrist_roll_deg }
        };

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "JOINT ANGLES");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const FloatRow& row : joints) {
            ValueRow(kLabelX, kValueX, kBlue, row.label, "%.0f°", row.value);
        }

        InnerSeparator();

        const FloatRow position[] = {
            { "X:", arm.ee_x_m },
            { "Y:", arm.ee_y_m },
            { "Z:", arm.ee_z_m }
        };

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "END-EFFECTOR POSITION");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const FloatRow& row : position) {
            ValueRow(kLabelX, kValueX, kPurple, row.label, "%.2f m", row.value);
        }

        InnerSeparator();

        ImGui::SetCursorPosX(kCardPadX);
        ImGui::TextColored(kMutedText, "GRIPPER STATE");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ValueRow(
            kLabelX,
            kValueX,
            kGreen,
            "STATE:",
            "%s",
            ToString(gripper.state)
        );

        ValueRow(
            kLabelX,
            kValueX,
            kOrange,
            "FORCE:",
            "%.0f N",
            gripper.force_n
        );
    }

    EndCard();
}

void ProbingView::RenderMissionProgressCard(const MissionProgress& mission) const
{
    if (BeginCard("MissionCard", ImVec2(0.0f, 160.0f))) {
        DrawHeader("MISSION PROGRESS");

        ImGui::Dummy(ImVec2(0.0f, 4.0f)); // Top padding

        float avail_width = ImGui::GetContentRegionAvail().x - kCardPadX - 12.0f;
        ImGui::SetCursorPosX(kCardPadX);

        // 1. The inner bordered box
        ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
        
        if (ImGui::BeginChild("ProgressBox", ImVec2(avail_width, 60.0f), true, ImGuiWindowFlags_NoScrollbar)) {
            
            // Left Text 1
            ImGui::SetCursorPos(ImVec2(10.0f, 10.0f));
            ImGui::TextColored(kMutedText, "PROBES COLLECTED");
            
            // Left Text 2
            ImGui::SetCursorPos(ImVec2(10.0f, 35.0f));
            ImGui::TextColored(kGreen, "TARGET: %d", mission.target);

            // Right Huge Text
            float old_scale = ImGui::GetFont()->Scale;
            ImGui::SetWindowFontScale(3.0f); // Make font 3x larger for the number
            
            char num_buf[16];
            snprintf(num_buf, sizeof(num_buf), "%d", mission.probes_collected);
            ImVec2 text_size = ImGui::CalcTextSize(num_buf);
            
            // Right-align and center vertically
            float text_x = ImGui::GetWindowWidth() - text_size.x - 15.0f;
            float text_y = (ImGui::GetWindowHeight() - text_size.y) * 0.5f;
            
            ImGui::SetCursorPos(ImVec2(text_x, text_y));
            ImGui::TextColored(kGreen, "%s", num_buf);
            
            ImGui::SetWindowFontScale(old_scale); // Always restore font scale!
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        // 2. The Buttons
        ImGui::SetCursorPosX(kCardPadX);
        float gap = ImGui::GetStyle().ItemSpacing.x;
        float btn_width = (avail_width - gap) * 0.5f;

        // Colors carefully matched to your image
        ImVec4 kLogBase  = ImVec4(0.12f, 0.22f, 0.15f, 1.0f);
        ImVec4 kLogHover = ImVec4(0.18f, 0.28f, 0.20f, 1.0f);
        ImVec4 kLogText  = ImVec4(0.50f, 0.85f, 0.60f, 1.0f);

        ImVec4 kResetBase  = ImVec4(0.25f, 0.12f, 0.12f, 1.0f);
        ImVec4 kResetHover = ImVec4(0.35f, 0.18f, 0.18f, 1.0f);
        ImVec4 kResetText  = ImVec4(0.85f, 0.40f, 0.40f, 1.0f);

        // Render Log Button (pushing custom text color first)
        ImGui::PushStyleColor(ImGuiCol_Text, kLogText);
        if (ColoredButton("+ LOG PROBE", ImVec2(btn_width, 32.0f), kLogBase, kLogHover)) {
            // Action goes here
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Render Reset Button
        ImGui::PushStyleColor(ImGuiCol_Text, kResetText);
        if (ColoredButton("RESET", ImVec2(btn_width, 32.0f), kResetBase, kResetHover)) {
            // Action goes here
        }
        ImGui::PopStyleColor();
    }

    EndCard();
}

const char* ProbingView::ToString(GripperState state) noexcept
{
    switch (state) {
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