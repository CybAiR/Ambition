#pragma once
#include "imgui.h"
#include <string>
#include "Views.h"

class View
{
public:
    View(ImGuiWindowFlags view_flags);
    virtual ~View() = default;

    enum class GripperState {
        Open,
        Closed,
        Holding,
        Error
    };

    struct GpsOdometry {
        float lat = 51.4883f;
        float lon = -0.1171f;
        float alt_m = 12.4f;
    };

    struct ArmTelemetry {
        float base_deg = 45.0f;
        float shoulder_deg = 60.0f;
        float elbow_deg = -30.0f;
        float wrist_pitch_deg = 10.0f;
        float wrist_roll_deg = 0.0f;

        float ee_x_m = 1.24f;
        float ee_y_m = 0.45f;
        float ee_z_m = 0.88f;
    };

    struct GripperTelemetry {
        GripperState state = GripperState::Open;
        float force_n = 2.0f;
    };

protected:
    float kHeaderHeight = 28.0f;
    float kGpsCardHeight = 116.0f;

    float kCardPadX = 12.0f;

    float kLabelX = 30.0f;
    float kValueX = 145.0f;

    ImVec4 kCardBg = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    ImVec4 kHeaderBg = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
    ImVec4 kBorder = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);

    ImVec4 kTitleText = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
    ImVec4 kMutedText = ImVec4(0.62f, 0.62f, 0.62f, 1.0f);
    ImVec4 kSectionText = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);

    ImVec4 kGreen = ImVec4(0.55f, 0.95f, 0.65f, 1.0f);
    ImVec4 kBlue = ImVec4(0.45f, 0.65f, 1.00f, 1.0f);
    ImVec4 kPurple = ImVec4(0.85f, 0.65f, 1.00f, 1.0f);
    ImVec4 kOrange = ImVec4(1.00f, 0.70f, 0.25f, 1.0f);

    ImGuiWindowFlags kNoScrollFlags =
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGuiWindowFlags kNoScrollNoBgFlags =
        kNoScrollFlags |
        ImGuiWindowFlags_NoBackground;

    void InnerSeparator() const;
    void RenderCameraContainer(float height, const char* title, bool is_screenshot_enabled) const;
    void RenderGpsCard(const GpsOdometry& gps) const;
    bool BeginCard(const char* id, const ImVec2& size) const;
    void DrawHeader(const char* title, float text_y = 5.0f) const;
    void ValueRow(float label_x, float value_x, const ImVec4& value_color, const char* label, const char* fmt, ...) const;
    void EndCard() const;

private:
    ImGuiWindowFlags view_flags;

};
