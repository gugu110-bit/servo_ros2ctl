#include "servo_hardware/servo_hardware.hpp"
#include <stdexcept>

namespace servo_hardware
{
hardware_interface::CallbackReturn ServoHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // 从 URDF 或参数中读取关节信息（示例为单关节）
  position_command_.resize(info.joints.size(), 1.57);  // 初始 ~90°
  position_state_.resize(info.joints.size(), 1.57);

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn ServoHardware::on_configure(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // 尝试打开蓝牙串口
  try
  {
    // 从参数服务器获取端口名和波特率（这里直接硬编码，可改为参数）
    io_service_ = std::make_shared<boost::asio::io_service>();
    serial_port_ = std::make_shared<boost::asio::serial_port>(*io_service_);
    serial_port_->open("/dev/rfcomm0");  
    serial_port_->set_option(boost::asio::serial_port_base::baud_rate(115200));
    RCLCPP_INFO(rclcpp::get_logger("ServoHardware"), "串口已打开");
  }
  catch (const std::exception & e)
  {
    RCLCPP_ERROR(rclcpp::get_logger("ServoHardware"), "打开串口失败: %s", e.what());
    return hardware_interface::CallbackReturn::ERROR;
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn ServoHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // 初始位置设为 90° (1.57 rad)
  for (size_t i = 0; i < position_command_.size(); ++i) {
    position_command_[i] = 1.57;   // 90°
    position_state_[i] = 1.57;
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn ServoHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // 关闭激光，回中
  for (size_t i = 0; i < position_command_.size(); ++i) {
    std::string cmd = (i == 0) ? "P:90\n" : "T:90\n";
    boost::asio::write(*serial_port_, boost::asio::buffer(cmd));
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type ServoHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // 这里假设无反馈，直接复制命令作为状态
  position_state_ = position_command_;
  return hardware_interface::return_type::OK;
}

// 导出状态接口：告诉 controller_manager 我们的关节状态存在哪里
std::vector<hardware_interface::StateInterface> ServoHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  for (size_t i = 0; i < info_.joints.size(); ++i)
  {
    state_interfaces.emplace_back(
      hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &position_state_[i]));
  }
  return state_interfaces;
}

// 导出命令接口：告诉 controller_manager 将命令写入我们提供的地址
std::vector<hardware_interface::CommandInterface> ServoHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (size_t i = 0; i < info_.joints.size(); ++i)
  {
    command_interfaces.emplace_back(
      hardware_interface::CommandInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &position_command_[i]));
  }
  return command_interfaces;
}

hardware_interface::return_type ServoHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  if (!serial_port_ || !serial_port_->is_open()) {
    return hardware_interface::return_type::OK;
  }

  for (size_t i = 0; i < info_.joints.size() && i < position_command_.size(); ++i) {
    double angle_rad = position_command_[i];
    int angle_deg = static_cast<int>(angle_rad * 180.0 / 3.14159265358979323846);
    if (angle_deg < 0) angle_deg = 0;
    if (angle_deg > 180) angle_deg = 180;

    std::string prefix = (i == 0) ? "P:" : "T:";
    std::string cmd = prefix + std::to_string(angle_deg) + "\n";

    try {
      boost::asio::write(*serial_port_, boost::asio::buffer(cmd));
    } catch (const std::exception & e) {
      RCLCPP_ERROR(rclcpp::get_logger("ServoHardware"), "发送失败: %s", e.what());
    }
  }
  return hardware_interface::return_type::OK;
}

}  // namespace servo_hardware

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(servo_hardware::ServoHardware, hardware_interface::SystemInterface)