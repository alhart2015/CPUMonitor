'''
Talking to the child process backend.

Authors:
    Andrew Keesler
    Alden Hart

5/28/2015
'''

import socket

LOOPBACK = '127.0.0.1'

MTU = 1500 # Eh? This is a total guess.

class ChildConnection:

  def __init__(self, port):
    """Initialize the child connection with a port."""
    self.port = port
    
    # Setup the socket.
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.sock.bind((LOOPBACK, self.port))
  
  def receive(self):
    return self.sock.recv(MTU)
