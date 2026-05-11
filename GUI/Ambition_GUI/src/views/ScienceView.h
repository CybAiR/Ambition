#pragma once

#include "View.h"
#include <array>

class ScienceView : public View
{
  public:
    ScienceView(ImGuiWindowFlags flags = 0);

    enum class Action
    {
        None,
        DrillStart,
        DrillStop,
        Slot1,
        Slot2,
        Slot3,
        Slot4
    };

    enum class DrillState
    {
        Idle,
        Running,
        Error
    };

    struct DrillTelemetry
    {
        DrillState state = DrillState::Idle;
        float depth_cm = 12.0f;
    };

    struct State
    {
        gpsOdometry_S gps;
        armTelemetry_S arm;
        DrillTelemetry drill;
        gripperTelemetry_S gripper;

        std::array<bool, 4> container_filled = {false, false, false, false};

        float scale_weight_g = 142.5f;
    };

  public:
    Action Render();
    Action Render(const State& state);

    State& Data() noexcept
    {
        return state_;
    }
    const State& Data() const noexcept
    {
        return state_;
    }

  private:
    Action RenderLeftColumn(const State& state, float width);
    Action RenderBottomPanel(const State& state) const;

    Action RenderContainersSection(const State& state, float width) const;
    Action RenderDrillSection(const DrillTelemetry& drill, float width) const;
    void RenderScaleSection(float scale_weight_g, float width) const;

    void RenderRightColumn(const State& state, float width) const;

    void RenderArmTelemetryCard(const armTelemetry_S& arm, const gripperTelemetry_S& gripper) const;

    bool RenderSlotButton(const char* id, const char* label, bool is_filled,
                          float cell_width) const;

    static const char* ToString(DrillState state) noexcept;
    static const char* ToString(gripperState_E state) noexcept;

    void InnerSeparator() const;

  private:
    State state_;
    bool is_camera_fullscreen_ = false;
};
