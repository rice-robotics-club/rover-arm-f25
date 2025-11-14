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
  const double PANEL_CENTER_X = 0.6;
  const double PANEL_FRONT_FACE_X = PANEL_CENTER_X - (PANEL_DEPTH / 2.0); // 0.575
  const double PANEL_HEIGHT = 0.4;
  const double PANEL_CENTER_Z = PANEL_HEIGHT / 2.0;                       // 0.2
  const double Y_POS = -0.30;

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
  panel_pose.position.y = Y_POS;
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
  // Panel Front Face X is 0.575
  axle_pose.position.x = PANEL_FRONT_FACE_X - (AXLE_HEIGHT / 2.0); // 0.575 - 0.02 = 0.555 meters
  axle_pose.position.y = Y_POS;
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
  knob_object.primitives[0].dimensions = { KNOB_THICKNESS, 0.03, 0.08 }; 

  geometry_msgs::msg::Pose knob_pose;
  // Position it in front of the axle's end. Axle runs from X=0.535 to X=0.575.
  // Knob Center X: Axle_Start_X - Knob_Thickness/2
  knob_pose.position.x = (PANEL_FRONT_FACE_X - AXLE_HEIGHT) - (KNOB_THICKNESS / 2.0); 
  knob_pose.position.x = 0.535 - 0.01; // 0.525 meters
  knob_pose.position.y = Y_POS;  // Moved right 5cm from -0.25
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

  // Disable warnings for this line, as it's a variable that's set but not used in this example
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  mtc::Stage* current_state_ptr = nullptr;  // Forward current_state on to grasp pose generator
  #pragma GCC diagnostic pop


  // Planners
  auto interpolation_planner = std::make_shared<mtc::solvers::JointInterpolationPlanner>();
  auto sampling_planner = std::make_shared<mtc::solvers::PipelinePlanner>(node_);

  auto cartesian_planner = std::make_shared<mtc::solvers::CartesianPath>();
  cartesian_planner->setMaxVelocityScalingFactor(1.0);
  cartesian_planner->setMaxAccelerationScalingFactor(1.0);
  cartesian_planner->setStepSize(.01);

  // Stages
  // start generator stage to get current position
  auto stage_state_current = std::make_unique<mtc::stages::CurrentState>("current");
  current_state_ptr = stage_state_current.get();
  task.add(std::move(stage_state_current));

  // open the hand with moveto generator stage
  auto stage_open_hand =
      std::make_unique<mtc::stages::MoveTo>("open hand", interpolation_planner);
  stage_open_hand->setGroup(hand_group_name);
  stage_open_hand->setGoal("open");
  task.add(std::move(stage_open_hand));

  auto stage_move_to_knob = std::make_unique<mtc::stages::Connect>(
    "move to knob",
    mtc::stages::Connect::GroupPlannerVector{ { arm_group_name, sampling_planner } }
  );
  stage_move_to_knob->setTimeout(0.5);
  stage_move_to_knob->properties().configureInitFrom(mtc::Stage::PARENT);
  task.add(std::move(stage_move_to_knob));

  // Disable warnings for this line, as it's a variable that's set but not used in this example
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  mtc::Stage* attach_object_stage = nullptr;
  #pragma GCC diagnostic pop

  {
    auto grasp = std::make_unique<mtc::SerialContainer>("grasp knob");
    task.properties().exposeTo(grasp->properties(), { "eef", "group", "ik_frame" });
    grasp->properties().configureInitFrom(mtc::Stage::PARENT, { "eef", "group", "ik_frame" });

    {
      auto stage = std::make_unique<mtc::stages::MoveRelative>("approach knob", cartesian_planner);
      stage->properties().set("marker_ns", "approach_knob");
      stage->properties().set("link", hand_frame);
      stage->properties().configureInitFrom(mtc::Stage::PARENT, { "group" });
      stage->setMinMaxDistance(0.1, 0.15);
      stage->setIKFrame(hand_frame);

      // Approach perpendicular to the knob's face (along negative X in world frame)
      // The knob's thin dimension (0.02m) is along X-axis, so we approach from +X toward -X
      geometry_msgs::msg::Vector3Stamped vec;
      vec.header.frame_id = "world";  // Use world frame for consistent direction
      vec.vector.x = 1.0;
      vec.vector.y = 0.0;
      vec.vector.z = 0.0;
      stage->setDirection(vec);

      grasp->insert(std::move(stage));
    }

    {
      auto stage = std::make_unique<mtc::stages::GenerateGraspPose>("generate grasp pose");
      stage->properties().configureInitFrom(mtc::Stage::PARENT);
      stage->properties().set("marker_ns", "grasp_pose");
      stage->setPreGraspPose("open");
      // Use the same object id as inserted into the planning scene in setupTwistKnobScene
      stage->setObject("rectangular_knob");
      stage->setAngleDelta(M_PI / 12);
      stage->setMonitoredStage(current_state_ptr);  // Hook into current state

      Eigen::Isometry3d grasp_frame_transform;
      Eigen::Quaterniond q = Eigen::AngleAxisd(M_PI / 2, Eigen::Vector3d::UnitX()) *
                            Eigen::AngleAxisd(M_PI / 2, Eigen::Vector3d::UnitY()) *
                            Eigen::AngleAxisd(M_PI / 2, Eigen::Vector3d::UnitZ());
      grasp_frame_transform.linear() = q.matrix();
      grasp_frame_transform.translation().z() = 0.1;

      auto wrapper =
        std::make_unique<mtc::stages::ComputeIK>("grasp pose IK", std::move(stage));
      wrapper->setMaxIKSolutions(8);
      wrapper->setMinSolutionDistance(1.0);
      wrapper->setIKFrame(grasp_frame_transform, hand_frame);
      wrapper->properties().configureInitFrom(mtc::Stage::PARENT, { "eef", "group" });
      wrapper->properties().configureInitFrom(mtc::Stage::INTERFACE, { "target_pose" });
      grasp->insert(std::move(wrapper));
    }

    {
      auto stage =
        std::make_unique<mtc::stages::ModifyPlanningScene>("allow collision (hand,object)");
      // Allow collisions between the hand and the knob itself (so fingers can overlap the knob)
      stage->allowCollisions("rectangular_knob",
                            task.getRobotModel()
                                ->getJointModelGroup(hand_group_name)
                                ->getLinkModelNamesWithCollisionGeometry(),
                            true);
      stage->allowCollisions("knob_axle",
                            task.getRobotModel()
                                ->getJointModelGroup(hand_group_name)
                                ->getLinkModelNamesWithCollisionGeometry(),
                            true);
      grasp->insert(std::move(stage));
    }

    {
      auto stage = std::make_unique<mtc::stages::MoveTo>("close hand", interpolation_planner);
      stage->setGroup(hand_group_name);
      stage->setGoal("close");
      grasp->insert(std::move(stage));
    }

    {
      auto stage = std::make_unique<mtc::stages::ModifyPlanningScene>("attach knob");
      // Attach the knob placed into the planning scene (id: rectangular_knob)
      stage->attachObject("rectangular_knob", hand_frame);
      attach_object_stage = stage.get();
      grasp->insert(std::move(stage));
    }

    {
      auto stage = std::make_unique<mtc::stages::MoveRelative>("twist knob", cartesian_planner);
      stage->properties().set("marker_ns", "twist_knob");
      stage->properties().configureInitFrom(mtc::Stage::PARENT, { "group" });

      geometry_msgs::msg::TwistStamped twist_direction;
      twist_direction.header.frame_id = hand_frame;

      twist_direction.twist.angular.x = 0;
      twist_direction.twist.angular.y = 0;
      twist_direction.twist.angular.z = 1.0;

      twist_direction.twist.linear.x = 0;
      twist_direction.twist.linear.y = 0;
      twist_direction.twist.linear.z = 0;

      stage->setDirection(twist_direction);

      stage->setIKFrame(hand_frame);
      // const double rotation_threshold = 0.05;
      stage->setMinMaxDistance(M_PI_2, M_PI_2);

      grasp->insert(std::move(stage));
    }


    task.add(std::move(grasp));
  }

  {
    auto release = std::make_unique<mtc::SerialContainer>("release knob");
    task.properties().exposeTo(release->properties(), { "eef", "group", "ik_frame" });
    release->properties().configureInitFrom(mtc::Stage::PARENT, { "eef", "group", "ik_frame" });
    
    {
      auto stage = std::make_unique<mtc::stages::MoveTo>("open hand", interpolation_planner);
      stage->setGroup(hand_group_name);
      stage->setGoal("open");
      release->insert(std::move(stage));
    }

    {
      auto stage =
          std::make_unique<mtc::stages::ModifyPlanningScene>("forbid collision (hand,knob)");
      stage->allowCollisions("rectangular_knob",
                            task.getRobotModel()
                                ->getJointModelGroup(hand_group_name)
                                ->getLinkModelNamesWithCollisionGeometry(),
                            false);
      release->insert(std::move(stage));
    }

    {
      auto stage = std::make_unique<mtc::stages::ModifyPlanningScene>("detach knob");
      stage->detachObject("rectangular_knob", hand_frame);
      release->insert(std::move(stage));
    }

    {
      auto stage = std::make_unique<mtc::stages::MoveRelative>("retreat", cartesian_planner);
      stage->properties().configureInitFrom(mtc::Stage::PARENT, { "group" });
      stage->setMinMaxDistance(0.1, 0.15);
      stage->setIKFrame(hand_frame);
      stage->properties().set("marker_ns", "retreat");

      // Retreat in positive X direction (away from panel)
      geometry_msgs::msg::Vector3Stamped vec;
      vec.header.frame_id = "world";
      vec.vector.x = -1.0;
      vec.vector.y = 0.0;
      vec.vector.z = 0.0;
      stage->setDirection(vec);
      release->insert(std::move(stage));
    }

    task.add(std::move(release));
  }

  {
    auto stage = std::make_unique<mtc::stages::MoveTo>("return home", interpolation_planner);
    stage->properties().configureInitFrom(mtc::Stage::PARENT, { "group" });
    stage->setGoal("ready");
    task.add(std::move(stage));
  }

  return task;
}
