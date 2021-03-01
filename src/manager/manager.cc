#include "manager.hh"
#include <sys/socket.h>

#include <iostream>
#include <thread>

void Manager::block_and_connect() {
  {
    output_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (output_fd_ == -1) {
      throw std::runtime_error("Failed to construct output socket");
    }

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, output_path_.c_str(), sizeof(addr.sun_path) - 1);

    int ret;
    if ((ret = bind(output_fd_, (const sockaddr *)&addr,
                    sizeof(sockaddr_un))) == -1) {
      throw std::runtime_error("Failed to bind to the socket");
    }

    if ((ret = listen(output_fd_, 1) == -1)) {
      std::cout << std::strerror(errno) << std::endl;
      throw std::runtime_error("Failed to listen on socket");
    }

    output_fd_ = accept(output_fd_, NULL, NULL);
    if (output_fd_ == -1) {
      throw std::runtime_error("Failed to accept on socket");
    }
  }

  {
    sensors_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sensors_fd_ == -1) {
      throw std::runtime_error("Failed to construct sensors socket");
    }

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sensors_path_.c_str(), sizeof(addr.sun_path) - 1);

    int sensors_fd = connect(sensors_fd_, (const sockaddr *) &addr, sizeof(sockaddr_un));
    while (sensors_fd == -1 && errno == ENOENT) {
      sensors_fd =
          connect(sensors_fd_, (const sockaddr *)&addr, sizeof(sockaddr_un));
      std::cout << "Trying to connect again" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (sensors_fd == -1) {
      std::cout << std::strerror(errno) << std::endl;
      throw std::runtime_error("Failed to connect on sensors socket");
    } else {
      sensors_fd_ = sensors_fd;
    }
  }
}
