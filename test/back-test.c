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
#include <unistd.h> // close

#include <sys/socket.h> // socket, recvfrom
#include <netinet/in.h> // INADDR_LOOPBACK, sockaddr_in
#include <sys/time.h> // timeval

#include "back/ipc.h"

#define announce() \
  do { printf("\n/*\n/* %s\n/*\n", __FILE__); } while (0);
#define expect(x)  \
  do { assert(x); printf("."); fflush(stdout); } while(0);
#define run(x)     \
  do { printf("(test = %s) [ ", #x); x(); printf(" ]\n"); } while (0);
#define note(x)    \
  do { printf("(%s = %d)", #x, x); } while (0)
                                         
#define PORT 55555

#define SMALL_BYTES_COUNT (5)
static uint8_t smallBytes[SMALL_BYTES_COUNT] = { 1, 2, 3, 4, 5, };

// -----------------------------------------------------------------------------
// Fake parent stuff

static int fakeParentSock;

static void startFakeParent(void)
{
  struct sockaddr_in loopbackAddress;

  if ((fakeParentSock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    note(errno);
    assert(0);
  }

  loopbackAddress.sin_family = AF_INET;
  loopbackAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  loopbackAddress.sin_port = htons(PORT);
  if (bind(fakeParentSock,
           (struct sockaddr *)&loopbackAddress,
           sizeof(loopbackAddress))
      == -1) {
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
  err = ipcInit(PORT);
  expect(!err);

  // Send some bytes.
  err = ipcTransmit(smallBytes, SMALL_BYTES_COUNT);
  expect(!err);

  // Receive those bytes.
  receiveBytes(rxBuffer, SMALL_BYTES_COUNT);
  expect(memcmp(rxBuffer, smallBytes, SMALL_BYTES_COUNT) == 0);

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
