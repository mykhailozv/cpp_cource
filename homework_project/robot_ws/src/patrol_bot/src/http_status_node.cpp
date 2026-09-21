// http_status_node — HTTP API на cpp-httplib (як у ДЗ15: header-only,
// third_party/httplib.h, зовнішні програми не викликаються).
//   GET  /status     — останній стан з /ugc_status;
//   GET  /perimeter  — заданий периметр;
//   POST /mode       — {"mode":"PATROL"|"MANUAL"} -> публікація /ugc_mode.

#include <httplib.h>

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

namespace patrol_bot {

class HttpStatusNode : public rclcpp::Node {
public:
    HttpStatusNode()
        : rclcpp::Node("http_status_node") {
        status_sub_ = create_subscription<std_msgs::msg::String>(
            "/ugc_status", 10,
            [this](std_msgs::msg::String::ConstSharedPtr msg) {
                std::lock_guard<std::mutex> lock(mutex_);
                last_status_ = msg->data;
            });
        mode_pub_ = create_publisher<std_msgs::msg::String>("/ugc_mode", 10);
        setupHttp();
    }

    ~HttpStatusNode() override {
        if (server_thread_.joinable()) {
            server_.stop();
            server_thread_.join();
        }
    }

private:
    void setupHttp() {
        server_.Get("/status", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (last_status_.empty()) {
                res.set_content(R"({"mode":"UNKNOWN"})", "application/json");
            } else {
                res.set_content(last_status_, "application/json");
            }
        });
        server_.Get("/perimeter", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mutex_);
            res.set_content(R"({"perimeter":[{"x":1.5,"y":1.5},{"x":-1.5,"y":1.5},)"
                            R"({"x":-1.5,"y":-1.5},{"x":1.5,"y":-1.5}]})", "application/json");
        });
        server_.Post("/mode", [this](const httplib::Request& req, httplib::Response& res) {
            const std::string& body = req.body;
            const auto found = body.find("\"mode\"");
            const auto colon = body.find(':', found == std::string::npos ? 0 : found);
            const auto q1 = body.find('"', colon);
            const auto q2 = q1 == std::string::npos ? std::string::npos
                                                    : body.find('"', q1 + 1);
            if (q1 == std::string::npos || q2 == std::string::npos) {
                res.status = 400;
                res.set_content(R"({"error":"потрібно {\"mode\":\"...\"}"})", "application/json");
                return;
            }
            const std::string mode = body.substr(q1 + 1, q2 - q1 - 1);
            std_msgs::msg::String msg;
            msg.data = mode;
            mode_pub_->publish(msg);
            res.set_content(R"({"ok":true})", "application/json");
        });

        server_thread_ = std::thread([this] {
            RCLCPP_INFO(get_logger(), "HTTP API: http://localhost:8080/status");
            server_.listen("0.0.0.0", 8080);
        });
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr status_sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr mode_pub_;
    httplib::Server server_;
    std::thread server_thread_;
    std::mutex mutex_;
    std::string last_status_;
};

}  // namespace patrol_bot

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<patrol_bot::HttpStatusNode>());
    rclcpp::shutdown();
    return 0;
}
