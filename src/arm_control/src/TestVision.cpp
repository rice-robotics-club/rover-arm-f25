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

#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

#include "arm_control/srv/update_goal_item.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses a fancy C++11 lambda
 * function to shorten the callback syntax, at the expense of making the
 * code somewhat more difficult to understand at first glance. */

class TestVision : public rclcpp::Node
{
public:
  TestVision()
  : Node("Vision")
  {
    //initialize some stuff
    goal_item_name_="NA";

    goal_pose_ = geometry_msgs::msg::PoseStamped();
    // remind me to give it actually sensible values
    goal_pose_.pose.position.x=0;
    goal_pose_.pose.position.y=0;
    goal_pose_.pose.position.z=0;
    goal_pose_.pose.orientation.w=0;

    publisher_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/goal_pose", 10);
    auto timer_callback =
      [this]() -> void {
        if (goal_item_name_ != "NA") {
          this->publisher_->publish(goal_pose_);
          // RCLCPP_INFO(this->get_logger(), "Publishing goal for: %s", goal_item_name_.c_str());
          RCLCPP_INFO(this->get_logger(), "X coord should be: %f", goal_pose_.pose.position.x);
        } else {
          RCLCPP_INFO(this->get_logger(), "No goal item set");
        }
      };
    //once per second for debugging purposes, remind me to swap this
    timer_ = this->create_wall_timer(1000ms, timer_callback);

    auto update_goal_item = [this](
        const std::shared_ptr<arm_control::srv::UpdateGoalItem::Request> request,
        std::shared_ptr<arm_control::srv::UpdateGoalItem::Response> response){
          RCLCPP_INFO(this->get_logger(), "Received Goal Item: %s", request -> goal_item_name.c_str());
          std::string receivedName= request -> goal_item_name;
          // this is where I would double check if the goal item was even in view before setting the response
          //  but I'll talk to vision about this later
          bool isFound=dummyUpdateGoalPose(receivedName);

          response -> response=isFound;
        };

    service_ = this -> create_service<arm_control::srv::UpdateGoalItem>(
      "/update_goal_item", 
      update_goal_item
      );
    
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr publisher_;
  geometry_msgs::msg::PoseStamped goal_pose_;

  std::string goal_item_name_;
  rclcpp::Service<arm_control::srv::UpdateGoalItem>::SharedPtr service_;
  /**This method is only for testing, DELETE ONCE DONE! */
  bool dummyUpdateGoalPose(std::string received){
    if (received=="1"){
      goal_pose_.pose.position.x=1;
      goal_item_name_=received;
      return true;
    } else{
      return false;
    }
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TestVision>());
  rclcpp::shutdown();
  return 0;
}
