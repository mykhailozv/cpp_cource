// turret_controller_logic.hpp
//
// Pure C++ decision logic for the turret controller. No rclcpp, no Node, no
// publishers/subscribers. It maps one perception TargetSample plus the current
// actuator readiness into:
//   * target evaluation (NONE / LOW_CONFIDENCE / LOCKED);
//   * yaw servo command (LEFT / CENTER / RIGHT);
//   * gimbal command (UP / DOWN / CENTER);
//   * a trigger decision (SKIP / REQUESTED / RELOADING);
//   * the TurretStatus summary fields.

#pragma once

#include "antidrone_turret/target_sequence.hpp"

#include <cstdint>

namespace antidrone_turret {

// Simplified camera model used to convert target pixels into aiming commands.
inline constexpr float kFrameCenterX = 320.0F;
inline constexpr float kFrameCenterY = 240.0F;

enum class TargetState : std::uint8_t {
  kTargetNone = 0,
  kTargetLowConfidence = 1,
  kTargetLocked = 2,
};

enum class ActionState : std::uint8_t {
  kActionIdle = 0,
  kActionTrack = 1,
};

enum class TriggerState : std::uint8_t {
  kTriggerSkip = 0,
  kTriggerRequested = 1,
  kTriggerReloading = 2,
};

// ServoCommand.direction values.
enum class ServoDirection : std::int8_t {
  kServoLeft = -1,
  kServoCenter = 0,
  kServoRight = 1,
};

// GimbalCommand.direction values.
enum class GimbalDirection : std::int8_t {
  kGimbalDown = -1,
  kGimbalCenter = 0,
  kGimbalUp = 1,
};

// Result of one decision cycle.
struct ControllerDecision {
  TargetState target_state{TargetState::kTargetNone};
  ActionState action{ActionState::kActionIdle};
  TriggerState trigger_state{TriggerState::kTriggerSkip};

  ServoDirection servo_direction{ServoDirection::kServoCenter};
  float target_x{0.0F};
  float error_x{0.0F};

  GimbalDirection gimbal_direction{GimbalDirection::kGimbalCenter};
  float target_y{0.0F};
  float error_y{0.0F};

  float confidence{0.0F};
  float distance_m{0.0F};
};

class TurretControllerLogic {
public:
  TurretControllerLogic(float confidence_threshold = 0.80F,
                        float max_distance_m = 30.0F)
    : confidence_threshold_(confidence_threshold)
    , max_distance_m_(max_distance_m)
  {}

  [[nodiscard]] float confidence_threshold() const { return confidence_threshold_; }
  [[nodiscard]] float max_distance_m() const { return max_distance_m_; }

  // Decide what to do for a fresh perception sample.
  [[nodiscard]] ControllerDecision decide(const TargetSample& target,
                                          bool actuator_ready) const
  {
    auto decision = ControllerDecision{};
    decision.confidence = target.confidence;
    decision.distance_m = target.distance_m;

    // 1) Target evaluation.
    if (!target.visible) {
      decision.target_state = TargetState::kTargetNone;
      decision.action = ActionState::kActionIdle;
      decision.trigger_state = TriggerState::kTriggerSkip;
      return decision;
    }
    if (target.confidence < confidence_threshold_) {
      decision.target_state = TargetState::kTargetLowConfidence;
      decision.action = ActionState::kActionIdle;
      decision.trigger_state = TriggerState::kTriggerSkip;
      return decision;
    }
    decision.target_state = TargetState::kTargetLocked;
    decision.action = ActionState::kActionTrack;

    // 2) Aiming commands for a tracked target.
    fill_gimbal(target, decision);
    fill_servo(target, decision);

    // 3) Trigger decision.
    decision.trigger_state = decide_trigger(target.distance_m, actuator_ready);
    return decision;
  }

private:
  static void fill_gimbal(const TargetSample& target, ControllerDecision& decision)
  {
    decision.target_y = target.y;
    decision.error_y = kFrameCenterY - target.y;
    if (decision.error_y > 0.0F) {
      decision.gimbal_direction = GimbalDirection::kGimbalUp;
    } else if (decision.error_y < 0.0F) {
      decision.gimbal_direction = GimbalDirection::kGimbalDown;
    } else {
      decision.gimbal_direction = GimbalDirection::kGimbalCenter;
    }
  }

  static void fill_servo(const TargetSample& target, ControllerDecision& decision)
  {
    decision.target_x = target.x;
    decision.error_x = target.x - kFrameCenterX;
    if (decision.error_x > 0.0F) {
      decision.servo_direction = ServoDirection::kServoRight;
    } else if (decision.error_x < 0.0F) {
      decision.servo_direction = ServoDirection::kServoLeft;
    } else {
      decision.servo_direction = ServoDirection::kServoCenter;
    }
  }

  [[nodiscard]] TriggerState decide_trigger(float distance_m,
                                            bool actuator_ready) const
  {
    if (distance_m <= max_distance_m_) {
      return actuator_ready ? TriggerState::kTriggerRequested
                            : TriggerState::kTriggerReloading;
    }
    return TriggerState::kTriggerSkip;
  }

  float confidence_threshold_;
  float max_distance_m_;
};

}  // namespace antidrone_turret
