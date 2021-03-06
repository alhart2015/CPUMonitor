'''
Talking to the child process backend.

Authors:
    Andrew Keesler
    Alden Hart

5/28/2015
'''

import socket
import os
import subprocess

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

  # TODO: better way to set this?
  RECEIVE_SIZE = 256
  CHILD_FILE_NAME = "./.child-file"

  def __init__(self, image, connection):
    """Initialize a child process to communicate with."""
    self.image = image
    self.connection = connection

    self.child_file = None

  def start(self, args):
    """Start the child process, passing the args. If args is None,
       then no additional arguments will be passed."""
    command = [self.image]
    command.append("-p")
    command.append(self.connection.address)
    if args is not None:
      for arg in args.split(" "):
        command.append(arg)

    # We should start with a clean file here.
    if os.path.exists(self.CHILD_FILE_NAME):
      os.unlink(self.CHILD_FILE_NAME)
    os.system("touch " + self.CHILD_FILE_NAME)
    self.child_file = open(self.CHILD_FILE_NAME, "r+")
    subprocess.Popen(command, stdout=self.child_file)

    # Accept the child process connection when it comes a callin.
    # We don't do this for the sanity test.
    if args != "-t sanityTest":
      self.connection.accept()

  def stop(self):
    """Stop the child process, and close the child file."""
    if self.child_file is not None:
      self.child_file.close()

  def dump(self):
    """Dump the contents of the child file. If the
       child file has not been created, this will return None. To
       create the child file, call start()."""
    # FIXME: we can't read the file? It has something to do with the creation
    # of the file?
    data = None
    if self.child_file is not None:
      self.child_file.flush()
      data = self.child_file.read()
    return data

  def test(self, name):
    """Run a child test called 'name'."""
    self.start("-t " + name)

  def send(self, data):
    """Send some data to the child."""
    self.connection.send(data)

  def receive(self):
    """Receive some data from the child."""
    self.connection.receive(self.RECEIVE_SIZE)
