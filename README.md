# "Ambition" Mars Rover Project

Please read the cybair_pedia repo before you begin your work guyz

## Cloning
Clone this repo with submodules:
```
git clone --recurse-submodules git@github.com:CybAiR/Ambition.git
```
or if already cloned
```
git submodule update --init --recursive
```
## Building
If you encounter an error while building:

first build only `candle_ros2`
```
colcon build --packages-select candle_ros2
```
then source the overlay
```
source install/setup.bash
```
and build the rest
```
colcon build
```

## Usage
### Launch joy teleop with 
```
ros2 launch ambition_commons teleop_vel.launch.py
```

### Launch motor control with
```
ros2 launch ambition_commons motor_control.launch.py
```
