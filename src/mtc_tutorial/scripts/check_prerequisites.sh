#!/bin/bash
# Check if prerequisites are running before starting action server

echo "Checking MTC Action Server Prerequisites..."
echo ""

# Check if move_group is running
if ros2 node list 2>/dev/null | grep -q "move_group"; then
    echo "✓ move_group is running"
else
    echo "✗ move_group is NOT running"
    echo ""
    echo "ERROR: You must start MoveIt environment first!"
    echo ""
    echo "Run this in another terminal:"
    echo "  ros2 launch moveit2_tutorials mtc_demo.launch.py"
    echo ""
    exit 1
fi

# Check if robot_description exists
if ros2 param list /move_group 2>/dev/null | grep -q "robot_description"; then
    echo "✓ robot_description parameter exists"
else
    echo "✗ robot_description parameter NOT found"
    echo ""
    echo "ERROR: MoveIt environment not fully initialized"
    echo "Wait a few more seconds for MoveIt to start"
    echo ""
    exit 1
fi

# Check if robot_description_semantic exists
if ros2 param list /move_group 2>/dev/null | grep -q "robot_description_semantic"; then
    echo "✓ robot_description_semantic parameter exists"
else
    echo "✗ robot_description_semantic parameter NOT found"
    echo ""
    echo "ERROR: SRDF not loaded"
    echo ""
    exit 1
fi

echo ""
echo "✓ All prerequisites met!"
echo "You can now start the MTC action server:"
echo "  ros2 launch mtc_tutorial mtc_action_server.launch.py"
echo ""

