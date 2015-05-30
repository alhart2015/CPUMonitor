'''
Talking to the child process backend.

Authors:
    Andrew Keesler
    Alden Hart

5/28/2015
'''

import socket
import os

class ChildConnection:

  def __init__(self, address):
    """Initialize the child connection with a port."""
    self.address = address
    
    # Setup the socket.
    if os.path.exists(address):
      os.unlink(address)
    self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    self.sock.bind(address)
    self.sock.listen(1) # only listen for the child

    # This is the connection sock we use to actually send/receive
    # stuff from the child.
    self.childSock = None

  def accept(self):
    """Accept a connection from the child."""
    (self.childSock, meh) = self.sock.accept()

  def receive(self, count):
    """Receive count bytes of data from the connection. This will
       return None if the socket has not been connected."""
    if self.childSock is None:
      return None
    else:
      return self.childSock.recv(count)
  
  def send(self, data):
    """Send the child data. This return return None if the socket
       has not been connected."""
    if self.childSock is None:
      return None
    else:
      return self.childSock.send(data)

class Child:

  def __init__(self, image):
    """Initialize a child process to communicate with."""
    self.image = image
