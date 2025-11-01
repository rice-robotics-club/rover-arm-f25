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
#include <chrono>
#include <functional>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "moveit_msgs/msg/collision_object.hpp"
#include "moveit_msgs/msg/move_it_error_codes.hpp"

#include "arm_control/srv/update_goal_item.hpp"
#include "arm_control/action/arm_movement.hpp"

#include "arm_control/visibility_control.h"

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

    client_ =
      this->create_client<arm_control::srv::UpdateGoalItem>("update_goal_item");

    //FOR TESTING! PLS DELETE ONCE DONE!
    timer_ = this->create_wall_timer(
      std::chrono::seconds(2),
      [this]() {
        this->changeGoalItem("BRICK");
        timer_->cancel();  
      });
  }
  /*
  This is a dummy method just to allow me to check that the update_goal_item service and goal_pose topic work as 
  expected*/
  bool changeGoalItem(std::string goal_item_name){
    auto request = std::make_shared<arm_control::srv::UpdateGoalItem::Request>();
    request -> goal_item_name=goal_item_name;
    while (!client_->wait_for_service(std::chrono::seconds(1))) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
        return false;
      }
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Waiting");
    }

    // Wait for the result and check if true
    auto result_future = client_->async_send_request(request,
    [this](rclcpp::Client<arm_control::srv::UpdateGoalItem>::SharedFuture future) {
      auto result = future.get();
      update_goal_success_=result->response;
      if (update_goal_success_) {
        RCLCPP_INFO(this->get_logger(), "Updated Goal Item");
      } else {
        RCLCPP_ERROR(this->get_logger(), "Service returned false");
        update_goal_success_=false;
      }
    });
    return update_goal_success_;
  }

private:
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;

  rclcpp::Client<arm_control::srv::UpdateGoalItem>::SharedPtr client_;

  //for debug, pls delete
  rclcpp::TimerBase::SharedPtr timer_;

  bool update_goal_success_;
  
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MoveIt>());
  rclcpp::shutdown();
  return 0;
}
