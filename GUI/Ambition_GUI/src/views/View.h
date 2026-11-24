#pragma once
#include "imgui.h"

class View
{
  public:
    View(ImGuiWindowFlags view_flags);
    virtual ~View() = default;

    enum class gripperState_E
    {
        Open,
        Closed,
        Holding,
        Error
    };

    struct gpsOdometry_S
    {
        float lat = 51.4883f;
        float lon = -0.1171f;
        float alt_m = 12.4f;
    };

    struct armTelemetry_S
    {
        float base_deg = 45.0f;
        float shoulder_deg = 60.0f;
        float elbow_deg = -30.0f;
        float wrist_pitch_deg = 10.0f;
        float wrist_roll_deg = 0.0f;

        float ee_x_m = 1.24f;
        float ee_y_m = 0.45f;
        float ee_z_m = 0.88f;
    };

    struct gripperTelemetry_S
    {
        gripperState_E state = gripperState_E::Open;
        float force_n = 2.0f;
    };

  protected:
    static constexpr float K_HEADER_HEIGHT = 28.0f;
    static constexpr float K_GPS_CARD_HEIGHT = 116.0f;

    static constexpr float K_CARD_PAD_X = 12.0f;

    static constexpr float K_LABEL_X = 30.0f;
    static constexpr float K_VALUE_X = 145.0f;

    const ImVec4 K_CARD_BG = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    const ImVec4 K_HEADER_BG = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
    const ImVec4 K_BORDER = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);

    const ImVec4 K_TITLE_TEXT = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
    const ImVec4 K_MUTED_TEXT = ImVec4(0.62f, 0.62f, 0.62f, 1.0f);
    const ImVec4 K_SECTION_TEXT = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);

    const ImVec4 K_GREEN = ImVec4(0.55f, 0.95f, 0.65f, 1.0f);
    const ImVec4 K_BLUE = ImVec4(0.45f, 0.65f, 1.00f, 1.0f);
    const ImVec4 K_PURPLE = ImVec4(0.85f, 0.65f, 1.00f, 1.0f);
    const ImVec4 K_ORANGE = ImVec4(1.00f, 0.70f, 0.25f, 1.0f);

    ImGuiWindowFlags kNoScrollFlags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGuiWindowFlags kNoScrollNoBgFlags = kNoScrollFlags | ImGuiWindowFlags_NoBackground;

    void innerSeparator() const;
    bool renderCameraContainer(float height, const char* title, bool is_screenshot_enabled,
                               bool is_fullscreen_button_enabled, bool is_fullscreen_active) const;
    void renderGpsCard(const gpsOdometry_S& gps) const;
    bool renderColoredButton(const char* label, const ImVec2& size, const ImVec4& base_color,
                             const ImVec4& hover_color) const;

    bool beginCard(const char* id, const ImVec2& size) const;
    void drawHeader(const char* title, float text_y = 5.0f) const;
    void valueRow(float label_x, float value_x, const ImVec4& value_color, const char* label,
                  const char* fmt, ...) const;
    void endCard() const;

  private:
    ImGuiWindowFlags view_flags_;
};
