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
#include <sys/un.h>     // sockaddr_un

#include <limits.h> // INT_MAX
#include <unistd.h> // close
#include <string.h> // memset, strcpy

#ifdef BACK_TEST
  #include <stdio.h>
  #define debugPrintln(format, ...) printf(format "\n", __VA_ARGS__)
#else
  #define debugPrintln(...)
#endif

static int socketFd = INT_MAX;

int ipcInit(const char *path)
{
  bool err;
  struct sockaddr_un address;

  err = ((socketFd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1);

  if (!err) {
    memset(&address, sizeof(address), 0);
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    err = (connect(socketFd,
                   (struct sockaddr *)&address,
                   sizeof(address)) == -1);
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
