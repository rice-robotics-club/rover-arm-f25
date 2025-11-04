from launch import LaunchDescription
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder

def generate_launch_description():
    moveit_config = MoveItConfigsBuilder("moveit_resources_panda").to_dict()
    launch_params = {'task': 'twist_knob'}

    # MTC Demo node
    pick_place_demo = Node(
        package="mtc_tutorial",
        executable="mtc_node",
        output="screen",
        parameters=[
            moveit_config, launch_params
        ],
    )

    return LaunchDescription([pick_place_demo])
