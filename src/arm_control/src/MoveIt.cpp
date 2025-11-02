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

namespace arm_control_cpp{
class MoveIt : public rclcpp::Node
{
public:
  using ArmMovement = arm_control::action::ArmMovement;
  using GoalHandleArm = rclcpp_action::ServerGoalHandle<ArmMovement>;

  explicit MoveIt(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("MoveIt", options)
  {
    //options will give you a warning that it's unused. apparently its helpful if we decide to remap topics
    //or node namespaces etc
    auto topic_callback =
      [this](geometry_msgs::msg::PoseStamped::UniquePtr msg) -> void {
        goal_pose_=*msg;
        RCLCPP_INFO(this->get_logger(), "I heard x coord '%f'", goal_pose_.pose.position.x);
      };
    subscription_ =
      this->create_subscription<geometry_msgs::msg::PoseStamped>("/goal_pose", 10, topic_callback);

    client_ =
      this->create_client<arm_control::srv::UpdateGoalItem>("/update_goal_item");

    //FOR TESTING! PLS DELETE ONCE DONE!
    timer_ = this->create_wall_timer(
      std::chrono::seconds(2),
      [this]() {
        //has to be in a thread or the callback is never processed
        std::thread{[this](){
          this->changeGoalItem("BRICK");  
        }}.detach();
        timer_->cancel();  
      });

    using namespace std::placeholders;
    auto handle_goal = [this](
      const rclcpp_action::GoalUUID & uuid,
      std::shared_ptr<const ArmMovement::Goal> goal
    ){
      RCLCPP_INFO(this ->get_logger(), "Goal item: %s", goal->goal_item_name.c_str());
      RCLCPP_INFO(this ->get_logger(), "Planning Component: %s", goal->planning_component_name.c_str());
      (void) uuid;
      return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    };

    auto handle_cancel = [this](
      const std::shared_ptr<GoalHandleArm> goal_handle
    ){
      RCLCPP_INFO(this->get_logger(), "Cancelling goal");
      (void) goal_handle;
      return rclcpp_action::CancelResponse::ACCEPT;
    };
    auto handle_accepted = [this](
    const std::shared_ptr<GoalHandleArm> goal_handle
    )
    {
      // this needs to return quickly to avoid blocking the executor,
      // so we declare a lambda function to be called inside a new thread
      auto execute_in_thread = [this, goal_handle](){return this->execute(goal_handle);};
      std::thread{execute_in_thread}.detach();
    };
    this->action_server_ = rclcpp_action::create_server<ArmMovement>(
      this,
      "arm_movement",
      handle_goal,
      handle_cancel,
      handle_accepted);
  }

private:
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;

  rclcpp::Client<arm_control::srv::UpdateGoalItem>::SharedPtr client_;

  rclcpp_action::Server<ArmMovement>::SharedPtr action_server_;

  //for debug, pls delete
  rclcpp::TimerBase::SharedPtr timer_;

  bool update_goal_success_;

  geometry_msgs::msg::PoseStamped goal_pose_;
  
  /**
   * This will eventually become the code I written down in the pseudocode doc
   */
  void execute(const std::shared_ptr<GoalHandleArm> goal_handle) {
    RCLCPP_INFO(this->get_logger(), "Executing");
    rclcpp::Rate loop_rate(1);
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<ArmMovement::Feedback>();
    auto result = std::make_shared<ArmMovement::Result>();
    //has to be in a thread or the callback is never processed
    std::thread{[this, goal](){
      this->changeGoalItem(goal->goal_item_name);  
    }}.detach();
    //check for termiante
    if (shouldTerminate()){
      return;
    }
    //for testing, pls change!
    int maxLoops=1;
    //swap for a ExectionStatus object that is casted to a string
    feedback->status = "UNKNOWN";
    for (int loop=0; loop<maxLoops && rclcpp::ok(); loop++){
      //check for termiante
      if (shouldTerminate()){
        return;
      }
      if (goal_handle->is_canceling()){
        result->success=false;
        result->error_code.val = moveit_msgs::msg::MoveItErrorCodes::ABORT;
        result->error_code.message = "Cancelled successfully";
        result->error_code.source = "MoveIt node";
      }
      goal_handle->publish_feedback(feedback);
      loop_rate.sleep();
    }
    result->success=true;
    result->error_code.val= moveit_msgs::msg::MoveItErrorCodes::SUCCESS;
    result->error_code.message= "Successfully performed movement";
    result->error_code.source = "MoveIt node";

    //check for termiante
    if (shouldTerminate()){
      return;
    }
    //when goal is done
    if (rclcpp::ok()) {
      //tell vision to stop publishing
      this->changeGoalItem("NA");
      goal_handle->succeed(result);
      RCLCPP_INFO(this->get_logger(), "Goal succeeded");
    }
  };

  /** 
   * This is a method to change the call the UpdateGoalItem service
   * @param goal_item_name A string to represent the name of the goal item the Vision node should find
  */
  bool changeGoalItem(std::string goal_item_name){
    auto request = std::make_shared<arm_control::srv::UpdateGoalItem::Request>();
    request -> goal_item_name=goal_item_name;
    while (!client_->wait_for_service(std::chrono::seconds(3))) {
      if (shouldTerminate()) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
        return false;
      }
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Waiting for UpdateGoalItem");
    }

    auto future = client_->async_send_request(request);

    // Wait up to 5 seconds
    if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
      RCLCPP_ERROR(this->get_logger(), "Timed out waiting for UpdateGoalItem service response");
      return false;
    }
    auto result =future.get();
  
    if (result->response) {
      RCLCPP_INFO(this->get_logger(), "Updated Goal Item");
      return true;
    } else {
      RCLCPP_ERROR(this->get_logger(), "Service returned false");
      return false;
    }
    
  }
  /**
     * This is a method to detect if Ctrl C is pressed because for some god damn
     * reason the node doesn't automatically stop when ctrl C is pressed
     * Returns true if Ctrl C was detected
     */
  bool shouldTerminate(){
    return !rclcpp::ok();
  }
};
}
RCLCPP_COMPONENTS_REGISTER_NODE(arm_control_cpp::MoveIt);
