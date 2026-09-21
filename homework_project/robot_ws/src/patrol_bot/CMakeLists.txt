cmake_minimum_required(VERSION 3.20)
project(patrol_bot)

# C++20 - базовий стандарт курсу.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Ядро логіки без ROS — так само, як mission_controller_core у ДЗ14.
ament_auto_add_library(patrol_controller_core INTERFACE)
target_include_directories(patrol_controller_core INTERFACE include)

ament_auto_add_executable(patrol_mission_node src/patrol_mission_node.cpp)
target_link_libraries(patrol_mission_node patrol_controller_core)

ament_auto_add_executable(http_status_node src/http_status_node.cpp)
target_link_libraries(http_status_node patrol_controller_core)
target_include_directories(http_status_node PRIVATE third_party)

# Явне встановлення executable у lib/<пакет>/ — ros2 run/launch шукають саме там.
install(TARGETS patrol_mission_node http_status_node
  RUNTIME DESTINATION lib/${PROJECT_NAME})

# Launch-файли треба явно встановлювати у share/<пакет>/ (їх шукає ros2 launch)
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(patrol_controller_test test/patrol_controller_test.cpp)
  target_link_libraries(patrol_controller_test patrol_controller_core)
endif()

ament_package()
