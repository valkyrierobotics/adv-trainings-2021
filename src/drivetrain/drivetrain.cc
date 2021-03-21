#include "drivetrain.hh"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>

bool Drivetrain::block_and_connect() {
  socket_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

  if (socket_fd_ == -1) {
    std::cerr << "Error creating a socket: " << std::strerror(errno)
              << std::endl;
    return false;
  }

  sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path + 1, socket_path_.c_str() + 1, sizeof(addr.sun_path) - 1);
  addr.sun_path[0] = socket_path_.c_str()[0];

  int result =
      connect(socket_fd_, (const sockaddr *)&addr, sizeof(sockaddr_un));
  while (result == -1 && errno == ENOENT) {
    std::cout << "Trying to connect again\n";
    result = connect(socket_fd_, (const sockaddr *)&addr, sizeof(sockaddr_un));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  if (result == -1) {
    std::cerr << "Failed to connect on drivetrain socket: "
              << std::strerror(errno) << std::endl;
    return false;
  }

  return true;
}

bool Drivetrain::iterate() {
    // read from the socket
    DrivetrainInput input;
    int result = read(socket_fd_, &input, sizeof(input));
    if (result == -1) {
      std::cerr << "Failed to read on drivetrain socket: "
                << std::strerror(errno) << std::endl;
      return false;
    } else if (result == 0) {
      std::cout << "End of file on read(2)\n";
      return false;
    } else if (result != sizeof(input)) {
      std::cerr << "Didn't receive the right message size\n";
      return false;
    }

    // process that data
    DrivetrainOutput output;
    output.left_voltage = std::max(
        -12., std::min(12.0, 12.0 * (input.joystick_y - input.joystick_x)));
    output.right_voltage = std::max(
        -12., std::min(12.0, 12.0 * (input.joystick_y + input.joystick_x)));

    // write back to the socket
    result = write(socket_fd_, &output, sizeof(output));
    if (result == -1) {
      std::cerr << "Failed to write on drivetrain socket: "
                << std::strerror(errno) << std::endl;
      return false;
    } else if (result == 0) {
      std::cout << "End of file on write(2)\n";
      return false;
    } else if (result != sizeof(input)) {
      std::cerr << "Didn't write the full message\n";
      return false;
    }

    return true;
}

void Drivetrain::run() {
  if (socket_fd_ == -1) {
    std::cerr << "Drivetrain::run() called before Drivetrain::block_and_connect()\n";
    return;
  }

  while (true) {
    if (iterate() == false) { return; };
  }
}
