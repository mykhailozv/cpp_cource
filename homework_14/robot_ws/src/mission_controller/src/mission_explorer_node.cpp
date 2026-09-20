// mission_explorer_node - керуюча нода, яка:
//   * читає /robot/local_scan   (локальні спостереження світу)
//   * читає /robot/result      (результат місії для раннього завершення)
//   * будує внутрішню пам'ять карти і обирає рухи (DFS у ExplorerBrain)
//   * публікує /robot/cmd_move
//   * публікує /student/status
//   * викликає сервіс /payload/trigger, коли видно контакт
//
// Контракт ROS 2, який виконує рішення:
//   * публікує /robot/cmd_move, /student/status
//   * викликає /payload/trigger (payload_action_node публікує /payload/enemy_down)

#include <chrono>
#include <cstdint>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "mission_controller/explorer_brain.hpp"
#include "underground_world/msg/local_scan.hpp"
#include "underground_world/msg/move_command.hpp"
#include "underground_world/msg/robot_result.hpp"
#include "underground_world/msg/student_status.hpp"
#include "underground_world/srv/payload_trigger.hpp"
#include "underground_world/state_qos.hpp"

using underground_world::msg::LocalScan;
using underground_world::msg::MoveCommand;
using underground_world::msg::RobotResult;
using underground_world::msg::StudentStatus;
using underground_world::srv::PayloadTrigger;

constexpr auto kScanTopic = "/robot/local_scan";
constexpr auto kResultTopic = "/robot/result";
constexpr auto kMoveTopic = "/robot/cmd_move";
constexpr auto kStatusTopic = "/student/status";
constexpr auto kTriggerService = "/payload/trigger";

namespace {

mission_controller::BrainScan to_brain_scan(const LocalScan& msg)
{
  mission_controller::BrainScan scan;
  scan.scenario_name = msg.scenario_name;
  scan.robot_x = msg.robot_x;
  scan.robot_y = msg.robot_y;
  scan.cells.reserve(msg.cells.size());
  for (const auto& cell : msg.cells) {
    mission_controller::BrainCell brain_cell;
    brain_cell.x = cell.x;
    brain_cell.y = cell.y;
    brain_cell.cell_type = cell.cell_type;
    brain_cell.contact_id = cell.contact_id;
    scan.cells.push_back(brain_cell);
  }
  return scan;
}

}  // namespace

class MissionExplorerNode final : public rclcpp::Node {
public:
  MissionExplorerNode()
    : Node("mission_explorer_node")
  {
    // Світ публікує state-топіки за допомогою make_state_qos() (reliable,
    // transient-local, keep-last 1). Застосовуємо той самий профіль і для
    // підписника, щоб пізній підписник усе одно отримав останній снапшот і
    // не виникала startup race (Update від 26-07-2026).
    const auto state_qos = underground_world::make_state_qos();
    const auto event_qos = rclcpp::QoS{10};

    scan_sub_ = create_subscription<LocalScan>(
      kScanTopic, state_qos, [this](const LocalScan::SharedPtr msg) { on_scan(*msg); });

    result_sub_ = create_subscription<RobotResult>(
      kResultTopic, state_qos, [this](const RobotResult::SharedPtr msg) { on_result(*msg); });

    move_pub_ = create_publisher<MoveCommand>(kMoveTopic, event_qos);
    status_pub_ = create_publisher<StudentStatus>(kStatusTopic, event_qos);
    trigger_client_ = create_client<PayloadTrigger>(kTriggerService);

    publish_status(mission_controller::kExploring);
    RCLCPP_INFO(get_logger(), "mission_explorer_node ready, trigger service %s", kTriggerService);
  }

private:
  void on_result(const RobotResult& msg)
  {
    brain_.set_world_status(msg.mission_result, msg.max_steps, msg.steps_taken);
    RCLCPP_INFO(get_logger(), "world result=%s reason=%s", msg.mission_result.c_str(), msg.reason.c_str());
  }

  void on_scan(const LocalScan& msg)
  {
    RCLCPP_INFO(get_logger(),
                "scan robot=(%d,%d) cells=%zu",
                msg.robot_x,
                msg.robot_y,
                msg.cells.size());

    brain_.process_scan(to_brain_scan(msg));
    act_on_brain();
  }

  void act_on_brain()
  {
    const auto action = brain_.next_action();
    publish_status(brain_.state());

    switch (action.kind) {
      case mission_controller::ActionKind::Trigger:
        call_trigger(action);
        break;
      case mission_controller::ActionKind::Move:
        publish_move(action.direction);
        break;
      case mission_controller::ActionKind::Idle:
      default:
        // Чекаємо, поки світ повторно опублікує стан, або місія завершена.
        break;
    }
  }
  void call_trigger(const mission_controller::BrainAction& action)
  {
    if (!trigger_client_->service_is_ready()) {
      if (!trigger_client_->wait_for_service(std::chrono::seconds(1))) {
        RCLCPP_WARN(get_logger(), "payload trigger service not ready, retrying on next scan");
        return;
      }
    }

    auto request = std::make_shared<PayloadTrigger::Request>();
    request->contact_id = action.contact_id;
    request->x = action.trigger_x;
    request->y = action.trigger_y;

    RCLCPP_INFO(get_logger(),
                "requesting trigger contact_id=%d x=%d y=%d",
                request->contact_id,
                request->x,
                request->y);

    // Fire-and-forget: відповідь сервісу не потрібна для логіки. Мозок далі
    // чекає новий огляд, який світ публікує після обробки /payload/enemy_down,
    // тому гонки між дією і наступним рухом не виникає.
    trigger_client_->async_send_request(request);
    RCLCPP_INFO(get_logger(), "trigger request sent");
  }

  void publish_move(std::uint8_t direction)
  {
    MoveCommand msg;
    switch (direction) {
      case mission_controller::kUp:
        msg.direction = MoveCommand::UP;
        break;
      case mission_controller::kDown:
        msg.direction = MoveCommand::DOWN;
        break;
      case mission_controller::kLeft:
        msg.direction = MoveCommand::LEFT;
        break;
      case mission_controller::kRight:
        msg.direction = MoveCommand::RIGHT;
        break;
      default:
        RCLCPP_WARN(get_logger(), "invalid direction %u, refusing", static_cast<unsigned>(direction));
        return;
    }
    move_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "published cmd_move direction=%u", static_cast<unsigned>(msg.direction));
  }

  void publish_status(std::uint8_t state)
  {
    StudentStatus msg;
    msg.state = state;
    status_pub_->publish(msg);
  }

  mission_controller::ExplorerBrain brain_;
  rclcpp::Subscription<LocalScan>::SharedPtr scan_sub_;
  rclcpp::Subscription<RobotResult>::SharedPtr result_sub_;
  rclcpp::Publisher<MoveCommand>::SharedPtr move_pub_;
  rclcpp::Publisher<StudentStatus>::SharedPtr status_pub_;
  rclcpp::Client<PayloadTrigger>::SharedPtr trigger_client_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MissionExplorerNode>());
  rclcpp::shutdown();
  return 0;
}