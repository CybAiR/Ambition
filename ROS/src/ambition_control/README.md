# Motor Control Node

## Overview
`motor_control` is a node that converts requested velocity into individual motor velocities and sends them to motor drivers via `candle_ros2`.

## Features
- Motor initialization
- Conversion from linear & angluar velocity to wheel velocities
- Publishing motor velocities


## Node info
- Subscribes to `/cmd_vel` (`geometry_msgs/msg/Twist`)
- Publishes to `/md/motion_command` (`candle_ros2/msg/MotionCmd`)
- Services:
    - `/md/add_mds`
    - `/md/enable`
    - `/md/set_mode`


## Parameters
- `kv` (double, default: `1.0`)  
  Linear velocity scaling factor.

- `kw` (double, default: `1.0`)  
  Angular velocity scaling factor.

- `v_max` (double, default: `10.0`)  
  Maximum wheel velocity (rad/s).

- `wheel_base` (double, default: `10.0`)  
  Distance between left and right wheels.

- `device_ids` (int64 array, default: `[509, 510, 511, 512]`)  
  Motor device IDs (must contain exactly 4 values).