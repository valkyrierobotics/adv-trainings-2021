# How to use sockets (for this project)

Since I brushed over it in the training, I figured I should probably go into a
bit more detail about how you will need to use sockets in order to complete this
project.

Also, this supercedes any of the "how to test your code" information in the
[README.part1.md](README.part1.md) until I go and update that file.

## What is a socket?

A socket is a standard piece of Unix design which acts as an endpoint for bidirectional communication. This means that processes with the socket can communicate with one another in a standard and sane way. Sockets are used to implement many different pieces of process communication on Unix systems, but most commonly show up in networking, acting as the base layer of abstraction for TCP and UDP networking.

One constructs a socket using the syscall `socket(2)`. This syscall takes three arguments, two of which are actually used commonly. The syscall looks like this:

```c
int socket(int domain, int type, int protocol)
```

The `domain` argument selects the protocol family which will be used for
communication. This includes options like `AF_INET` for IP networking,
`AF_BLUETOOTH` for low-level bluetooth communication, and the one we will be
using, `AF_UNIX` or `AF_LOCAL` which allows processes on the same machine to
communicate via a socket.

The `type` argument specifies on a conceptual level how the socket
communications (the communication semantics). There are four types readily in
use:
+ `SOCK_STREAM` which provides sequenced (in-order) connection-based byte
  streams. This means that the basic unit of communication is the byte, and a
  connection must be established before the socket can be used.
+ `SOCK_DGRAM` which provides connectionless, unreliable messages which come in
  chunks (packets). The basic unit is the packet, and no connection is needed to
  communicate on these sockets.
+ `SOCK_SEQPACKET` is basically a combination of `SOCK_DGRAM` and `SOCK_STREAM`
  in that it supports sequenced, reliable connection-based data transmission but
  for packets. The "gotcha" is that each input call must read a whole packet.
+ `SOCK_RAW`

For this project, we will be using `SOCK_STREAM` due to its connection-based
nature. Ideally we would use `SOCK_SEQPACKET` but it isn't supported on Mac OS X
for `AF_UNIX` domain sockets.

Applications use sockets by writing to them and reading from them.

## How do I use (connection-based) sockets

This will not cover how to use `SOCK_DGRAM` sockets.

For each socket, there is a server-side which announces the availability of the
socket (i.e "I'm here! Connect to me!") and a client-side which connects to the
listening socket.

### File Descriptors
File descriptors are a tiny piece of shared state between the user and the kernel in Unix-based operating systems. A file descriptor is just an integer which the kernel can tie to a complex structure which one can operate on.

### Error handling note
System calls typically return 0 or an integer value when they succeed, and -1 when they fail. In order to figure out the specific error that occured when you see a -1, you need to check the value of `errno`, which is a global variable holding the most recent error. This can be turned into a string via `strerror(errno)` in C or `std::strerror(errno)` in C++. I will discuss a few key `errno` values throughout this. The full list of errors (and an explanation) that each syscall returns can be found via `man 2 <syscall>`, e.g `man 2 socket`.

### Server Side
On the server side, our goal is to create a socket, announce that it can be connected to, and then handle any connections.

So, the first step is to ask the kernel to make a socket and hand us back a file descriptor which can be used to refer to it.

**Note:** I'm eliding all error handling. Anything that returns could return -1 which would require error handling.

Constructing a socket requires the following call to the `socket(2)` syscall:

```c
int socket = socket(AF_UNIX, SOCK_STREAM, 0)
```

Assuming this doesn't return an error, we now have an integer which refers to a socket in the kernel side.

The next piece is to announce that there is a server that can be connected to. This is done via the `bind` syscall which adds a name to the socket. The bind syscall looks like this:
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
```

The bind syscall takes a file descriptor of a socket as one could figure, and then a pointer to a `sockaddr` structure as well as the length of that structure. The length is required because (as you'll see) there are `sockaddr` structures of different types for each family and the kernel needs to know how big the one you are passing it is.

We will build a `sockaddr_un` structure for our `AF_UNIX` address. This structure looks like:
```c
struct sockaddr_un {
    sa_family_t sun_family;
    char sun_path[108];
}
```

The `sun_family` is always `AF_UNIX`, and the `sun_path` is the name we want to give the socket. In C++, this means retrieving the underlying C string from a `std::string` and copying it into the structure as follows:

```cpp
std::string socket_name = "mysocket";
sockaddr_un addr;
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, socket_name.c_str(), sizeof(addr.sun_path));
```

Binding this address to a socket will create a file on your system called "mysocket".

Unix sockets can also be given an "abstract" address, which does not make a file. This would be done as follows:
```cpp
std::string socket_name = "mysocket";
sockaddr_un addr;
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path + 1, socket_name.c_str(), sizeof(addr.sun_path) - 1);
addr.sun_path[0] = '\0';
```
The difference is that in the second case the first character of the `sun_path` is set to 0.

For this project I recommmend you *do not* use abstract namespace sockets until a later stage.

Once you've made an address, it is easy to call bind:
```c
bind(socket, (const sockaddr *) &addr, sizeof(sockaddr_un))
```

`bind(2)` can return a few errors which we might find interesting. I recommend that you watch out for `EADDRINUSE` which will just mean that the file already exists, in which case you could delete it via `unlink(2)`.

After we've bound the socket, other processes can see it and try to connect, so we need to notify the kernel that we're ready to handle those connections. We do this via a call to `listen(2)`, which looks like:

```c
int listen(int sockfd, int backlog);
```

As you might expect, this takes a socket file descriptor which we have bound. It also takes an integer value referring to how many connections it should let build up with us not handling them. If a process tries to connect when the queue is full, the process trying to connect could get a `ECONNREFUSED`. We should probably set this to 1 for this project and ignore it.

Finally, we can call `accept(2)` which will hang until a process attempts to connect to our socket, at which point it will return a *new* file descriptor we can use to communicate. This call looks like

```c
accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
```

What's cool about this call is that it returns the address of the process which connected to you, which could be useful if we were doing Internet networking. We aren't, so you can just set both pointers to `NULL` and ignore them, like:

```c
int conn_socket = accept(socket, NULL, NULL);
```

Now we have a socket file descriptor which can be used for reading and writing!

### Client Side

The client side is much simpler. All you do is make a socket, just like the server. Domain, type, and protocol all have to match or you will get an error, so

```c
int conn_socket = socket(AF_UNIX, SOCK_STREAM, 0);
```

Then, construct a `sockaddr_un` same as the server side, and finally call
`connect(2)` like:

```c
int connect(conn_socket, (const sockaddr *) &addr, sizeof(sockaddr_un))
```

At this point, your `conn_socket` is connected and ready for reading/writing.

`connect(2)` returns two errors we might find interesting:
+ `ECONNREFUSED` means the socket exists but no one is listening. This shouldn't happen in your code, and probably means that the server side died and didn't delete the socket file.
+ `ENOENT` means that the socket you're trying to doesn't exist, and you should probably loop on your call to `connect(2)` and wait until `connect(2)` succeeds or you get a different error.

### Using your connected socket
We will be using the syscalls `write(2)` and `read(2)` in order to use
the sockets.

Let's start by discussing `read(2)`. It looks like this:
```c
ssize_t read(int fd, void *buf, size_t count);
```

So, it takes a file descriptor (duh) as well as a buffer to read into and a count of the number of bytes to read. This could look like

```cpp
std::array<unsigned char, 4> data;

int amount = read(conn_socket, data.data(), 4);
```

This snippet tries to read 4 bytes and returns the number of bytes it read (or -1 for error) into amount, and then stores the bytes it read into the underlying data array for the C++ `std::array` `data`.

Basically, since you're (hopefully) only ever writing 4 bytes at a time, your code should never read anything other than 4 bytes (probably check for this and throw an error). If it reads 0 bytes, the socket has closed on the other end and you should probably exit.

When you call `read` on a socket without any data to read, your call will wait
for data to arrive. An idiom then is something like:

```cpp
int ret = 4;
std::array<unsigned char, 4> data;
while (ret == 4) {
  ret = read(conn_socket, data.data(), 4);
  process_the_data(data);
}

if (ret == -1) {
  std::cerr << "Oh no we got a read error! " << std::strerror(errno) << std::endl;
} else if (ret != 0) {
  std::cerr << "Somehow we read " << ret << " bytes instead of 4" << std::endl;
}

close(conn_socket);
```

On the write side, we will be using the `write(2)` syscall, which looks like:
```c
ssize_t write(int fd, const void *buf, size_t count);
```

aka exactly the same as the `read(2)` side. It will always return the
number of bytes it wrote. This could be less than `count` for reasons
like a full disk or an interruption, but you should probably just
have that fail. In a real robot system, the solution might be to
ignore malformed reads, and resend malformed writes.

One could do a write like:
```c
// hint: what returns std::array<unsigned char, 4> in our code?
std::array<unsigned char, 4> data = get_some_data();

int ret = write(conn_socket, data.data(), 4);
if (ret == -1) {
  std::cerr << "Oh no we got a write error! " << std::strerror(errno) << std::endl;
} else if (ret != 4) {
  std::cerr << "Somehow we didn't complete our write." << std::endl;
}
```

### Closing your socket
It is good practice to close your socket at the end using `close(2)`.
File descriptors will be cleaned up once everyone with a reference
to them has called `close(2)` on them or died.

## How to test your robot
I have provided a helpful Python for you in `src/util/test_hook.py`
which can both listen to and connect to a socket. It follows my
mental model of this by (on the listen side) creating a socket
and waiting for connections. When it receives one, it will read
and print what it receives, and then let you enter a hex value
to send back. You can also just press enter to skip the send and
just read again.

On the connect side, it connects to the socket you specify, and
then lets you enter a hex value to write. It will then print
anything it reads. As before, you can just press enter to skip
the write if that's how your program works.
