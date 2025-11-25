#!/bin/bash
# Debug script to check MoveIt status

echo "=== Checking MoveIt Status ==="
echo ""

echo "1. Checking if move_group node is running:"
if ros2 node list 2>/dev/null | grep -q "move_group"; then
    echo "   ✓ move_group node is running"
else
    echo "   ✗ move_group node is NOT running"
    echo ""
    echo "   Start MoveIt first:"
    echo "   ros2 launch moveit2_tutorials mtc_demo.launch.py"
    exit 1
fi

echo ""
echo "2. Checking robot_description parameter:"
if ros2 param get /move_group robot_description >/dev/null 2>&1; then
    echo "   ✓ robot_description exists"
    SIZE=$(ros2 param get /move_group robot_description 2>/dev/null | wc -c)
    echo "   Size: $SIZE bytes"
else
    echo "   ✗ robot_description NOT found"
fi

echo ""
echo "3. Checking robot_description_semantic parameter:"
if ros2 param get /move_group robot_description_semantic >/dev/null 2>&1; then
    echo "   ✓ robot_description_semantic exists"
    SIZE=$(ros2 param get /move_group robot_description_semantic 2>/dev/null | wc -c)
    echo "   Size: $SIZE bytes"
else
    echo "   ✗ robot_description_semantic NOT found"
fi

echo ""
echo "4. Checking all move_group parameters:"
ros2 param list /move_group 2>/dev/null | grep -E "robot_description|planning_plugin" | head -10

echo ""
echo "5. Checking if move_group is publishing on /robot_description:"
timeout 2 ros2 topic echo /robot_description --once >/dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "   ✓ /robot_description topic is publishing"
else
    echo "   ✗ /robot_description topic not found or not publishing"
fi

echo ""
echo "6. All active topics:"
ros2 topic list 2>/dev/null | grep -E "robot_description|planning" | head -10

echo ""
echo "=== Diagnosis Complete ==="

