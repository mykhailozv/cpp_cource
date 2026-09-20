# ДЗ 14 — робот у траншеях

"underground world" знає всю карту, але показує роботу тільки 3x3 навколо. Треба пройти все, що можна пройти, "зняти" і записати в rosbag.

## Що додано

пакет `mission_controller` з двома нодами:

- **mission_explorer** — головний "мозок". Слухає `/robot/local_scan`,
  запам'ятовує побачене у карту, пише рухи в `/robot/cmd_move` і
  статус у `/student/status`. Коли бачить контакт `C` — викликає сервіс
  `/payload/trigger`.
- **payload_action** — тримає сервіс `/payload/trigger` і після запиту
  публікує `/payload/enemy_down`, який і "знімає" контакт у "світі".

Launch `system.launch.py` — стартує обидві ноди разом зі світом.

## Алгоритм

Звичайний DFS:

1. є активний контакт в огляді — спершу тригер, рух чекаю (інакше рух/зняття контакту);
2. немає — йду на сусідню прохідну клітинку, в якій ще не був;
3. всі сусіди відвідані — відступаю назад стеком DFS;
4. стек порожній - все пройдено - SUCCESS.


начебто всі 4 сценарії — SUCCESS

## Як запустити

```bash
cd homework_14/robot_ws
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash

# один сценарій
ros2 launch underground_world system.launch.py scenario:=small_rooms.yaml

# всі сценарії + запис bag
bash run_all.sh
```


У скрипті місія йде з `move_commit_period_ms:=1000`, бо не встигає підписатись на топіки і перші повідомлення не потрапляють у bag.

Бонус із поверненням не робив

## Перевірка без ROS

можна ганяти окремо проти справжньої `WorldModel`:

```bash
cd robot_ws/src/mission_controller
UW=../underground_world
clang++ -std=c++20 -O2 -I include tools/offline_sim.cpp src/explorer_brain.cpp \
    $UW/src/scenario_loader.cpp $UW/src/world_model.cpp \
    -I $UW/include -DUNDERGROUND_WORLD_SOURCE_DIR=\"$UW\" -o /tmp/sim
/tmp/sim
```
