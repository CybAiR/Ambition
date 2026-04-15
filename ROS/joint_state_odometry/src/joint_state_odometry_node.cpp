#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

class JointStateOdometry : public rclcpp::Node
{
public:
  JointStateOdometry() : Node("joint_state_odometry_node"), x_(0.0), y_(0.0), theta_(0.0)
  {
    this->declare_parameter<std::vector<std::string>>("left_wheel_joints", {"Joint 509", "Joint 510"});
    this->declare_parameter<std::vector<std::string>>("right_wheel_joints", {"Joint 511", "Joint 512"});
    this->declare_parameter<double>("wheel_radius", 0.105);
    this->declare_parameter<double>("wheel_separation", 2.0);
    this->declare_parameter<bool>("publish_tf", true);
    this->declare_parameter<std::string>("odom_frame_id", "odom");
    this->declare_parameter<std::string>("base_frame_id", "base_link");

    this->get_parameter("left_wheel_joints", left_wheel_joints_);
    this->get_parameter("right_wheel_joints", right_wheel_joints_);
    this->get_parameter("wheel_radius", wheel_radius_);
    this->get_parameter("wheel_separation", wheel_separation_);
    this->get_parameter("publish_tf", publish_tf_);
    this->get_parameter("odom_frame_id", odom_frame_id_);
    this->get_parameter("base_frame_id", base_frame_id_);

    odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    joint_state_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
        "md80/joint_states", 10,
        std::bind(&JointStateOdometry::jointStateCallback, this, std::placeholders::_1));

    last_time_ = this->now();
  }

private:
  void jointStateCallback(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    double left_vel_sum = 0.0;
    double right_vel_sum = 0.0;
    int left_count = 0;
    int right_count = 0;

    for (size_t i = 0; i < msg->name.size(); i++)
    {
      const auto &name = msg->name[i];
      double vel = msg->velocity[i];

      if (std::find(left_wheel_joints_.begin(), left_wheel_joints_.end(), name) != left_wheel_joints_.end())
      {
        left_vel_sum += vel;
        left_count++;
      }
      else if (std::find(right_wheel_joints_.begin(), right_wheel_joints_.end(), name) != right_wheel_joints_.end())
      {
        right_vel_sum += vel;
        right_count++;
      }
    }

    if (left_count == 0 || right_count == 0)
      return;

    double left_vel = (left_vel_sum / left_count) * wheel_radius_;
    double right_vel = (right_vel_sum / right_count) * wheel_radius_;

    auto current_time = this->now();
    double dt = (current_time - last_time_).seconds();
    last_time_ = current_time;

    double vx = (right_vel + left_vel) / 2.0;
    double vth = (right_vel - left_vel) / wheel_separation_;

    x_ += vx * cos(theta_) * dt;
    y_ += vx * sin(theta_) * dt;
    theta_ += vth * dt;

    publishOdometry(current_time, vx, vth);
  }

  void publishOdometry(const rclcpp::Time &time, double vx, double vth)
  {
    nav_msgs::msg::Odometry odom;
    odom.header.stamp = time;
    odom.header.frame_id = odom_frame_id_;
    odom.child_frame_id = base_frame_id_;

    odom.pose.pose.position.x = x_;
    odom.pose.pose.position.y = y_;

    tf2::Quaternion q;
    q.setRPY(0, 0, theta_);
    odom.pose.pose.orientation.x = q.x();
    odom.pose.pose.orientation.y = q.y();
    odom.pose.pose.orientation.z = q.z();
    odom.pose.pose.orientation.w = q.w();

    odom.twist.twist.linear.x = vx;
    odom.twist.twist.angular.z = vth;

    odom_pub_->publish(odom);

    if (publish_tf_)
    {
      geometry_msgs::msg::TransformStamped transformStamped;
      transformStamped.header.stamp = time;
      transformStamped.header.frame_id = odom_frame_id_;
      transformStamped.child_frame_id = base_frame_id_;
      transformStamped.transform.translation.x = x_;
      transformStamped.transform.translation.y = y_;
      transformStamped.transform.translation.z = 0.0;
      transformStamped.transform.rotation.x = q.x();
      transformStamped.transform.rotation.y = q.y();
      transformStamped.transform.rotation.z = q.z();
      transformStamped.transform.rotation.w = q.w();

      tf_broadcaster_->sendTransform(transformStamped);
    }
  }

  std::vector<std::string> left_wheel_joints_;
  std::vector<std::string> right_wheel_joints_;
  double wheel_radius_, wheel_separation_;
  double x_, y_, theta_;
  bool publish_tf_;
  std::string odom_frame_id_, base_frame_id_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  rclcpp::Time last_time_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<JointStateOdometry>());
  rclcpp::shutdown();
  return 0;
}

