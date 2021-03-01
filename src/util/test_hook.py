import argparse
import errno
import os
import socket
import sys

class PathCleaner(object):
    def __init__(self, path):
        self.path = path

    def __enter__(self):
        pass
    
    def __exit__(self, *kwargs):
        print(exit, self.path)
        if os.path.exists(self.path):
            os.unlink(self.path)

def listen(args):
    listen_socket = socket.socket(family=socket.AF_UNIX, type=socket.SOCK_STREAM)
    with PathCleaner(args.path):
        try:
            listen_socket.bind(args.path)
        except OSError as e:
            if e.errno == errno.EADDRINUSE:
                os.unlink(args.path)
            listen_socket.bind(args.path)
        listen_socket.listen()
        conn_socket, _ = listen_socket.accept()
        print("Write:")
        for line in sys.stdin:
            input_bytes = bytes.fromhex(line)
            if len(input_bytes) == 0:
                print("Skipping...")
                conn_socket.send(b'\xff')
            else:
                conn_socket.send(input_bytes)
            data = conn_socket.recv(4)
            if (len(data) == 1 and data == b'\xff'):
                pass
            else:
                print("Read {} bytes: {}".format(len(data), data.hex()))
            print("Write:")


def connect(args):
    conn_socket = socket.socket(family=socket.AF_UNIX, type=socket.SOCK_STREAM)
    conn_socket.connect(args.path)
    data = [1]
    while len(data) != 0:
        data = conn_socket.recv(4)
        if (len(data) == 1 and data == b'\xff'):
            pass
        else:
            print("Read {} bytes: {}".format(len(data), data.hex()))
        print("Write:")
        line = sys.stdin.readline()
        if len(line) == 1 and line == "\n":
            print("Skipping...")
            conn_socket.send(b'\xff')
        else:
            conn_socket.send(bytes.fromhex(line))


def build_parser(parser):
    subparsers = parser.add_subparsers()
    listen_args = subparsers.add_parser('listen')
    listen_args.add_argument('path')
    listen_args.set_defaults(func=listen)
    connect_args = subparsers.add_parser('connect')
    connect_args.add_argument('path')
    connect_args.set_defaults(func=connect)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Simple test program for Unix sockets')
    build_parser(parser)
    args = parser.parse_args(sys.argv[1:])
    args.func(args)
