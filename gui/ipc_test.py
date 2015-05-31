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
    raise
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

def sanity_test():
  '''Silly sanity test.'''
  connection = ipc.ChildConnection(ADDRESS)
  progress()
  child = ipc.Child("build/back", connection)
  progress()

  child.test("sanityTest")
  progress()

  child_stdout = child.dump()
  progress()

  child.stop()
  progress()

  # TODO: if we solve the issue of the child_file, then uncomment this.
  #expect(child_stdout, "sanityTest: ./.monitor-sock")

# keep this in sync with main.c!
ECHO_COUNT = 5
echo_words = [ "foo","bar","bat","tuna","fish", ];
def echo_test():
  '''Echo what the parent says 5 times.'''
  connection = ipc.ChildConnection(ADDRESS)
  progress()
  child = ipc.Child("build/back", connection)
  progress()

  child.test("echoTest")
  progress()

  for word in echo_words:
    # Make sure we send the size of the word first.
    child.send(("%d" % (len(word))))
    progress()
    child.send(word)
    progress()

    rx = child.receive()
    expect(rx, word)

  child.stop()
  progress()

if __name__ == '__main__':
  print ''
  print '/*'
  print '/* ipc_test.py'
  print '/*'

  run('ipc_test', ipc_test)
  run('sanity_test', sanity_test)
  run('echo_test', echo_test)
