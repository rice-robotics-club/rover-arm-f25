# Rover Arm Fall 25
[Link to Node Diagram](https://docs.google.com/presentation/d/15b-iz1ovfDO4KFOj-s4Y4iy0UtaOKQtQMYVCW6hXz68/edit?slide=id.g38d27cd0724_0_2#slide=id.g38d27cd0724_0_2)
# Installs
Install ROS2 Jazzy
Install the following moveit dependencies
```
sudo apt install ros-$ROS_DISTRO-moveit-ros-control-interface
sudo apt install ros-$ROS_DISTRO-moveit-ros-planning-interface
sudo apt install ros-$ROS_DISTRO-moveit-core
```
# How to test
Build the image
```
cd rover_arm_f25
docker build -t thing .
```

Run the nodes
```
docker run -it --rm thing ros2 launch arm_control arm_communication.launch.py
```
The moveItNode terminal should print "I heard x coord '1.000000'"\
The vision terminal should print "X coord should be: 1.000000'"
## Changelog 
### 27 October 2025
- Made working publisher and subscriber to goal_pose topic. 
### 6 November 2025
- Forgot to update this. Created working /update_goal_item service, and /goal_pose topic.
- Also created a working ArmMovement action Server within the MoveItNode node.
### 12 January 2026
- Fixed the Mutlithreading issues by using callback groups
## To Do
- Make a dummy Teleop Node to issue commands to the moveItNode server
