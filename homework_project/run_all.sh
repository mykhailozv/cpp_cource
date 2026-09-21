#!/usr/bin/env bash
# Запуск усього стеку одним скриптом (патерн run_all.sh з ДЗ14):
# Gazebo + TurtleBot3 -> colcon build -> launch нод -> rosbag record.
# Без `set -u`: ROS-скрипти setup.bash звертаються до необов'язкових
# змінних (AMENT_TRACE_SETUP_FILES тощо) і з -u падають.
set -e -o pipefail

cd "$(dirname "$0")/robot_ws"
source /opt/ros/jazzy/setup.bash
export TURTLEBOT3_MODEL=${TURTLEBOT3_MODEL:-burger}
export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:/opt/ros/jazzy/share/turtlebot3_gazebo/models

cleanup() { kill 0 2>/dev/null || true; }
trap cleanup EXIT INT TERM

echo "==> colcon build"
colcon build --symlink-install
source install/setup.bash

echo "==> Gazebo + TurtleBot3 (turtlebot3_world)"
ros2 launch turtlebot3_gazebo turtlebot3_world.launch.py &
sleep 10

echo "==> patrol_bot (mission + http) + rosbag"
BAG_DIR="../bags/patrol_$(date +%Y%m%d_%H%M%S)"
mkdir -p ../bags
ros2 launch patrol_bot patrol.launch.py &
sleep 3
ros2 bag record -o "$BAG_DIR" /odom /scan /cmd_vel /ugc_status /ugc_mode >/dev/null 2>&1 &

echo
echo "Стек запущено:"
echo "  Статус:   curl http://localhost:8080/status"
echo "  Периметр: curl http://localhost:8080/perimeter"
echo "  MANUAL:   curl -X POST http://localhost:8080/mode -d '{\"mode\":\"MANUAL\"}'"
echo "  PATROL:   curl -X POST http://localhost:8080/mode -d '{\"mode\":\"PATROL\"}'"
echo "  Ручне керування: ros2 run teleop_twist_keyboard teleop_twist_keyboard"
echo
echo "Ctrl+C — зупинити все (bag збережено у $BAG_DIR)"
wait
