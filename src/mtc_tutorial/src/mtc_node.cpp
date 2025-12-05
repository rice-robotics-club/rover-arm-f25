// ros, moveit, other dependency packages
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/executors.hpp>
#include <moveit/planning_scene/planning_scene.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>
#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>
#if __has_include(<tf2_geometry_msgs/tf2_geometry_msgs.hpp>)
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#else
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#endif
#if __has_include(<tf2_eigen/tf2_eigen.hpp>)
#include <tf2_eigen/tf2_eigen.hpp>
#else
#include <tf2_eigen/tf2_eigen.h>
#endif

// mtc node class
#include "mtc_node.hpp"

static const rclcpp::Logger LOGGER = rclcpp::get_logger("mtc_tutorial");

MTCTaskNode::MTCTaskNode(const rclcpp::NodeOptions& options)
  : node_{ std::make_shared<rclcpp::Node>("mtc_node", options) }
{
  // Get execution parameter (declared via launch file or use default)
  // Use declare_parameter with default only if not already declared
  if (!node_->has_parameter("execute")) {
    node_->declare_parameter("execute", false);
  }
  execute_enabled_ = node_->get_parameter("execute").as_bool();
  
  if (!execute_enabled_) {
    RCLCPP_WARN(LOGGER, "Execution is DISABLED - plan-only mode (set 'execute:=true' to enable)");
  } else {
    RCLCPP_INFO(LOGGER, "Execution is ENABLED");
  }
  
  // Initialize Tasks
  task_map_.emplace("pick_place", TaskEntry{
    std::bind(&MTCTaskNode::createPickPlaceTask, this),
    std::bind(&MTCTaskNode::setupPickPlaceScene, this)
  });

  task_map_.emplace("twist_knob", TaskEntry{
    std::bind(&MTCTaskNode::createTwistKnobTask, this),
    std::bind(&MTCTaskNode::setupTwistKnobScene, this)
  });

  task_map_.emplace("move_home", TaskEntry{
    std::bind(&MTCTaskNode::createMoveHomeTask, this),
    std::bind(&MTCTaskNode::setupMoveHomeScene, this)
  });

  // Create action server
  action_server_ = rclcpp_action::create_server<ExecuteTask>(
    node_,
    "execute_manipulation_task",
    std::bind(&MTCTaskNode::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
    std::bind(&MTCTaskNode::handle_cancel, this, std::placeholders::_1),
    std::bind(&MTCTaskNode::handle_accepted, this, std::placeholders::_1)
  );

  RCLCPP_INFO(LOGGER, "MTC Action Server ready - waiting for goals");
}

rclcpp::node_interfaces::NodeBaseInterface::SharedPtr MTCTaskNode::getNodeBaseInterface()
{
  return node_->get_node_base_interface();
}

rclcpp::Node::SharedPtr MTCTaskNode::getNode() {
  return node_;
}

// Handle incoming goal
rclcpp_action::GoalResponse MTCTaskNode::handle_goal(
  const rclcpp_action::GoalUUID & uuid,
  std::shared_ptr<const ExecuteTask::Goal> goal)
{
  (void)uuid;  // Unused parameter
  
  RCLCPP_INFO(LOGGER, "Received goal request for task: %s", goal->task_type.c_str());
  
  // Check if task is already in progress
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (task_in_progress_) {
      RCLCPP_WARN(LOGGER, "Task already in progress, rejecting new goal");
      return rclcpp_action::GoalResponse::REJECT;
    }
  }
  
  // Validate task type
  if (goal->task_type != "pick_place" && goal->task_type != "twist_knob" && goal->task_type != "move_home") {
    RCLCPP_ERROR(LOGGER, "Invalid task type: %s", goal->task_type.c_str());
    return rclcpp_action::GoalResponse::REJECT;
  }
  
  RCLCPP_INFO(LOGGER, "Goal accepted for task: %s", goal->task_type.c_str());
  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

// Handle cancel request
rclcpp_action::CancelResponse MTCTaskNode::handle_cancel(
  const std::shared_ptr<GoalHandleExecuteTask> goal_handle)
{
  (void)goal_handle;  // Unused parameter
  RCLCPP_INFO(LOGGER, "Received request to cancel goal");
  return rclcpp_action::CancelResponse::ACCEPT;
}

// Start executing the task
void MTCTaskNode::handle_accepted(const std::shared_ptr<GoalHandleExecuteTask> goal_handle)
{
  // Execute in a separate thread to not block
  std::thread{std::bind(&MTCTaskNode::execute_task, this, std::placeholders::_1), goal_handle}.detach();
}

// Execute the actual task
void MTCTaskNode::execute_task(const std::shared_ptr<GoalHandleExecuteTask> goal_handle)
{
  const auto goal = goal_handle->get_goal();
  auto feedback = std::make_shared<ExecuteTask::Feedback>();
  auto result = std::make_shared<ExecuteTask::Result>();
  
  auto start_time = std::chrono::steady_clock::now();
  
  // Set task in progress
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    task_in_progress_ = true;
    current_task_type_ = goal->task_type;
  }
  
  RCLCPP_INFO(LOGGER, "Starting execution of task: %s", goal->task_type.c_str());
  
  // Store the received coordinates
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    target_pose_ = goal->target_pose;
    place_pose_ = goal->place_pose;
    has_coordinates_ = true;
  }
  
  RCLCPP_INFO(LOGGER, "Target pose: x=%.2f, y=%.2f, z=%.2f",
              goal->target_pose.position.x, 
              goal->target_pose.position.y, 
              goal->target_pose.position.z);
  
  // Send feedback: Setting up scene
  feedback->current_stage = "Setting up planning scene";
  feedback->progress_percentage = 10.0;
  feedback->estimated_time_remaining = 30.0;
  goal_handle->publish_feedback(feedback);
  
  // Setup scene with received coordinates
  try {
    setupPlanningScene(goal->task_type);
  } catch (const std::exception& e) {
    RCLCPP_ERROR(LOGGER, "Failed to setup planning scene: %s", e.what());
    result->success = false;
    result->message = "Failed to setup planning scene";
    result->execution_time = 0.0;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      task_in_progress_ = false;
      has_coordinates_ = false;
    }
    goal_handle->abort(result);
    return;
  }
  
  // Send feedback: Creating task
  feedback->current_stage = "Creating task";
  feedback->progress_percentage = 20.0;
  feedback->estimated_time_remaining = 25.0;
  goal_handle->publish_feedback(feedback);
  
  // Create task
  try {
    task_ = createTask(goal->task_type);
  } catch (const std::exception& e) {
    RCLCPP_ERROR(LOGGER, "Failed to create task: %s", e.what());
    result->success = false;
    result->message = "Failed to create task";
    result->execution_time = 0.0;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      task_in_progress_ = false;
      has_coordinates_ = false;
    }
    goal_handle->abort(result);
    return;
  }
  
  // Send feedback: Initializing
  feedback->current_stage = "Initializing task";
  feedback->progress_percentage = 30.0;
  feedback->estimated_time_remaining = 20.0;
  goal_handle->publish_feedback(feedback);
  
  // Initialize task
  try {
    task_.init();
  } catch (mtc::InitStageException& e) {
    RCLCPP_ERROR_STREAM(LOGGER, "Task initialization failed: " << e);
    result->success = false;
    result->message = "Task initialization failed";
    result->execution_time = 0.0;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      task_in_progress_ = false;
      has_coordinates_ = false;
    }
    goal_handle->abort(result);
    return;
  }
  
  // Check for cancellation
  if (goal_handle->is_canceling()) {
    result->success = false;
    result->message = "Task cancelled";
    result->execution_time = 0.0;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      task_in_progress_ = false;
      has_coordinates_ = false;
    }
    goal_handle->canceled(result);
    RCLCPP_INFO(LOGGER, "Task cancelled");
    return;
  }
  
  // Send feedback: Planning
  feedback->current_stage = "Planning motion";
  feedback->progress_percentage = 50.0;
  feedback->estimated_time_remaining = 15.0;
  goal_handle->publish_feedback(feedback);
  
  // Plan task
  if (!task_.plan(5)) {
    RCLCPP_ERROR(LOGGER, "Task planning failed");
    result->success = false;
    result->message = "Planning failed - no solution found";
    result->execution_time = 0.0;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      task_in_progress_ = false;
      has_coordinates_ = false;
    }
    goal_handle->abort(result);
    return;
  }
  
  RCLCPP_INFO(LOGGER, "Planning succeeded, found %zu solutions", task_.solutions().size());
  
  // Check for cancellation before execution
  if (goal_handle->is_canceling()) {
    result->success = false;
    result->message = "Task cancelled before execution";
    result->execution_time = 0.0;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      task_in_progress_ = false;
      has_coordinates_ = false;
    }
    goal_handle->canceled(result);
    RCLCPP_INFO(LOGGER, "Task cancelled before execution");
    return;
  }
  
  // Send feedback: Executing
  feedback->current_stage = "Executing motion";
  feedback->progress_percentage = 75.0;
  feedback->estimated_time_remaining = 10.0;
  goal_handle->publish_feedback(feedback);
  
  // Publish solution for visualization (always do this)
  RCLCPP_INFO(LOGGER, "Publishing solution for visualization...");
  task_.introspection().publishSolution(*task_.solutions().front());
  
  // Execute task (if enabled)
  if (execute_enabled_) {
    RCLCPP_INFO(LOGGER, "Calling task_.execute() - connecting to 'execute_task_solution' action server...");
    RCLCPP_INFO(LOGGER, "NOTE: Make sure 'ros2 launch moveit2_tutorials mtc_demo.launch.py' is running!");
    auto exec_result = task_.execute(*task_.solutions().front());
    RCLCPP_INFO(LOGGER, "task_.execute() returned with code: %d", exec_result.val);
    
    if (exec_result.val != moveit_msgs::msg::MoveItErrorCodes::SUCCESS) {
      RCLCPP_ERROR(LOGGER, "Task execution failed with error code: %d", exec_result.val);
      result->success = false;
      result->message = "Execution failed";
      result->execution_time = 0.0;
      {
        std::lock_guard<std::mutex> lock(state_mutex_);
        task_in_progress_ = false;
        has_coordinates_ = false;
      }
      goal_handle->abort(result);
      return;
    }
  } else {
    RCLCPP_WARN(LOGGER, "Execution disabled - solution published for visualization only");
    RCLCPP_INFO(LOGGER, "To enable execution, launch with: execute:=true");
  }
  
  // Calculate execution time
  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  float execution_time = duration.count() / 1000.0;
  
  // Success!
  feedback->current_stage = "Complete";
  feedback->progress_percentage = 100.0;
  feedback->estimated_time_remaining = 0.0;
  goal_handle->publish_feedback(feedback);
  
  result->success = true;
  result->message = "Task completed successfully";
  result->execution_time = execution_time;
  
  RCLCPP_INFO(LOGGER, "Task completed successfully in %.2f seconds", execution_time);
  
  // Reset state
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    task_in_progress_ = false;
    has_coordinates_ = false;
  }
  
  goal_handle->succeed(result);
}

void MTCTaskNode::setupPlanningScene(const std::string &task_name)
{
  auto it = task_map_.find(task_name);
  if (it != task_map_.end()) {
    it->second.setup_scene();
    return;
  }

  // Default to pickplace for now
  RCLCPP_WARN(LOGGER, "Unknown task_name %s, default to pick_place task", task_name.c_str());
  setupPickPlaceScene();
}

mtc::Task MTCTaskNode::createTask(const std::string &task_name) {
  auto it = task_map_.find(task_name);
  if (it != task_map_.end()) {
    return it->second.create_task();
  }

  // Default to pickplace for now
  RCLCPP_WARN(LOGGER, "Unknown task_name %s, default to pick_place task", task_name.c_str());
  return createPickPlaceTask();
}

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.automatically_declare_parameters_from_overrides(true);

  auto mtc_task_node = std::make_shared<MTCTaskNode>(options);
  rclcpp::executors::MultiThreadedExecutor executor;

  executor.add_node(mtc_task_node->getNodeBaseInterface());
  
  RCLCPP_INFO(LOGGER, "MTC Action Server spinning - send goals to 'execute_manipulation_task'");
  
  executor.spin();
  
  rclcpp::shutdown();
  return 0;
}
