#include "src/util/util.hh"

#include <cstring>
#include <stdexcept>

#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

class Manager {
public:
  Manager(std::string sensors_path, std::string output_path, std::string joystick_path)
    : sensors_path_{sensors_path},
      output_path_{output_path},
      joystick_path_{joystick_path} {}

  void block_and_connect();

  ~Manager() {
    if (sensors_fd_ != -1) {
      close(sensors_fd_);
    }
    if (joystick_fd_ != -1) {
      close(joystick_fd_);
    }
    if (output_fd_ != -1) {
      close(output_fd_);
    }
  }
private:
  int sensors_fd_ = -1;
  std::string sensors_path_;
  int output_fd_ = -1;
  std::string output_path_;
  int joystick_fd_ = -1;
  std::string joystick_path_;
};
