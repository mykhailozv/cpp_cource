#pragma once

// patrol_controller — чиста логіка патрулювання без залежностей від ROS.
// Так само, як explorer_brain у ДЗ14: можна тестувати gtest-ом і запускати
// офлайн (tools/offline_check.cpp) без ROS.

#include <cmath>
#include <utility>
#include <vector>

namespace patrol_bot {

struct Twist2D {
    double linear = 0.0;   // м/с
    double angular = 0.0;  // рад/с
};

enum class Mode {
    Patrol,  // автономний об'їзд периметра
    Manual,  // керування оператором (вузол не публікує /cmd_vel)
};

struct Params {
    double pos_tol = 0.15;   // поріг досягнення точки, м
    double yaw_tol = 0.08;   // поріг кута повороту, рад
    double turn_speed = 0.8; // макс. кутова швидкість, рад/с
    double max_speed = 0.3;  // макс. лінійна швидкість, м/с
    double k_yaw = 2.0;      // P-коефіцієнт повороту
    double k_dist = 1.0;     // P-коефіцієнт швидкості
};

class PatrolController {
public:
    using Point = std::pair<double, double>;

    PatrolController(std::vector<Point> waypoints, Params params = {})
        : waypoints_(std::move(waypoints)), params_(params) {}

    Mode mode() const { return mode_; }
    void setMode(Mode m) { mode_ = m; }

    int waypointIndex() const { return wp_index_; }
    const Point& target() const { return waypoints_[wp_index_]; }
    const std::vector<Point>& waypoints() const { return waypoints_; }

    // Оновлення стану контролера одометрією. Повертає команду для /cmd_vel.
    // У режимі Manual завжди повертає нульову команду (вузол її не публікує).
    Twist2D update(double x, double y, double yaw) {
        have_odom_ = true;
        if (have_prev_) {
            trip_m_ += std::hypot(x - prev_x_, y - prev_y_);
        }
        prev_x_ = x;
        prev_y_ = y;
        have_prev_ = true;
        last_yaw_ = yaw;

        if (mode_ == Mode::Manual) {
            return {};
        }

        const auto [tx, ty] = target();
        const double dist = std::hypot(tx - x, ty - y);

        if (dist < params_.pos_tol) {
            wp_index_ = (wp_index_ + 1) % static_cast<int>(waypoints_.size());
            return {};
        }

        const double desired_yaw = std::atan2(ty - y, tx - x);
        const double err_yaw = wrapAngle(desired_yaw - yaw);

        Twist2D cmd;
        if (std::abs(err_yaw) > params_.yaw_tol) {
            cmd.angular = clamp(params_.k_yaw * err_yaw, params_.turn_speed);
        } else {
            cmd.linear = clamp(params_.k_dist * dist, params_.max_speed);
        }
        return cmd;
    }

    bool haveOdom() const { return have_odom_; }
    double tripMeters() const { return trip_m_; }

private:
    static double wrapAngle(double a) {
        return std::fmod(a + M_PI, 2.0 * M_PI) - M_PI;
    }
    static double clamp(double v, double limit) {
        return std::max(-limit, std::min(limit, v));
    }

    std::vector<Point> waypoints_;
    Params params_;
    Mode mode_ = Mode::Patrol;
    int wp_index_ = 0;
    bool have_odom_ = false;
    double trip_m_ = 0.0;
    double last_yaw_ = 0.0;
    bool have_prev_ = false;
    double prev_x_ = 0.0;
    double prev_y_ = 0.0;
};

}  // namespace patrol_bot
