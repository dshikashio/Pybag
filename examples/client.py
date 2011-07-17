import socket


s = socket.socket()
s.connect(('127.0.0.1', 30000))

print s
