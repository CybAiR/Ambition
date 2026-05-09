#pragma once

#include "View.h"
#include <array>
#include <map>

class NavigationView : public View
{
  public:
    NavigationView(ImGuiWindowFlags flags = 0);

    enum class StatusState
    {
        AUTO,
        MANUAL
    };

    enum class WaypointState
    {
        DONE,
        ACTIVE,
        PENDING
    };

    struct Status
    {
        StatusState type = StatusState::MANUAL;
    };

    struct EstimatedKinematics
    {
        float speed = 1.2f;
        int heading = 45;
        GpsOdometry gps = {51.4883f, -0.1171f, 12.4f};
    };

    struct Waypoint
    {
        int id;
        float x;
        float y;
        WaypointState state;
    };

    struct WaypointEditor
    {
        float input_x = 0.0f;
        float input_y = 0.0f;
        std::map<int, Waypoint> waypoints = {{1, {1, 12.4f, -4.2f, WaypointState::DONE}},
                                             {2, {2, 25.1f, 0.8f, WaypointState::ACTIVE}},
                                             {3, {3, 40.5f, 12.3f, WaypointState::PENDING}},
                                             {4, {4, 55.0f, 20.1f, WaypointState::PENDING}}};
    };

    struct State
    {
        Status status;
        EstimatedKinematics kinematics;
        WaypointEditor editor;

        std::array<bool, 2> container_filled = {false, false};
    };

  public:
    void Render();
    void Render(State& state);

    State& Data() noexcept
    {
        return state_;
    }
    const State& Data() const noexcept
    {
        return state_;
    }

  private:
    void RenderLeftColumn(const State& state, float width) const;
    void RenderRightColumn(State& state, float width);
    void RenderMapContainer(float height) const;
    void RenderStatus(Status& status);
    void RenderEstimatedKinematics(const EstimatedKinematics& kinematics) const;
    void RenderWaypointEditor(WaypointEditor& editor);

    void InnerSeparator() const;

    static const char* ToString(StatusState state) noexcept;
    static const char* ToString(WaypointState state) noexcept;

  private:
    State state_;
};