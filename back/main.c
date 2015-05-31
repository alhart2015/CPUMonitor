//
// main.c
//
// Alden Hart
// Andrew Keesler
//
// May 22, 2015
//
// Main code for monitor backend.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h> // strcmp
#include <assert.h>
// #include <sys/sysinfo.h>    // Looks like this is a good library we should use

#include "ipc.h"

#define MIN_ARGC 2
#define USAGE "-p socket-path [-t test]"

// A test function only takes a socket path.
typedef void (*TestFunction)(const char *);
static TestFunction findTest(const char *name);

int main(int argc, char *argv[])
{
  uint8_t argi = 1;
  const char *socketPath = NULL, *testName = NULL;
  TestFunction test;

  if (argc < MIN_ARGC) {
    printf("usage: %s\n", USAGE);
    return 1;
  }

  for (; argi < argc; argi ++) {
    if (strcmp(argv[argi], "-p") == 0) {
      argi ++;
      socketPath = argv[argi];
    } else if (strcmp(argv[argi], "-t") == 0) {
      argi ++;
      testName = argv[argi];
    } else {
      printf("error: unrecognized option %s\n", argv[argi]);
      return 1;
    }
  }

  if (!socketPath) {
    printf("error: socket path required\n");
    return 1;
  }

  if (testName) {
    if (!(test = findTest(testName))) {
      printf("error: unknown test %s\n", testName);
    } else {
      test(socketPath);
    }
  }

  return 0;
}

// -----------------------------------------------------------------------------
// Testing

#define TEST(name) void name(const char *socketPath)

#define TRY_TEST(name, potentialMatch, test)    \
  if (strcmp(#name, potentialMatch) == 0) {     \
    extern void name(const char *socketPath);   \
    test = name;                                \
  }

static TestFunction findTest(const char *name)
{
  TestFunction test = NULL;

  TRY_TEST(sanityTest, name, test);
  TRY_TEST(echoTest, name, test);

  return test;
}

TEST(sanityTest)
{
  printf("sanityTest: %s\n", socketPath); // yeah
  fflush(stdout);
  exit(0);
}

// Echo what the parent says 5 times.
// Keep this in sync with ipc_test.py!
#define ECHO_COUNT 5
static const char *echoWords[ECHO_COUNT] = { "foo","bar","bat","tuna","fish", };
TEST(echoTest)
{
  uint8_t i, buffer[32], wordLength;
  bool fail = false;

  assert(ipcInit(socketPath) == 0);

  for (i = 0; i < ECHO_COUNT && !fail; i ++) {
    memset(buffer, 0, sizeof(buffer) / sizeof(buffer[0]));

    // Get the length of the data.
    ipcReceive(buffer, 1);
    wordLength = atoi((const char *)&(buffer[0]));
    printf("got the length of the data: %d\n", wordLength);

    // Get the rest of the data.
    ipcReceive(buffer, wordLength);
    printf("got the rest of the data: %s\n", buffer);

    // Make sure the data is correct.
    fail = (strcmp((const char *)&(buffer[0]), echoWords[i]) != 0);
    if (!fail) printf("the data is correct\n");

    // Ship it back.
    if (!fail) {
      printf("sending the data back (err = %d)\n",
             ipcTransmit(buffer, wordLength));
    }
  }

  ipcDeinit();

  exit(fail ? 1 : 0);
}
