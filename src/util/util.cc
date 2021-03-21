#include "util.hh"

bool check_range(float value, float lower, float upper) {
  if (value < lower) {
    return false;
  } else if (value > upper) {
    return false;
  }
  return true;
}

std::array<unsigned char, 4> encode_joystick(const Joystick &joystick) {
  std::array<unsigned char, 4> ret;

  // Place the header byte in the high byte
  ret[3] = 0xFE;

  if (!check_range(joystick.x, -1.0, 1.0) ||
      !check_range(joystick.y, -1.0, 1.0)) {
    throw std::runtime_error("Invalid joystick value");
  }

  // Add 1 to get value between 0 and 2, multiply by 127.5 to get between 0 and
  // 255
  ret[2] = static_cast<unsigned char>(std::floor(((joystick.x + 1) * 127)));
  ret[1] = static_cast<unsigned char>(std::floor(((joystick.y + 1) * 127)));
  ret[0] = 0;

  ret[0] |= (joystick.enabled << 7);
  ret[0] |= (joystick.gripper_toggle << 2);
  ret[0] |= (joystick.roller_fwd << 1);
  ret[0] |= (joystick.roller_rev << 0);
  return ret;
}

std::optional<Joystick>
decode_joystick(const std::array<unsigned char, 4> &raw) {
  if (raw[3] != 0xFE) {
    return std::nullopt;
  }

  Joystick ret;
  ret.x = ((static_cast<float>(raw[2]) - 127) / 127);
  ret.y = ((static_cast<float>(raw[1]) - 127) / 127);

  ret.enabled = raw[0] & (1 << 7);
  ret.gripper_toggle = raw[0] & (1 << 2);
  ret.roller_fwd = raw[0] & (1 << 1);
  ret.roller_rev = raw[0] & (1 << 0);

  return std::optional(ret);
}

std::array<unsigned char, 4> encode_output(const Output &raw) {
  std::array<unsigned char, 4> ret;

  if (!check_range(raw.dt_left_voltage, -12, 12) ||
      !check_range(raw.dt_right_voltage, -12, 12) ||
      !check_range(raw.arm_voltage, -12, 12)) {
    throw std::runtime_error("Invalid output voltage value");
  }

  // go from -12 to 12 to -127 to 127, add 127
  ret[3] = static_cast<unsigned char>(
      std::floor((raw.dt_left_voltage * (127. / 12)) + 127));
  ret[2] = static_cast<unsigned char>(
      std::floor((raw.dt_right_voltage * (127. / 12)) + 127));
  ret[1] = static_cast<unsigned char>(
      std::floor((raw.arm_voltage * (127. / 12)) + 127));
  ret[0] = 0;
  ret[0] |= raw.gripper_open << 2;
  ret[0] |= raw.roller_forward << 1;
  ret[0] |= raw.roller_backwards << 0;

  return ret;
}

std::optional<Output> decode_output(const std::array<unsigned char, 4> &raw) {
  Output ret;

  ret.dt_left_voltage = ((static_cast<float>(raw[3]) - 127) / 127 * 12);
  ret.dt_right_voltage = ((static_cast<float>(raw[2]) - 127) / 127 * 12);
  ret.arm_voltage = ((static_cast<float>(raw[1]) - 127) / 127 * 12);

  ret.gripper_open = raw[0] & (1 << 2);
  ret.roller_forward = raw[0] & (1 << 1);
  ret.roller_backwards = raw[0] & (1 << 0);

  return std::optional(ret);
}

std::array<unsigned char, 4> encode_sensors(const Sensors &sensors) {
  std::array<unsigned char, 4> ret;

  ret[3] = 0xAF;

  // this part is complicated... make an int in the right
  // range and then break it up.

  // this value should be between 0 and 1
  auto fraction_of_pi = sensors.arm_position / M_PI;
  if (!check_range(fraction_of_pi, 0, 1)) {
    throw std::runtime_error("Invalid arm position");
  }

  int stored_value = static_cast<int>(fraction_of_pi * (1 << 12));
  ret[2] = stored_value >> 4;
  ret[1] = (stored_value & 0x00F) << 4;
  ret[0] = 0;
  ret[0] |= sensors.lower_limit_on << 1;
  ret[0] |= sensors.upper_limit_on << 0;

  return ret;
}

std::optional<Sensors> decode_sensors(const std::array<unsigned char, 4> &raw) {
  Sensors ret;
  if (raw[3] != 0xAF) {
    return std::nullopt;
  }

  int unpacked_value = (raw[2] << 4) | (raw[1] >> 4);
  auto fraction_of_pi = static_cast<float>(unpacked_value) / (1 << 12);

  ret.arm_position = fraction_of_pi * M_PI;

  ret.lower_limit_on = raw[0] & 0x2;
  ret.upper_limit_on = raw[0] & 0x1;

  return ret;
}
