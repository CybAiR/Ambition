#include "ScienceView.h"

#include <imgui.h>

#include <cstdarg>

namespace
{
    constexpr float kRightPanelWidth = 300.0f;
    constexpr float kMinLeftWidth = 240.0f;
    constexpr float kMinRightWidth = 180.0f;

    constexpr float kOuterPadX = 10.0f;
    constexpr float kOuterPadY = 10.0f;

    constexpr float kHeaderHeight = 28.0f;
    constexpr float kGpsCardHeight = 116.0f;
    constexpr float kBottomPanelHeight = 170.0f;

    constexpr float kCardPadX = 12.0f;
    constexpr float kSectionPadX = 16.0f;

    constexpr float kLabelX = 30.0f;
    constexpr float kValueX = 145.0f;

    constexpr float kSlotButtonSize = 32.0f;
    constexpr float kSlotCellWidth = 72.0f;

    const ImVec4 kPanelBg = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    const ImVec4 kCardBg = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    const ImVec4 kHeaderBg = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
    const ImVec4 kBorder = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);

    const ImVec4 kTitleText = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
    const ImVec4 kMutedText = ImVec4(0.62f, 0.62f, 0.62f, 1.0f);
    const ImVec4 kSectionText = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);

    const ImVec4 kGreen = ImVec4(0.55f, 0.95f, 0.65f, 1.0f);
    const ImVec4 kBlue = ImVec4(0.45f, 0.65f, 1.00f, 1.0f);
    const ImVec4 kPurple = ImVec4(0.85f, 0.65f, 1.00f, 1.0f);
    const ImVec4 kOrange = ImVec4(1.00f, 0.70f, 0.25f, 1.0f);

    const ImVec4 kDrillStateText = ImVec4(0.30f, 0.50f, 0.90f, 1.0f);

    const ImVec4 kStartButton = ImVec4(0.15f, 0.35f, 0.15f, 1.0f);
    const ImVec4 kStartButtonHover = ImVec4(0.20f, 0.45f, 0.20f, 1.0f);

    const ImVec4 kStopButton = ImVec4(0.45f, 0.15f, 0.15f, 1.0f);
    const ImVec4 kStopButtonHover = ImVec4(0.55f, 0.20f, 0.20f, 1.0f);

    const ImVec4 kSlotGreen = ImVec4(0.18f, 0.49f, 0.20f, 1.0f);
    const ImVec4 kSlotGreenFilled = ImVec4(0.18f, 0.49f, 0.20f, 0.4f);
    const ImVec4 kSlotGreenHover = ImVec4(0.18f, 0.49f, 0.20f, 0.6f);
    const ImVec4 kTransparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    constexpr ImGuiWindowFlags kNoScrollFlags =
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    constexpr ImGuiWindowFlags kNoScrollNoBgFlags =
        kNoScrollFlags |
        ImGuiWindowFlags_NoBackground;

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

    void DrawHeader(
        const char* title,
        const ImVec4& text_color = kTitleText,
        float text_x = kCardPadX,
        float text_y = 5.0f
    )
    {
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

    bool BeginCard(const char* id, const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, kCardBg);
        ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        return ImGui::BeginChild(id, size, true, kNoScrollFlags);
    }

    void EndCard()
    {
        ImGui::EndChild();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }

    void ValueRow(
        float label_x,
        float value_x,
        const ImVec4& value_color,
        const char* label,
        const char* fmt,
        ...
    ) IM_FMTARGS(5);

    void ValueRow(
        float label_x,
        float value_x,
        const ImVec4& value_color,
        const char* label,
        const char* fmt,
        ...
    )
    {
        ImGui::SetCursorPosX(label_x);
        ImGui::TextColored(kMutedText, "%s", label);

        ImGui::SameLine(value_x);

        va_list args;
        va_start(args, fmt);
        ImGui::TextColoredV(value_color, fmt, args);
        va_end(args);
    }

    void InnerSeparator()
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

ScienceView::Action ScienceView::Render()
{
    return Render(state_);
}

ScienceView::Action ScienceView::Render(const State& state) const
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

    const Action action = RenderLeftColumn(state, left_width);

    ImGui::SameLine(0.0f, gap_x);
    RenderRightColumn(state, right_width);

    ImGui::PopID();

    return action;
}

ScienceView::Action ScienceView::RenderLeftColumn(
    const State& state,
    float width
) const
{
    Action action = Action::None;

    if (ImGui::BeginChild("LeftColumn", ImVec2(width, 0.0f), false, kNoScrollFlags)) {
        const float gap_y = ImGui::GetStyle().ItemSpacing.y;
        const ImVec2 avail = ImGui::GetContentRegionAvail();

        float camera_height = avail.y - kBottomPanelHeight - gap_y;
        camera_height = Max(camera_height, 1.0f);

        RenderCameraContainer(camera_height);
        action = RenderBottomPanel(state);
    }

    ImGui::EndChild();

    return action;
}

void ScienceView::RenderCameraContainer(float height) const
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

ScienceView::Action ScienceView::RenderBottomPanel(const State& state) const
{
    Action action = Action::None;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanelBg);

    if (ImGui::BeginChild(
        "BottomPanel",
        ImVec2(0.0f, kBottomPanelHeight),
        true,
        kNoScrollFlags
    )) {
        DrawHeader("SAMPLE DATA", kTitleText, 10.0f, 5.0f);

        ImGui::SetCursorPos(ImVec2(0.0f, kHeaderHeight + 6.0f));

        const ImVec2 region = ImGui::GetContentRegionAvail();
        const ImVec2 content_pos = ImGui::GetCursorScreenPos();

        const float section_width = Max(region.x / 3.0f, 1.0f);
        const float last_section_width = Max(region.x - section_width * 2.0f, 1.0f);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImU32 line_color = ImGui::GetColorU32(kBorder);

        draw_list->AddLine(
            ImVec2(content_pos.x + section_width, content_pos.y),
            ImVec2(content_pos.x + section_width, content_pos.y + region.y),
            line_color
        );

        draw_list->AddLine(
            ImVec2(content_pos.x + section_width * 2.0f, content_pos.y),
            ImVec2(content_pos.x + section_width * 2.0f, content_pos.y + region.y),
            line_color
        );

        action = RenderContainersSection(state, section_width);

        ImGui::SameLine(0.0f, 0.0f);

        const Action drill_action = RenderDrillSection(state.drill, section_width);

        if (action == Action::None)
            action = drill_action;

        ImGui::SameLine(0.0f, 0.0f);

        RenderScaleSection(state.scale_weight_g, last_section_width);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    return action;
}

ScienceView::Action ScienceView::RenderContainersSection(
    const State& state,
    float width
) const
{
    Action action = Action::None;

    if (ImGui::BeginChild(
        "ContainersSection",
        ImVec2(width, 0.0f),
        false,
        kNoScrollNoBgFlags
    )) {
        constexpr float top_reserved_height = 24.0f;
        constexpr float row_gap = 4.0f;

        ImGui::SetCursorPosX(kSectionPadX);
        ImGui::TextColored(kSectionText, "CONTAINERS");

        const float inner_width = Max(width - kSectionPadX * 2.0f, 1.0f);
        const float text_height = ImGui::GetTextLineHeight();

        const float block_height =
            kSlotButtonSize +
            ImGui::GetStyle().ItemSpacing.y +
            text_height;

        const float total_height = block_height * 2.0f + row_gap;

        float y1 =
            top_reserved_height +
            (ImGui::GetWindowHeight() - top_reserved_height - total_height) * 0.5f;

        y1 = Max(y1, top_reserved_height + 2.0f);

        const float y2 = y1 + block_height + row_gap;

        const float x1 =
            kSectionPadX +
            inner_width * 0.25f -
            kSlotCellWidth * 0.5f;

        const float x2 =
            kSectionPadX +
            inner_width * 0.75f -
            kSlotCellWidth * 0.5f;

        const char* labels[4] = {
            "SLOT 1",
            "SLOT 2",
            "SLOT 3",
            "SLOT 4"
        };

        const Action actions[4] = {
            Action::Slot1,
            Action::Slot2,
            Action::Slot3,
            Action::Slot4
        };

        for (int i = 0; i < 4; ++i) {
            const bool right_column = (i % 2) == 1;
            const bool bottom_row = i >= 2;

            const float x = right_column ? x2 : x1;
            const float y = bottom_row ? y2 : y1;

            ImGui::SetCursorPos(ImVec2(Max(x, kSectionPadX), y));
            ImGui::PushID(i);

            if (RenderSlotButton(
                "##slot",
                labels[i],
                state.container_filled[i],
                kSlotCellWidth
            )) {
                action = actions[i];
            }

            ImGui::PopID();
        }
    }

    ImGui::EndChild();

    return action;
}

ScienceView::Action ScienceView::RenderDrillSection(
    const DrillTelemetry& drill,
    float width
) const
{
    Action action = Action::None;

    if (ImGui::BeginChild(
        "DrillSection",
        ImVec2(width, 0.0f),
        false,
        kNoScrollNoBgFlags
    )) {
        constexpr float value_column_width = 80.0f;
        constexpr float min_value_offset = 75.0f;
        constexpr float button_gap = 10.0f;

        const float inner_width = Max(width - kSectionPadX * 2.0f, 1.0f);

        ImGui::SetCursorPosX(kSectionPadX);
        ImGui::TextColored(kSectionText, "DRILL CONTROL");
        ImGui::Spacing();

        float value_x = width - kSectionPadX - value_column_width;
        value_x = Max(value_x, kSectionPadX + min_value_offset);

        ImGui::SetCursorPosX(kSectionPadX);
        ImGui::Text("STATE:");
        ImGui::SameLine(value_x);
        ImGui::TextColored(kDrillStateText, "%s", ToString(drill.state));

        ImGui::SetCursorPosX(kSectionPadX);
        ImGui::Text("DEPTH:");
        ImGui::SameLine(value_x);
        ImGui::Text("%.0f cm", drill.depth_cm);

        ImGui::Spacing();
        ImGui::Spacing();

        const float button_width = Max((inner_width - button_gap) * 0.5f, 1.0f);

        ImGui::SetCursorPosX(kSectionPadX);

        if (ColoredButton(
            "START",
            ImVec2(button_width, 40.0f),
            kStartButton,
            kStartButtonHover
        )) {
            action = Action::DrillStart;
        }

        ImGui::SameLine(0.0f, button_gap);

        if (ColoredButton(
            "STOP",
            ImVec2(button_width, 40.0f),
            kStopButton,
            kStopButtonHover
        )) {
            action = Action::DrillStop;
        }
    }

    ImGui::EndChild();

    return action;
}

void ScienceView::RenderScaleSection(float scale_weight_g, float width) const
{
    if (ImGui::BeginChild(
        "ScaleSection",
        ImVec2(width, 0.0f),
        false,
        kNoScrollNoBgFlags
    )) {
        const float text_height = ImGui::GetTextLineHeight();
        const float y = Max((ImGui::GetWindowHeight() - text_height) * 0.5f, 0.0f);

        ImGui::SetCursorPos(ImVec2(kSectionPadX, y));

        ImGui::TextColored(kSectionText, "SCALE:");
        ImGui::SameLine();
        ImGui::Text("%.1f g", scale_weight_g);
    }

    ImGui::EndChild();
}

void ScienceView::RenderRightColumn(const State& state, float width) const
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kOuterPadX, kOuterPadY));

    if (ImGui::BeginChild(
        "RightColumn",
        ImVec2(width, 0.0f),
        true,
        kNoScrollFlags
    )) {
        ImGui::TextColored(kTitleText, "SENSORS & KINEMATICS");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        RenderGpsCard(state.gps);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        RenderArmTelemetryCard(state.arm, state.gripper);
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

bool ScienceView::RenderSlotButton(
    const char* id,
    const char* label,
    bool is_filled,
    float cell_width
)
{
    ImGui::BeginGroup();

    const float start_x = ImGui::GetCursorPosX();

    const float button_x =
        start_x +
        Max((cell_width - kSlotButtonSize) * 0.5f, 0.0f);

    ImGui::SetCursorPosX(button_x);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, kSlotButtonSize * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        is_filled ? kSlotGreenFilled : kTransparent
    );

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kSlotGreenHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kSlotGreen);
    ImGui::PushStyleColor(ImGuiCol_Border, kSlotGreen);

    const bool clicked = ImGui::Button(
        id,
        ImVec2(kSlotButtonSize, kSlotButtonSize)
    );

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    const ImVec2 text_size = ImGui::CalcTextSize(label);

    const float text_x =
        start_x +
        Max((cell_width - text_size.x) * 0.5f, 0.0f);

    ImGui::SetCursorPosX(text_x);
    ImGui::TextColored(kSectionText, "%s", label);

    ImGui::EndGroup();

    return clicked;
}

void ScienceView::RenderGpsCard(const GpsOdometry& gps) const
{
    if (BeginCard("GpsCard", ImVec2(0.0f, kGpsCardHeight))) {
        DrawHeader("GPS / ODOMETRY");

        ValueRow(kLabelX, kValueX, kGreen, "LAT:", "%.4f N", gps.lat);
        ValueRow(kLabelX, kValueX, kGreen, "LON:", "%.4f W", gps.lon);
        ValueRow(kLabelX, kValueX, kGreen, "ALT:", "%.1f m", gps.alt_m);
    }

    EndCard();
}

void ScienceView::RenderArmTelemetryCard(
    const ArmTelemetry& arm,
    const GripperTelemetry& gripper
) const
{
    if (BeginCard("ArmCard", ImVec2(0.0f, 0.0f))) {
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

const char* ScienceView::ToString(DrillState state) noexcept
{
    switch (state) {
    case DrillState::Idle:
        return "IDLE";

    case DrillState::Running:
        return "RUNNING";

    case DrillState::Error:
        return "ERROR";
    }

    return "UNKNOWN";
}

const char* ScienceView::ToString(GripperState state) noexcept
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