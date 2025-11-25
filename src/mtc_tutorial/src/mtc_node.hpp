#include <string>
#include <mutex>
#include <functional>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>
#include <moveit/planning_scene/planning_scene.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>
#include <moveit/move_group_interface/move_group_interface.hpp>
#include <mtc_tutorial/action/execute_task.hpp>

namespace mtc = moveit::task_constructor;

class MTCTaskNode
{
public:
  using ExecuteTask = mtc_tutorial::action::ExecuteTask;
  using GoalHandleExecuteTask = rclcpp_action::ServerGoalHandle<ExecuteTask>;

  MTCTaskNode(const rclcpp::NodeOptions& options);

  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr getNodeBaseInterface();
  rclcpp::Node::SharedPtr getNode();

private:
  // Action server callbacks
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const ExecuteTask::Goal> goal);
  
  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleExecuteTask> goal_handle);
  
  void handle_accepted(const std::shared_ptr<GoalHandleExecuteTask> goal_handle);
  
  void execute_task(const std::shared_ptr<GoalHandleExecuteTask> goal_handle);

  // Task creation and setup
  mtc::Task createTask(const std::string &task_name);
  void setupPlanningScene(const std::string &task_name);

  mtc::Task createPickPlaceTask();
  void setupPickPlaceScene();

  mtc::Task createTwistKnobTask();
  void setupTwistKnobScene();

  struct TaskEntry {
    std::function<mtc::Task()> create_task;
    std::function<void()> setup_scene;
  };

  std::unordered_map<std::string, TaskEntry> task_map_;

  // Action server
  rclcpp_action::Server<ExecuteTask>::SharedPtr action_server_;

  // Task state
  mtc::Task task_;
  rclcpp::Node::SharedPtr node_;
  std::mutex state_mutex_;
  bool task_in_progress_ = false;
  
  // Coordinate storage
  geometry_msgs::msg::Pose target_pose_;
  geometry_msgs::msg::Pose place_pose_;
  bool has_coordinates_ = false;
  std::string current_task_type_;
};
