cmake_minimum_required(VERSION 3.20)

project(homework_15 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(dz15_poster src/main.cpp)

if(WIN32)
    target_link_libraries(dz15_poster PRIVATE ws2_32)
endif()

target_include_directories(dz15_poster PRIVATE third_party)