// Офлайн-перевірка логіки без ROS (патерн offline_sim з ДЗ14):
//   clang++ -std=c++20 -O2 -I include tools/offline_check.cpp -o /tmp/offline_check
// Симулює рух по периметру з P-контролером і друкує трасу.

#include <cmath>
#include <cstdio>

#include "patrol_bot/patrol_controller.hpp"

int main() {
    using namespace patrol_bot;
    PatrolController controller({{1.5, 1.5}, {-1.5, 1.5}, {-1.5, -1.5}, {1.5, -1.5}});
    double x = 0.0, y = 0.0, yaw = 0.0;
    const double dt = 0.1;
    int waypoints_done = 0;
    const int last_index = 3;

    for (int step = 0; step < 5000; ++step) {
        const int before = controller.waypointIndex();
        const Twist2D cmd = controller.update(x, y, yaw);
        x += cmd.linear * dt * std::cos(yaw);
        y += cmd.linear * dt * std::sin(yaw);
        yaw += cmd.angular * dt;
        if (controller.waypointIndex() != before) {
            ++waypoints_done;
            std::printf("t=%.1fs wp %d досягнуто: (%.2f, %.2f)\n",
                        step * dt, before, x, y);
            if (before == last_index) {
                std::printf("Периметр замкнуто. Пройдено: %.2f м\n",
                            controller.tripMeters());
                return 0;
            }
        }
    }
    std::printf("FAIL: периметр не замкнуто (зроблено %d точок)\n", waypoints_done);
    return 1;
}
