# from launch import LaunchDescription
# from launch_ros.actions import Node
# from moveit_configs_utils import MoveItConfigsBuilder

# def generate_launch_description():
#     moveit_config = MoveItConfigsBuilder("moveit_resources_panda").to_dict()

#     # MTC Demo node
#     keyboard = Node(
#         package="arm_control",
#         executable="arm_control_node",
#         output="screen",
#         parameters=[
#             moveit_config,
#         ],
#     )
# moveit_task_constructor_demo
#     return LaunchDescription([keyboard])


import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    moveit_config = (
        MoveItConfigsBuilder("moveit_resources_panda")
        .robot_description(file_path="config/panda.urdf.xacro")
        .to_moveit_configs()
    )

    package = "arm_control"
    package_shared_path = get_package_share_directory(package)
    node = Node(
        package=package,
        executable=LaunchConfiguration("exe"),
        output="screen",
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            moveit_config.joint_limits,
            moveit_config.planning_pipelines,
            os.path.join(package_shared_path, "config", "panda_config.yaml"),
        ],
    )

    arg = DeclareLaunchArgument(name="exe")
    return LaunchDescription([arg, node])
