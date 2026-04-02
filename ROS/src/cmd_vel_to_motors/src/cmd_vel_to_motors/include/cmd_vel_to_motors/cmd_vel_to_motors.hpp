#ifndef CMD_VEL_TO_MOTORS_HPP
#define CMD_VEL_TO_MOTORS_HPP

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>

#include "candle_ros2/msg/impedance_command.hpp"
#include "candle_ros2/msg/motion_command.hpp"
#include "candle_ros2/msg/position_pid_command.hpp"
#include "candle_ros2/msg/velocity_pid_command.hpp"
#include "candle_ros2/srv/add_md80s.hpp"
#include "candle_ros2/srv/generic_md80_msg.hpp"
#include "candle_ros2/srv/set_limits_md80.hpp"
#include "candle_ros2/srv/set_mode_md80s.hpp"

class CmdVelToMotors : public rclcpp::Node
{
public:
    CmdVelToMotors(): Node("cmd_vel_to_motors")
    {
        add_md80_client_ = this->create_client<candle_ros2::srv::AddMd80s>("/candle_ros2_node/add_md80s");
        zero_md80_client_ = this->create_client<candle_ros2::srv::GenericMd80Msg>("/candle_ros2_node/zero_md80s");
        set_mode_md80_client_ = this->create_client<candle_ros2::srv::SetModeMd80s>("/candle_ros2_node/set_mode_md80s");
        enable_md80_client_ = this->create_client<candle_ros2::srv::GenericMd80Msg>("/candle_ros2_node/enable_md80s");
        disable_md80_client_ = this->create_client<candle_ros2::srv::GenericMd80Msg>("/candle_ros2_node/disable_md80s");
        set_limits_md80_client_ = this->create_client<candle_ros2::srv::SetLimitsMd80>("/candle_ros2_node/set_limits_md80s");

        velocity_pid_pub_ = this->create_publisher<candle_ros2::msg::MotionCommand>("/md80/motion_command", 10);

        // Create a subscription to the "cmd_vel" topic
        cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "cmd_vel", 10,
            std::bind(&CmdVelToMotors::cmdVelCallback, this, std::placeholders::_1));

        // Configure motors (placeholder function)
        this->configureMotors();

        RCLCPP_INFO(this->get_logger(), "CmdVelToMotors node has been started.");
    }

private:

    std::vector<uint16_t> ids = {509, 510, 511, 512};

    rclcpp::Client<candle_ros2::srv::AddMd80s>::SharedPtr add_md80_client_;
    rclcpp::Client<candle_ros2::srv::GenericMd80Msg>::SharedPtr zero_md80_client_;
    rclcpp::Client<candle_ros2::srv::SetModeMd80s>::SharedPtr set_mode_md80_client_;
    rclcpp::Client<candle_ros2::srv::GenericMd80Msg>::SharedPtr enable_md80_client_;
    rclcpp::Client<candle_ros2::srv::GenericMd80Msg>::SharedPtr disable_md80_client_;
    rclcpp::Client<candle_ros2::srv::SetLimitsMd80>::SharedPtr set_limits_md80_client_;

    void configureMotors();
    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg);

    rclcpp::Publisher<candle_ros2::msg::MotionCommand>::SharedPtr velocity_pid_pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;

    float R = 0.10; // Wheel radius in meters
    float W = 0.5;  // Distance between wheels in meters
};

#endif // CMD_VEL_TO_MOTORS_HPP