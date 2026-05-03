#pragma once

#include "View.h"
#include <array>

class ProbingView : public View {
public:
    ProbingView(ImGuiWindowFlags flags = 0);

    struct MissionProgress {
        int probes_collected = 3;
        int target = 5;

    };

    struct State {
        ArmTelemetry arm;
        GripperTelemetry gripper;
        MissionProgress mission;

        std::array<bool, 3> container_filled = {
            false,
            false,
            false
        };
    };

public:
    void Render(); 
    void Render(const State& state) const;

    State& Data() noexcept { return state_; }
    const State& Data() const noexcept { return state_; }

private:
    void RenderRightColumn(const State& state, float width) const;
    void RenderArmTelemetryCard(
        const ArmTelemetry& arm,
        const GripperTelemetry& gripper
    ) const;
    
    // Add this declaration
    void RenderMissionProgressCard(const MissionProgress& mission) const;

    static const char* ToString(GripperState state) noexcept;

    void InnerSeparator() const; 

private:
    State state_;
};