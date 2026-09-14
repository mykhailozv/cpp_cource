// yaw_servo_driver_node.cpp
//
// Simplified yaw-servo driver: subscribes to /servo/cmd and logs the received
// typed ServoCommand (no real hardware is controlled in this homework).

#include "antidrone_turret/msg/servo_command.hpp"
#include "rclcpp/rclcpp.hpp"

namespace {

constexpr auto kServoCommandTopic = "/servo/cmd";

const char* direction_name(const std::int8_t direction)
{
  if (direction == antidrone_turret::msg::ServoCommand::RIGHT) {
    return "RIGHT";
  }
  if (direction == antidrone_turret::msg::ServoCommand::LEFT) {
    return "LEFT";
  }
  return "CENTER";
}

}  // namespace

class YawServoDriverNode final : public rclcpp::Node {
public:
  YawServoDriverNode()
    : Node("yaw_servo_driver_node")
  {
    subscription_ = create_subscription<antidrone_turret::msg::ServoCommand>(
      kServoCommandTopic,
      10,
      [this](const antidrone_turret::msg::ServoCommand& message) {
        on_command(message);
      });
  }

private:
  void on_command(const antidrone_turret::msg::ServoCommand& message)
  {
    RCLCPP_INFO(
      get_logger(),
      "yaw_servo_driver_node отримав: direction=%s target_x=%.1f error_x=%.1f",
      direction_name(message.direction),
      message.target_x,
      message.error_x);
  }

  rclcpp::Subscription<antidrone_turret::msg::ServoCommand>::SharedPtr subscription_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<YawServoDriverNode>());
  rclcpp::shutdown();
  return 0;
}
