#include <functional>
#include <future>
#include <memory>
#include <string>
#include <sstream>

#include "arm_control/action/arm_movement.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "moveit_msgs/msg/collision_object.hpp"
#include "moveit_msgs/msg/move_it_error_codes.hpp"

namespace arm_control_cpp
{
class Teleop : public rclcpp::Node
{
public:
  using ArmMovement = arm_control::action::ArmMovement;
  using GoalHandleArm = rclcpp_action::ServerGoalHandle<ArmMovement>;

  explicit Teleop(const rclcpp::NodeOptions & options)
  : Node("Teleop", options)
  {
    this->client_ptr_ = rclcpp_action::create_client<ArmMovement>(
      this,
      "arm_movement");

    //Test case 1: goal item not in vision, moving arm, no attached object
    auto timer_callback_lambda = [this](){ return this->send_goal("NO ITEM"); };
    this->timer_ = this->create_wall_timer(
      std::chrono::milliseconds(500),
      timer_callback_lambda);
  }

  

private:
  rclcpp_action::Client<ArmMovement>::SharedPtr client_ptr_;
  rclcpp::TimerBase::SharedPtr timer_;

  /**
   * Takes in a goal_item and tells the moveItNode to move to that goal_item
   */
  void send_goal(std::string goal_item)
  {
    using namespace std::placeholders;

    this->timer_->cancel();

    if (!this->client_ptr_->wait_for_action_server()) {
      RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
      rclcpp::shutdown();
    }

    auto goal_msg = ArmMovement::Goal();
    //defining the request

    goal_msg.goal_item_name = goal_item;
    goal_msg.planning_component_name = "arm";
    //by not setting attached object, it is default empty

    RCLCPP_INFO(this->get_logger(), "Sending goal");

    auto send_goal_options = rclcpp_action::Client<ArmMovement>::SendGoalOptions();
    send_goal_options.goal_response_callback = [this](const GoalHandleArm::SharedPtr & goal_handle)
    {
      if (!goal_handle) {
        RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
      } else {
        RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
      }
    };

    send_goal_options.feedback_callback = [this](
      GoalHandleArm::SharedPtr,
      const std::shared_ptr<const ArmMovement::Feedback> feedback)
    {
      std::string status = feedback -> status;
      RCLCPP_INFO(this->get_logger(), status);
    };

    send_goal_options.result_callback = [this](const GoalHandleArm::WrappedResult & result)
    {
      switch (result.code) {
        case rclcpp_action::ResultCode::SUCCEEDED:
          break;
        case rclcpp_action::ResultCode::ABORTED:
          RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
          return;
        case rclcpp_action::ResultCode::CANCELED:
          RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
          return;
        default:
          RCLCPP_ERROR(this->get_logger(), "Unknown result code");
          return;
      }
      if(result->success){
        RCLCPP_INFO(this->get_logger(), "Action succeeded");
        rclcpp::shutdown();
      } else {
        RCLCPP_ERROR(this->get_logger(), result->error_code.message);
      }
    };
    this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
  }
};  
}  

RCLCPP_COMPONENTS_REGISTER_NODE(arm_control_cpp::Teleop)