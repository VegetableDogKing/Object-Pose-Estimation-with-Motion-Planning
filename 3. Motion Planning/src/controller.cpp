#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include "tf2/exceptions.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "vision_msgs/msg/detection3_d_array.hpp"

int main(int argc, char * argv[])
{
  // Initialize ROS and create the Node
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions node_options;
  node_options.automatically_declare_parameters_from_overrides(true);
  auto node = rclcpp::Node::make_shared("controller", node_options);

  // auto const node = std::make_shared<rclcpp::Node>(
  //   "controller",
  //   rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true)
  // );

  // We spin up a SingleThreadedExecutor for the current state monitor to get information
  // about the robot's state.
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread([&executor]() { executor.spin(); }).detach();

  // Create a ROS logger
  auto const logger = rclcpp::get_logger("controller");

  // Create a TF buffer and listener
  auto const tf_buffer = std::make_shared<tf2_ros::Buffer>(node->get_clock());
  auto const tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

  // Create the MoveIt MoveGroup Interface
  using moveit::planning_interface::MoveGroupInterface;
  auto move_group_interface = MoveGroupInterface(node, "tm5_arm");
  // move_group_interface.setPlanningPipelineId("move_group");
  const moveit::core::JointModelGroup* joint_model_group =
    move_group_interface.getCurrentState()->getJointModelGroup("tm5_arm");
  RCLCPP_INFO(logger, "getCurrentState");

  // rclcpp::sleep_for(std::chrono::seconds(1000)); // Retry after a short delay
  
  // Declare variables
  geometry_msgs::msg::Pose detection_pose;
  geometry_msgs::msg::Pose target_pose;
  bool valid_transform_received = false;
  bool valid_detection_received = false;

  // Detection callback to handle incoming detections
  auto detection_callback = [&detection_pose, &valid_detection_received, logger](const vision_msgs::msg::Detection3DArray::SharedPtr msg) {
    if (!msg->detections.empty()) {
      // Take the first detection from the array
      detection_pose = msg->detections[0].results[0].pose.pose;
      valid_detection_received = true;

      // Log the detection pose
      RCLCPP_INFO(logger, "Detection Pose: position: [%.2f, %.2f, %.2f], orientation: [%.2f, %.2f, %.2f, %.2f]",
                  detection_pose.position.x, detection_pose.position.y, detection_pose.position.z,
                  detection_pose.orientation.x, detection_pose.orientation.y, detection_pose.orientation.z, detection_pose.orientation.w);
    }
  };

  // Subscribe to /detections topic
  auto detection_subscription = node->create_subscription<vision_msgs::msg::Detection3DArray>(
    "/detections", 10, detection_callback
  );

  while (rclcpp::ok() && !(valid_transform_received && valid_detection_received)) {
    try {
      // Lookup the transform from "world" to "flange"
      geometry_msgs::msg::TransformStamped transform_world_to_flange =
          tf_buffer->lookupTransform("world", "flange", tf2::TimePointZero);

      geometry_msgs::msg::TransformStamped transform_world_to_dope =
          tf_buffer->lookupTransform("world", "dope_object1", tf2::TimePointZero);

      if (valid_detection_received) {
        // Set the target pose using the transform and detection pose
        // target_pose.position.x = transform_world_to_flange.transform.translation.x - detection_pose.position.x-0.1;
        // target_pose.position.y = transform_world_to_flange.transform.translation.y + detection_pose.position.z; // Adjust Y as requested
        // target_pose.position.z = transform_world_to_flange.transform.translation.z + detection_pose.position.y;

        // target_pose.orientation.x = transform_world_to_flange.transform.rotation.x;
        // target_pose.orientation.y = transform_world_to_flange.transform.rotation.y;
        // target_pose.orientation.z = transform_world_to_flange.transform.rotation.z;
        // target_pose.orientation.w = transform_world_to_flange.transform.rotation.w;

        target_pose.position.x = transform_world_to_dope.transform.translation.x+0.2;
        target_pose.position.y = transform_world_to_dope.transform.translation.y;
        target_pose.position.z = transform_world_to_dope.transform.translation.z;

        target_pose.orientation.x = transform_world_to_dope.transform.rotation.x;
        target_pose.orientation.y = transform_world_to_dope.transform.rotation.y;
        target_pose.orientation.z = transform_world_to_dope.transform.rotation.z;
        target_pose.orientation.w = transform_world_to_dope.transform.rotation.w;

        valid_transform_received = true;

        // Log the transformed and target poses
        RCLCPP_INFO(logger, "Transform Pose: position: [%.2f, %.2f, %.2f], orientation: [%.2f, %.2f, %.2f, %.2f]",
                    transform_world_to_flange.transform.translation.x, transform_world_to_flange.transform.translation.y, transform_world_to_flange.transform.translation.z,
                    transform_world_to_flange.transform.rotation.x, transform_world_to_flange.transform.rotation.y, transform_world_to_flange.transform.rotation.z, transform_world_to_flange.transform.rotation.w);

        RCLCPP_INFO(logger, "Target Pose: position: [%.2f, %.2f, %.2f], orientation: [%.2f, %.2f, %.2f, %.2f]",
                    target_pose.position.x, target_pose.position.y, target_pose.position.z,
                    target_pose.orientation.x, target_pose.orientation.y, target_pose.orientation.z, target_pose.orientation.w);

        RCLCPP_INFO(logger, "Dope Pose: position: [%.2f, %.2f, %.2f], orientation: [%.2f, %.2f, %.2f, %.2f]",
                    transform_world_to_dope.transform.translation.x, transform_world_to_dope.transform.translation.y, transform_world_to_dope.transform.translation.z,
                    transform_world_to_dope.transform.rotation.x, transform_world_to_dope.transform.rotation.y, transform_world_to_dope.transform.rotation.z, transform_world_to_dope.transform.rotation.w);

      }
    } catch (const tf2::TransformException &ex) {
      RCLCPP_WARN(logger, "Could not transform from world to flange: %s", ex.what());
      rclcpp::sleep_for(std::chrono::milliseconds(100)); // Retry after a short delay
    }

    // rclcpp::spin_some(node);
  }

  // If we received a valid transform and detection, proceed with motion planning
  if (valid_transform_received) {
    RCLCPP_INFO(logger, "Valid transform and detection received, proceeding with motion planning...");
    moveit::core::RobotState start_state(*move_group_interface.getCurrentState());
    start_state.setFromIK(joint_model_group, target_pose);
    move_group_interface.setStartState(start_state);
    // Set the target pose for MoveIt
    move_group_interface.setPoseTarget(target_pose);

    // Create a plan to that target pose
    auto const [success, plan] = [&move_group_interface]{
      moveit::planning_interface::MoveGroupInterface::Plan msg;
      auto const ok = static_cast<bool>(move_group_interface.plan(msg));
      return std::make_pair(ok, msg);
    }();

    // Execute the plan
    // if (success) {
    //   move_group_interface.execute(plan);
    // } else {
    //   RCLCPP_ERROR(logger, "Planning failed!");
    // }
  } else {
    RCLCPP_ERROR(logger, "Failed to get a valid transform or detection!");
  }

  // Shutdown ROS
  rclcpp::shutdown();
  return 0;
}
