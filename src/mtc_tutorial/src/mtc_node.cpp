// ros, moveit, other dependency packages
#include <rclcpp/rclcpp.hpp>
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
  // Initialize Tasks
  task_map_.emplace("pick_place", TaskEntry{
    std::bind(&MTCTaskNode::createPickPlaceTask, this),
    std::bind(&MTCTaskNode::setupPickPlaceScene, this)
  });

  task_map_.emplace("twist_knob", TaskEntry{
    std::bind(&MTCTaskNode::createTwistKnobTask, this),
    std::bind(&MTCTaskNode::setupTwistKnobScene, this)
  });
}

rclcpp::node_interfaces::NodeBaseInterface::SharedPtr MTCTaskNode::getNodeBaseInterface()
{
  return node_->get_node_base_interface();
}

rclcpp::Node::SharedPtr MTCTaskNode::getNode() {
  return node_;
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

void MTCTaskNode::doTask(const std::string &task_name)
{
  setupPlanningScene(task_name);
  task_ = createTask(task_name);

  try
  {
    task_.init();
  }
  catch (mtc::InitStageException& e)
  {
    RCLCPP_ERROR_STREAM(LOGGER, e);
    return;
  }

  if (!task_.plan(5))
  {
    RCLCPP_ERROR_STREAM(LOGGER, "Task planning failed");
    return;
  }
  task_.introspection().publishSolution(*task_.solutions().front());

  auto result = task_.execute(*task_.solutions().front());
  if (result.val != moveit_msgs::msg::MoveItErrorCodes::SUCCESS)
  {
    RCLCPP_ERROR_STREAM(LOGGER, "Task execution failed");
    return;
  }

  return;
}

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  options.automatically_declare_parameters_from_overrides(true);

  auto mtc_task_node = std::make_shared<MTCTaskNode>(options);
  rclcpp::executors::MultiThreadedExecutor executor;

  auto spin_thread = std::make_unique<std::thread>([&executor, &mtc_task_node]() {
    executor.add_node(mtc_task_node->getNodeBaseInterface());
    executor.spin();
    executor.remove_node(mtc_task_node->getNodeBaseInterface());
  });

  std::string task_name = "pick_place";
  mtc_task_node->getNode()->get_parameter_or("task", task_name, task_name);

  mtc_task_node->doTask(task_name);

  spin_thread->join();
  rclcpp::shutdown();
  return 0;
}