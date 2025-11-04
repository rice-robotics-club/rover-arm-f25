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
#include "../mtc_node.hpp"

void MTCTaskNode::setupTwistKnobScene()
{
moveit::planning_interface::PlanningSceneInterface psi;
  std::vector<moveit_msgs::msg::CollisionObject> objects;

  // Panel dimensions: x=0.05 (depth), y=0.3 (width), z=0.4 (height)
  const double PANEL_DEPTH = 0.05;
  const double PANEL_CENTER_X = 0.5;
  const double PANEL_FRONT_FACE_X = PANEL_CENTER_X - (PANEL_DEPTH / 2.0); // 0.475
  const double PANEL_HEIGHT = 0.4;
  const double PANEL_CENTER_Z = PANEL_HEIGHT / 2.0;                       // 0.2

  // Axle dimensions: Height=0.04 (along X-axis), Radius=0.01
  const double AXLE_HEIGHT = 0.04;

  // Knob dimensions: x=0.02 (thickness)
  const double KNOB_THICKNESS = 0.02;

  // --- 1. Define the Electrical Panel (Vertical) ---
  moveit_msgs::msg::CollisionObject panel_object;
  panel_object.id = "electrical_panel";
  panel_object.header.frame_id = "world";

  panel_object.primitives.resize(1);
  panel_object.primitives[0].type = shape_msgs::msg::SolidPrimitive::BOX;
  panel_object.primitives[0].dimensions = { PANEL_DEPTH, 0.3, PANEL_HEIGHT }; 

  geometry_msgs::msg::Pose panel_pose;
  panel_pose.position.x = PANEL_CENTER_X;
  panel_pose.position.y = -0.25;
  panel_pose.position.z = PANEL_CENTER_Z; // Standing on the floor (Z=0)
  panel_pose.orientation.w = 1.0;         // No rotation needed
  panel_object.pose = panel_pose;

  objects.push_back(panel_object);

  // --- 2. Define the Connecting Axle (The small cylinder) ---
  moveit_msgs::msg::CollisionObject axle_object;
  axle_object.id = "knob_axle"; 
  axle_object.header.frame_id = "world";

  axle_object.primitives.resize(1);
  axle_object.primitives[0].type = shape_msgs::msg::SolidPrimitive::CYLINDER;
  axle_object.primitives[0].dimensions = { AXLE_HEIGHT, 0.01 }; // {height, radius}

  geometry_msgs::msg::Pose axle_pose;
  // Position the axle's center at X = Panel_Front_Face_X - Axle_Height/2
  // Panel Front Face X is 0.475
  axle_pose.position.x = PANEL_FRONT_FACE_X - (AXLE_HEIGHT / 2.0); // 0.475 - 0.02 = 0.455 meters
  axle_pose.position.y = -0.25; 
  axle_pose.position.z = PANEL_CENTER_Z; 

  // Rotate 90 degrees around Y-axis to make its length align with the X-axis (sticking out).
  tf2::Quaternion axle_q;
  axle_q.setRPY(0, M_PI_2, 0); 
  axle_pose.orientation = tf2::toMsg(axle_q);
  axle_object.pose = axle_pose;

  objects.push_back(axle_object);
  
  // --- 3. Define the Graspable Knob (The rectangle) ---
  moveit_msgs::msg::CollisionObject knob_object;
  knob_object.id = "rectangular_knob"; 
  knob_object.header.frame_id = "world";

  knob_object.primitives.resize(1);
  knob_object.primitives[0].type = shape_msgs::msg::SolidPrimitive::BOX;
  knob_object.primitives[0].dimensions = { KNOB_THICKNESS, 0.08, 0.03 }; 

  geometry_msgs::msg::Pose knob_pose;
  // Position it in front of the axle's end. Axle runs from X=0.435 to X=0.475.
  // Knob Center X: Axle_Start_X - Knob_Thickness/2
  knob_pose.position.x = (PANEL_FRONT_FACE_X - AXLE_HEIGHT) - (KNOB_THICKNESS / 2.0); 
  knob_pose.position.x = 0.435 - 0.01; // 0.425 meters
  knob_pose.position.y = -0.25; 
  knob_pose.position.z = PANEL_CENTER_Z; 

  knob_pose.orientation.w = 1.0; 
  knob_object.pose = knob_pose;

  objects.push_back(knob_object);

  // --- 4. Apply the objects to the planning scene ---
  psi.applyCollisionObjects(objects);
}

mtc::Task MTCTaskNode::createTwistKnobTask()
{
  mtc::Task task;
  task.stages()->setName("twist-knob-demo");
  task.loadRobotModel(node_);

  const auto& arm_group_name = "panda_arm";
  const auto& hand_group_name = "hand";
  const auto& hand_frame = "panda_hand";

  // Set task properties
  task.setProperty("group", arm_group_name);
  task.setProperty("eef", hand_group_name);
  task.setProperty("ik_frame", hand_frame);

  auto stage_state_current = std::make_unique<mtc::stages::CurrentState>("current");
  task.add(std::move(stage_state_current));

  auto interpolation_planner = std::make_shared<mtc::solvers::JointInterpolationPlanner>();

  auto stage_open_hand =
      std::make_unique<mtc::stages::MoveTo>("open hand", interpolation_planner);
  stage_open_hand->setGroup(hand_group_name);
  stage_open_hand->setGoal("open");
  task.add(std::move(stage_open_hand));

  {
    auto stage = std::make_unique<mtc::stages::MoveTo>("return home", interpolation_planner);
    stage->properties().configureInitFrom(mtc::Stage::PARENT, { "group" });
    stage->setGoal("ready");
    task.add(std::move(stage));
  }

  return task;
}
