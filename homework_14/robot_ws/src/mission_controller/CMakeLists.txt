cmake_minimum_required(VERSION 3.20)
project(mission_controller)

# C++20 - базовий стандарт курсу.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Інтерфейс msg/srv живе в пакеті underground_world. Ми його перевикористовуємо
find_package(underground_world REQUIRED)

ament_auto_add_library(mission_controller_core
  src/explorer_brain.cpp
)
target_include_directories(mission_controller_core PUBLIC include)

ament_auto_add_executable(mission_explorer_node src/mission_explorer_node.cpp)
target_link_libraries(mission_explorer_node mission_controller_core ${underground_world_LIBRARIES})

ament_auto_add_executable(payload_action_node src/payload_action_node.cpp)
target_link_libraries(payload_action_node ${underground_world_LIBRARIES})

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