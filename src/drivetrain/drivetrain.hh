#include <stdexcept>
#include <unistd.h>

struct DrivetrainInput {
  float joystick_x;
  float joystick_y;
};

struct DrivetrainOutput {
  float left_voltage;
  float right_voltage;
};

class Drivetrain {
public:
  Drivetrain(std::string socket_path) : socket_path_{socket_path}, socket_fd_{-1} {};
  ~Drivetrain() {
    if (socket_fd_ != -1) {
      close(socket_fd_);
    }
  }

  // returns false on error, true on success. Blocks until connection is established.
  bool block_and_connect();

  void run();

  bool iterate();

private:
  std::string socket_path_;
  int socket_fd_;
};
