from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    """
    Launch the MTC Action Server.
    
    This server waits for manipulation task goals and executes them.
    Send goals using the action clients (task_client.py or interactive_client.py).
    
    IMPORTANT: Start the MoveIt environment first:
      ros2 launch moveit2_tutorials mtc_demo.launch.py
    
    That launch file provides:
      - move_group with ExecuteTaskSolutionCapability (required for execution)
      - robot_state_publisher
      - static TF (world -> panda_link0)
      - ros2_control with fake hardware
      - RViz for visualization
    
    Arguments:
      execute:=true/false  - Enable/disable trajectory execution (default: false)
                            Set to false for plan-only mode (avoids move_group crash)
    """
    
    # Declare launch argument for execution control
    execute_arg = DeclareLaunchArgument(
        'execute',
        default_value='false',  # Default to false due to move_group crash issue
        description='Enable trajectory execution (set to false for plan-only mode)'
    )
    
    # Load complete MoveIt configuration for Panda robot
    moveit_config = (
        MoveItConfigsBuilder("moveit_resources_panda")
        .robot_description(file_path="config/panda.urdf.xacro")
        .robot_description_semantic(file_path="config/panda.srdf")
        .robot_description_kinematics(file_path="config/kinematics.yaml")
        .joint_limits(file_path="config/joint_limits.yaml")
        .planning_pipelines(pipelines=["ompl"])
        .to_moveit_configs()
    )
    
    mtc_node = Node(
        package='mtc_tutorial',
        executable='mtc_node',
        name='mtc_node',
        output='screen',
        parameters=[
            {
                'use_sim_time': False,
                'execute': LaunchConfiguration('execute'),
            },
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            moveit_config.joint_limits,
            moveit_config.planning_pipelines,
        ]
    )
    
    return LaunchDescription([
        execute_arg,
        mtc_node
    ])

