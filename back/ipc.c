//
// ipc.c
//
// Alden Hart
// Andrew Keesler
//
// May 25, 2015
//
// Inter-process communication with parent GUI.
//

#include "ipc.h"

#include <stdbool.h> // bool, true, false

#include <sys/socket.h> // socket
#include <netinet/in.h> // INADDR_LOOPBACK, sockaddr_in
#include <arpa/inet.h>  // htons, htonl

#include <limits.h> // INT_MAX
#include <unistd.h> // close
#include <string.h> // memset

#ifdef BACK_TEST
  #include <stdio.h>
  #define debugPrintln(format, ...) printf(format "\n", __VA_ARGS__)
#else
  #define debugPrintln(...)
#endif

int socketFd = INT_MAX;

int ipcInit(uint16_t port)
{
  bool err;
  struct sockaddr_in loopbackAddress;

  err = ((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1);

  if (!err) {
    memset(&loopbackAddress, sizeof(loopbackAddress), 0);
    loopbackAddress.sin_family = AF_INET;
    loopbackAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    loopbackAddress.sin_port = htons(port);
    err = (connect(socketFd,
                   (struct sockaddr *)&loopbackAddress,
                   sizeof(loopbackAddress)) == -1);
  }

  return (err ? errno : 0);
}

void ipcDeinit(void)
{
  close(socketFd);
}

static int transmitOrReceive(uint8_t *bytes, uint32_t count, bool transmit)
{
  bool err = true;

  errno = EINVAL;
  if (socketFd != INT_MAX) {
    err = ((transmit
            ? send(socketFd, bytes, count, 0) // flags
            : recv(socketFd, bytes, count, 0) // flags
            )
           == -1);
  }

  return (err ? errno : 0);
}

int ipcTransmit(uint8_t *bytes, uint32_t count)
{
  return transmitOrReceive(bytes, count, true); // transmit?
}

int ipcReceive(uint8_t *bytes, uint32_t count)
{
  return transmitOrReceive(bytes, count, false); // transmit?
}
