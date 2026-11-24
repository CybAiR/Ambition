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
        armTelemetry_S arm;
        gripperTelemetry_S gripper;
        missionProgress_S mission;

        bool container_filled[3] = {false, false, false};
    };

    void render();

  private:
    void renderLeftColumn(float width);
    void renderRightColumn(float width) const;
    void renderArmTelemetryCard(const armTelemetry_S& arm, const gripperTelemetry_S& gripper) const;
    void renderMissionProgressCard(const missionProgress_S& mission) const;
    void innerSeparator() const;

    static const char* toString(gripperState_E state);

    state_S state_;
    bool is_camera_fullscreen_ = false;
};