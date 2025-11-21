from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    """Launch the pose publisher node."""
    
    pose_publisher_node = Node(
        package='mtc_tutorial',
        executable='pose_publisher.py',
        name='pose_publisher',
        output='screen',
        parameters=[{
            'use_sim_time': False
        }]
    )
    
    return LaunchDescription([
        pose_publisher_node
    ])

