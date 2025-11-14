#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class ActionInterpreter : public rclcpp::Node
{
public:
  ActionInterpreter()
  : Node("ActionInterpreter")
  {
    auto topic_callback =
      [this](std_msgs::msg::String::UniquePtr msg) -> void {
        const char* str = msg->data.c_str();
        char val = str[0];
        // RCLCPP_INFO(this->get_logger(), "I heard: '%s'", str);
        if (val == '0') {
            RCLCPP_INFO(this->get_logger(), "Starting action 0");
        } else if (val == '1') {
            RCLCPP_INFO(this->get_logger(), "Starting action 1");
        } else if (val == '2') {
            RCLCPP_INFO(this->get_logger(), "Starting action 2");
        } else if (val == '3') {
            RCLCPP_INFO(this->get_logger(), "Starting action 3");
        } else if (val == '4') {
            RCLCPP_INFO(this->get_logger(), "Starting action 4");
        } else if (val == '5') {
            RCLCPP_INFO(this->get_logger(), "Starting action 5");
        } else if (val == '6') {
            RCLCPP_INFO(this->get_logger(), "Starting action 6");
        } else if (val == '7') {
            RCLCPP_INFO(this->get_logger(), "Starting action 7");
        } else if (val == '8') {
            RCLCPP_INFO(this->get_logger(), "Starting action 8");
        } else if (val == '9') {
            RCLCPP_INFO(this->get_logger(), "Starting action 9");
        } else if (val == 'L') {
            RCLCPP_INFO(this->get_logger(), "Making small adjustment left");
        } else if (val == 'R') {
            RCLCPP_INFO(this->get_logger(), "Making small adjustment right");
        } else if (val == 'U') {
            RCLCPP_INFO(this->get_logger(), "Making small adjustment up");
        } else if (val == 'D') {
            RCLCPP_INFO(this->get_logger(), "Making small adjustment down");
        }
      };
    subscription_ =
      this->create_subscription<std_msgs::msg::String>("keyboard", 10, topic_callback);
  }

private:
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ActionInterpreter>());
  rclcpp::shutdown();
  return 0;
}