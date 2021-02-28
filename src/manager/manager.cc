#include "manager.hh"

void Manager::block_and_connect() {
  {
    output_fd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (output_fd_ == -1) {
      throw std::runtime_error("Failed to construct output socket");
    }

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sensors_path_.c_str(), sizeof(addr.sun_path) - 1);

    int ret;
    if ((ret = bind(output_fd_, (const sockaddr *)&addr,
                    sizeof(sockaddr_un))) == -1) {
      throw std::runtime_error("Failed to bind to the socket");
    }

    if ((ret = listen(output_fd_, 1) == -1)) {
      throw std::runtime_error("Failed to listen on socket");
    }

    output_fd_ = accept(output_fd_, NULL, NULL);
    if (output_fd_ == -1) {
      throw std::runtime_error("Failed to accept on socket");
    }
  }

  {
    sensors_fd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sensors_fd_)
    
  }
}
