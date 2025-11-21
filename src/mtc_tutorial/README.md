# MoveIt Task Constructor Testing (Panda Arm Demo)

This package contains the C++ node and launch files to run the MoveIt Task Constructor (MTC) Pick-and-Place demo.

## Setup

Assuming you have cloned the main repository and run the initial `vcs import` and `rosdep install` steps:

1.  **Rebuild Package:** Navigate to the workspace root and build this specific package.
    ```bash
    cd ~/rover-arm-f25
    colcon build --mixin debug --packages-select mtc_tutorial
    ```

## Usage

To run the full Pick-and-Place demo, you must launch the MoveIt planning environment in one terminal and the application logic (this package's node) in a second terminal.

**IMPORTANT:** You must run the `source` command in **both** terminals before proceeding.

### Terminal 1: Start the MoveIt Environment
This launches the Panda arm model, the `move_group` node, and RViz.
```bash
source install/setup.bash
ros2 launch moveit2_tutorials mtc_demo.launch.py
```

### Terminal 2: Run the Pick + Place Application
This runs the C++ node containing the MTC task sequence. Or, choose the twist_knob launch file for that task sequence. 
```bash
source install/setup.bash
ros2 launch mtc_tutorial pick_place_demo.launch.py
```