#!/usr/bin/env bash
# Збірка ДЗ 14 і запуск сценаріїв
#
# Використання:
#   cd homework_14/robot_ws
#   source /opt/ros/jazzy/setup.bash
#   bash run_all.sh
# set -u не використовуємо: скрипти ROS (setup.bash) несумісні з nounset.
set -eo pipefail

cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source /opt/ros/jazzy/setup.bash

colcon build --symlink-install
source install/setup.bash

# Директорія з записами: <root>/bags/<scenario>
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "${ROOT}/bags"

declare -a SCENARIOS=(
  "training_corridor training_corridor.yaml"
  "small_rooms small_rooms.yaml"
  "branching_trench branching_trench.yaml"
  "dead_end_bunker dead_end_bunker.yaml"
)

for entry in "${SCENARIOS[@]}"; do
  read -r name yaml <<<"$entry"
  bag_dir="${ROOT}/bags/${name}"
  echo ">>> recording ${name} into ${bag_dir}"
  rm -rf "${bag_dir}"
  # ros2 bag record стартує в фоновому режимі.
  ros2 bag record -a -o "${bag_dir}" >/tmp/bag_record_${name}.log 2>&1 &
  BAG_PID=$!
  # Невелика затримка, щоб record піднявся до старту системи.
  sleep 5

  # launch не завершується сам після SUCCESS, тому запускаємо його у фоні
  # і чекаємо фінальний результат або вичерпуємо безпечний таймаут.
  # Прогін робимо повільним (1000 мс на застосування команди): бо на швидкій місії перші повідомлення не потрапляють у bag.
  LOG=/tmp/launch_${name}.log
  ros2 launch underground_world system.launch.py scenario:=${yaml} move_commit_period_ms:=1000 >"${LOG}" 2>&1 &
  LAUNCH_PID=$!
  RESULT=""
  for _ in $(seq 1 150); do
    RESULT=$(grep -oE 'world result=(RUNNING|SUCCESS|FAILED_MAX_STEPS)' "${LOG}" | tail -1 || true)
    if [[ "${RESULT}" == *SUCCESS* || "${RESULT}" == *FAILED* ]]; then
      break
    fi
    sleep 1
  done
  sleep 1
  kill "${LAUNCH_PID}" 2>/dev/null || true
  wait "${LAUNCH_PID}" 2>/dev/null || true

  # прибираємо випадково живі дочірні ноди
  pkill -f underground_world_node 2>/dev/null || true
  pkill -f mission_explorer_node 2>/dev/null || true
  pkill -f payload_action_node 2>/dev/null || true
  sleep 1

  kill "${BAG_PID}" 2>/dev/null || true
  wait "${BAG_PID}" 2>/dev/null || true
  echo ">>> ${name}: ${RESULT:-NO_RESULT_IN_TIME}"
  echo ">>> bag info:"
  ros2 bag info "${bag_dir}" | grep -E '/robot|/payload|/student' || true
done

echo ">>> done"