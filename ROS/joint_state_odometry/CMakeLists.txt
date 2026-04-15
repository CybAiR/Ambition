cmake_minimum_required(VERSION 3.8)
project(joint_state_odometry)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

add_executable(joint_state_odometry_node src/joint_state_odometry_node.cpp)
ament_target_dependencies(joint_state_odometry_node
  rclcpp
  sensor_msgs
  nav_msgs
  geometry_msgs
  tf2_ros
  tf2
)

add_executable(print_odom_pose src/print_odom_pose.cpp)
ament_target_dependencies(print_odom_pose
  rclcpp
  nav_msgs
  tf2_ros
  tf2
)

install(TARGETS
  joint_state_odometry_node
  print_odom_pose
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME}/)

ament_package()
