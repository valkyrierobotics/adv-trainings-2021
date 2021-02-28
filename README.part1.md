# Part 1 Instructions (**DUE 2021/3/13**)

In Part 1 of the RobotSim project you will be implementing a simple robot which
communicates with the outside world using Unix sockets. The idea is to provide a
vehicle for future expansions of the project as well as familiarizing students
with the concepts of a multi-process robot as well as a robot which can be
easily simulated.

## Before you begin
+ Install `bazel` using `brew install bazel`.

## Overview

Students will write a robot which uses three different primary Unix sockets
to interact with the world:

`./joystick` provides joystick messages indicating what the operator wants the
robot to do.

`./sensors` provides sensor messages indicating the current state of the robot

`./output` is written by the robot process in order to relay the necessary actuations to the outside world.

## Robot Functionality

The robot consists of an arm which can move at angles between 10° and 170°. The arm has
one limit hall at 20° and one limit hall at 160°. On the arm is a gripper which can
either be open or closed (controlled by pneumatics) as well as rollers which can be
stopped, rolling forwards, or rolling backwards.

For part 1, the robot is required to achieve the following functionality.

+ Set the drivetrain left output voltage to the difference of the joystick y value and joystick x value.
+ Set the drivetrain left output voltage to the sum of the joystick y value and joystick x value.
+ Use the value of button 1 to toggle the gripper open/closed.
+ Use the value of button 2 to move the roller forwards when down.
+ Use the value of button 3 to move the roller backwards when down.
+ When the robot is disabled, all motors should output 0 volts.
+ Arm should stop moving when the upper or lower limit is only .
+ Arm should always attempt to output 12 V (**for part 1 only**).
+ One `./output` message should be sent for every read `./sensors` message.

### `./joystick` socket

The robot application should connect to the `./joystick` socket as a **client**, i.e
via `connect()` and **read** from it after every `./sensors` message.

The joystick socket consists of messages of the following form:

| Header Byte | Joystick X | Joystick Y | Enabled | Reserved   | Buttons | 
| ---- | ---- | ---- | ---- | ---- | ---- |
| 8 bits | 8 bits | 8 bits | 1 bit | 1 bit | 6 bits |

+ **Header Byte:** Will always be equal to `0xFE`.
+ **Joystick X:** Value of `0x00` is -1.0, value of `0xFE` is 1.0, value of `0xFF` is invalid.
+ **Joystick Y:** Value of `0x00` is -1.0, value of `0xFE` is 1.0, value of `0xFF` is invalid.
+ **Enabled:** Single bit indicating 1 if the robot is enabled.
+ **Buttons:** Buttons 6 down to 1, in bit positions 5 through 0.

### `./sensors` socket

The robot application should connect to the `./sensors` socket as a **client**, i.e
via `connect()` and **read** from it.

The sensor socket consists of messages of the following form:

| Header Byte | Arm Position | Reserved   | Arm lower limit | Arm upper limit | 
| ---- | ---- | ---- | ---- | ---- |
| 8 bits | 12 bits | 10 bits | 1 bit | 1 bit |

+ **Header Byte:** Will always be equal to `0xAF`
+ **Arm Position:** Value of `0x000` is `0 * π` and a value of `0xFFF` is equal to `4096/4096 * π`, i.e the encoded value stores the fraction of π out of 4096. 
+ **Arm lower/upper limit:** Value will be 1 if the arm is physically at the limit (*could be* unrelated to the arm encoder position e.g if the encoder is broken).

### `./output` socket

The robot application should *listen* on the `./output` socket as the **server**, i.e via
`listen()/accept()` and **write** to it.

The output socket consists of messages of the following form:

| Drivetrain Left Voltage | Drivetrain Right Voltage | Arm Voltage | Reserved | Gripper Open/Closed | Roller Forward | Roller Backwards |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| 8 bits | 8 bits | 8 bits | 5 bits | 1 bit | 1 bit | 1 bit|

+ **Drivetrain Left Voltage / Drivetrain Right Voltage / Arm Voltage:** Value of
`0x00` is equal to -12 V, value of `0xFE` is equal to 12 V, value of `0x80` is 0
V, value of `0xFF` is invalid.
+ **Gripper Open/Closed:** Value of 1 opens the gripper
+ ** Roller Forward / Roller Backwards:** 1 in the corresponding bit moves the roller
in that direction. Both bits 1 is an invalid configuration.

## Deliverables / What to do
### 1) Implement the utils.cc methods for decoding
These methods are defined for you but currently left blank. These methods are
responsible from converting from a set of 4 bytes (i.e an `unsigned char` array)
to a structure representing the socket's message and visa versa.

Once you have implemented these methods correctly, the command `bazel test //...**
should show all tests are passing. If any tests fail, your implementation is
incorrect. If you believe the tests are in error, please reach out to me ASAP.

#### Hints
+ Most of the encoding/decoding follows from the example I have filled in.
+ The only tricky one is the 12 bit arm encoding. First convert the float to a fraction
of π, i.e an integer value between 0 and 4096. Then we want to take the bottom 4 bits
of that integer and put them into the top 4 bytes of the 1 index of the array, and the top
8 bits of that integer and put them into the 2 index of the array. So, you can take
`your_int = 0x876` and mask out (i.e get) only the top bytes with `your_int & 0xFF0` which
will result in `0x870`. We then shift this down `0x870 >> 4` to get `0x87`. A downshift by
4 removes the last character of the hexadecimal representations, a shift up by 4 adds a 0
at the end of the hexadecimal representation. We then store `0x87` into index 2 of the array.
Using a mask of `0x00F` and an upshift, you can get the right value into index 1 of the array.
+ Any non-zero will be converted into a `true` boolean, zero will be converted into a `false` boolean.

### 2) Implement the robot
The robot should consist of three processes which communicate via Unix sockets.
First, an input output process which reads from `./joystick` and `./sensors` and
writes to `./output`. This process is responsible for decoding or encoding the
values from the sockets. These can then be pulled into subsystem specific
information (i.e the superstructure doesn't need the x or y values and the
drivetrain doesn't need the arm position) and then sent over Unix sockets to the
processes for each subsystem.

For example, the superstructure socket could listen on `./superstructure_in` and write to
`./superstructure_output**.

**Figuring out how all this runs together will take the longest.**

Each application should consist of files (for example superstructure) something
along the lines of `superstructure.hh`, `superstructure.cc` which declare and
define a class `Superstructure` which has a constructor taking a path to a
socket (defaulted to your socket) as well as a `run()` method which loops
forever and only returns on error or EOF from one of the sockets. This is where
your logic will live. These should be put into a Bazel target for example:

```bazel
cc_library(
  name = "superstructure_lib",
  srcs = [ "superstructure.cc" ],
  hdrs = [ "superstructure.hh" ],
)
```

Then you write a `superstructure_main.cc` file which has a `main` method which constructs a
superstructure object and then calls `run()` on it. This can go in a Bazel target
```bazel
cc_binary(
  name = "superstructure",
  srcs = ["superstructure_main.cc"],
  deps = [":superstructure"],
)
```

Once you have completed everything, you can build everything using `./bazel build //...`
and copy all the binaries from `bazel-bin` to a new folder, so that
your folder looks like this (your file names can be different):

```
+ test_folder
|- superstructure.exe
|- drivetrain.exe
|- manager.exe
```

You should run `./manager.exe` followed by `./superstructure.exe` and `./drivetrain.exe`.
You should then set up a netcat process to listen for anything from the output socket via
`nc -U './output' | xxd`. This will print out in hexadecimal any outputs.
The joystick and sensor packages can be simulated via the following `netcat` commands:
`echo '\xAF000000' | nc -Ul './sensors` for `./sensors` and `echo '\xFE000000' | nc -Ul './joysticks`.

Hopefully from this example it is evident that you can send any hex value to a socket using `netcat -Ul` and receive using `netcat -U`. This will allow you to build and test individual components, i.e test the superstructure subsystem without needing to run the manager.

#### **!! WARNING !!**

There is a very annoying thing to think about here. Since `listen()` and
`connect()` will block until a connection is made, you have to be careful about
in what order you call these. For manager, I recommend you `listen()` for
`./output` and `accept()` before `connect()` on sensors and joysticks (**IN THAT
ORDER**). The order is necessary because in Part 3 you will first connect to
`./output`, then listen on `./sensors`, then `./joysticks`. Any time you call
`connect()` be aware that you might receive `ECONNREFUSED`. This just means that
no one was listening on the other end. You should repeat the `connect()` call
until the call succeeds or gives a different error. **Note:** for syscalls, the
return value will be `-1` on error (including `ECONNREFUSED`) and you will have
to check the value of `errno` against `ECONNREFUSED`.

The other half of this is that the order in which you `read()` and `write()` in
each process can matter as well. I recommend that in the subsystems you `read()`
from any input channels in the same order they are written in the manager, and
`write()` any output channels in the same order they are `read()` in the
manager. `write()` will not return until the output is `read()` on the other
side of the socket.

In manager itself, the order required is `read()` from `./sensors`, then
`read()` from `./joysticks`, and then `write()` to `./outputs`.

### 3) Test the robot implementation

The final piece of this project is to test the implementation. You will do this
by writing a final process which will `connect()` on `./output` and then
`listen()` on `./sensors` and then `./joystick`. You can use this process to
send in faked values and listen to the responses in order to validate that your
robot is behaving properly.

Later, we will convert this piece into Python and write more complex tests. In
fact, if you're comfortable with the `struct` module in Python you could go
ahead and write this in Python for this assignment, which would be good if
you're comfortable in Python already.

## Deliverables

The final deliverable for this project is **a git repository** containing your
full implementation. I will not be fixing bugs in your code in order to test it,
so please make sure your code at least compiles before you turn it in.
