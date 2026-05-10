#include "View.h"
#include <../fonts/IconsFontAwesome6.h>
#include "imgui.h"
#include <SDL2/SDL.h>


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

bool View::RenderCameraContainer(
    float height,
    const char* title,
    bool is_screenshot_enabled,
    bool is_fullscreen_button_enabled,
    bool is_fullscreen_active
) const
{
    bool is_fullscreen_toggled = false;

    if (ImGui::BeginChild(
        "CameraContainer",
        ImVec2(0.0f, height),
        true,
        kNoScrollFlags
    ))
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const ImVec2 win_pos = ImGui::GetWindowPos();
        const ImVec2 win_size = ImGui::GetWindowSize();

        constexpr float TITLE_H = 28.0f;
        constexpr float PAD = 8.0f;

        draw_list->AddRectFilled(
            win_pos,
            ImVec2(win_pos.x + win_size.x, win_pos.y + TITLE_H),
            ImGui::GetColorU32(kHeaderBg)
        );
        draw_list->AddLine(
            ImVec2(win_pos.x, win_pos.y + TITLE_H),
            ImVec2(win_pos.x + win_size.x, win_pos.y + TITLE_H),
            ImGui::GetColorU32(kBorder)
        );
        draw_list->AddRect(
            win_pos,
            ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y),
            ImGui::GetColorU32(kBorder)
        );

        ImGui::SetCursorPos(ImVec2(kCardPadX, 5.0f));
        ImGui::TextColored(kTitleText, "%s", title);

        const ImVec2 image_min = ImVec2(win_pos.x + PAD, win_pos.y + TITLE_H + PAD);
        const ImVec2 image_max =
            ImVec2(win_pos.x + win_size.x - PAD, win_pos.y + win_size.y - PAD);

        draw_list->AddRectFilled(
            image_min,
            image_max,
            ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.10f, 1.0f))
        );
        draw_list->AddRect(
            image_min,
            image_max,
            ImGui::GetColorU32(ImVec4(0.24f, 0.24f, 0.24f, 1.0f))
        );

        if (is_screenshot_enabled)
        {
            constexpr float BTN_SIZE = 44.0f;
            constexpr float BTN_Y_OFFSET = 14.0f;
            constexpr float ICON_CENTER_COMPENSATION_X = 3.0f;
            const float btn_x = (image_min.x + image_max.x - BTN_SIZE) * 0.5f;
            const float btn_y = image_max.y - BTN_SIZE - BTN_Y_OFFSET;

            ImGui::SetCursorScreenPos(ImVec2(btn_x, btn_y));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, BTN_SIZE * 0.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.58f, 0.50f, 0.40f, 0.70f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.66f, 0.58f, 0.48f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.66f, 0.58f, 0.48f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.92f, 0.92f, 0.92f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.96f, 0.96f, 0.0f));
            ImGui::Button("##screenshot_btn", ImVec2(BTN_SIZE, BTN_SIZE));

            const ImVec2 btn_min = ImGui::GetItemRectMin();
            const ImVec2 btn_max = ImGui::GetItemRectMax();
            const char* icon = ICON_FA_CAMERA;
            const ImVec2 icon_size = ImGui::CalcTextSize(icon);
            const float icon_x =
                btn_min.x + (btn_max.x - btn_min.x - icon_size.x) * 0.5f + ICON_CENTER_COMPENSATION_X;
            const float icon_y = btn_min.y + (btn_max.y - btn_min.y - icon_size.y) * 0.5f;

            draw_list->AddText(
                ImVec2(icon_x, icon_y),
                ImGui::GetColorU32(ImVec4(0.96f, 0.96f, 0.96f, 1.0f)),
                icon
            );

            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar(2);
        }

        if (is_fullscreen_button_enabled)
        {
            constexpr float BTN_SIZE = 34.0f;
            constexpr float BTN_PADDING = 12.0f;
            constexpr float ICON_CENTER_COMPENSATION_X = 3.0f;
            const float btn_x = image_max.x - BTN_SIZE - BTN_PADDING;
            const float btn_y = image_min.y + BTN_PADDING;

            ImGui::SetCursorScreenPos(ImVec2(btn_x, btn_y));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.04f, 0.04f, 0.55f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.10f, 0.10f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.14f, 0.14f, 0.88f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.75f, 0.75f, 0.75f, 0.45f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.96f, 0.96f, 0.0f));

            if (ImGui::Button("##camera_fullscreen_btn", ImVec2(BTN_SIZE, BTN_SIZE)))
            {
                is_fullscreen_toggled = true;
            }

            const ImVec2 btn_min = ImGui::GetItemRectMin();
            const ImVec2 btn_max = ImGui::GetItemRectMax();
            const char* icon = is_fullscreen_active ? ICON_FA_COMPRESS : ICON_FA_EXPAND;
            const ImVec2 icon_size = ImGui::CalcTextSize(icon);
            const float icon_x =
                btn_min.x + (btn_max.x - btn_min.x - icon_size.x) * 0.5f + ICON_CENTER_COMPENSATION_X;
            const float icon_y = btn_min.y + (btn_max.y - btn_min.y - icon_size.y) * 0.5f;

            draw_list->AddText(
                ImVec2(icon_x, icon_y),
                ImGui::GetColorU32(ImVec4(0.96f, 0.96f, 0.96f, 1.0f)),
                icon
            );

            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar(2);
        }
    }

    ImGui::EndChild();

    return is_fullscreen_toggled;
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
