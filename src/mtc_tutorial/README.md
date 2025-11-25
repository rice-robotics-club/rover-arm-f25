# MoveIt Task Constructor - Action-Based Manipulation

This package provides an action server for executing manipulation tasks (pick-and-place, twist knob) with MoveIt Task Constructor.

## Setup

Build the package:
```bash
cd ~/rover-arm-f25
colcon build --packages-select mtc_tutorial
source install/setup.bash
```

## Quick Start

⚠️ **IMPORTANT**: Start terminals in this exact order!

### Terminal 1: Start MoveIt Environment (REQUIRED FIRST!)
```bash
source install/setup.bash
ros2 launch moveit2_tutorials mtc_demo.launch.py
```
**Wait for RViz to open and show the robot before continuing!**

### Terminal 2: Start MTC Action Server
```bash
source install/setup.bash
ros2 launch mtc_tutorial mtc_action_server.launch.py
```
**Wait for**: `MTC Action Server ready - waiting for goals`

### Terminal 3: Send Task Goals

**Option A - Interactive CLI:**
```bash
source install/setup.bash
ros2 run mtc_tutorial interactive_client.py
```

**Option B - Simple Client:**
```bash
source install/setup.bash
ros2 run mtc_tutorial task_client.py
```

## Action Interface

The action server accepts goals on `/execute_manipulation_task`:

**Goal:**
- `task_type` (string): "pick_place" or "twist_knob"
- `target_pose` (Pose): Object or knob location
- `place_pose` (Pose): Where to place (for pick_place)

**Feedback:**
- `current_stage` (string): Current execution stage
- `progress_percentage` (float): 0-100
- `estimated_time_remaining` (float): Seconds

**Result:**
- `success` (bool): Task completion status
- `message` (string): Result description
- `execution_time` (float): Total time in seconds

## Python Clients

### Interactive Client
Menu-driven interface for sending tasks:
```bash
ros2 run mtc_tutorial interactive_client.py
```

### Simple Client
Programmatic example:
```bash
ros2 run mtc_tutorial task_client.py
```

## Legacy Demos (Deprecated)

Old one-shot execution demos (use action server instead):
```bash
ros2 launch mtc_tutorial pick_place_demo.launch.py
ros2 launch mtc_tutorial twist_knob.launch.py
```

## Additional Tools

### Pose Publisher
Test tool that publishes example poses:
```bash
ros2 launch mtc_tutorial pose_publisher.launch.py
```

See `scripts/README.md` for details.