# MTC Action Server - Usage Guide

## Overview

The MTC package now uses a ROS2 Action Server architecture for on-demand task execution. This allows you to send manipulation goals dynamically with custom coordinates.

## Architecture

```
Python Client → Action Goal → C++ Action Server → MTC Planning → Robot Execution
```

## Quick Start

### 1. Start MoveIt Environment
```bash
# Terminal 1
cd ~/rover-arm-f25
source install/setup.bash
ros2 launch moveit2_tutorials mtc_demo.launch.py
```

### 2. Start Action Server
```bash
# Terminal 2
source install/setup.bash
ros2 launch mtc_tutorial mtc_action_server.launch.py
```

You should see: `MTC Action Server ready - waiting for goals`

### 3. Send Goals

**Interactive Client (Recommended):**
```bash
# Terminal 3
source install/setup.bash
ros2 run mtc_tutorial interactive_client.py
```

Follow the menu to select tasks and enter coordinates.

## Action Interface Details

### Action Name
`/execute_manipulation_task`

### Message Type
`mtc_tutorial/action/ExecuteTask`

### Goal Structure
```
string task_type                    # "pick_place" or "twist_knob"
geometry_msgs/Pose target_pose      # Object or knob location
geometry_msgs/Pose place_pose       # Place location (pick_place only)
```

### Feedback Structure
```
string current_stage                # e.g., "Planning motion", "Executing"
float32 progress_percentage         # 0.0 to 100.0
float32 estimated_time_remaining    # Seconds
```

### Result Structure
```
bool success                        # True if task completed
string message                      # Result description
float32 execution_time              # Total time in seconds
```

## Usage Examples

### Example 1: Pick and Place

```python
from mtc_tutorial.action import ExecuteTask
from geometry_msgs.msg import Pose

goal = ExecuteTask.Goal()
goal.task_type = "pick_place"

# Object location
goal.target_pose.position.x = 0.5
goal.target_pose.position.y = -0.25
goal.target_pose.position.z = 0.0
goal.target_pose.orientation.w = 1.0

# Place location
goal.place_pose.position.x = 0.3
goal.place_pose.position.y = 0.3
goal.place_pose.position.z = 0.2
goal.place_pose.orientation.w = 1.0

# Send goal...
```

### Example 2: Twist Knob

```python
goal = ExecuteTask.Goal()
goal.task_type = "twist_knob"

# Knob location
goal.target_pose.position.x = 0.6
goal.target_pose.position.y = -0.30
goal.target_pose.position.z = 0.2
goal.target_pose.orientation.w = 1.0

# place_pose not used for twist_knob
goal.place_pose.orientation.w = 1.0

# Send goal...
```

## Coordinate Frames

- **Frame**: `world` (base frame)
- **Units**: Meters
- **Orientation**: Quaternion (x, y, z, w)

### Typical Workspace
- X: 0.2 to 0.8 meters (forward)
- Y: -0.5 to 0.5 meters (left/right)
- Z: 0.0 to 0.6 meters (up)

## Task Types

### pick_place
1. Opens gripper
2. Moves to approach pose above object
3. Descends to grasp
4. Closes gripper
5. Lifts object
6. Moves to place location
7. Descends to place
8. Opens gripper
9. Retracts

**Required**: `target_pose` (object), `place_pose` (destination)

### twist_knob
1. Opens gripper
2. Approaches knob
3. Grasps knob
4. Twists 90 degrees
5. Releases
6. Retracts

**Required**: `target_pose` (knob location)

## Feedback Stages

During execution, you'll see these stages:

1. **Setting up planning scene** (10%)
2. **Creating task** (20%)
3. **Initializing task** (30%)
4. **Planning motion** (50%)
5. **Executing motion** (75%)
6. **Complete** (100%)

## Error Handling

### Goal Rejected
- Task already in progress
- Invalid task_type
- Server not ready

### Planning Failed
- Unreachable coordinates
- Collision detected
- IK solution not found

### Execution Failed
- Controller error
- Unexpected collision
- Hardware issue

## Command Line Tools

### Check Action Server Status
```bash
ros2 action list
# Should show: /execute_manipulation_task

ros2 action info /execute_manipulation_task
# Shows action type and active goals
```

### Send Goal from CLI
```bash
ros2 action send_goal /execute_manipulation_task mtc_tutorial/action/ExecuteTask \
  "{task_type: 'pick_place', 
    target_pose: {position: {x: 0.5, y: -0.25, z: 0.0}, orientation: {w: 1.0}},
    place_pose: {position: {x: 0.3, y: 0.3, z: 0.2}, orientation: {w: 1.0}}}" \
  --feedback
```

## Python Client Template

```python
#!/usr/bin/env python3
import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node
from mtc_tutorial.action import ExecuteTask
from geometry_msgs.msg import Pose

class MyClient(Node):
    def __init__(self):
        super().__init__('my_client')
        self._client = ActionClient(self, ExecuteTask, 'execute_manipulation_task')
    
    def send_goal(self, task_type, target, place):
        goal = ExecuteTask.Goal()
        goal.task_type = task_type
        goal.target_pose = target
        goal.place_pose = place
        
        self._client.wait_for_server()
        future = self._client.send_goal_async(goal, feedback_callback=self.feedback_cb)
        future.add_done_callback(self.goal_response_cb)
    
    def feedback_cb(self, msg):
        print(f"Progress: {msg.feedback.progress_percentage}%")
    
    def goal_response_cb(self, future):
        goal_handle = future.result()
        if goal_handle.accepted:
            result_future = goal_handle.get_result_async()
            result_future.add_done_callback(self.result_cb)
    
    def result_cb(self, future):
        result = future.result().result
        print(f"Success: {result.success}, Time: {result.execution_time}s")

def main():
    rclpy.init()
    client = MyClient()
    
    target = Pose()
    target.position.x = 0.5
    target.position.y = -0.25
    target.orientation.w = 1.0
    
    place = Pose()
    place.position.x = 0.3
    place.position.y = 0.3
    place.position.z = 0.2
    place.orientation.w = 1.0
    
    client.send_goal('pick_place', target, place)
    rclpy.spin(client)

if __name__ == '__main__':
    main()
```

## Troubleshooting

### Server Not Starting
```bash
# Check if node is running
ros2 node list | grep mtc_node

# Check for errors
ros2 launch mtc_tutorial mtc_action_server.launch.py
```

### Goals Rejected
- Ensure only one goal at a time
- Check task_type is valid
- Verify server is ready

### Planning Failures
- Check coordinates are reachable
- Verify no collisions in scene
- Try different target poses

### No Feedback
- Ensure client has feedback callback
- Check action connection
- Verify server is processing goal

## Integration with Vision

To use with vision system:

1. Vision node detects object
2. Publishes pose to topic or service
3. Your client receives pose
4. Sends action goal with detected pose
5. MTC executes task

## Performance

- **Planning time**: 1-5 seconds
- **Execution time**: 10-30 seconds
- **Total time**: 15-40 seconds per task

## Next Steps

- Integrate with vision system
- Add custom task types
- Create web dashboard (React + rosbridge)
- Implement task queuing
- Add trajectory visualization

