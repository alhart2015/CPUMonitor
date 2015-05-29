'''
Test for GUI IPC side.

Authors:
    Andrew Keesler
    Alden Hart

5/28/2015
'''

import ipc
import socket

################################################################################
# Unit test stuffs

def progress():
  print '.',

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

PORT = 55555

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

fake_child_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
def setup_fake_child():
  fake_child_sock.connect((ipc.LOOPBACK, PORT))
def send_from_fake_child(data):
  fake_child_sock.send(data)

def ipc_test():
  connection = ipc.ChildConnection(PORT)
  progress()

  setup_fake_child()
  progress()

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

if __name__ == '__main__':
  print ''
  print '/*'
  print '/* ipc_test.py'
  print '/*'

  run('ipc_test', ipc_test)

