#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/twist.hpp>

#include "candle_ros2/srv/add_devices.hpp"
#include "candle_ros2/srv/generic.hpp"
#include "candle_ros2/srv/set_mode.hpp"

#include "candle_ros2/msg/motion_cmd.hpp"

class MotorControl : public rclcpp::Node
{
  public:
    MotorControl() : Node("motor_control")
    {
        pAdd_devices_client = create_client<candle_ros2::srv::AddDevices>("/md/add_mds");
        pEnable_client      = create_client<candle_ros2::srv::Generic>("/md/enable");
        pDisable_client     = create_client<candle_ros2::srv::Generic>("/md/disable");
        pSetMode_client     = create_client<candle_ros2::srv::SetMode>("/md/set_mode");
    };

    ~MotorControl() {};

    void init()
    {
        while (!pAdd_devices_client->wait_for_service(std::chrono::seconds(1)))
        {
            if (!rclcpp::ok())
            {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for service");
                return;
            }

            RCLCPP_INFO(this->get_logger(), "Waiting for /md/add_mds service...");
        }

        auto request        = std::make_shared<candle_ros2::srv::AddDevices::Request>();
        request->device_ids = {509, 510, 511, 512};

        auto future = pAdd_devices_client->async_send_request(request);
        auto result = rclcpp::spin_until_future_complete(shared_from_this(), future);

        if (result != rclcpp::FutureReturnCode::SUCCESS)
        {
            RCLCPP_ERROR(this->get_logger(), "Service call failed");
            return;
        }

        auto response = future.get();

        bool all_ok = true;
        for (size_t i = 0; i < response->success.size(); ++i)
            if (!response->success[i])
            {
                all_ok = false;
                RCLCPP_ERROR(
                    this->get_logger(), "Motor %d failed to initialize", request->device_ids[i]);
            }

        if (!all_ok)
        {
            RCLCPP_INFO(this->get_logger(), "Motor initialization failed");
            return;
        }

        RCLCPP_INFO(this->get_logger(), "Motor initialization completed successfully");
    }

  private:
    rclcpp::Client<candle_ros2::srv::AddDevices>::SharedPtr pAdd_devices_client = nullptr;
    rclcpp::Client<candle_ros2::srv::Generic>::SharedPtr    pEnable_client      = nullptr;
    rclcpp::Client<candle_ros2::srv::Generic>::SharedPtr    pDisable_client     = nullptr;
    rclcpp::Client<candle_ros2::srv::SetMode>::SharedPtr    pSetMode_client     = nullptr;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MotorControl>();
    node->init();

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}