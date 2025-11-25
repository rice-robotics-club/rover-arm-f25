# MTC Tutorial Python Scripts

Python tools for MoveIt Task Constructor manipulation tasks.

## Action Clients

### interactive_client.py
Interactive menu-driven client for sending manipulation tasks.

```bash
ros2 run mtc_tutorial interactive_client.py
```

Features:
- Pick and place with default or custom coordinates
- Twist knob with default or custom coordinates
- Real-time feedback display
- Easy-to-use menu interface

### task_client.py
Simple programmatic example of sending action goals.

```bash
ros2 run mtc_tutorial task_client.py
```

Modify this script to send custom goals programmatically.

## Testing Tools

### pose_publisher.py
Simple Python node that publishes target poses for testing.

## Quick Start

```bash
# Build
cd ~/rover-arm-f25
colcon build --mixin debug --packages-select mtc_tutorial
source install/setup.bash

# Run publisher
ros2 launch mtc_tutorial pose_publisher.launch.py

# Test it (in another terminal)
ros2 run mtc_tutorial test_pose_subscriber.py
```

## What It Does

- Publishes `geometry_msgs/PoseStamped` to `/target_pose` topic
- Cycles through 5 example poses every 10 seconds
- Uses "world" reference frame

## Example Poses

| # | Position (x, y, z) | Description |
|---|-------------------|-------------|
| 1 | (0.4, 0.0, 0.3) | Forward Center |
| 2 | (0.5, -0.2, 0.4) | Forward Right |
| 3 | (0.5, 0.2, 0.4) | Forward Left |
| 4 | (0.3, 0.0, 0.5) | High Position |
| 5 | (0.4, 0.0, 0.2) | Low Position |

## Customization

### Change Publishing Rate
Edit `pose_publisher.py` line ~27:
```python
self.timer = self.create_timer(5.0, self.timer_callback)
```

### Add Custom Pose
In `create_example_poses()` method:
```python
pose_new = PoseStamped()
pose_new.header.frame_id = "world"
pose_new.pose.position.x = 0.6
pose_new.pose.position.y = 0.0
pose_new.pose.position.z = 0.4
pose_new.pose.orientation.x = 1.0
pose_new.pose.orientation.y = 0.0
pose_new.pose.orientation.z = 0.0
pose_new.pose.orientation.w = 0.0
poses.append(("My Pose", pose_new))
```

### Change Reference Frame
```python
pose.header.frame_id = "panda_link0"  # instead of "world"
```

## Useful Commands

```bash
# Monitor published poses
ros2 topic echo /target_pose

# Check publishing rate
ros2 topic hz /target_pose

# List all topics
ros2 topic list
```

## Next Steps: MoveIt2 Integration

To use these poses with MoveIt2, create a C++ node that:

1. Subscribes to `/target_pose`
2. Uses `MoveGroupInterface` to plan motion
3. Executes the planned trajectory

Example C++ subscriber:
```cpp
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit/move_group_interface/move_group_interface.h>

class MoveItPlannerNode : public rclcpp::Node {
public:
    MoveItPlannerNode() : Node("moveit_planner_node") {
        subscription_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/target_pose", 10,
            std::bind(&MoveItPlannerNode::poseCallback, this, std::placeholders::_1));
    }
    
private:
    void poseCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        auto move_group = moveit::planning_interface::MoveGroupInterface(
            shared_from_this(), "panda_arm");
        
        move_group.setPoseTarget(msg->pose);
        
        moveit::planning_interface::MoveGroupInterface::Plan plan;
        if (move_group.plan(plan) == moveit::core::MoveItErrorCode::SUCCESS) {
            move_group.execute(plan);
        }
    }
    
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;
};
```

## Architecture

```
Python Publisher → /target_pose → C++ MoveIt2 Node → move_group → Robot
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No executable found | `colcon build --packages-select mtc_tutorial && source install/setup.bash` |
| Topic not found | Check node is running: `ros2 node list` |
| Frame errors | Change `frame_id` to match your robot's base frame |
