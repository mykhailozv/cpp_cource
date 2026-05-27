cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 06. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(ballistics_cli CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ballistics src/ballistics.cpp)
target_include_directories(ballistics PUBLIC include)
# Один executable для задачі з ballistics_cli.
add_executable(ballistics_cli src/main.cpp)
target_link_libraries(ballistics_cli PRIVATE ballistics)

# Базові compiler warnings допомагають побачити підозрілі місця до runtime.
target_compile_options(ballistics_cli PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)

    include(FetchContent)

    include(GoogleTest)

    FetchContent_Declare(
        googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
                       DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

    FetchContent_MakeAvailable(googletest)

    add_executable(ballistics_tests tests/ballistics_tests.cpp)

    target_compile_definitions(ballistics_tests
                               PRIVATE TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    target_link_libraries(ballistics_tests PRIVATE ballistics GTest::gtest_main)

    gtest_discover_tests(ballistics_tests DISCOVERY_MODE PRE_TEST)

endif()
