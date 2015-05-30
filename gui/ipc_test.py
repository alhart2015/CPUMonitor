'''
Test for GUI IPC side.

Authors:
    Andrew Keesler
    Alden Hart

5/28/2015
'''

import ipc
import socket
import sys

################################################################################
# Unit test stuffs

def progress():
  print '.',
  sys.stdout.flush()

def expect(a, b):
  if a == b:
    progress()
  else:
    print 'X <- a(', a, ') != b(', b, ')',
    exit(1)

def run(name, func):
  print '(test = %s) [' % (name),
  try:
    func()
  except Exception as e:
    print e
    exit(1)
  print ']'

################################################################################
# Tests

ADDRESS = './.monitor-sock'

MTU = 1500

SMALL_DATA = 'tuna'
LARGE_DATA = '''
RAGHURAM RAMANUJAN
EDUCATION
M.S., Ph.D. Cornell University
B.S. Purdue University
BACKGROUND
I joined the Mathematics department at Davidson College in 2012 after
completing my PhD in Computer Science at Cornell University. I attended
Purdue University as an undergraduate, graduating with a degree in
Electrical and Computer Engineering.
I teach across the computer science curriculum at Davidson, and look
forward to supporting and expanding our course offerings in this area.
My research interests span multiple areas of Artificial Intelligence,
including automated planning, combinatorial search, and machine
learning. I supervise student research projects and theses on related topics.

In my free time, I practice with DUFF, Davidson's Ultimate Frisbee team,
and engage in a variety of outdoor pursuits including cycling, hiking
and nature photography.
'''

fake_child_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
def setup_fake_child():
  fake_child_sock.connect(ADDRESS)
def send_from_fake_child(data):
  fake_child_sock.send(data)
def receive_from_fake_child(count):
  return fake_child_sock.recv(count)

def ipc_test():
  connection = ipc.ChildConnection(ADDRESS)
  progress()

  setup_fake_child()
  progress()

  connection.accept()
  progress()

  #
  # Child to parent
  #

  send_from_fake_child(SMALL_DATA)
  progress()

  rx = connection.receive(MTU)
  progress()

  expect(rx, SMALL_DATA)

  send_from_fake_child(LARGE_DATA)
  progress()

  rx = connection.receive(MTU)
  progress()
  
  expect(rx, LARGE_DATA)

  #
  # Parent to child
  #

  connection.send(SMALL_DATA)
  progress()

  rx = receive_from_fake_child(MTU)
  progress()

  expect(rx, SMALL_DATA)

  connection.send(LARGE_DATA)
  progress()

  rx = receive_from_fake_child(MTU)
  progress()

  expect(rx, LARGE_DATA)

def child_test():
  child = ipc.Child("TODO: make a safe process to run here")
  progress()

if __name__ == '__main__':
  print ''
  print '/*'
  print '/* ipc_test.py'
  print '/*'

  run('ipc_test', ipc_test)
  run('child_test', child_test)

