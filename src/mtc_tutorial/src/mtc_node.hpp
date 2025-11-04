namespace mtc = moveit::task_constructor;

class MTCTaskNode
{
public:
  MTCTaskNode(const rclcpp::NodeOptions& options);

  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr getNodeBaseInterface();

  void doTask(const std::string &task_name);

  rclcpp::Node::SharedPtr getNode();

private:
  // Compose an MTC task from a series of stages.
  mtc::Task createTask(const std::string &task_name);
  void setupPlanningScene(const std::string &task_name);

  mtc::Task createPickPlaceTask();
  void setupPickPlaceScene();

  mtc::Task createTwistKnobTask();
  void setupTwistKnobScene();

  struct TaskEntry {
    std::function<mtc::Task()> create_task;
    std::function<void()> setup_scene;
  };

  std::unordered_map<std::string, TaskEntry> task_map_;

  mtc::Task task_;
  rclcpp::Node::SharedPtr node_;
};
