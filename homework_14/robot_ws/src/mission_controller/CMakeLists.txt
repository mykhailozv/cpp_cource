cmake_minimum_required(VERSION 3.20)
project(mission_controller)

# C++20 - базовий стандарт курсу.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Інтерфейс msg/srv живе в пакеті underground_world. Ми його перевикористовуємо,
# тому рішенню достатньо дотримуватися контракту топіків та сервісу.
find_package(underground_world REQUIRED)

# Чистий, агностичний до ROS мозок дослідження (тестується без ROS).
ament_auto_add_library(mission_controller_core
  src/explorer_brain.cpp
)
target_include_directories(mission_controller_core PUBLIC include)

# Керуюча нода: читає /robot/local_scan + /robot/result, публікує
# /robot/cmd_move + /student/status, викликає /payload/trigger.
ament_auto_add_executable(mission_explorer_node src/mission_explorer_node.cpp)
# underground_world_LIBRARIES містить абсолютні шляхи до underground_world_core
# та typesupport-бібліотек (експортовані пакетом underground_world).
target_link_libraries(mission_explorer_node mission_controller_core ${underground_world_LIBRARIES})

# Нода дії: надає /payload/trigger, публікує /payload/enemy_down.
ament_auto_add_executable(payload_action_node src/payload_action_node.cpp)
target_link_libraries(payload_action_node ${underground_world_LIBRARIES})

# Типи повідомлень/сервісів беруться з underground_world; його typesupport
# є імпортованим CMake-таргетом, експортованим пакетом underground_world.
set(cpp_typesupport_target underground_world::underground_world__rosidl_typesupport_cpp)
target_link_libraries(mission_explorer_node "${cpp_typesupport_target}")
target_link_libraries(payload_action_node "${cpp_typesupport_target}")

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(explorer_brain_test
    test/explorer_brain_test.cpp
  )
  target_include_directories(explorer_brain_test PRIVATE include)
  target_link_libraries(explorer_brain_test mission_controller_core ${underground_world_LIBRARIES})
  target_compile_definitions(explorer_brain_test
    PRIVATE UNDERGROUND_WORLD_SOURCE_DIR="${CMAKE_SOURCE_DIR}/../underground_world"
  )
endif()

ament_export_dependencies(rosidl_default_runtime)

ament_auto_package()