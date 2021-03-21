#include <gtest/gtest.h>

#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "drivetrain.hh"

class DrivetrainTest : public ::testing::Test {
public:

  /// At the end of the setup method, you have a Drivetrain
  /// object connected to the socket.
  void SetUp() override {

    // create a socket
    listen_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;

    std::string socket_path = "thisshouldberandombut";
    // socket is an anonymous socket so we don't accidentally have name collisions
    strncpy(addr.sun_path + 1, socket_path.c_str(), sizeof(addr.sun_path) - 1);
    addr.sun_path[0] = '\0';

    int result = bind(listen_fd_, (const sockaddr *)&addr, sizeof(sockaddr_un));
    if (result == -1) {
      FAIL() << std::strerror(errno) << std::endl;
    }

    // listen to the socket
    ASSERT_NE(-1, listen(listen_fd_, 1));

    // we need to spawn a thread to call accept since block_and_connect() will
    // block on `connect` unless an accept is running at the same time.
    thread_handle_ = std::thread([&]() {
      connect_fd_ = ::accept(listen_fd_, NULL, NULL);
    });

    std::string path = socket_path;
    path.insert(0, 1, '\0');

    /// construct the Drivetrain
    drivetrain = Drivetrain{path};

    // Connect to the socket
    ASSERT_TRUE(drivetrain->block_and_connect());

    // Join the thread, i.e this means that the thread has ended
    thread_handle_.join();
  }

  void TearDown() override {
    // close both file descriptors if they're a valid number (i.e not -1)
    if (connect_fd_ != -1) {
      ASSERT_EQ(0, close(connect_fd_));
    }

    if (listen_fd_ != -1) {
      if (0 != close(listen_fd_)) {
        FAIL() << std::strerror(errno) << std::endl;
      }
    }
  }

protected:
  int listen_fd_;
  int connect_fd_;
  std::thread thread_handle_;
  std::optional<Drivetrain> drivetrain;
};

TEST_F(DrivetrainTest, BasicTest) {
  DrivetrainInput input{0.8, 0.5};
  ASSERT_EQ(sizeof(DrivetrainInput),
            write(connect_fd_, &input, sizeof(DrivetrainInput)));
  ASSERT_TRUE(drivetrain->iterate());

  DrivetrainOutput output;
  ASSERT_EQ(sizeof(DrivetrainOutput),
            read(connect_fd_, &output, sizeof(DrivetrainOutput)));
  ASSERT_NEAR(output.left_voltage, 12 * -0.3, 0.01);
  ASSERT_NEAR(output.right_voltage, 12, 0.01);
}

TEST_F(DrivetrainTest, ShouldFail) {
  int a = 6;
  write(connect_fd_, &a, 4);
  ASSERT_FALSE(drivetrain->iterate());
}

// TODO write more tests
