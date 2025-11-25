#!/usr/bin/env python3
"""
Interactive CLI for MTC Task Execution

Provides a menu-driven interface for sending manipulation tasks.
"""

import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node
from mtc_tutorial.action import ExecuteTask
from geometry_msgs.msg import Pose
import sys


class InteractiveMTCClient(Node):
    """Interactive action client for MTC tasks."""

    def __init__(self):
        super().__init__('interactive_mtc_client')
        self._action_client = ActionClient(
            self,
            ExecuteTask,
            'execute_manipulation_task'
        )
        self.goal_active = False
        self.get_logger().info('Interactive MTC Client ready')

    def send_goal(self, task_type, target_pose, place_pose):
        """Send a manipulation task goal."""
        goal_msg = ExecuteTask.Goal()
        goal_msg.task_type = task_type
        goal_msg.target_pose = target_pose
        goal_msg.place_pose = place_pose
        
        self.get_logger().info('Waiting for action server...')
        if not self._action_client.wait_for_server(timeout_sec=5.0):
            self.get_logger().error('Action server not available!')
            return False
        
        self.get_logger().info(f'Sending {task_type} goal...')
        self.goal_active = True
        
        self._send_goal_future = self._action_client.send_goal_async(
            goal_msg,
            feedback_callback=self.feedback_callback
        )
        
        self._send_goal_future.add_done_callback(self.goal_response_callback)
        return True

    def feedback_callback(self, feedback_msg):
        """Handle feedback."""
        feedback = feedback_msg.feedback
        print(f'\r[{feedback.progress_percentage:5.1f}%] {feedback.current_stage:30s} '
              f'(~{feedback.estimated_time_remaining:4.0f}s)', end='', flush=True)

    def goal_response_callback(self, future):
        """Handle goal response."""
        goal_handle = future.result()
        
        if not goal_handle.accepted:
            print('\n✗ Goal rejected by server')
            self.goal_active = False
            return
        
        print('\n✓ Goal accepted, executing...')
        
        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        """Handle result."""
        result = future.result().result
        
        print()  # New line after progress
        if result.success:
            print(f'✓ SUCCESS: {result.message}')
            print(f'  Execution time: {result.execution_time:.2f}s')
        else:
            print(f'✗ FAILED: {result.message}')
        
        self.goal_active = False


def print_menu():
    """Display the main menu."""
    print("\n" + "="*60)
    print("  MTC Interactive Task Client")
    print("="*60)
    print("1. Pick and Place (default location)")
    print("2. Pick and Place (custom location)")
    print("3. Twist Knob (default location)")
    print("4. Twist Knob (custom location)")
    print("5. Quick test - Pick and Place")
    print("q. Quit")
    print("="*60)


def get_pose_input(prompt="Enter pose"):
    """Get pose coordinates from user."""
    print(f"\n{prompt}:")
    try:
        x = float(input("  x (meters): "))
        y = float(input("  y (meters): "))
        z = float(input("  z (meters): "))
        
        pose = Pose()
        pose.position.x = x
        pose.position.y = y
        pose.position.z = z
        pose.orientation.w = 1.0
        
        return pose
    except ValueError:
        print("Invalid input! Using default values.")
        return None


def main(args=None):
    rclpy.init(args=args)
    
    client = InteractiveMTCClient()
    
    # Spin in background thread
    import threading
    spin_thread = threading.Thread(target=rclpy.spin, args=(client,), daemon=True)
    spin_thread.start()
    
    print("\n🤖 MTC Interactive Client Started")
    print("Connect to the MTC action server to send manipulation tasks.")
    
    try:
        while rclpy.ok():
            # Wait if goal is active
            if client.goal_active:
                import time
                time.sleep(0.1)
                continue
            
            print_menu()
            choice = input("\nSelect option: ").strip().lower()
            
            if choice == 'q':
                print("Exiting...")
                break
            
            elif choice == '1':
                # Pick and place - default
                target = Pose()
                target.position.x = 0.5
                target.position.y = -0.25
                target.position.z = 0.0
                target.orientation.w = 1.0
                
                place = Pose()
                place.position.x = 0.3
                place.position.y = 0.3
                place.position.z = 0.2
                place.orientation.w = 1.0
                
                print(f"\nTarget: ({target.position.x}, {target.position.y}, {target.position.z})")
                print(f"Place:  ({place.position.x}, {place.position.y}, {place.position.z})")
                
                client.send_goal('pick_place', target, place)
            
            elif choice == '2':
                # Pick and place - custom
                target = get_pose_input("Target object location")
                if target is None:
                    continue
                
                place = get_pose_input("Place location")
                if place is None:
                    continue
                
                client.send_goal('pick_place', target, place)
            
            elif choice == '3':
                # Twist knob - default
                knob = Pose()
                knob.position.x = 0.6
                knob.position.y = -0.30
                knob.position.z = 0.2
                knob.orientation.w = 1.0
                
                print(f"\nKnob location: ({knob.position.x}, {knob.position.y}, {knob.position.z})")
                
                # Place pose not used for twist_knob
                place = Pose()
                place.orientation.w = 1.0
                
                client.send_goal('twist_knob', knob, place)
            
            elif choice == '4':
                # Twist knob - custom
                knob = get_pose_input("Knob location")
                if knob is None:
                    continue
                
                # Place pose not used for twist_knob
                place = Pose()
                place.orientation.w = 1.0
                
                client.send_goal('twist_knob', knob, place)
            
            elif choice == '5':
                # Quick test
                print("\n🚀 Quick test: Pick and Place")
                target = Pose()
                target.position.x = 0.5
                target.position.y = -0.25
                target.position.z = 0.0
                target.orientation.w = 1.0
                
                place = Pose()
                place.position.x = 0.3
                place.position.y = 0.3
                place.position.z = 0.2
                place.orientation.w = 1.0
                
                client.send_goal('pick_place', target, place)
            
            else:
                print("Invalid option. Please try again.")
    
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
    finally:
        client.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

