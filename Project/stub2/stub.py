#!/usr/bin/env python3

import sys
import socket
import struct
import time

if len(sys.argv) != 3:
    print("Usage: {} <path_to_file> <port>".format(sys.argv[0]))
    sys.exit(1)

data = []

with open(sys.argv[1], 'r') as fp:
    for line in fp.readlines():
        l=line.split(';')
        data.append([float(i) for i in l])

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
address = ('localhost', int(sys.argv[2]))

for d in data:
    package = struct.pack("<3f", d[0], d[1], d[2])
    sent=sock.sendto(package, address)
    print("Sent {} bytes".format(sent))
    time.sleep(0.005)

print("Finished")
