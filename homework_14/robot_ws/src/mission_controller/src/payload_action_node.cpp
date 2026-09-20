// payload_action_node - надає сервіс /payload/trigger і після прийнятого
// запиту симулює зняття контакту, публікуючи /payload/enemy_down. Його
// споживає underground_world_node, який виконує справжню валідацію та
// облік метрик.
//
// Виконується контракт ROS 2:
//   * надає сервіс /payload/trigger
//   * публікує /payload/enemy_down

#include <cstdint>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "underground_world/msg/enemy_down.hpp"
#include "underground_world/srv/payload_trigger.hpp"

using underground_world::msg::EnemyDown;
using underground_world::srv::PayloadTrigger;

constexpr auto kEnemyDownTopic = "/payload/enemy_down";
constexpr auto kTriggerService = "/payload/trigger";

class PayloadActionNode final : public rclcpp::Node {
public:
  PayloadActionNode()
    : Node("payload_action_node")
  {
    // Стандартний reliable QoS збігається з підпискою underground_world_node.
    enemy_down_pub_ = create_publisher<EnemyDown>(kEnemyDownTopic, rclcpp::QoS{10});

    trigger_srv_ = create_service<PayloadTrigger>(
      kTriggerService,
      [this](const std::shared_ptr<PayloadTrigger::Request> request,
             std::shared_ptr<PayloadTrigger::Response> response) {
        on_trigger(*request, *response);
      });

    RCLCPP_INFO(get_logger(), "payload_action_node serving %s", kTriggerService);
  }

private:
  void on_trigger(const PayloadTrigger::Request& req, PayloadTrigger::Response& response)
  {
    RCLCPP_INFO(get_logger(),
                "trigger request contact_id=%d x=%d y=%d",
                req.contact_id,
                req.x,
                req.y);

    // Симульована дія: будь-який запит приймається і перетворюється на подію
    // enemy_down. Світ є джерелом істини для валідності та метрик.
    EnemyDown down;
    down.contact_id = req.contact_id;
    down.x = req.x;
    down.y = req.y;
    enemy_down_pub_->publish(down);

    response.accepted = true;
    response.reason = "enemy_down dispatched";
    RCLCPP_INFO(get_logger(), "published enemy_down contact_id=%d", req.contact_id);
  }

  rclcpp::Publisher<EnemyDown>::SharedPtr enemy_down_pub_;
  rclcpp::Service<PayloadTrigger>::SharedPtr trigger_srv_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PayloadActionNode>());
  rclcpp::shutdown();
  return 0;
}