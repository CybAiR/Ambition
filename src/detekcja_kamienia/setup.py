from setuptools import find_packages, setup
import os
from glob import glob

package_name = 'detekcja_kamienia'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        # Konfiguracja: Kopiuj wszystkie pliki .launch.py
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*launch.[pxy][yma]*'))),
        # Konfiguracja: Kopiuj modele .pt z folderu models
        (os.path.join('share', package_name, 'models'), glob(os.path.join('models', '*.pt'))),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='student',
    maintainer_email='twoj@email.com',
    description='Detekcja kamieni za pomocą OAK-D i YOLO',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            # TUTAJ JEST ENDPOINT:
            # Komenda 'yolo_node' uruchomi funkcję 'main' z pliku 'detection_node.py'
            'yolo_node = detekcja_kamienia.detection_node:main',
            'distance_node = detekcja_kamienia.distance_node:main'
        ],
    },
)