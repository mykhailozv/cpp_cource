# homework_13/CMakeLists.txt
#
# Окремий CMake-проєкт unit-тестів пакета antidrone_turret. Можна збирати:
#   * напряму:  cmake -S . -B build && cmake --build build && ctest --test-dir build
#   * через root: після `add_subdirectory(homework_13)` у головному CMakeLists.txt
# Це НЕ ROS-пакет: тести використовують лише чисту C++ логіку + googletest.

cmake_minimum_required(VERSION 3.20)
project(antidrone_turret_homework CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# Надає макрос gtest_discover_tests().
include(GoogleTest)

set(ANTIDRONE_TURRET_PKG "${CMAKE_CURRENT_SOURCE_DIR}/robot_ws/src/antidrone_turret")
set(PKG_INCLUDE "${ANTIDRONE_TURRET_PKG}/include")

# ---- googletest: перевикористати, якщо вже є (наприклад, root вже має
# homework_06 з тим самим googletest). Інакше — локальна копія або FetchContent.
if(NOT TARGET GTest::gtest_main)
  set(_local_gt "")
  foreach(_cand
      "${CMAKE_CURRENT_SOURCE_DIR}/../../../build/_deps/googletest-src"
      "/home/famil/cpp_cource/build/_deps/googletest-src")
    if(EXISTS "${_cand}/CMakeLists.txt")
      set(_local_gt "${_cand}")
      break()
    endif()
  endforeach()

  if(_local_gt)
    message(STATUS "[homework_13] reusing local googletest: ${_local_gt}")
    add_subdirectory("${_local_gt}" "${CMAKE_BINARY_DIR}/googletest-reuse" EXCLUDE_FROM_ALL)
  else()
    include(FetchContent)
    FetchContent_Declare(
      googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
                     DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

# ---- Тест логіки контролера ----
add_executable(turret_controller_logic_test
  "${ANTIDRONE_TURRET_PKG}/test/turret_controller_logic_test.cpp")
target_include_directories(turret_controller_logic_test PRIVATE "${PKG_INCLUDE}")
target_compile_options(turret_controller_logic_test PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(turret_controller_logic_test PRIVATE GTest::gtest_main)
gtest_discover_tests(turret_controller_logic_test DISCOVERY_MODE PRE_TEST)

# ---- Наданий тест логіки (Start package) ----
add_executable(provided_logic_test
  "${ANTIDRONE_TURRET_PKG}/test/provided_logic_test.cpp")
target_include_directories(provided_logic_test PRIVATE "${PKG_INCLUDE}")
target_compile_definitions(provided_logic_test PRIVATE
  ANTIDRONE_TURRET_SOURCE_DIR="${ANTIDRONE_TURRET_PKG}")
target_compile_options(provided_logic_test PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(provided_logic_test PRIVATE GTest::gtest_main)
gtest_discover_tests(provided_logic_test DISCOVERY_MODE PRE_TEST)
