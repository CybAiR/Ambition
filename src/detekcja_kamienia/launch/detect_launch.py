import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    # Ścieżki do pakietów
    pkg_dir = get_package_share_directory('detekcja_kamienia')
    depthai_prefix = get_package_share_directory('depthai_ros_driver')
    
    # Ścieżka do modelu YOLO
    model_path = os.path.join(pkg_dir, 'models', 'best.pt')

    # 1. Uruchomienie kamery OAK-D
    camera_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(depthai_prefix, 'launch', 'camera.launch.py')
        ),
        launch_arguments={'name': 'oak', 'camera.i_nn_type':'none'}.items()
    )

    # 2. Uruchomienie Twojego węzła detekcji
    yolo_node = Node(
        package='detekcja_kamienia',
        executable='yolo_node', # To jest nazwa z endpointu w setup.py
        name='yolov8_object_detection',
        parameters=[{
            'model': model_path,
            'score_thresh': 0.5,
            'publish_rate_hz': 15.0
        }]
    )

    distance_node = Node(
        package='detekcja_kamienia',
        executable='distance_node',
        name='distance_calculator',
        prefix=['python3'] # Bezpiecznie uruchamiamy to w naszej bańce
    )

    return LaunchDescription([
        camera_launch,
        yolo_node,
        distance_node
    ])