#ifndef __UTIL_H_
#define __UTIL_H_

#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>

// The packed format is as follows:
// Bits 31-24: Header byte (ALWAYS 0xFE)
// Bits 23-16: Joystick x byte (0 maps to -1, 255 maps to 1)
// Bits 15-8: Joystick y byte (0 maps to -1, 255 maps to 1)
// Bit  8   : Enabled
// Bit  7-3 : Reserved
// Bit 2    : Gripper open/close toggle
// Bit 1    : Roller forwards when held
// Bit 0    : Roller reversed when held
struct Joystick {
  float x, y;
  bool enabled, gripper_toggle, roller_fwd, roller_rev;
};

// Bits 31-24: Check byte 0xAF
// Bits 23-12: arm_position encoded as fractions of pi
// Bits 12-2: Reserved
// Bit  1   : arm lower limit hall
// Bit  0   : arm upper limit hall
struct Sensors {
  // from 0 to pi
  float arm_position;

  bool lower_limit_on, upper_limit_on;
};

// Bits 31-24: DT Left Voltage
// Bits 23-16: DT Right Voltage
// Bits 15-8 : Arm voltage
// Bits 7-3  : Reserved
// Bit    2  : Gripper open
// Bit    1  : Roller forwards
// Bit    0  : Roller reverse
struct Output {
  // Between -12 and 12 V
  float dt_left_voltage, dt_right_voltage;

  float arm_voltage;

  bool gripper_open, roller_forward, roller_backwards;
};

std::array<unsigned char, 4> encode_joystick(const Joystick &joystick);

std::optional<Joystick>
decode_joystick(const std::array<unsigned char, 4> &raw);

std::array<unsigned char, 4> encode_output(const Output &output);

std::optional<Output> decode_output(const std::array<unsigned char, 4> &raw);

std::array<unsigned char, 4> encode_sensors(const Sensors &output);

std::optional<Sensors> decode_sensors(const std::array<unsigned char, 4> &raw);

#endif // __UTIL_H_
