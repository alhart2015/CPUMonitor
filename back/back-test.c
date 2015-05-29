//
// back-test.c
//
// Alden Hart
// Andrew Keesler
//
// May 25, 2015
//
// Unit test for the backend child process.
//

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h> // memcmp
#include <unistd.h> // close, unlink

#include <sys/socket.h> // socket, recv
#include <sys/un.h>     // sockaddr_un

#include "back/ipc.h"

#define announce() \
  do { printf("\n/*\n/* %s\n/*\n", __FILE__); } while (0);
#define expect(x)  \
  do { assert(x); printf(". "); fflush(stdout); } while(0);
#define run(x)     \
  do { printf("(test = %s) [ ", #x); x(); printf("]\n"); } while (0);
#define note(x)    \
  do { printf("(%s = %d)", #x, x); } while (0)
                                         
#define PATH "./.back-test-sock"

#define SMALL_BYTES_COUNT (5)
static uint8_t smallBytes[SMALL_BYTES_COUNT] = { 1, 2, 3, 4, 5, };

#define LARGE_BYTES_COUNT (64)
static uint8_t largeBytes[LARGE_BYTES_COUNT] = {
  1, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 3, 4, 5, 6, 7, 8,
};

// -----------------------------------------------------------------------------
// Fake parent stuff

static int fakeParentSock;

static void startFakeParent(void)
{
  struct sockaddr_un address;

  if (unlink(PATH) == -1) {
    note(errno);
    assert(0);
  }

  if ((fakeParentSock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
    note(errno);
    assert(0);
  }

  memset(&address, sizeof(address), 0);
  address.sun_family = AF_UNIX;
  strcpy(address.sun_path, PATH);
  if (bind(fakeParentSock, (struct sockaddr *)&address, sizeof(address)) == -1) {
    note(errno);
    assert(0);
  }
}

static void receiveBytes(uint8_t *bytes, uint8_t count)
{
  if (recv(fakeParentSock, bytes, count, 0) == -1) {
    note(errno);
    assert(0);
  }
}

// -----------------------------------------------------------------------------
// Tests

static void socketTest(void)
{
  uint8_t rxBuffer[256];
  int err;

  // Make sure the socket initializes correctly.
  err = ipcInit(PATH);
  expect(!err);

  // Send some bytes.
  err = ipcTransmit(smallBytes, SMALL_BYTES_COUNT);
  expect(!err);

  // Receive those bytes.
  receiveBytes(rxBuffer, SMALL_BYTES_COUNT);
  expect(memcmp(rxBuffer, smallBytes, SMALL_BYTES_COUNT) == 0);

  // Send some bytes.
  err = ipcTransmit(largeBytes, LARGE_BYTES_COUNT);
  expect(!err);

  // Receive those bytes.
  receiveBytes(rxBuffer, LARGE_BYTES_COUNT);
  expect(memcmp(rxBuffer, largeBytes, LARGE_BYTES_COUNT) == 0);

  // Close the socket.
  ipcDeinit();
}

int main(void)
{
  announce();

  startFakeParent();

  run(socketTest);

  close(fakeParentSock);

  return 0;
}
