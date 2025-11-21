#!/usr/bin/env python3
"""
Simple ROS2 Python node that publishes example target poses for MoveIt2.

This node publishes geometry_msgs/PoseStamped messages to the /target_pose topic.
It cycles through several predefined example poses to demonstrate motion planning.
"""

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseStamped

class PosePublisher(Node):
    """
    Publishes example target poses for robot motion planning.
    """

    def __init__(self):
        super().__init__('pose_publisher')
        
        # Create publisher for target poses
        self.publisher_ = self.create_publisher(
            PoseStamped,
            '/target_pose',
            10
        )
        
        # Timer to publish poses periodically (every 10 seconds)
        self.timer = self.create_timer(10.0, self.timer_callback)
        
        # Counter to cycle through different example poses
        self.pose_index = 0
        
        # Define several example poses
        self.example_poses = self.create_example_poses()
        
        self.get_logger().info('Pose Publisher Node Started')
        self.get_logger().info(f'Publishing to topic: /target_pose')
        self.get_logger().info(f'Number of example poses: {len(self.example_poses)}')
        self.get_logger().info('Press Ctrl+C to stop')

    def create_example_poses(self):
        """
        Create a list of example target poses.
        
        Returns:
            list: List of PoseStamped messages with different target poses
        """
        poses = []
        
        # Pose 1: Forward and centered
        pose1 = PoseStamped()
        pose1.header.frame_id = "world"
        pose1.pose.position.x = 0.4
        pose1.pose.position.y = 0.0
        pose1.pose.position.z = 0.3
        pose1.pose.orientation.x = 1.0
        pose1.pose.orientation.y = 0.0
        pose1.pose.orientation.z = 0.0
        pose1.pose.orientation.w = 0.0
        poses.append(("Forward Center", pose1))
        
        # Pose 2: Forward and to the right
        pose2 = PoseStamped()
        pose2.header.frame_id = "world"
        pose2.pose.position.x = 0.5
        pose2.pose.position.y = -0.2
        pose2.pose.position.z = 0.4
        pose2.pose.orientation.x = 0.924
        pose2.pose.orientation.y = 0.383
        pose2.pose.orientation.z = 0.0
        pose2.pose.orientation.w = 0.0
        poses.append(("Forward Right", pose2))
        
        # Pose 3: Forward and to the left
        pose3 = PoseStamped()
        pose3.header.frame_id = "world"
        pose3.pose.position.x = 0.5
        pose3.pose.position.y = 0.2
        pose3.pose.position.z = 0.4
        pose3.pose.orientation.x = 0.924
        pose3.pose.orientation.y = -0.383
        pose3.pose.orientation.z = 0.0
        pose3.pose.orientation.w = 0.0
        poses.append(("Forward Left", pose3))
        
        # Pose 4: Higher position
        pose4 = PoseStamped()
        pose4.header.frame_id = "world"
        pose4.pose.position.x = 0.3
        pose4.pose.position.y = 0.0
        pose4.pose.position.z = 0.5
        pose4.pose.orientation.x = 1.0
        pose4.pose.orientation.y = 0.0
        pose4.pose.orientation.z = 0.0
        pose4.pose.orientation.w = 0.0
        poses.append(("High Position", pose4))
        
        # Pose 5: Lower position
        pose5 = PoseStamped()
        pose5.header.frame_id = "world"
        pose5.pose.position.x = 0.4
        pose5.pose.position.y = 0.0
        pose5.pose.position.z = 0.2
        pose5.pose.orientation.x = 1.0
        pose5.pose.orientation.y = 0.0
        pose5.pose.orientation.z = 0.0
        pose5.pose.orientation.w = 0.0
        poses.append(("Low Position", pose5))
        
        return poses

    def timer_callback(self):
        """
        Callback function that publishes the next example pose.
        """
        # Get the current pose
        pose_name, pose = self.example_poses[self.pose_index]
        
        # Update timestamp
        pose.header.stamp = self.get_clock().now().to_msg()
        
        # Publish the pose
        self.publisher_.publish(pose)
        
        # Log the published pose
        self.get_logger().info(f'Published Pose #{self.pose_index + 1}: {pose_name}')
        self.get_logger().info(f'  Position: x={pose.pose.position.x:.3f}, '
                              f'y={pose.pose.position.y:.3f}, '
                              f'z={pose.pose.position.z:.3f}')
        self.get_logger().info(f'  Orientation: x={pose.pose.orientation.x:.3f}, '
                              f'y={pose.pose.orientation.y:.3f}, '
                              f'z={pose.pose.orientation.z:.3f}, '
                              f'w={pose.pose.orientation.w:.3f}')
        
        # Move to next pose (cycle through)
        self.pose_index = (self.pose_index + 1) % len(self.example_poses)

    def publish_single_pose(self, x, y, z, qx=1.0, qy=0.0, qz=0.0, qw=0.0):
        """
        Publish a single pose with specified coordinates.
        
        Args:
            x, y, z: Position coordinates
            qx, qy, qz, qw: Orientation quaternion (default: pointing down)
        """
        pose = PoseStamped()
        pose.header.frame_id = "world"
        pose.header.stamp = self.get_clock().now().to_msg()
        
        pose.pose.position.x = x
        pose.pose.position.y = y
        pose.pose.position.z = z
        
        pose.pose.orientation.x = qx
        pose.pose.orientation.y = qy
        pose.pose.orientation.z = qz
        pose.pose.orientation.w = qw
        
        self.publisher_.publish(pose)
        self.get_logger().info(f'Published custom pose: ({x}, {y}, {z})')


def main(args=None):
    rclpy.init(args=args)
    
    pose_publisher = PosePublisher()
    
    try:
        rclpy.spin(pose_publisher)
    except KeyboardInterrupt:
        pose_publisher.get_logger().info('Shutting down Pose Publisher Node')
    except Exception as e:
        pose_publisher.get_logger().error(f'Exception in pose publisher: {e}')
    finally:
        pose_publisher.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()

