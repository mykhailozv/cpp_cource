from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # Сценарій можна змінити без редагування launch-файлу:
    # ros2 launch underground_world system.launch.py scenario:=small_rooms.yaml
    scenario = LaunchConfiguration("scenario")
    move_commit_period_ms = LaunchConfiguration("move_commit_period_ms")
    scenario_path = PathJoinSubstitution(
        [FindPackageShare("underground_world"), "config", scenario]
    )

    world_node = Node(
        package="underground_world",
        executable="underground_world_node",
        name="underground_world_node",
        output="screen",
        parameters=[
            {
                "scenario_path": scenario_path,
                "move_commit_period_ms": ParameterValue(
                    move_commit_period_ms, value_type=int
                ),
            }
        ],
    )

    # --- Ноди рішення (ДЗ 14) ---
    # payload_action: надає /payload/trigger і публікує /payload/enemy_down.
    payload_action_node = Node(
        package="mission_controller",
        executable="payload_action_node",
        name="payload_action_node",
        output="screen",
    )

    # mission_explorer: читає /robot/local_scan та /robot/result, будує пам'ять
    # карти, обирає рухи, публікує /robot/cmd_move і /student/status, викликає
    # /payload/trigger.
    mission_explorer_node = Node(
        package="mission_controller",
        executable="mission_explorer_node",
        name="mission_explorer_node",
        output="screen",
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "scenario",
                default_value="training_corridor.yaml",
                description="Scenario YAML file from underground_world/config",
            ),
            DeclareLaunchArgument(
                "move_commit_period_ms",
                default_value="50",
                description="Delay before applying queued move commands",
            ),
            world_node,
            payload_action_node,
            mission_explorer_node,
        ]
    )
