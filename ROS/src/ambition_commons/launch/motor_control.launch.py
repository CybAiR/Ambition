from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    joy_node = Node(
        package='joy',
        executable='joy_node',
        name='joy_node',
        output='screen',
        parameters=[{
            'dev': '/dev/input/js0',
            'deadzone': 0.05,
            'autorepeat_rate': 20.0,
        }]
    )

    teleop_node = Node(
        package='teleop_twist_joy',
        executable='teleop_node',
        name='teleop_twist_joy',
        output='screen',
        parameters=[{
            'axis_linear.x': 1,
            'axis_angular.yaw': 0,
            'scale_linear.x': 0.5,
            'scale_angular.yaw': 1.0,
            'require_enable_button': False,
    }])

    motor_node = Node(
        package='ambition_control',
        executable='motor_control',
        name='motor_control'
    )
    

    return LaunchDescription([
        joy_node,
        teleop_node
    ])