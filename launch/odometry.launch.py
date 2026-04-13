from launch import LaunchDescription
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    config_path = os.path.join(
        get_package_share_directory('joint_state_odometry'),
        'config',
        'odometry_params.yaml'
    )

    return LaunchDescription([
        Node(
            package='joint_state_odometry',
            executable='joint_state_odometry_node',
            name='joint_state_odometry_node',
            output='screen',
            parameters=[config_path]
        )
    ])

