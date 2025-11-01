cmake_minimum_required(VERSION 3.8)
project(arm_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)


#For update_goal_item service
rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/UpdateGoalItem.srv"
  "action/ArmMovement.action"
  DEPENDENCIES moveit_msgs
)
# Get typesupport target for linking
rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME} rosidl_typesupport_cpp)

#For Test Vision
add_executable(vision src/TestVision.cpp)
ament_target_dependencies(vision rclcpp geometry_msgs)
target_link_libraries(vision "${cpp_typesupport_target}")

#For MoveIt
add_executable(moveIt src/MoveIt.cpp)
ament_target_dependencies(moveIt rclcpp geometry_msgs)
target_link_libraries(moveIt "${cpp_typesupport_target}")

install(TARGETS
  vision
  moveIt
  DESTINATION lib/${PROJECT_NAME})



ament_package()

