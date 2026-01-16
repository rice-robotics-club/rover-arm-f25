from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    # from MoveIt tutorial. No idea why I need it
    # moveit_config = MoveItConfigsBuilder("moveit_resources_panda").to_moveit_configs()

    vision_node = Node(
        package="arm_control",
        executable="vision",
        output="screen",
        # parameters=[
        #     moveit_config.robot_description,
        #     moveit_config.robot_description_semantic,
        #     moveit_config.robot_description_kinematics,
        # ],
    )

    moveit_node = Node(
        package="arm_control",
        executable="moveItNode",
        output="screen",
    )

    return LaunchDescription([vision_node, moveit_node])