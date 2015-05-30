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
// #include <sys/sysinfo.h>    // Looks like this is a good library we should use

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

  return test;
}

TEST(sanityTest)
{
  printf("sanityTest: %s\n", socketPath); // yeah
  exit(0);
}
