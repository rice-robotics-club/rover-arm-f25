namespace mtc = moveit::task_constructor;

class MTCTaskNode
{
public:
  MTCTaskNode(const rclcpp::NodeOptions& options);

  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr getNodeBaseInterface();

  void doTask(const std::string &task_name);

  void setupPlanningScene();

private:
  // Compose an MTC task from a series of stages.
  mtc::Task createTask(const std::string &task_name);
  mtc::Task createPickPlaceTask();
  mtc::Task createTwistKnobTask();
  mtc::Task task_;
  rclcpp::Node::SharedPtr node_;
};
