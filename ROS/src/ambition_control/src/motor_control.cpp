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
        using std::placeholders::_1;

        this->declare_parameter<double>("kv", 1);
        this->declare_parameter<double>("kw", 1);
        this->declare_parameter<double>("v_max", 10);
        this->declare_parameter<double>("wheel_base", 10);
        this->declare_parameter<double>("wheel_radius", 0.105);
        this->declare_parameter<std::vector<int64_t>>("device_ids", {509, 510, 511, 512});

        kv_           = this->get_parameter("kv").as_double();
        kw_           = this->get_parameter("kw").as_double();
        v_max_        = this->get_parameter("v_max").as_double();
        wheel_base_   = this->get_parameter("wheel_base").as_double();
        wheel_radius_ = this->get_parameter("wheel_radius").as_double();
        auto tmp_vec  = this->get_parameter("device_ids").as_integer_array();

        if (tmp_vec.size() != 4)
        {
            RCLCPP_ERROR(this->get_logger(), "Wrong number of device ids");
            return;
        }

        device_ids_ = std::vector<uint16_t>(tmp_vec.begin(), tmp_vec.end());

        pAdd_devices_client_ = create_client<candle_ros2::srv::AddDevices>("/md/add_mds");
        pEnable_client_      = create_client<candle_ros2::srv::Generic>("/md/enable");
        pDisable_client_     = create_client<candle_ros2::srv::Generic>("/md/disable");
        pSetMode_client_     = create_client<candle_ros2::srv::SetMode>("/md/set_mode");

        pMotion_publisher_ =
            this->create_publisher<candle_ros2::msg::MotionCmd>("/md/motion_command", 10);

        pCmdVel_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10, std::bind(&MotorControl::cmdVelCallback, this, _1));
    };

    ~MotorControl() = default;

    void init()
    {
        // addDevice
        {
            while (!pAdd_devices_client_->wait_for_service(std::chrono::seconds(1)))
            {
                if (!rclcpp::ok())
                {
                    RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for service");
                    return;
                }

                RCLCPP_INFO(this->get_logger(), "Waiting for /md/add_mds service...");
            }

            auto request        = std::make_shared<candle_ros2::srv::AddDevices::Request>();
            request->device_ids = device_ids_;

            auto future = pAdd_devices_client_->async_send_request(request);
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
                        this->get_logger(), "Failed to add motor %d", request->device_ids[i]);
                }

            if (!all_ok)
            {
                RCLCPP_ERROR(this->get_logger(), "Motor initialization failed");
                return;
            }

            RCLCPP_INFO(this->get_logger(), "Added motors successfully");
        }

        // enable
        {
            while (!pEnable_client_->wait_for_service(std::chrono::seconds(1)))
            {
                if (!rclcpp::ok())
                {
                    RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for service");
                    return;
                }

                RCLCPP_INFO(this->get_logger(), "Waiting for /md/enable service...");
            }

            auto request        = std::make_shared<candle_ros2::srv::Generic::Request>();
            request->device_ids = device_ids_;

            auto future = pEnable_client_->async_send_request(request);
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
                        this->get_logger(), "Failed to enable motor %d", request->device_ids[i]);
                }

            if (!all_ok)
            {
                RCLCPP_INFO(this->get_logger(), "Motor initialization failed");
                return;
            }

            RCLCPP_INFO(this->get_logger(), "Enabled motors successfully");
        }

        // setMode
        {
            while (!pSetMode_client_->wait_for_service(std::chrono::seconds(1)))
            {
                if (!rclcpp::ok())
                {
                    RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for service");
                    return;
                }

                RCLCPP_INFO(this->get_logger(), "Waiting for /md/set_mode service...");
            }

            auto request        = std::make_shared<candle_ros2::srv::SetMode::Request>();
            request->device_ids = device_ids_;
            request->mode       = {"VELOCITY_PID", "VELOCITY_PID", "VELOCITY_PID", "VELOCITY_PID"};

            auto future = pSetMode_client_->async_send_request(request);
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
                        this->get_logger(), "Failed set motor %d mode", request->device_ids[i]);
                }

            if (!all_ok)
            {
                RCLCPP_INFO(this->get_logger(), "Motor initialization failed");
                return;
            }

            RCLCPP_INFO(this->get_logger(), "Motor initialization completed successfully");
        }
    }

  private:
    std::vector<uint16_t> device_ids_ = {509, 510, 511, 512};

    rclcpp::Client<candle_ros2::srv::AddDevices>::SharedPtr pAdd_devices_client_ = nullptr;
    rclcpp::Client<candle_ros2::srv::Generic>::SharedPtr    pEnable_client_      = nullptr;
    rclcpp::Client<candle_ros2::srv::Generic>::SharedPtr    pDisable_client_     = nullptr;
    rclcpp::Client<candle_ros2::srv::SetMode>::SharedPtr    pSetMode_client_     = nullptr;

    rclcpp::Publisher<candle_ros2::msg::MotionCmd>::SharedPtr  pMotion_publisher_  = nullptr;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr pCmdVel_subscriber_ = nullptr;
    rclcpp::TimerBase::SharedPtr                               pTimer_;

    double kv_           = 1.0;
    double kw_           = 1.0;
    float  v_max_        = 10.0;
    double wheel_base_   = 1;
    double wheel_radius_ = 0.105;

  private:
    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        double v = kv_ * msg->linear.x;
        double w = kw_ * msg->angular.z;

        float v_left  = (v - (wheel_base_ / 2.0) * w) / wheel_radius_;
        float v_right = (v + (wheel_base_ / 2.0) * w) / wheel_radius_;

        v_left  = std::clamp(v_left, -v_max_, v_max_);
        v_right = std::clamp(v_right, -v_max_, v_max_);

        auto pub_msg            = candle_ros2::msg::MotionCmd();
        pub_msg.device_ids      = std::vector<uint32_t>(device_ids_.begin(), device_ids_.end());
        pub_msg.target_position = {0.0, 0.0, 0.0, 0.0};
        pub_msg.target_torque   = {0.0, 0.0, 0.0, 0.0};
        pub_msg.target_velocity = {v_left, -v_right, v_left, -v_right};

        pMotion_publisher_->publish(pub_msg);
    }
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