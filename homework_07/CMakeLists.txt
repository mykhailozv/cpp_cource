cmake_minimum_required(VERSION 3.20)

project(homework_07 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



add_library(mission_core
    src/geometry/Coord.cpp
    src/simulation/StepTimer.cpp
    src/simulation/SimStep.cpp
    src/simulation/Target.cpp
    src/exporters/JsonSimulationExporter.cpp
    src/MissionProcessor.cpp
    src/utils/MathUtils.cpp
    src/solvers/AnalyticalSolver.cpp
    src/providers/JsonTargetProvider.cpp
    src/config/FileConfigLoader.cpp
    src/config/ComponentFactory.cpp
)

target_include_directories(mission_core PUBLIC
    include
)

add_executable(homework_07 src/main.cpp)

target_link_libraries(homework_07 PRIVATE
    mission_core
)

target_compile_options(homework_07 PRIVATE -Wall -Wextra -pedantic)
