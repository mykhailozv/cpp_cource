// gimbal_driver_node.cpp
//
// Simplified gimbal driver: subscribes to /gimbal/cmd and logs the received
// typed GimbalCommand (no real hardware is controlled in this homework).

#include "antidrone_turret/msg/gimbal_command.hpp"
#include "rclcpp/rclcpp.hpp"

namespace {

constexpr auto kGimbalCommandTopic = "/gimbal/cmd";

const char* direction_name(const std::int8_t direction)
{
  if (direction == antidrone_turret::msg::GimbalCommand::UP) {
    return "UP";
  }
  if (direction == antidrone_turret::msg::GimbalCommand::DOWN) {
    return "DOWN";
  }
  return "CENTER";
}

}  // namespace

class GimbalDriverNode final : public rclcpp::Node {
public:
  GimbalDriverNode()
    : Node("gimbal_driver_node")
  {
    subscription_ = create_subscription<antidrone_turret::msg::GimbalCommand>(
      kGimbalCommandTopic,
      10,
      [this](const antidrone_turret::msg::GimbalCommand& message) {
        on_command(message);
      });
  }

private:
  void on_command(const antidrone_turret::msg::GimbalCommand& message)
  {
    RCLCPP_INFO(
      get_logger(),
      "gimbal_driver_node отримав: direction=%s target_y=%.1f error_y=%.1f",
      direction_name(message.direction),
      message.target_y,
      message.error_y);
  }

  rclcpp::Subscription<antidrone_turret::msg::GimbalCommand>::SharedPtr subscription_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<GimbalDriverNode>());
  rclcpp::shutdown();
  return 0;
}
