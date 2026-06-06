import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterFile
import xacro

def generate_launch_description():
    pkg_share = get_package_share_directory('servo_hardware')

    # URDF 文件
    urdf_file = os.path.join(pkg_share, 'urdf', 'servo.urdf.xacro')
    robot_desc = xacro.process_file(urdf_file).toxml()

    # 控制器参数文件（已包含所有控制器定义）
    controller_yaml = os.path.join(pkg_share, 'config', 'servo_controllers.yaml')

    return LaunchDescription([
        # robot_state_publisher（用于发布 TF，可选）
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_desc}],
            output='screen'
        ),
        # ros2_control 节点，加载 YAML 和 URDF
        Node(
            package='controller_manager',
            executable='ros2_control_node',
            parameters=[
                {'robot_description': robot_desc},
                ParameterFile(controller_yaml)
            ],
            output='screen'
        ),
        # 启动控制器
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=['joint_state_broadcaster',
                       'pan_position_controller'
                       ],
            output='screen'
        ),
    ])