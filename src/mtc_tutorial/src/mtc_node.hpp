#include <string>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <unordered_map>
#include <functional>
#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/solvers.h>
#include <moveit/task_constructor/stages.h>
#include <moveit/planning_scene/planning_scene.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>
#include <moveit/move_group_interface/move_group_interface.hpp>

namespace mtc = moveit::task_constructor;

class MTCTaskNode
{
public:
  MTCTaskNode(const rclcpp::NodeOptions& options);

  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr getNodeBaseInterface();

  void doTask(const std::string &task_name);

  rclcpp::Node::SharedPtr getNode();

private:
  // Compose an MTC task from a series of stages.
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

  mtc::Task task_;
  rclcpp::Node::SharedPtr node_;

  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr object_pose_sub_;
  geometry_msgs::msg::Pose detected_object_pose_;
  bool object_detected_ = false;
};
