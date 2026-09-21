// patrol_mission_node — ROS-обгортка навколо PatrolController.
//   підписка /odom, таймер 10 Гц -> публікація /cmd_vel;
//   /ugc_mode (std_msgs/String: PATROL|MANUAL) — перемикання режиму;
//   /ugc_status (std_msgs/String, JSON) 1 Гц — для http_node і rosbag.

#include "patrol_bot/patrol_controller.hpp"

#include <cmath>
#include <cstdio>
#include <string>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

namespace patrol_bot {

class PatrolMissionNode : public rclcpp::Node {
public:
    PatrolMissionNode()
        : rclcpp::Node("patrol_mission_node"),
          controller_({{1.5, 1.5}, {-1.5, 1.5}, {-1.5, -1.5}, {1.5, -1.5}}) {
        // Jazzy: міст Gazebo очікує TwistStamped на /cmd_vel (не плаский Twist).
        cmd_pub_ = create_publisher<geometry_msgs::msg::TwistStamped>("/cmd_vel", 10);
        status_pub_ = create_publisher<std_msgs::msg::String>("/ugc_status", 10);
        mode_sub_ = create_subscription<std_msgs::msg::String>(
            "/ugc_mode", 10,
            [this](std_msgs::msg::String::ConstSharedPtr msg) {
                const std::string m = msg->data;
                if (m == "PATROL" || m == "patrol") {
                    controller_.setMode(Mode::Patrol);
                } else if (m == "MANUAL" || m == "manual") {
                    controller_.setMode(Mode::Manual);
                } else {
                    RCLCPP_WARN(get_logger(), "невідомий режим: %s", m.c_str());
                }
            });
        odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
            "/odom", 10,
            [this](nav_msgs::msg::Odometry::ConstSharedPtr msg) {
                last_odom_ = *msg;
                have_odom_ = true;
            });

        control_timer_ = create_wall_timer(
            std::chrono::milliseconds(100), [this] { controlLoop(); });
        status_timer_ = create_wall_timer(
            std::chrono::seconds(1), [this] { publishStatus(); });

        RCLCPP_INFO(get_logger(), "patrol_mission_node запущено, режим PATROL");
    }

private:
    void controlLoop() {
        if (!have_odom_ || controller_.mode() == Mode::Manual) {
            return;  // оператор керує сам
        }
        const auto& p = last_odom_.pose.pose.position;
        const auto& q = last_odom_.pose.pose.orientation;
        const double yaw = std::atan2(2.0 * (q.w * q.z + q.x * q.y),
                                      1.0 - 2.0 * (q.y * q.y + q.z * q.z));
        const Twist2D cmd = controller_.update(p.x, p.y, yaw);
        geometry_msgs::msg::TwistStamped out;
        out.header.stamp = now();
        out.header.frame_id = "base_footprint";
        out.twist.linear.x = cmd.linear;
        out.twist.angular.z = cmd.angular;
        cmd_pub_->publish(out);
    }

    void publishStatus() {
        char buf[512];
        const auto& [tx, ty] = controller_.target();
        std::snprintf(buf, sizeof(buf),
                      R"({"mode":"%s","position":{"x":%.3f,"y":%.3f},)"
                      R"("waypoint":{"index":%d,"x":%.3f,"y":%.3f},)"
                      R"("trip_m":%.2f})",
                      controller_.mode() == Mode::Patrol ? "PATROL" : "MANUAL",
                      have_odom_ ? last_odom_.pose.pose.position.x : 0.0,
                      have_odom_ ? last_odom_.pose.pose.position.y : 0.0,
                      controller_.waypointIndex(), tx, ty,
                      controller_.tripMeters());
        std_msgs::msg::String msg;
        msg.data = buf;
        status_pub_->publish(msg);
    }

    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr mode_sub_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
    rclcpp::TimerBase::SharedPtr control_timer_;
    rclcpp::TimerBase::SharedPtr status_timer_;
    nav_msgs::msg::Odometry last_odom_;
    bool have_odom_ = false;
    PatrolController controller_;
};

}  // namespace patrol_bot

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<patrol_bot::PatrolMissionNode>());
    rclcpp::shutdown();
    return 0;
}
