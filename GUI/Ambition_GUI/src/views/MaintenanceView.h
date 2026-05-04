#pragma once

#include "View.h"
#include <array>

class MaintenanceView : public View
{
  public:
    MaintenanceView(ImGuiWindowFlags flags = 0);

    struct JointDiagnostics
    {
        float base_deg_cel = 45.0f;
        float shoulder_deg_cel = 68.0f;
        float elbow_deg_cel = 52.0f;

        float base_current_a = 1.2f;
        float shoulder_current_a = 3.4f;
        float elbow_current_a = 2.1f;
    };

    struct State
    {
        ArmTelemetry arm;
        GripperTelemetry gripper;
        JointDiagnostics joints;

        std::array<bool, 3> container_filled = {false, false, false};
    };

  public:
    void Render();
    void Render(const State& state) const;

    State& Data() noexcept
    {
        return state_;
    }
    const State& Data() const noexcept
    {
        return state_;
    }

  private:
    void RenderRightColumn(const State& state, float width) const;
    void RenderArmTelemetryCard(const ArmTelemetry& arm, const GripperTelemetry& gripper) const;

    // Add this declaration
    void RenderJointDiagnostics(const JointDiagnostics& joints) const;

    static const char* ToString(GripperState state) noexcept;

    void InnerSeparator() const;

  private:
    State state_;
};