#pragma once

#include <array>

class ScienceView {
public:
    enum class Action {
        None,
        DrillStart,
        DrillStop,
        Slot1,
        Slot2,
        Slot3,
        Slot4
    };

    enum class DrillState {
        Idle,
        Running,
        Error
    };

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

    struct DrillTelemetry {
        DrillState state = DrillState::Idle;
        float depth_cm = 12.0f;
    };

    struct GripperTelemetry {
        GripperState state = GripperState::Open;
        float force_n = 2.0f;
    };

    struct State {
        GpsOdometry gps;
        ArmTelemetry arm;
        DrillTelemetry drill;
        GripperTelemetry gripper;

        std::array<bool, 4> container_filled = {
            false,
            false,
            false,
            false
        };

        float scale_weight_g = 142.5f;
    };

public:
    Action Render();
    Action Render(const State& state) const;

    State& Data() noexcept { return state_; }
    const State& Data() const noexcept { return state_; }

private:
    Action RenderLeftColumn(const State& state, float width) const;
    void RenderCameraContainer(float height) const;
    Action RenderBottomPanel(const State& state) const;

    Action RenderContainersSection(const State& state, float width) const;
    Action RenderDrillSection(const DrillTelemetry& drill, float width) const;
    void RenderScaleSection(float scale_weight_g, float width) const;

    void RenderRightColumn(const State& state, float width) const;

    void RenderGpsCard(const GpsOdometry& gps) const;
    void RenderArmTelemetryCard(
        const ArmTelemetry& arm,
        const GripperTelemetry& gripper
    ) const;

    static bool RenderSlotButton(
        const char* id,
        const char* label,
        bool is_filled,
        float cell_width
    );

    static const char* ToString(DrillState state) noexcept;
    static const char* ToString(GripperState state) noexcept;

private:
    State state_;
};