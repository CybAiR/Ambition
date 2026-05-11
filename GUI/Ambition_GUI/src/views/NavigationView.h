#pragma once

#include "View.h"
#include <map>

class NavigationView : public View
{
  public:
    enum class statusState_E
    {
        AUTO,
        MANUAL
    };
    enum class waypointState_E
    {
        DONE,
        ACTIVE,
        PENDING
    };

    struct estimatedKinematics_S
    {
        float speed = 1.2f;
        int heading = 45;
        gpsOdometry_S gps = {51.4883f, -0.1171f, 12.4f};
    };

    struct waypoint_S
    {
        int id;
        float x, y;
        waypointState_E state;
    };

    struct waypointEditor_S
    {
        float input_x = 0.0f;
        float input_y = 0.0f;
        std::map<int, waypoint_S> waypoints = {
            {1, {1, 12.4f, -4.2f, waypointState_E::DONE}},
            {2, {2, 25.1f, 0.8f, waypointState_E::ACTIVE}},
            {3, {3, 40.5f, 12.3f, waypointState_E::PENDING}},
            {4, {4, 55.0f, 20.1f, waypointState_E::PENDING}},
        };
    };

    struct state_S
    {
        statusState_E status_type = statusState_E::MANUAL;
        estimatedKinematics_S kinematics;
        waypointEditor_S editor;
    };

    NavigationView(ImGuiWindowFlags flags = 0) : View(flags)
    {
    }

    void render();

  private:
    void renderLeftColumn(float width) const;
    void renderRightColumn(float width);
    void renderMapContainer(float height) const;
    void renderStatus();
    void renderEstimatedKinematics() const;
    void renderWaypointEditor();
    void InnerSeparator() const;

    static const char* toString(statusState_E state);
    static const char* toString(waypointState_E state);

    state_S state_;
};