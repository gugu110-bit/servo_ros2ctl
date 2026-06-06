cmake_minimum_required(VERSION 3.8)
project(servo_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(Boost COMPONENTS system serialization REQUIRED)

install(DIRECTORY urdf DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})

add_library(servo_hardware SHARED
  src/servo_hardware.cpp
)
target_include_directories(servo_hardware PRIVATE include)
ament_target_dependencies(servo_hardware hardware_interface pluginlib rclcpp)
target_link_libraries(servo_hardware Boost::system Boost::serialization)

pluginlib_export_plugin_description_file(hardware_interface servo_hardware.xml)

install(TARGETS servo_hardware
  DESTINATION lib
)
install(DIRECTORY include/
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
