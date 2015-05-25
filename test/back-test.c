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

#include <sys/socket.h> // socket, recvfrom
#include <netinet/in.h> // INADDR_LOOPBACK, sockaddr_in
#include <sys/time.h> // timeval

#include "back/ipc.h"

#define expect(x) \
  do { assert(x); printf("."); fflush(stdout); } while(0);
#define run(x)    \
  do { printf("(test = %s) [ ", #x); x(); printf(" ]\n"); } while (0);
#define note(x)   \
  do { printf("(%s = %d)", #x, x); } while (0)
                                         
#define PORT 12345

#define SMALL_BYTES_COUNT (5)
static uint8_t smallBytes[SMALL_BYTES_COUNT] = { 1, 2, 3, 4, 5, };

static void socketTest(void)
{
  //uint8_t rxBuffer[256];
  int err;

  // Make sure the socket initializes correctly.
  err = ipcInit(PORT);
  expect(!err);

  // Send some bytes.
  err = ipcTransmit(smallBytes, SMALL_BYTES_COUNT);
  expect(!err);

  // Receive those bytes.
  // FIXME:
  //err = ipcReceive(rxBuffer, SMALL_BYTES_COUNT);
  //note(err);
  //expect(memcmp(rxBuffer, smallBytes, SMALL_BYTES_COUNT) == 0);

  // Close the socket.
  ipcDeinit();
}

int main(void)
{
  run(socketTest);
  return 0;
}
