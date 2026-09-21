"""launch/patrol.launch.py — стартує обидві ноди patrol_bot разом."""
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(package="patrol_bot", executable="patrol_mission_node",
             output="screen"),
        Node(package="patrol_bot", executable="http_status_node",
             output="screen"),
    ])
