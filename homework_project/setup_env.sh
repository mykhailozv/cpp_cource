#!/usr/bin/env bash

CODENAME=$(. /etc/os-release && echo $VERSION_CODENAME)
if [[ "$CODENAME" != "noble" ]]; then
  echo "Помилка: потрібен Ubuntu 24.04 (noble), у вас $CODENAME."
  echo "У WSL: wsl --install -d Ubuntu-24.04 (з PowerShell)."
  exit 1
fi


# Встановлення середовища (Ubuntu 24.04 / noble — як у робочому ДЗ14).
set -euo pipefail

sudo apt-get update
sudo apt-get install -y curl gnupg lsb-release software-properties-common
sudo add-apt-repository -y universe
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" \
  | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
sudo apt-get update

# Jazzy + Gazebo (входить у desktop) + TurtleBot3 + teleop
sudo apt-get install -y ros-jazzy-desktop ros-dev-tools \
  ros-jazzy-turtlebot3-gazebo ros-jazzy-turtlebot3-bringup \
  ros-jazzy-teleop-twist-keyboard

if ! grep -q "TURTLEBOT3_MODEL" ~/.bashrc; then
  cat >> ~/.bashrc <<'EOF'

# --- ROS2 (patrol_bot, setup_env.sh) ---
source /opt/ros/jazzy/setup.bash
export TURTLEBOT3_MODEL=burger
export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:/opt/ros/jazzy/share/turtlebot3_gazebo/models
EOF
fi

echo "Готово. Перезапустіть термінал або: source ~/.bashrc. Далі: bash run_all.sh"
