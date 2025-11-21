#!/usr/bin/env python3
"""
Simple test subscriber to verify pose publisher is working correctly.

This node subscribes to /target_pose and prints received poses.
Useful for testing the pose publisher without needing MoveIt2.
"""

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseStamped


class TestPoseSubscriber(Node):
    """
    Simple subscriber node for testing pose publisher.
    """

    def __init__(self):
        super().__init__('test_pose_subscriber')
        
        # Create subscription to target poses
        self.subscription = self.create_subscription(
            PoseStamped,
            '/target_pose',
            self.pose_callback,
            10
        )
        
        self.pose_count = 0
        
        self.get_logger().info('Test Pose Subscriber Started')
        self.get_logger().info('Listening to /target_pose topic...')
        self.get_logger().info('Waiting for poses...')

    def pose_callback(self, msg):
        """
        Callback function that receives and displays poses.
        
        Args:
            msg: PoseStamped message containing target pose
        """
        self.pose_count += 1
        
        self.get_logger().info('=' * 60)
        self.get_logger().info(f'Received Pose #{self.pose_count}')
        self.get_logger().info(f'Frame: {msg.header.frame_id}')
        self.get_logger().info(f'Timestamp: {msg.header.stamp.sec}.{msg.header.stamp.nanosec}')
        self.get_logger().info('-' * 60)
        self.get_logger().info('Position:')
        self.get_logger().info(f'  x: {msg.pose.position.x:.4f} m')
        self.get_logger().info(f'  y: {msg.pose.position.y:.4f} m')
        self.get_logger().info(f'  z: {msg.pose.position.z:.4f} m')
        self.get_logger().info('Orientation (quaternion):')
        self.get_logger().info(f'  x: {msg.pose.orientation.x:.4f}')
        self.get_logger().info(f'  y: {msg.pose.orientation.y:.4f}')
        self.get_logger().info(f'  z: {msg.pose.orientation.z:.4f}')
        self.get_logger().info(f'  w: {msg.pose.orientation.w:.4f}')
        self.get_logger().info('=' * 60)
        
        # Validate pose
        self.validate_pose(msg)

    def validate_pose(self, msg):
        """
        Perform basic validation on received pose.
        
        Args:
            msg: PoseStamped message to validate
        """
        # Check if position is reasonable (within typical robot workspace)
        pos = msg.pose.position
        if abs(pos.x) > 2.0 or abs(pos.y) > 2.0 or abs(pos.z) > 2.0:
            self.get_logger().warn('⚠️  Position values seem large - check if units are correct')
        
        # Check if orientation quaternion is normalized
        ori = msg.pose.orientation
        magnitude = (ori.x**2 + ori.y**2 + ori.z**2 + ori.w**2)**0.5
        if abs(magnitude - 1.0) > 0.01:
            self.get_logger().warn(f'⚠️  Quaternion not normalized! Magnitude: {magnitude:.4f}')
        else:
            self.get_logger().info('✓ Quaternion is normalized')
        
        # Check frame_id
        if msg.header.frame_id == '':
            self.get_logger().warn('⚠️  Frame ID is empty!')
        else:
            self.get_logger().info(f'✓ Frame ID: {msg.header.frame_id}')


def main(args=None):
    rclpy.init(args=args)
    
    test_subscriber = TestPoseSubscriber()
    
    try:
        rclpy.spin(test_subscriber)
    except KeyboardInterrupt:
        test_subscriber.get_logger().info(f'\nReceived {test_subscriber.pose_count} poses total')
        test_subscriber.get_logger().info('Shutting down Test Pose Subscriber')
    except Exception as e:
        test_subscriber.get_logger().error(f'Exception: {e}')
    finally:
        test_subscriber.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()

