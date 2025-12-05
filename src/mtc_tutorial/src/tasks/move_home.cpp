// Super simple MTC task that just moves the robot to its home position
// This task cannot fail and is useful for testing the action server

#include <rclcpp/rclcpp.hpp>
#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>

#include "../mtc_node.hpp"

static const rclcpp::Logger LOGGER = rclcpp::get_logger("mtc_tutorial");

void MTCTaskNode::setupMoveHomeScene()
{
  // No scene setup needed - just moving the robot
  RCLCPP_INFO(LOGGER, "Move Home: No scene setup needed");
}

mtc::Task MTCTaskNode::createMoveHomeTask()
{
  RCLCPP_INFO(LOGGER, "Creating simple move_home task");
  
  mtc::Task task;
  task.stages()->setName("move-home");
  task.loadRobotModel(node_);

  const auto& arm_group_name = "panda_arm";
  const auto& hand_group_name = "hand";

  // Set task properties
  task.setProperty("group", arm_group_name);
  task.setProperty("eef", hand_group_name);

  auto interpolation_planner = std::make_shared<mtc::solvers::JointInterpolationPlanner>();

  // Stage 1: Get current state
  auto stage_state_current = std::make_unique<mtc::stages::CurrentState>("current");
  task.add(std::move(stage_state_current));

  // Stage 2: Open hand (simple movement)
  {
    auto stage = std::make_unique<mtc::stages::MoveTo>("open hand", interpolation_planner);
    stage->setGroup(hand_group_name);
    stage->setGoal("open");
    task.add(std::move(stage));
  }

  // Stage 3: Move arm to ready position
  {
    auto stage = std::make_unique<mtc::stages::MoveTo>("move to ready", interpolation_planner);
    stage->setGroup(arm_group_name);
    stage->setGoal("ready");
    task.add(std::move(stage));
  }

  RCLCPP_INFO(LOGGER, "Move Home task created successfully");
  return task;
}

