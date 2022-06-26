#ifndef CommandLine_h
#define CommandLine_h

#include <Arduino.h>

/* Helper functions for a command-line interface over the serial port */

/* scanRawInput() reads one line of input over the serial port and stores it in
the rawInput buffer. It also echoes it back to the user (unless the NO_ECHO flag
is set) because the Arduino serial console doesn't echo user input, and it's
helpful for the user to be able to see what they typed. */
#define NO_ECHO (1 << 0)
void scanRawInput(int flags = 0);
extern char rawInput[128];
extern int rawInputLen;

/* parseNumber() tries to interpret rawInput as a positive number:
- If valid, it stores it in *valOut and returns true.
- If invalid, it returns false and prints an error message.
- If empty, it returns false, but doesn't print an error unless the
    PRINT_IF_EMPTY flag is set.
If the QUARTERS flag is set, the input can be a decimal ending in .0, .25, .5,
or .75. The returned value will be multiplied by 4. */
#define PRINT_IF_EMPTY (1 << 1)
#define QUARTERS (1 << 2)
bool parseNumber(int16_t *valOut, int flags = 0);

/* parseWord() returns true if rawInput matches the given word, false if not.
Matching is case-insensitive. */
bool parseWord(const char *word);

/* scanNumber() is a convenience function that just combines scanRawInput() with
parseNumber(). */
bool scanNumber(int16_t *valOut, int flags = 0);

#endif /* CommandLine_h */
