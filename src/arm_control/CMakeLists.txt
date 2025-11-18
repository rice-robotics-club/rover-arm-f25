cmake_minimum_required(VERSION 3.8)
project(arm_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(moveit_msgs_DIR, "~/ws_moveit/src/moveit_msgs")

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(moveit_core REQUIRED)
find_package(moveit_ros_planning_interface)
find_package(moveit_ros_control_interface REQUIRED)
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

# For keyboard reading
add_executable(keyboard_reader src/KeyboardReader.cpp)
target_link_libraries(keyboard_reader PUBLIC rclcpp::rclcpp ${std_msgs_TARGETS})

add_executable(action_interpreter src/ActionInterpreter.cpp)
target_link_libraries(action_interpreter PUBLIC rclcpp::rclcpp ${std_msgs_TARGETS})

# add_executable(cartesian src/cartesian.cpp)
# target_link_libraries(cartesian PUBLIC rclcpp::rclcpp ${std_msgs_TARGETS})

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
  moveit_msgs
  moveit_core
  moveit_ros_planning_interface
  moveit_ros_control_interface
  )
target_link_libraries(moveIt "${cpp_typesupport_target}")
#make it executable
rclcpp_components_register_node(moveIt 
  PLUGIN "arm_control_cpp::MoveIt" 
  EXECUTABLE moveItNode)
#For Teleop node
add_library(Teleop SHARED 
  src/Teleop.cpp
)
target_include_directories(Teleop PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_definitions(Teleop
  PRIVATE "CUSTOM_ACTION_CPP_BUILDING_DLL")
ament_target_dependencies(Teleop
  rclcpp 
  rclcpp_action
  rclcpp_components
  moveit_msgs
  )
rosidl_target_interfaces(Teleop
  ${PROJECT_NAME} "rosidl_typesupport_cpp")
#make it executable
rclcpp_components_register_node(Teleop 
  PLUGIN "arm_control_cpp::Teleop" 
    EXECUTABLE teleopNode)


#install vision
install(TARGETS
  keyboard_reader
  action_interpreter
  vision
  # cartesian
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})
#install moveIt and Teleop lib (which is also an executable)
install(TARGETS
  Teleop
  moveIt
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)



ament_package()

