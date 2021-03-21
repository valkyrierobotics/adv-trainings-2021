#include "util.hh"

#include "gtest/gtest.h"

TEST(JoystickTests, RoundtripJoystick) {
  Joystick joystick;
  joystick.x = 1.0;
  joystick.y = -1.0;
  joystick.enabled = true;
  joystick.gripper_toggle = false;
  joystick.roller_fwd = false;
  joystick.roller_rev = true;

  auto ret = encode_joystick(joystick);
  unsigned int value = *((unsigned int *)ret.data());

  EXPECT_EQ(value, 0xFEFE0081);

  auto decoded_joystick = decode_joystick(ret);
  ASSERT_TRUE(decoded_joystick.has_value());

  EXPECT_TRUE(decoded_joystick->enabled);
  EXPECT_TRUE(decoded_joystick->roller_rev);
  EXPECT_FALSE(decoded_joystick->roller_fwd);
  EXPECT_FALSE(decoded_joystick->gripper_toggle);
  EXPECT_TRUE(std::abs(decoded_joystick->x - 1.0) < 1e-4);
  EXPECT_TRUE(std::abs(decoded_joystick->y + 1.0) < 1e-4);
}

TEST(OutputTests, RoundtripOutput) {
  Output output;
  output.dt_left_voltage = 12.0;
  output.dt_right_voltage = 0.0;
  output.arm_voltage = -12.0;
  output.gripper_open = true;
  output.roller_forward = false;
  output.roller_backwards = true;

  auto ret = encode_output(output);
  unsigned int value = *((unsigned int *)ret.data());

  EXPECT_EQ(value, 0xFE7F0005);

  auto decoded_output = decode_output(ret);

  ASSERT_TRUE(decoded_output.has_value());
  EXPECT_TRUE(std::abs(decoded_output->dt_left_voltage - 12.) < 1e-4);
  EXPECT_TRUE(std::abs(decoded_output->dt_right_voltage) < 1e-4);
  EXPECT_TRUE(std::abs(decoded_output->arm_voltage + 12.0) < 1e-4);
  EXPECT_TRUE(decoded_output->gripper_open);
  EXPECT_FALSE(decoded_output->roller_forward);
  EXPECT_TRUE(decoded_output->roller_backwards);
}

TEST(SensorTests, RoundtripSensor) {
  Sensors sensors;
  sensors.arm_position = .5 * M_PI;
  sensors.lower_limit_on = true;
  sensors.upper_limit_on = false;

  auto ret = encode_sensors(sensors);
  unsigned int value = *((unsigned int *)ret.data());

  EXPECT_EQ(value, 0xAF800002);

  auto decoded_sensor = decode_sensors(ret);

  ASSERT_TRUE(decoded_sensor.has_value());
  EXPECT_TRUE(decoded_sensor->lower_limit_on);
  EXPECT_FALSE(decoded_sensor->upper_limit_on);
  EXPECT_TRUE(std::abs(decoded_sensor->arm_position - 0.5 * M_PI) < 1e-4);
}
