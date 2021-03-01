#include "manager.hh"

int main() {
  Manager manager("./sensors", "./output", "./joystick");
  manager.block_and_connect();
}
