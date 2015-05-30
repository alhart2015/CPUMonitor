#!/bin/sh

#
# back-test.sh
#
# Alden Hart
# Andrew Keesler
#
# May 30, 2015
#
# Sanity testing the backend command line.
#

ME=`basename $0`

EXEC=build/back
SOCKET_PATH=./monitor-socket

# Announce the test all pretty like.
echo
echo "/*"
echo "/* $ME"
echo "/*"
echo "(test = $ME)"

# Build the command line executable.
make $EXEC > /dev/null
if [ $? != "0" ]; then
  echo "FAIL: nould not build back executable."
  exit 1
else
  echo "...built exec..."
fi

# Make sure no arguments warrants an error.
$EXEC | grep -q "usage:"
if [ $? != "0" ]; then
  echo "FAIL: no arguments does not print usage."
  exit 1
else
  echo "...no args prints usage..."
fi

# Make sure not passing the socket-path warrants an error.
$EXEC -t hey | grep -q "error:.*socket path"
if [ $? != "0" ]; then
  echo "FAIL: no socket-path does not print error."
  exit 1
else
  echo "...no socket-path prints error..."
fi

# Unrecognized options should not be accepted.
$EXEC -p $SOCKET_PATH -a | grep -q "error:.*unrecognized"
if [ $? != "0" ]; then
  echo "FAIL: unrecognized option '-a' accepted."
  exit 1
else
  echo "...unrecognized option not accepted..."
fi

$EXEC -p $SOCKET_PATH -b tuna | grep -q "error:.*unrecognized"
if [ $? != "0" ]; then
  echo "FAIL: unrecognized option '-b tuna' accepted."
  exit 1
else
  echo "...unrecognized options not accepted..."
fi

# Try to run the sanity test.
$EXEC -p $SOCKET_PATH -t sanityTest | grep -q "sanityTest"
if [ $? != "0" ]; then
  echo "FAIL: unable to run the sanityTest."
  exit 1
else
  echo "...sanityTest runs..."
fi

# Make sure an unknown test throws an error.
$EXEC -p $SOCKET_PATH -t tuna | grep -q "error: unknown test tuna"
if [ $? != "0" ]; then
  echo "FAIL: no error thrown for unknown test."
  exit 1
else
  echo "...unknown test throws error..."
fi

echo "...PASS."
exit 0