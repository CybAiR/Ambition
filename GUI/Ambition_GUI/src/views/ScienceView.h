#pragma once

#include "View.h"
#include <array>

class ScienceView : public View
{
  public:
    ScienceView(ImGuiWindowFlags flags = 0) : View(flags)
    {
    }

    enum class action_E
    {
        None,
        DrillStart,
        DrillStop,
        Slot1,
        Slot2,
        Slot3,
        Slot4
    };

    enum class drillState_E
    {
        Idle,
        Running,
        Error
    };

    struct drillTelemetry_S
    {
        drillState_E state = drillState_E::Idle;
        float depth_cm = 12.0f;
    };

    struct state_S
    {
        gpsOdometry_S gps;
        armTelemetry_S arm;
        drillTelemetry_S drill;
        gripperTelemetry_S gripper;

        std::array<bool, 4> container_filled = {false, false, false, false};

        float scale_weight_g = 142.5f;
    };

  public:
    action_E render();

  private:
    action_E renderLeftColumn(float width);
    action_E renderBottomPanel() const;

    action_E renderContainersSection(float width) const;
    action_E renderDrillSection(const drillTelemetry_S& drill, float width) const;
    void renderScaleSection(float scale_weight_g, float width) const;

    void renderRightColumn(float width) const;

    void renderArmTelemetryCard(const armTelemetry_S& arm, const gripperTelemetry_S& gripper) const;

    bool renderSlotButton(const char* id, const char* label, bool is_filled,
                          float cell_width) const;

    static const char* toString(drillState_E state) noexcept;
    static const char* toString(gripperState_E state) noexcept;

    void innerSeparator() const;

  private:
    state_S state_;
    bool is_camera_fullscreen_ = false;
};
