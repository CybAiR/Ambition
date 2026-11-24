#pragma once

#include "View.h"

class MaintenanceView : public View
{
  public:
    MaintenanceView(ImGuiWindowFlags flags = 0) : View(flags)
    {
    }

    struct jointDiagnostics_S
    {
        float base_deg_cel = 45.0f;
        float shoulder_deg_cel = 68.0f;
        float elbow_deg_cel = 52.0f;

        float base_current_a = 1.2f;
        float shoulder_current_a = 3.4f;
        float elbow_current_a = 2.1f;
    };

    struct state_S
    {
        armTelemetry_S arm;
        gripperTelemetry_S gripper;
        jointDiagnostics_S joints;
    };

    void render();

  private:
    void renderLeftColumn(float width);
    void renderRightColumn(float width) const;
    void renderArmTelemetryCard(const armTelemetry_S& arm, const gripperTelemetry_S& gripper) const;
    void renderJointDiagnostics(const jointDiagnostics_S& joints) const;
    void innerSeparator() const;

    static const char* toString(gripperState_E state);

    state_S state_;
    bool is_camera_fullscreen_ = false;
};