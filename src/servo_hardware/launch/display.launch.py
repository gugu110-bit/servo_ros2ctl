import launch
import launch_ros
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory
import xacro
def generate_launch_description():
    package_name = 'servo_hardware'
    package_share_directory = get_package_share_directory(package_name)
    robot_description_path = os.path.join(package_share_directory, 'urdf', 'model.urdf.xacro')
    robot_description_path='/media/gugu/DATA/servo_ws/src/servo_hardware/urdf/model.urdf.xacro'
    robot_description = xacro.process_file(robot_description_path).toxml()

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description}]
    )

    joint_state_publisher = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher',
        output='screen',
    )

    rviz=Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', os.path.join(package_share_directory, 'config', 'display.rviz')]
    )

    return launch.LaunchDescription([
        #robot_state_publisher,
        #joint_state_publisher,
        rviz,
        ])