#include "cmd_vel_to_motors/cmd_vel_to_motors.hpp"


void CmdVelToMotors::configureMotors()
{
    // Placeholder for motor configuration logic
    RCLCPP_INFO(this->get_logger(), "Configuring motors");
    // Here you would add the actual service calls to configure the motor

    // Example service call
    auto add_request = std::make_shared<candle_ros2::srv::AddMd80s::Request>();
    add_request->drive_ids = ids;

    while (!add_md80_client_->wait_for_service(std::chrono::seconds(1))) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
            return;
        }
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "service not available, waiting again...");
    }

    // Call the service (assuming the service client is properly set up)
    auto add_result = add_md80_client_->async_send_request(add_request);

    // Handle the result as needed
    auto add_status = add_result.wait_for(std::chrono::seconds(1));
    // RCLCPP_INFO(this->get_logger(), "Service call status: %s", 
    //     status == std::future_status::ready ? "ready" : 
    //     status == std::future_status::timeout ? "timeout" : "deferred");
    
    // if (result.get()->drives_success[0])
    // {
    //     if (result.get()->drives_success[0]) {
    //         RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Successfully added %d drives", result.get()->total_number_of_drives);
    //     } 
    //     else {
    //         RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Drives were not added successfully.");
    //         rclcpp::shutdown();
    //     }
    // }
    // else
    // {
    //     RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to call service add_md80s");
    //     rclcpp::shutdown();
    // }
    
    RCLCPP_INFO(this->get_logger(), "Motors configured.");

    // Placeholder for motor configuration logic
    RCLCPP_INFO(this->get_logger(), "Setting zero position for motors");
    // Here you would add the actual service calls to configure the motor

    // Example service call
    auto zero_request = std::make_shared<candle_ros2::srv::GenericMd80Msg::Request>();
    zero_request->drive_ids = ids;

    while (!zero_md80_client_->wait_for_service(std::chrono::seconds(1))) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
            return;
        }
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "service not available, waiting again...");
    }

    // Call the service (assuming the service client is properly set up)
    auto zero_result = zero_md80_client_->async_send_request(zero_request);

    // Handle the result as needed
    auto zero_status = zero_result.wait_for(std::chrono::seconds(1));

    RCLCPP_INFO(this->get_logger(), "Motors zeroed.");

    // Placeholder for motor configuration logic
    RCLCPP_INFO(this->get_logger(), "Configuring Modes for motors");
    // Here you would add the actual service calls to configure the motor

    // Example service call
    auto mode_request = std::make_shared<candle_ros2::srv::SetModeMd80s::Request>();
    mode_request->drive_ids = ids;

    mode_request->mode = std::vector<std::string>(ids.size(), "VELOCITY_PID");

    while (!set_mode_md80_client_->wait_for_service(std::chrono::seconds(1))) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
            return;
        }
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "service not available, waiting again...");
    }

    // Call the service (assuming the service client is properly set up)
    auto mode_result = set_mode_md80_client_->async_send_request(mode_request);

    // Handle the result as needed
    auto mode_status = mode_result.wait_for(std::chrono::seconds(1));

    RCLCPP_INFO(this->get_logger(), "Motors modes configured.");

    // Placeholder for motor configuration logic
    RCLCPP_INFO(this->get_logger(), "Enabling motors");
    // Here you would add the actual service calls to configure the motor

    // Example service call
    auto enable_request = std::make_shared<candle_ros2::srv::GenericMd80Msg::Request>();
    enable_request->drive_ids = ids;

    while (!enable_md80_client_->wait_for_service(std::chrono::seconds(1))) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
            return;
        }
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "service not available, waiting again...");
    }

    // Call the service (assuming the service client is properly set up)
    auto enable_result = enable_md80_client_->async_send_request(enable_request);

    // Handle the result as needed
    auto enable_status = enable_result.wait_for(std::chrono::seconds(1));

    RCLCPP_INFO(this->get_logger(), "Motors enabled.");
}

void CmdVelToMotors::cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    auto motion_command = candle_ros2::msg::MotionCommand();

    // Convert uint16_t to uint32_t for drive_ids
    motion_command.drive_ids.resize(this->ids.size());
    for (size_t i = 0; i < this->ids.size(); ++i) {
        motion_command.drive_ids[i] = static_cast<uint32_t>(this->ids[i]);
    }

    float vel_lin = msg->linear.x * this->R;
    float vel_ang = msg->angular.z * this->W / 2;

    motion_command.target_position = std::vector<float_t>(this->ids.size(), 0.0);
    // motion_command.target_velocity = std::vector<float_t>(this->ids.size(), msg->angular.z);
    motion_command.target_torque = std::vector<float_t>(this->ids.size(), 0.0);

    motion_command.target_velocity = {
        vel_lin - vel_ang,  // Front Left
        (vel_lin + vel_ang)*(-1),  // Front Right
        vel_lin - vel_ang,  // Rear Left
        (vel_lin + vel_ang)*(-1)   // Rear Right
    };

    velocity_pid_pub_->publish(motion_command);
}

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CmdVelToMotors>());
  rclcpp::shutdown();
  return 0;
}