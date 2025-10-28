// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

// TODO: CHANGE THE TEMPLATE CODE!
class MoveIt : public rclcpp::Node
{
public:
  MoveIt()
  : Node("MoveIt")
  {
    auto topic_callback =
      [this](geometry_msgs::msg::PoseStamped::UniquePtr msg) -> void {
        RCLCPP_INFO(this->get_logger(), "I heard x coord '%f'", msg -> pose.position.x);
      };
    subscription_ =
      this->create_subscription<geometry_msgs::msg::PoseStamped>("goal_pose", 10, topic_callback);
  }

private:
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MoveIt>());
  rclcpp::shutdown();
  return 0;
}
