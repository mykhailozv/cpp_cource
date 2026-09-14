// turret_controller_node.cpp
//
// Central turret controller. Subscribes to /perception/target and
// /actuator/status, runs the pure C++ TurretControllerLogic on every target
// sample and translates the decision into:
//   * GimbalCommand published on /gimbal/cmd;
//   * ServoCommand published on /servo/cmd;
//   * a /actuator/trigger service call when a shot should be requested;
//   * a TurretStatus summary published on /turret/status.
// No decision logic lives here - it lives in turret_controller_logic.hpp.

#include "antidrone_turret/target_sequence.hpp"
#include "antidrone_turret/turret_controller_logic.hpp"

#include "antidrone_turret/msg/actuator_status.hpp"
#include "antidrone_turret/msg/gimbal_command.hpp"
#include "antidrone_turret/msg/servo_command.hpp"
#include "antidrone_turret/msg/target.hpp"
#include "antidrone_turret/msg/turret_status.hpp"
#include "antidrone_turret/srv/trigger_actuator.hpp"
#include "rclcpp/rclcpp.hpp"

#include <memory>

namespace {

constexpr auto kTargetTopic = "/perception/target";
constexpr auto kActuatorStatusTopic = "/actuator/status";
constexpr auto kGimbalCommandTopic = "/gimbal/cmd";
constexpr auto kServoCommandTopic = "/servo/cmd";
constexpr auto kTurretStatusTopic = "/turret/status";
constexpr auto kTriggerService = "/actuator/trigger";

antidrone_turret::TargetSample to_sample(const antidrone_turret::msg::Target& message)
{
  auto sample = antidrone_turret::TargetSample{};
  sample.visible = message.visible;
  sample.x = message.x;
  sample.y = message.y;
  sample.distance_m = message.distance_m;
  sample.confidence = message.confidence;
  return sample;
}

}  // namespace

namespace antidrone_turret {

class TurretControllerNode final : public rclcpp::Node {
public:
  TurretControllerNode()
    : Node("turret_controller_node")
  {
    const auto confidence_threshold =
      declare_parameter<double>("confidence_threshold", 0.80);
    const auto max_distance_m = declare_parameter<double>("max_distance_m", 30.0);

    logic_ = TurretControllerLogic{
      static_cast<float>(confidence_threshold),
      static_cast<float>(max_distance_m),
    };

    target_subscription_ = create_subscription<msg::Target>(
      kTargetTopic,
      10,
      [this](const msg::Target& message) { on_target(message); });
    actuator_subscription_ = create_subscription<msg::ActuatorStatus>(
      kActuatorStatusTopic,
      10,
      [this](const msg::ActuatorStatus& status) { on_actuator_status(status); });

    gimbal_publisher_ = create_publisher<msg::GimbalCommand>(kGimbalCommandTopic, 10);
    servo_publisher_ = create_publisher<msg::ServoCommand>(kServoCommandTopic, 10);
    status_publisher_ = create_publisher<msg::TurretStatus>(kTurretStatusTopic, 10);

    trigger_client_ = create_client<srv::TriggerActuator>(kTriggerService);
  }

private:
  void on_actuator_status(const msg::ActuatorStatus& status)
  {
    actuator_ready_ = status.state == msg::ActuatorStatus::READY;
  }

  void on_target(const msg::Target& message)
  {
    const auto decision = logic_.decide(to_sample(message), actuator_ready_);
    apply_decision(decision);
  }

  void apply_decision(const ControllerDecision& decision)
  {
    if (decision.action == ActionState::kActionTrack) {
      publish_gimbal(decision);
      publish_servo(decision);
    }
    publish_status(decision);
    maybe_call_trigger(decision);
  }

  void publish_gimbal(const ControllerDecision& decision)
  {
    auto message = msg::GimbalCommand{};
    message.direction = static_cast<std::int8_t>(decision.gimbal_direction);
    message.target_y = decision.target_y;
    message.error_y = decision.error_y;
    gimbal_publisher_->publish(message);
  }

  void publish_servo(const ControllerDecision& decision)
  {
    auto message = msg::ServoCommand{};
    message.direction = static_cast<std::int8_t>(decision.servo_direction);
    message.target_x = decision.target_x;
    message.error_x = decision.error_x;
    servo_publisher_->publish(message);
  }

  void publish_status(const ControllerDecision& decision)
  {
    auto message = msg::TurretStatus{};
    message.target_state = static_cast<std::uint8_t>(decision.target_state);
    message.action = static_cast<std::uint8_t>(decision.action);
    message.trigger_state = static_cast<std::uint8_t>(decision.trigger_state);
    message.confidence = decision.confidence;
    message.distance_m = decision.distance_m;
    status_publisher_->publish(message);
  }

  void maybe_call_trigger(const ControllerDecision& decision)
  {
    if (decision.trigger_state != TriggerState::kTriggerRequested) {
      return;
    }
    auto request = std::make_shared<srv::TriggerActuator::Request>();
    request->confidence = decision.confidence;
    request->distance_m = decision.distance_m;
    trigger_client_->async_send_request(request);
  }

  TurretControllerLogic logic_{0.80F, 30.0F};
  bool actuator_ready_{true};
  rclcpp::Subscription<msg::Target>::SharedPtr target_subscription_;
  rclcpp::Subscription<msg::ActuatorStatus>::SharedPtr actuator_subscription_;
  rclcpp::Publisher<msg::GimbalCommand>::SharedPtr gimbal_publisher_;
  rclcpp::Publisher<msg::ServoCommand>::SharedPtr servo_publisher_;
  rclcpp::Publisher<msg::TurretStatus>::SharedPtr status_publisher_;
  rclcpp::Client<srv::TriggerActuator>::SharedPtr trigger_client_;
};

}  // namespace antidrone_turret

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<antidrone_turret::TurretControllerNode>());
  rclcpp::shutdown();
  return 0;
}
