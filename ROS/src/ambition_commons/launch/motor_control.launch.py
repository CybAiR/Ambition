from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    motor_node = Node(
        package='ambition_control',
        executable='motor_control',
        name='motor_control',
        parameters=['/root/Shared/Ambition/ROS/src/ambition_commons/config/motor_control.yaml']
    )

    return LaunchDescription([
        motor_node
    ])