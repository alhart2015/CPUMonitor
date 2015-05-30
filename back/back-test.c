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
#include <string.h>  // memcmp
#include <unistd.h>  // close, unlink, access
#include <pthread.h> // pthread

#include <sys/socket.h> // socket, recv
#include <sys/un.h>     // sockaddr_un
#include <sys/time.h>   // timeval

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
static uint8_t smallBytes[SMALL_BYTES_COUNT] = { 5, 4, 3, 2, 1, };

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

#define printBuffer(buffer, len)                \
  do {                                          \
    uint8_t i_XXX = 0;                          \
    printf("( ");                               \
    while (i_XXX < len) {                       \
      printf("0x%02X ", buffer[i_XXX++]);       \
    }                                           \
    printf(")");                                \
    fflush(stdout);                             \
  } while(0);

// -----------------------------------------------------------------------------
// Fake parent stuff

static int fakeParentSock;
struct sockaddr_un address;

static pthread_t parentThread;
static uint8_t rxBuffer[64], txBuffer[64], bufferCount;
static uint8_t parentMask = 0;

#define MASK_SPIN (1 << 0)
#define MASK_RX   (1 << 1)
#define MASK_TX   (1 << 2)

#define MASK_READY (1 << 3)
#define MASK_DONE  (1 << 4)

static void *fakeParentTask(void *sockData)
{
  int anotherSock;
  socklen_t sizeOfAddress = sizeof(address);

  if ((anotherSock = accept(fakeParentSock,
                            (struct sockaddr *)&address,
                            &sizeOfAddress))
      == -1) {
    note(errno);
    assert(0);
  }

  parentMask |= MASK_READY;
  while (parentMask & MASK_SPIN) {
    if (parentMask & MASK_RX) {
      if (recv(anotherSock, rxBuffer, bufferCount, 0) == -1) {
        note(errno);
        assert(0);
      }
      parentMask &= ~MASK_RX;
      parentMask |= MASK_DONE;
    } else if (parentMask & MASK_TX) {
      if (send(anotherSock, txBuffer, bufferCount, 0) == -1) {
        note(errno);
        assert(0);
      }
      parentMask &= ~MASK_TX;
      parentMask |= MASK_DONE;
    }
  }

  close(anotherSock);

  pthread_exit(NULL);
}

static void startFakeParent(void)
{
  int err;
  struct timeval timeout;

  if (access(PATH, F_OK) != -1 && unlink(PATH) == -1) {
    note(errno);
    assert(0);
  }

  if ((fakeParentSock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    note(errno);
    assert(0);
  }

  timeout.tv_sec  = 1; // 1 second
  timeout.tv_usec = 0; // 0 microseconds
  if ((setsockopt(fakeParentSock,
                 SOL_SOCKET,
                 SO_SNDTIMEO,
                 &timeout,
                 sizeof(timeout))
       == -1)
      || (setsockopt(fakeParentSock,
                     SOL_SOCKET,
                     SO_RCVTIMEO,
                     &timeout,
                     sizeof(timeout))
          == -1)) {
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

  if (listen(fakeParentSock, 1) == -1) { // listen for 1 child connection
    note(errno);
    assert(0);
  }

  parentMask = MASK_SPIN;
  if ((err = pthread_create(&parentThread,
                            NULL,
                            fakeParentTask,
                            NULL))
      != 0) {
    note(err);
    assert(0);
  }
}

static void stopFakeParent(void)
{
  parentMask &= ~MASK_SPIN;
  pthread_join(parentThread, NULL);
  close(fakeParentSock);
}

#define parentSend(buffer, count)                       \
  do {                                                  \
    memcpy(txBuffer, buffer, count);                    \
    bufferCount = count;                                \
    parentMask &= ~MASK_DONE;                           \
    parentMask |= MASK_TX;                              \
    while (!(parentMask & MASK_DONE)) /* wait */ ;      \
  } while (0);

#define parentReceive(count)                            \
  do {                                                  \
    bufferCount = count;                                \
    parentMask &= ~MASK_DONE;                           \
    parentMask |= MASK_RX;                              \
    while(!(parentMask & MASK_DONE)) /* wait */ ;       \
  } while (0);

// -----------------------------------------------------------------------------
// Tests

static void socketTest(void)
{
  int err;

  // Make sure the socket initializes correctly.
  err = ipcInit(PATH);
  expect(!err);
  
  // Wait for parent to be ready.
  while (!(parentMask & MASK_READY)) ;

  //
  // Child to parent.
  //

  memset(txBuffer, 0, sizeof(txBuffer) / sizeof(txBuffer[0]));
  memset(rxBuffer, 0, sizeof(rxBuffer) / sizeof(rxBuffer[0]));

  // Send some bytes (from the child).
  err = ipcTransmit(smallBytes, SMALL_BYTES_COUNT);
  expect(!err);

  // Receive those bytes (on the parent).
  parentReceive(SMALL_BYTES_COUNT);
  expect(memcmp(rxBuffer, smallBytes, SMALL_BYTES_COUNT) == 0);

  // Send some bytes (from the child).
  err = ipcTransmit(largeBytes, LARGE_BYTES_COUNT);
  expect(!err);

  // Receive those bytes (on the parent).
  parentReceive(LARGE_BYTES_COUNT);
  expect(memcmp(rxBuffer, largeBytes, LARGE_BYTES_COUNT) == 0);

  //
  // Parent to child.
  //

  memset(txBuffer, 0, sizeof(txBuffer) / sizeof(txBuffer[0]));
  memset(rxBuffer, 0, sizeof(rxBuffer) / sizeof(rxBuffer[0]));

  // Send some bytes (from the parent).
  parentSend(smallBytes, SMALL_BYTES_COUNT);

  // Receive those bytes (on the child).
  err = ipcReceive(rxBuffer, SMALL_BYTES_COUNT);
  expect(err == 0);
  expect(memcmp(rxBuffer, smallBytes, SMALL_BYTES_COUNT) == 0);

  // Send some bytes (from the parent).
  parentSend(largeBytes, LARGE_BYTES_COUNT);

  // Receive those bytes (on the child).
  err = ipcReceive(rxBuffer, LARGE_BYTES_COUNT);
  expect(err == 0);
  expect(memcmp(rxBuffer, largeBytes, LARGE_BYTES_COUNT) == 0);

  // Close the socket.
  ipcDeinit();
}

int main(void)
{
  announce();

  startFakeParent();

  run(socketTest);

  stopFakeParent();

  return 0;
}
