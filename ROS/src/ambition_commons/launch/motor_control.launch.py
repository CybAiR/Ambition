from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    candle_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('candle_ros2'),
                'launch',
                'md_node_launch.py'
            )
        )
    )
    
    motor_node = Node(
        package='ambition_control',
        executable='motor_control',
        name='motor_control',
        parameters=[os.path.join(
            get_package_share_directory('ambition_commons'),
            'config',
            'motor_control.yaml'
        )]
    )

    return LaunchDescription([
        candle_launch,
        motor_node
    ])