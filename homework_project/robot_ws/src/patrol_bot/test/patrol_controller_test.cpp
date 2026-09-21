// gtest на чисту логіку PatrolController — без ROS.
#include "patrol_bot/patrol_controller.hpp"

#include <gtest/gtest.h>

using patrol_bot::Mode;
using patrol_bot::PatrolController;
using patrol_bot::Twist2D;

TEST(PatrolController, TurnsTowardTargetFirst) {
    PatrolController c({{2.0, 0.0}});
    const auto cmd = c.update(0.0, 0.0, 0.0);  // ціль прямо попереду
    EXPECT_NEAR(cmd.linear, 0.3, 1e-9);
    EXPECT_NEAR(cmd.angular, 0.0, 1e-9);
}

TEST(PatrolController, RotatesInPlaceWhenYawErrorLarge) {
    PatrolController c({{0.0, 2.0}});
    const auto cmd = c.update(0.0, 0.0, 0.0);  // ціль зліва (pi/2)
    EXPECT_NEAR(cmd.linear, 0.0, 1e-9);
    EXPECT_NEAR(cmd.angular, 0.8, 1e-9);       // обмеження turn_speed
}

TEST(PatrolController, ReachesWaypointAndAdvances) {
    PatrolController c({{1.0, 0.0}, {0.0, -1.0}});
    EXPECT_EQ(c.waypointIndex(), 0);
    c.update(0.0, 0.0, 0.0);
    // рух уперед по осі X до точки (1,0)
    double x = 0.0, y = 0.0, yaw = 0.0;
    for (int i = 0; i < 50; ++i) {
        const auto cmd = c.update(x, y, yaw);
        x += cmd.linear * 0.1;
        yaw += cmd.angular * 0.1;
    }
    EXPECT_NEAR(x, 1.0, 0.2);
    EXPECT_EQ(c.waypointIndex(), 1);  // досягнуто, перехід до наступної
}

TEST(PatrolController, WaypointsWrapAround) {
    PatrolController c({{1.0, 0.0}, {2.0, 0.0}});
    c.update(1.0, 0.0, 0.0);   // у точці 0 -> перехід до 1
    EXPECT_EQ(c.waypointIndex(), 1);
    c.update(2.0, 0.0, 0.0);   // у точці 1 -> перехід до 0
    EXPECT_EQ(c.waypointIndex(), 0);
}

TEST(PatrolController, ManualModeStopsCommands) {
    PatrolController c({{2.0, 0.0}});
    c.setMode(Mode::Manual);
    const auto cmd = c.update(0.0, 0.0, 0.0);
    EXPECT_NEAR(cmd.linear, 0.0, 1e-9);
    EXPECT_NEAR(cmd.angular, 0.0, 1e-9);
    c.setMode(Mode::Patrol);
    const auto cmd2 = c.update(0.0, 0.0, 0.0);
    // з (0,0) ціль (2,0) прямо попереду — має поїхати
    EXPECT_GT(cmd2.linear, 0.0);
}

TEST(PatrolController, TripDistanceAccumulates) {
    PatrolController c({{10.0, 0.0}});
    c.update(0.0, 0.0, 0.0);
    c.update(1.0, 0.0, 0.0);
    c.update(3.0, 0.0, 0.0);
    EXPECT_NEAR(c.tripMeters(), 3.0, 1e-9);
}
