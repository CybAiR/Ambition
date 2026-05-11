#pragma once

#include "View.h"

class ProbingView : public View
{
  public:
    ProbingView(ImGuiWindowFlags flags = 0) : View(flags)
    {
    }

    struct missionProgress_S
    {
        int probes_collected = 3;
        int target = 5;
    };

    struct state_S
    {
        ArmTelemetry arm;
        GripperTelemetry gripper;
        missionProgress_S mission;

        bool container_filled[3] = {false, false, false};
    };

    void render();

  private:
    void renderLeftColumn(float width) const;
    void renderRightColumn(float width) const;
    void renderArmTelemetryCard(const ArmTelemetry& arm, const GripperTelemetry& gripper) const;
    void renderMissionProgressCard(const missionProgress_S& mission) const;
    bool renderColoredButton(const char* label, const ImVec2& size, const ImVec4& base_color,
                             const ImVec4& hover_color) const;
    void innerSeparator() const;

    static const char* toString(GripperState state);

    state_S state_;
};