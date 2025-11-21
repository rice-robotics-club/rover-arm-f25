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
//ros stuff
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"
//messages
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "moveit_msgs/msg/collision_object.hpp"
#include "moveit_msgs/msg/move_it_error_codes.hpp"
//moveitstuff
#include <moveit/controller_manager/controller_manager.hpp>
#include <moveit/moveit_cpp/moveit_cpp.hpp>
#include <moveit/moveit_cpp/planning_component.hpp>

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
        //copy the value from the pointer
        goal_pose_=*msg;
        RCLCPP_INFO(this->get_logger(), "I heard x coord '%f'", goal_pose_.pose.position.x);
      };
    subscription_ =
      this->create_subscription<geometry_msgs::msg::PoseStamped>("/goal_pose", 10, topic_callback);

    client_ =
      this->create_client<arm_control::srv::UpdateGoalItem>("/update_goal_item");

    //FOR TESTING! PLS DELETE ONCE DONE!
    //to test if it can publish
    timer_ = this->create_wall_timer(
      std::chrono::seconds(2),
      [this]() {
        //has to be in a thread or the callback is never processed

        rclcpp::Rate loop_rate(1);
        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        bool goalItemFound=false;

        std::thread{[this, promise](){
          bool goalItemInSight = this->changeGoalItem("1");  
          promise->set_value(goalItemInSight);
        }}.detach();

        
        // Wait for the service call to complete (with timeout)
        if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
          goalItemFound = future.get();
          if (goalItemFound){
            RCLCPP_INFO(this->get_logger(), "Service Response Received");
          }
        }
        
        timer_->cancel();  
      });
    

    //to test if it can return false
    // timer_ = this->create_wall_timer(
    //   std::chrono::seconds(2),
    //   [this]() {
    //     //has to be in a thread or the callback is never processed
    //     std::thread{[this](){
    //       this->changeGoalItem("blah");  
    //     }}.detach();
    //     timer_->cancel();  
    //   });

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

  geometry_msgs::msg::PoseStamped goal_pose_;

  //for debug, pls delete
  rclcpp::TimerBase::SharedPtr timer_;
  
  /**
   * This will eventually become the code I written down in the pseudocode doc
   */
  void execute(const std::shared_ptr<GoalHandleArm> goal_handle) {
    //for readability
    using moveit_controller_manager::ExecutionStatus;
    using moveit_msgs::msg::MoveItErrorCodes;

    RCLCPP_INFO(this->get_logger(), "Executing");
    rclcpp::Rate loop_rate(1);
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<ArmMovement::Feedback>();
    auto result = std::make_shared<ArmMovement::Result>();
    //check if arm has something attached to it
    if (!goal->attached_object.id.empty()){
      moveit_msgs::msg::CollisionObject attachedObject = goal->attached_object;
    }
    auto promise = std::make_shared<std::promise<bool>>();
    auto future = promise->get_future();
    bool goalItemNotFound;
    std::thread{[this, goal, promise](){
        bool goalItemInSight = this->changeGoalItem(goal->goal_item_name);
        promise->set_value(goalItemInSight);
    }}.detach(); 

    // Poll without blocking
    while (rclcpp::ok()) {
      if (future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        //the changeGoalItem method returns true when the goal item is in sight, false otherwise
        goalItemNotFound = !future.get();
        break;
      }
      rclcpp::spin_some(this->get_node_base_interface());
      loop_rate.sleep();
    }

    //check for termiante
    if (shouldTerminate()){
      return;
    }
    if (goalItemNotFound){
      result->success=false;
      result->error_code.val=MoveItErrorCodes::UNABLE_TO_AQUIRE_SENSOR_DATA;
      result->error_code.message = "Goal Item not in sight";
      result->error_code.source = "MoveIt node";
    }
    ExecutionStatus status = ExecutionStatus(ExecutionStatus::RUNNING);

    //FOR TESTING, PLS CHANGE!
    int maxLoops=5;
    for (int loop=0; loop<maxLoops && rclcpp::ok(); loop++){
      //check for termiante
      if (shouldTerminate()){
        return;
      }
      if (goal_handle->is_canceling()){
        result->success=false;
        result->error_code.val = MoveItErrorCodes::ABORT;
        result->error_code.message = "Cancelled successfully";
        result->error_code.source = "MoveIt node";
      }
      feedback->status = status.asString();
      goal_handle->publish_feedback(feedback);
      loop_rate.sleep();
    }
    //autocasts the ExecutionStatus object to a boolean. Is true when SUCCEEDED, false otherwise
    result->success=status;
    result->error_code.val= MoveItErrorCodes::SUCCESS;
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
