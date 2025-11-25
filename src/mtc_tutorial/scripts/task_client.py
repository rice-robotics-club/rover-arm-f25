#!/usr/bin/env python3
"""
Simple ROS2 Action Client for MTC Tasks

Sends manipulation task goals to the MTC action server.
"""

import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node
from mtc_tutorial.action import ExecuteTask
from geometry_msgs.msg import Pose


class MTCTaskClient(Node):
    """
    Action client for sending manipulation tasks to MTC server.
    """

    def __init__(self):
        super().__init__('mtc_task_client')
        self._action_client = ActionClient(
            self,
            ExecuteTask,
            'execute_manipulation_task'
        )
        self.get_logger().info('MTC Task Client initialized')

    def send_goal(self, task_type, target_pose, place_pose=None):
        """
        Send a manipulation task goal.
        
        Args:
            task_type (str): "pick_place" or "twist_knob"
            target_pose (Pose): Target object or knob location
            place_pose (Pose, optional): Place location for pick_place
        """
        goal_msg = ExecuteTask.Goal()
        goal_msg.task_type = task_type
        goal_msg.target_pose = target_pose
        
        if place_pose:
            goal_msg.place_pose = place_pose
        else:
            # Default place pose
            goal_msg.place_pose = Pose()
            goal_msg.place_pose.position.x = 0.3
            goal_msg.place_pose.position.y = 0.3
            goal_msg.place_pose.position.z = 0.2
            goal_msg.place_pose.orientation.w = 1.0
        
        self.get_logger().info(f'Waiting for action server...')
        self._action_client.wait_for_server()
        
        self.get_logger().info(f'Sending {task_type} goal...')
        self.get_logger().info(f'  Target: x={target_pose.position.x:.2f}, '
                              f'y={target_pose.position.y:.2f}, '
                              f'z={target_pose.position.z:.2f}')
        
        self._send_goal_future = self._action_client.send_goal_async(
            goal_msg,
            feedback_callback=self.feedback_callback
        )
        
        self._send_goal_future.add_done_callback(self.goal_response_callback)

    def feedback_callback(self, feedback_msg):
        """Handle feedback from action server."""
        feedback = feedback_msg.feedback
        self.get_logger().info(
            f'[{feedback.progress_percentage:5.1f}%] {feedback.current_stage} '
            f'(~{feedback.estimated_time_remaining:.0f}s remaining)'
        )

    def goal_response_callback(self, future):
        """Handle goal acceptance/rejection."""
        goal_handle = future.result()
        
        if not goal_handle.accepted:
            self.get_logger().error('Goal rejected by server')
            return
        
        self.get_logger().info('Goal accepted by server')
        
        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        """Handle final result."""
        result = future.result().result
        
        if result.success:
            self.get_logger().info(
                f'✓ Task completed successfully in {result.execution_time:.2f}s'
            )
            self.get_logger().info(f'  Message: {result.message}')
        else:
            self.get_logger().error(f'✗ Task failed: {result.message}')
        
        # Shutdown after receiving result
        rclpy.shutdown()


def main(args=None):
    rclpy.init(args=args)
    
    client = MTCTaskClient()
    
    # Example: Pick and place task
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
    
    try:
        rclpy.spin(client)
    except KeyboardInterrupt:
        client.get_logger().info('Client interrupted')
    finally:
        client.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()

