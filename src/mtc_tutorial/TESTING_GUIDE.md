# MTC Action Server - Testing Guide

## The Problem You Encountered

**Error**: `Could not find parameter robot_description_semantic`

**Cause**: The MoveIt environment (`move_group` node) wasn't running, so the robot description wasn't available.

**Solution**: Always start MoveIt BEFORE starting the action server.

---

## Correct Testing Procedure

### Step 1: Start MoveIt Environment (REQUIRED)

**Terminal 1:**
```bash
cd ~/rover-arm-f25
source install/setup.bash
ros2 launch moveit2_tutorials mtc_demo.launch.py
```

**Wait for:**
- RViz window to open
- You see the Panda robot in RViz
- Console shows: `You can start planning now!`

**This provides:**
- `robot_description` parameter
- `robot_description_semantic` parameter
- `move_group` node for planning
- RViz for visualization

---

### Step 2: Start MTC Action Server

**Terminal 2:**
```bash
cd ~/rover-arm-f25
source install/setup.bash
ros2 launch mtc_tutorial mtc_action_server.launch.py
```

**Wait for:**
```
[INFO] [mtc_tutorial]: MTC Action Server ready - waiting for goals
[INFO] [mtc_tutorial]: MTC Action Server spinning - send goals to 'execute_manipulation_task'
```

**No errors about robot_description!**

---

### Step 3: Send Task Goal

**Terminal 3:**
```bash
cd ~/rover-arm-f25
source install/setup.bash
ros2 run mtc_tutorial interactive_client.py
```

Then press `5` for quick test.

---

## Verification Checklist

Before sending goals, verify:

```bash
# Check move_group is running
ros2 node list | grep move_group
# Should show: /move_group

# Check robot_description exists
ros2 param list /move_group | grep robot_description
# Should show: robot_description and robot_description_semantic

# Check action server is running
ros2 action list
# Should show: /execute_manipulation_task

# Check action server info
ros2 action info /execute_manipulation_task
# Should show: Action servers: 1
```

---

## Common Issues

### Issue 1: "Could not find parameter robot_description"
**Solution**: Start MoveIt environment first (Terminal 1)

### Issue 2: "Action server not available"
**Solution**: 
1. Make sure Terminal 1 (MoveIt) is running
2. Start Terminal 2 (action server)
3. Wait 5 seconds
4. Then start Terminal 3 (client)

### Issue 3: "Planning failed"
**Solution**: 
- Make sure RViz shows the robot
- Try default coordinates first (option 5)
- Check for collision objects in RViz

### Issue 4: Multiple "Publisher already registered" warnings
**Solution**: These are harmless warnings from MTC creating multiple nodes internally. Ignore them.

---

## Full Test Sequence

### Test 1: Quick Pick and Place
1. Start all 3 terminals in order
2. In Terminal 3, press `5`
3. **Expected**: Robot picks object and places it
4. **Time**: 20-40 seconds

### Test 2: Custom Pick and Place
1. In Terminal 3, press `2`
2. Enter object location: `0.5, -0.2, 0.0`
3. Enter place location: `0.3, 0.3, 0.2`
4. **Expected**: Robot picks from custom location

### Test 3: Twist Knob
1. In Terminal 3, press `3`
2. **Expected**: Robot grasps and twists knob

---

## What Success Looks Like

### Terminal 1 (MoveIt):
```
[INFO] [move_group]: Ready to take commands for planning group panda_arm
```

### Terminal 2 (Action Server):
```
[INFO] [mtc_tutorial]: Received goal request for task: pick_place
[INFO] [mtc_tutorial]: Goal accepted for task: pick_place
[INFO] [mtc_tutorial]: Starting execution of task: pick_place
[INFO] [mtc_tutorial]: Using action goal target pose: x=0.50, y=-0.25, z=0.00
[INFO] [mtc_tutorial]: Planning succeeded, found 1 solutions
[INFO] [mtc_tutorial]: Task completed successfully in 23.45 seconds
```

### Terminal 3 (Client):
```
✓ Goal accepted by server
[10.0%] Setting up planning scene (~30s remaining)
[20.0%] Creating task (~25s remaining)
[30.0%] Initializing task (~20s remaining)
[50.0%] Planning motion (~15s remaining)
[75.0%] Executing motion (~10s remaining)
[100.0%] Complete (~0s remaining)
✓ Task completed successfully in 23.45s
```

### RViz:
- Trajectory displayed in Motion Planning panel
- Robot animates the motion
- Object moves from pick to place location

---

## Troubleshooting Commands

### Check if everything is running:
```bash
# Should show: move_group, mtc_node
ros2 node list

# Should show: /execute_manipulation_task
ros2 action list

# Should show: robot_description parameters
ros2 param list /move_group | grep robot_description
```

### Kill stuck processes:
```bash
# Kill all ROS2 nodes
pkill -9 -f ros2

# Then restart from Terminal 1
```

### Check logs:
```bash
# View action server logs
ros2 run rqt_console rqt_console

# Or check specific node
ros2 node info /mtc_node
```

---

## Performance Expectations

- **Planning time**: 2-5 seconds
- **Execution time**: 15-30 seconds  
- **Total time**: 20-40 seconds per task
- **Success rate**: >90% with default coordinates

---

## Next Steps After Successful Test

1. ✅ Verify pick-and-place works
2. ✅ Verify twist-knob works
3. ✅ Test custom coordinates
4. 🚀 Integrate with vision system
5. 🚀 Create web dashboard
6. 🚀 Deploy to real robot

---

## Quick Reference

**Start Order:**
1. MoveIt → 2. Action Server → 3. Client

**Stop Order:**
1. Client (Ctrl+C) → 2. Action Server (Ctrl+C) → 3. MoveIt (Ctrl+C)

**Restart:**
Kill all, then start from Terminal 1 again.

