//
// ipc.h
//
// Alden Hart
// Andrew Keesler
//
// May 25, 2015
//
// Inter-process communication with parent GUI.
//

#include <stdint.h>
#include <errno.h>

// Initialize the inter-process communication channel with the parent.
// If the channel could not be initialized, it will return the errno
// value that was defined for the failed socket system call.
// The argument is the port for the connection.
int ipcInit(uint16_t port);

// This will close the socket to the parent.
void ipcDeinit(void);

// Send some bytes to the parent.
// If the transmit failed, it will return the errno value associated
// with the failure.
// Note that ipcInit() must be called before transmitting bytes.
// If it has not been called, this function will return EINVAL.
int ipcTransmit(uint8_t *bytes, uint32_t count);

// Receive some bytes.
// Rumor has it that the looback socket is full-duplex...does that
// mean that we can receive from the same guy too?
// FIXME:
int ipcReceive(uint8_t *bytes, uint32_t count);


