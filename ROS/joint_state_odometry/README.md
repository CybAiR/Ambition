The purpose of this repository is determining odometry of the lunar rover. Its implementation is based on ROS2 Humble. It works with AK80-9 motors produced by MAB Robotics. The codes read actual measurements from the motors and convert it directly to actual rover position and orientation. We have to notice that it doesn't include some additional constructions like e.g. Kalman filter so in case of the wheel slip the result will be falsified.
\
In the first step, it is obligatory to install repository from MAB Robotics which in neccessary to communicate with motors. Even if you've done it already, please, read this section.
1. Create workspace folder.
2. In workspace folder create "src" folder.
3. From the terminal side, go to the "src" folder and use command "git clone https://github.com/mabrobotics/candle_ros2.git".
4. Go to "candle_ros2" folder and use command "git checkout 957fb87663ba940043fe939d8c2fab79d09c000f" - this repo only works with older version.
5. Stay in this folder and use command "git clone https://github.com/mabrobotics/candle.git".

Also, it is important to have some repo which configures all the motors. I use private repo "cmd_vel_to_motors" created by Bartłomiej Biesiada.
\
If you want to control your robot from the computer level, you have to install "teleop_twist_keyboard" package by using "sudo apt install ros-humble-teleop-twist-keyboard" command.
\
Next, we can go to loading this repo to workspace. If you are in the "src" folder, type this command: "git clone https://github.com/LukaszMajchrzak3/joint_state_odometry.git".
\
After "building" and "sourcing" all the workspace, it's time to run the odometry calculations.
1. Use command "ros2 run candle_ros2 candle_ros2_node USB 1M" to initialize communication with motors via USB.
2. It is recommended to try if informations from motors are transmitted. They are in "md80/joint_states" topic so you should type "ros2 topic echo /md80/joint_states". If informations are present, reading is realised properly.
3. In another terminal type "ros2 run teleop_twist_keyboard teleop_twist_keyboard" to run control of the motors.
4. In other terminal type "ros2 launch joint_state_odometry odometry.launch.py". You can also use "ros2 run joint_state_odometry joint_state_odometry_node".
5. The results are published in "odom" topic. If you want to see them, you can use the code which is the part of this repository by typing "ros2 run joint_state_odometry print_odom_pose" in console.

Now, if everything was done correctly, you should see actual position and orientation of the rover.
