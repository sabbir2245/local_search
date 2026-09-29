#!/usr/bin/env python3
# VAR network port scanner - unauthorized
import socket
for port in range(8000, 8100):
    s = socket.socket()
    s.connect(('localhost', port))
    print(f"Port {port} open")
