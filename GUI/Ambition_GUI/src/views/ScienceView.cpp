#include "ScienceView.h"
#include <../fonts/IconsFontAwesome6.h>

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

constexpr float K_BOTTOM_PANEL_HEIGHT = 170.0f;

constexpr float K_SECTION_PAD_X = 16.0f;

constexpr float K_SLOT_BUTTON_SIZE = 32.0f;
constexpr float K_SLOT_CELL_WIDTH = 72.0f;

const ImVec4 K_PANEL_BG = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
const ImVec4 K_DRILL_STATE_TEXT = ImVec4(0.30f, 0.50f, 0.90f, 1.0f);

const ImVec4 K_START_BUTTON = ImVec4(0.15f, 0.35f, 0.15f, 1.0f);
const ImVec4 K_START_BUTTON_HOVER = ImVec4(0.20f, 0.45f, 0.20f, 1.0f);

const ImVec4 K_STOP_BUTTON = ImVec4(0.45f, 0.15f, 0.15f, 1.0f);
const ImVec4 K_STOP_BUTTON_HOVER = ImVec4(0.55f, 0.20f, 0.20f, 1.0f);

const ImVec4 K_SLOT_GREEN = ImVec4(0.18f, 0.49f, 0.20f, 1.0f);
const ImVec4 K_SLOT_GREEN_FILLED = ImVec4(0.18f, 0.49f, 0.20f, 0.4f);
const ImVec4 K_SLOT_GREEN_HOVER = ImVec4(0.18f, 0.49f, 0.20f, 0.6f);
const ImVec4 K_TRANSPARENT = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
} // namespace

ScienceView::action_E ScienceView::render()
{
    ImGui::PushID(this);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    action_E action = action_E::None;

    if (is_camera_fullscreen_)
    {
        action = renderLeftColumn(avail.x);
    }
    else
    {
        const float gap_x = ImGui::GetStyle().ItemSpacing.x;
        float right_width = K_RIGHT_PANEL_WIDTH;

        if (avail.x < K_MIN_LEFT_WIDTH + right_width + gap_x)
        {
            right_width = avail.x - K_MIN_LEFT_WIDTH - gap_x;
        }

        right_width = std::clamp(right_width, K_MIN_RIGHT_WIDTH, K_RIGHT_PANEL_WIDTH);

        if (avail.x < K_MIN_LEFT_WIDTH + K_MIN_RIGHT_WIDTH + gap_x)
        {
            right_width = std::max(avail.x * 0.35f, 1.0f);
        }

        float left_width = avail.x - right_width - gap_x;
        left_width = std::max(left_width, 1.0f);

        action = renderLeftColumn(left_width);

        ImGui::SameLine(0.0f, gap_x);
        renderRightColumn(right_width);
    }

    ImGui::PopID();

    return action;
}

ScienceView::action_E ScienceView::renderLeftColumn(float width)
{
    action_E action = action_E::None;

    if (ImGui::BeginChild("LeftColumn", ImVec2(width, 0.0f), false, kNoScrollFlags))
    {
        const float gap_y = ImGui::GetStyle().ItemSpacing.y;
        const ImVec2 avail = ImGui::GetContentRegionAvail();

        const bool was_fullscreen = is_camera_fullscreen_;
        float camera_height = was_fullscreen ? avail.y : (avail.y - K_BOTTOM_PANEL_HEIGHT - gap_y);
        camera_height = std::max(camera_height, 1.0f);

        const bool is_fullscreen_toggled = renderCameraContainer(
            camera_height, "MAIN CAMERA (SCIENCE)", true, true, was_fullscreen);

        if (is_fullscreen_toggled)
        {
            is_camera_fullscreen_ = !is_camera_fullscreen_;
        }

        if (!was_fullscreen && !is_fullscreen_toggled)
        {
            action = renderBottomPanel();
        }
    }

    ImGui::EndChild();

    return action;
}

ScienceView::action_E ScienceView::renderBottomPanel() const
{
    action_E action = action_E::None;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, K_PANEL_BG);

    if (ImGui::BeginChild("BottomPanel", ImVec2(0.0f, K_BOTTOM_PANEL_HEIGHT), true, kNoScrollFlags))
    {
        drawHeader("SAMPLE DATA");

        ImGui::SetCursorPos(ImVec2(0.0f, K_HEADER_HEIGHT + 6.0f));

        const ImVec2 region = ImGui::GetContentRegionAvail();
        const ImVec2 content_pos = ImGui::GetCursorScreenPos();

        const float section_width = std::max(region.x / 3.0f, 1.0f);
        const float last_section_width = std::max(region.x - section_width * 2.0f, 1.0f);

        ImDrawList* pDraw_list = ImGui::GetWindowDrawList();
        const ImU32 line_color = ImGui::GetColorU32(K_BORDER);

        pDraw_list->AddLine(ImVec2(content_pos.x + section_width, content_pos.y),
                            ImVec2(content_pos.x + section_width, content_pos.y + region.y),
                            line_color);

        pDraw_list->AddLine(ImVec2(content_pos.x + section_width * 2.0f, content_pos.y),
                            ImVec2(content_pos.x + section_width * 2.0f, content_pos.y + region.y),
                            line_color);

        action = renderContainersSection(section_width);

        ImGui::SameLine(0.0f, 0.0f);

        const action_E drill_action = renderDrillSection(state_.drill, section_width);

        if (action == action_E::None)
            action = drill_action;

        ImGui::SameLine(0.0f, 0.0f);

        renderScaleSection(state_.scale_weight_g, last_section_width);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    return action;
}

ScienceView::action_E ScienceView::renderContainersSection(float width) const
{
    action_E action = action_E::None;

    if (ImGui::BeginChild("ContainersSection", ImVec2(width, 0.0f), false, kNoScrollNoBgFlags))
    {
        constexpr float TOP_RESERVED_HEIGHT = 20.0f;
        constexpr float ROW_GAP = 4.0f;

        ImGui::SetCursorPosX(K_SECTION_PAD_X);
        ImGui::TextColored(K_SECTION_TEXT, "CONTAINERS");

        const float inner_width = std::max(width - K_SECTION_PAD_X * 2.0f, 1.0f);
        const float text_height = ImGui::GetTextLineHeight();

        const float block_height =
            K_SLOT_BUTTON_SIZE + ImGui::GetStyle().ItemSpacing.y + text_height;

        const float total_height = block_height * 2.0f + ROW_GAP;

        float y1 = TOP_RESERVED_HEIGHT +
                   (ImGui::GetWindowHeight() - TOP_RESERVED_HEIGHT - total_height) * 0.5f;

        y1 = std::max(y1, TOP_RESERVED_HEIGHT + 2.0f);

        const float y2 = y1 + block_height + ROW_GAP;

        const float x1 = K_SECTION_PAD_X + inner_width * 0.25f - K_SLOT_CELL_WIDTH * 0.5f;

        const float x2 = K_SECTION_PAD_X + inner_width * 0.75f - K_SLOT_CELL_WIDTH * 0.5f;

        const char* labels[4] = {"SLOT 1", "SLOT 2", "SLOT 3", "SLOT 4"};

        const action_E actions[4] = {action_E::Slot1, action_E::Slot2, action_E::Slot3,
                                     action_E::Slot4};

        for (int i = 0; i < 4; ++i)
        {
            const bool right_column = (i % 2) == 1;
            const bool bottom_row = i >= 2;

            const float x = right_column ? x2 : x1;
            const float y = bottom_row ? y2 : y1;

            ImGui::SetCursorPos(ImVec2(std::max(x, K_SECTION_PAD_X), y));
            ImGui::PushID(i);

            if (renderSlotButton("##slot", labels[i], state_.container_filled[i],
                                 K_SLOT_CELL_WIDTH))
            {
                action = actions[i];
            }

            ImGui::PopID();
        }
    }

    ImGui::EndChild();

    return action;
}

ScienceView::action_E ScienceView::renderDrillSection(const drillTelemetry_S& drill,
                                                      float width) const
{
    action_E action = action_E::None;

    if (ImGui::BeginChild("DrillSection", ImVec2(width, 0.0f), false, kNoScrollNoBgFlags))
    {
        constexpr float VALUE_COLUMN_WIDTH = 80.0f;
        constexpr float MIN_VALUE_OFFSET = 75.0f;
        constexpr float BUTTON_GAP = 10.0f;

        const float inner_width = std::max(width - K_SECTION_PAD_X * 2.0f, 1.0f);

        ImGui::SetCursorPosX(K_SECTION_PAD_X);
        ImGui::TextColored(K_SECTION_TEXT, "DRILL CONTROL");
        ImGui::Spacing();

        float value_x = width - K_SECTION_PAD_X - VALUE_COLUMN_WIDTH;
        value_x = std::max(value_x, K_SECTION_PAD_X + MIN_VALUE_OFFSET);

        ImGui::SetCursorPosX(K_SECTION_PAD_X);
        ImGui::Text("STATE:");
        ImGui::SameLine(value_x);
        ImGui::TextColored(K_DRILL_STATE_TEXT, "%s", toString(drill.state));

        ImGui::SetCursorPosX(K_SECTION_PAD_X);
        ImGui::Text("DEPTH:");
        ImGui::SameLine(value_x);
        ImGui::Text("%.0f cm", drill.depth_cm);

        ImGui::Spacing();
        ImGui::Spacing();

        const float button_width = std::max((inner_width - BUTTON_GAP) * 0.5f, 1.0f);

        ImGui::SetCursorPosX(K_SECTION_PAD_X);

        if (renderColoredButton("START", ImVec2(button_width, 40.0f), K_START_BUTTON,
                                K_START_BUTTON_HOVER))
        {
            action = action_E::DrillStart;
        }

        ImGui::SameLine(0.0f, BUTTON_GAP);

        if (renderColoredButton("STOP", ImVec2(button_width, 40.0f), K_STOP_BUTTON,
                                K_STOP_BUTTON_HOVER))
        {
            action = action_E::DrillStop;
        }
    }

    ImGui::EndChild();

    return action;
}

void ScienceView::renderScaleSection(float scale_weight_g, float width) const
{
    if (ImGui::BeginChild("ScaleSection", ImVec2(width, 0.0f), false, kNoScrollNoBgFlags))
    {
        const float text_height = ImGui::GetTextLineHeight();
        const float y = std::max((ImGui::GetWindowHeight() - text_height) * 0.5f, 0.0f);

        ImGui::SetCursorPos(ImVec2(K_SECTION_PAD_X, y));

        ImGui::TextColored(K_SECTION_TEXT, "SCALE:");
        ImGui::SameLine();
        ImGui::Text("%.1f g", scale_weight_g);
    }

    ImGui::EndChild();
}

void ScienceView::renderRightColumn(float width) const
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, K_PANEL_BG);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(K_OUTER_PAD_X, K_OUTER_PAD_Y));

    if (ImGui::BeginChild("RightColumn", ImVec2(width, 0.0f), true, kNoScrollFlags))
    {
        ImGui::TextColored(K_TITLE_TEXT, "SENSORS & KINEMATICS");
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        renderGpsCard(state_.gps);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        renderArmTelemetryCard(state_.arm, state_.gripper);
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

bool ScienceView::renderSlotButton(const char* pId, const char* pLabel, bool is_filled,
                                   float cell_width) const
{
    ImGui::BeginGroup();

    const float start_x = ImGui::GetCursorPosX();

    const float button_x = start_x + std::max((cell_width - K_SLOT_BUTTON_SIZE) * 0.5f, 0.0f);

    ImGui::SetCursorPosX(button_x);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, K_SLOT_BUTTON_SIZE * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, is_filled ? K_SLOT_GREEN_FILLED : K_TRANSPARENT);

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, K_SLOT_GREEN_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, K_SLOT_GREEN);
    ImGui::PushStyleColor(ImGuiCol_Border, K_SLOT_GREEN);

    const bool clicked = ImGui::Button(pId, ImVec2(K_SLOT_BUTTON_SIZE, K_SLOT_BUTTON_SIZE));
    const ImVec2 btn_min = ImGui::GetItemRectMin();
    const ImVec2 btn_max = ImGui::GetItemRectMax();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    ImDrawList* pDraw_list = ImGui::GetWindowDrawList();
    const char* pSlot_icon = ICON_FA_VIAL;
    constexpr float ICON_CENTER_COMPENSATION_X = 3.0f;
    const ImVec2 icon_size = ImGui::CalcTextSize(pSlot_icon);
    const float icon_x =
        btn_min.x + (btn_max.x - btn_min.x - icon_size.x) * 0.5f + ICON_CENTER_COMPENSATION_X;
    const float icon_y = btn_min.y + (btn_max.y - btn_min.y - icon_size.y) * 0.5f;
    const ImVec4 icon_color = is_filled ? K_SLOT_GREEN : K_MUTED_TEXT;
    pDraw_list->AddText(ImVec2(icon_x, icon_y), ImGui::GetColorU32(icon_color), pSlot_icon);

    const ImVec2 text_size = ImGui::CalcTextSize(pLabel);

    const float text_x = start_x + std::max((cell_width - text_size.x) * 0.5f, 0.0f);

    ImGui::SetCursorPosX(text_x);
    ImGui::TextColored(K_SECTION_TEXT, "%s", pLabel);

    ImGui::EndGroup();

    return clicked;
}

void ScienceView::renderArmTelemetryCard(const armTelemetry_S& arm,
                                         const gripperTelemetry_S& gripper) const
{
    if (beginCard("ArmCard", ImVec2(0.0f, 0.0f)))
    {
        drawHeader("ARM TELEMETRY");

        struct floatRow_S
        {
            const char* pLabel;
            float value;
        };

        const floatRow_S joints[] = {{"BASE:", arm.base_deg},
                                     {"SHOULDER:", arm.shoulder_deg},
                                     {"ELBOW:", arm.elbow_deg},
                                     {"WRIST PITCH:", arm.wrist_pitch_deg},
                                     {"WRIST ROLL:", arm.wrist_roll_deg}};

        ImGui::SetCursorPosX(K_CARD_PAD_X);
        ImGui::TextColored(K_MUTED_TEXT, "JOINT ANGLES");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const floatRow_S& row : joints)
        {
            valueRow(K_LABEL_X, K_VALUE_X, K_BLUE, row.pLabel, "%.0f°", row.value);
        }

        View::innerSeparator();

        const floatRow_S position[] = {{"X:", arm.ee_x_m}, {"Y:", arm.ee_y_m}, {"Z:", arm.ee_z_m}};

        ImGui::SetCursorPosX(K_CARD_PAD_X);
        ImGui::TextColored(K_MUTED_TEXT, "END-EFFECTOR POSITION");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        for (const floatRow_S& row : position)
        {
            valueRow(K_LABEL_X, K_VALUE_X, K_PURPLE, row.pLabel, "%.2f m", row.value);
        }

        View::innerSeparator();

        ImGui::SetCursorPosX(K_CARD_PAD_X);
        ImGui::TextColored(K_MUTED_TEXT, "GRIPPER STATE");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        valueRow(K_LABEL_X, K_VALUE_X, K_GREEN, "STATE:", "%s", toString(gripper.state));

        valueRow(K_LABEL_X, K_VALUE_X, K_ORANGE, "FORCE:", "%.0f N", gripper.force_n);
    }

    endCard();
}

const char* ScienceView::toString(drillState_E state) noexcept
{
    switch (state)
    {
    case drillState_E::Idle:
        return "IDLE";

    case drillState_E::Running:
        return "RUNNING";

    case drillState_E::Error:
        return "ERROR";
    }

    return "UNKNOWN";
}

const char* ScienceView::toString(gripperState_E state) noexcept
{
    switch (state)
    {
    case gripperState_E::Open:
        return "OPEN";

    case gripperState_E::Closed:
        return "CLOSED";

    case gripperState_E::Holding:
        return "HOLDING";

    case gripperState_E::Error:
        return "ERROR";
    }

    return "UNKNOWN";
}
