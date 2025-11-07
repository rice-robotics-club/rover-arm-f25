from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder

def generate_launch_description():
    moveit_config = MoveItConfigsBuilder("moveit_resources_panda").to_dict()

    # MTC Demo node
    keyboard = Node(
        package="arm_control",
        executable="arm_control_node",
        output="screen",
        parameters=[
            moveit_config,
        ],
    )

    return LaunchDescription([keyboard])