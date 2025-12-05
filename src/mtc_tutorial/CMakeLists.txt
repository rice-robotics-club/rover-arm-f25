cmake_minimum_required(VERSION 3.8)
project(mtc_tutorial)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(moveit_task_constructor_core REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(action_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Generate action interfaces
rosidl_generate_interfaces(${PROJECT_NAME}
  "action/ExecuteTask.action"
  DEPENDENCIES geometry_msgs action_msgs
)

add_executable(mtc_node src/mtc_node.cpp src/tasks/pick_place.cpp src/tasks/twist_knob.cpp src/tasks/move_home.cpp)
target_include_directories(mtc_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_compile_features(mtc_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  mtc_node
  "moveit_task_constructor_core"
  "rclcpp"
  "rclcpp_action"
  "action_msgs"
  "geometry_msgs"
)

# Link against generated action interfaces
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(mtc_node "${cpp_typesupport_target}")

# Add include directories for generated action interfaces
target_include_directories(mtc_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp>)

install(TARGETS mtc_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
  )

install(PROGRAMS
  scripts/pose_publisher.py
  scripts/test_pose_subscriber.py
  scripts/task_client.py
  scripts/interactive_client.py
  DESTINATION lib/${PROJECT_NAME}
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
