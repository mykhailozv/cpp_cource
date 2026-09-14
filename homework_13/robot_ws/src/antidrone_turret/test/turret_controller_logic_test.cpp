// turret_controller_logic_test.cpp
//
// Minimal tests for the pure C++ turret controller logic. At least one test
// per required part:
//   * target evaluation (low confidence -> ACTION_IDLE / TRIGGER_SKIP);
//   * yaw-servo command (x > 320 -> RIGHT, error_x > 0);
//   * gimbal command (y < 240 -> UP, error_y > 0);
//   * trigger decision (close + READY -> TRIGGER_REQUESTED;
//                        close + RELOADING -> TRIGGER_RELOADING);
//   * turret status (far valid target -> LOCKED / ACTION_TRACK / TRIGGER_SKIP).

#include "antidrone_turret/target_sequence.hpp"
#include "antidrone_turret/turret_controller_logic.hpp"

#include <gtest/gtest.h>

namespace antidrone_turret {
namespace {

TargetSample locked_near(float x, float y, float distance_m)
{
  return TargetSample{true, x, y, distance_m, 0.95F};
}

TEST(TurretControllerLogic, LowConfidenceIsIdleAndSkip)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  const auto decision = logic.decide(
    TargetSample{true, 320.0F, 240.0F, 10.0F, 0.60F},
    /*actuator_ready=*/true);

  EXPECT_EQ(decision.target_state, TargetState::kTargetLowConfidence);
  EXPECT_EQ(decision.action, ActionState::kActionIdle);
  EXPECT_EQ(decision.trigger_state, TriggerState::kTriggerSkip);
}

TEST(TurretControllerLogic, ServoCommandTurnsRightForXRightOfCenter)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  const auto decision = logic.decide(locked_near(420.0F, 240.0F, 40.0F), true);

  EXPECT_EQ(decision.servo_direction, ServoDirection::kServoRight);
  EXPECT_FLOAT_EQ(decision.target_x, 420.0F);
  EXPECT_FLOAT_EQ(decision.error_x, 100.0F);
  EXPECT_GT(decision.error_x, 0.0F);
}

TEST(TurretControllerLogic, GimbalCommandPointsUpWhenTargetAboveCenter)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  const auto decision = logic.decide(locked_near(320.0F, 180.0F, 40.0F), true);

  EXPECT_EQ(decision.gimbal_direction, GimbalDirection::kGimbalUp);
  EXPECT_FLOAT_EQ(decision.target_y, 180.0F);
  EXPECT_FLOAT_EQ(decision.error_y, 60.0F);
  EXPECT_GT(decision.error_y, 0.0F);
}

TEST(TurretControllerLogic, ReadyActuatorRequestsTriggerForNearTarget)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  const auto decision = logic.decide(locked_near(320.0F, 240.0F, 20.0F), true);

  EXPECT_EQ(decision.trigger_state, TriggerState::kTriggerRequested);
}

TEST(TurretControllerLogic, ReloadingActuatorYieldsReloadingForNearTarget)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  const auto decision = logic.decide(locked_near(320.0F, 240.0F, 20.0F), false);

  EXPECT_EQ(decision.trigger_state, TriggerState::kTriggerReloading);
}

TEST(TurretControllerLogic, FarValidTargetIsLockedTrackSkip)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  const auto decision = logic.decide(locked_near(320.0F, 240.0F, 50.0F), true);

  EXPECT_EQ(decision.target_state, TargetState::kTargetLocked);
  EXPECT_EQ(decision.action, ActionState::kActionTrack);
  EXPECT_EQ(decision.trigger_state, TriggerState::kTriggerSkip);
}

TEST(TurretControllerLogic, InvisibleTargetIsNoneAndIdle)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  const auto decision = logic.decide(TargetSample{}, true);

  EXPECT_EQ(decision.target_state, TargetState::kTargetNone);
  EXPECT_EQ(decision.action, ActionState::kActionIdle);
  EXPECT_EQ(decision.trigger_state, TriggerState::kTriggerSkip);
}

// Exercise the expected outcomes for the approach track.
TEST(TurretControllerLogic, ApproachTrackReachesTriggerWindow)
{
  TurretControllerLogic logic(0.80F, 30.0F);
  // approach_trigger.csv closest row (distance 12, conf 0.95).
  const auto decision = logic.decide(locked_near(476.0F, 158.0F, 12.0F), true);

  EXPECT_EQ(decision.target_state, TargetState::kTargetLocked);
  EXPECT_EQ(decision.trigger_state, TriggerState::kTriggerRequested);
}

}  // namespace
}  // namespace antidrone_turret
