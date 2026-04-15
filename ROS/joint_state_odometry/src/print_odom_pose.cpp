#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"
#include <cmath>

class OdomPrinter : public rclcpp::Node
{
public:
  OdomPrinter() : Node("odom_printer")
  {
    // Subskrypcja topicu /odom
    subscription_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/odom", 10,
      std::bind(&OdomPrinter::odomCallback, this, std::placeholders::_1)
    );

    RCLCPP_INFO(this->get_logger(), "Odom printer node started — listening to /odom");
  }

private:
  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    const auto &pos = msg->pose.pose.position;
    const auto &ori = msg->pose.pose.orientation;

    // 1. Inicjalizacja kwaternionu tf2 na podstawie danych z wiadomości
    tf2::Quaternion q(ori.x, ori.y, ori.z, ori.w);
    
    // 2. Konwersja do macierzy i wyciągnięcie kątów Eulera
    tf2::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);

    // 3. Konwersja radianów na stopnie
    double yaw_deg = yaw * 180.0 / M_PI;

    std::cout << std::fixed;
    std::cout.precision(4);
    std::cout << "x: " << pos.x
              << ", y: " << pos.y
//              << ", z: " << pos.z
              << " | Yaw: " << yaw_deg << "°"
              << std::endl;
  }

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OdomPrinter>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
