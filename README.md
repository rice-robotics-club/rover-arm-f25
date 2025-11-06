# Rover Arm Fall 25
[Link to Node Diagram](https://docs.google.com/presentation/d/15b-iz1ovfDO4KFOj-s4Y4iy0UtaOKQtQMYVCW6hXz68/edit?slide=id.g38d27cd0724_0_2#slide=id.g38d27cd0724_0_2)
# How to test
Make package
```
cd rover_arm_f25
colcon build
```
Source the setup.bash
```
source install/setup.bash
```

Run vision node 
```
ros2 run arm_control vision
```
You should see "No goal item set"\
Run the moveit node in a separate terminal
```
ros2 run arm_control moveItNode
```
The moveItNode terminal should print "I heard x coord '1.000000'"\
The vision terminal should print "Publishing goal for: BRICK"
## Changelog 
### 27 October 2025
- Made working publisher and subscriber to goal_pose topic. 
### 6 November 2025
- Forgot to update this. Created working /update_goal_item service, and /goal_pose topic.
- Also created a working ArmMovement action Server within the MoveItNode node.
##To Do
- Make a dummy Teleop Node to issue commands to the moveItNode server
