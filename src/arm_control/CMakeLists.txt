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
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)


#For the services and actions
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

#For MoveIt, has to be a library to turn it into an action server
add_library(moveIt SHARED 
src/MoveIt.cpp)
#for the stuff in the include folder
target_include_directories(moveIt PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(moveIt 
  rclcpp 
  rclcpp_action
  rclcpp_components
  geometry_msgs 
  moveit_msgs)
target_link_libraries(moveIt "${cpp_typesupport_target}")
#make it executable
rclcpp_components_register_node(moveIt 
  PLUGIN "arm_control::MoveItActionServer" 
  EXECUTABLE moveit_action_server)

#install vision
install(TARGETS
  vision
  DESTINATION lib/${PROJECT_NAME})
#install moveIt lib (which is also an executable)
install(TARGETS
  moveIt
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)



ament_package()

