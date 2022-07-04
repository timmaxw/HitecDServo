#include "CommandLine.h"

char rawInput[32];
int rawInputLen;

void scanRawInput(int flags) {
  /* Discard any leftover data in the serial buffer; it would have been
  sent before the prompt was printed, so it probably wasn't meant as input to
  the prompt. */
  bool discardedLeftoverData = false;
  while (Serial.available()) {
    Serial.read();
    discardedLeftoverData = true;
  }
  if (discardedLeftoverData) {
    Serial.println(F("Warning: Ignoring unexpected input in serial buffer."));
  }

  rawInputLen = 0;
  while (true) {
    if (!Serial.available()) {
      continue;
    }
    char next = Serial.read();
    if (next == '\r' || next == '\n') {
      if (rawInputLen < (int16_t)sizeof(rawInput)) {
        if (next == '\r') {
          /* Check for and discard a LF character (second half of CRLF) */
          delay(10);
          if (Serial.available() && Serial.peek() == '\n') Serial.read();
        }
        /* OK, we have a valid input line. */
        break;
      } else {
        /* We overflowed the buffer. Discard any remaining data, show an
        error, and restart. */
        delay(1000);
        while (Serial.available()) Serial.read();
        Serial.println(F("Error: Input was too long. Please try again:"));
        rawInputLen = 0;
        continue;
      }
    }
    if (rawInputLen == sizeof(rawInput)) {
      /* Ignore the too-long input for now, but we'll error later. */
      continue;
    }
    rawInput[rawInputLen] = next;
    ++rawInputLen;
  }

  if (!(flags & NO_ECHO)) {
    if (rawInputLen > 0) {
      Serial.print(F("You entered: \""));
      Serial.write(rawInput, rawInputLen);
      Serial.println('\"');
    } else {
      Serial.println(F("You entered nothing."));
    }
  }
}

bool parseNumber(int16_t *valOut, int flags) {
  if (rawInputLen == 0) {
    if (flags & PRINT_IF_EMPTY) {
      Serial.println(F("Error: Input was empty."));
    }
    return false;
  }

  if (rawInput[0] == '-') {
    Serial.println(F("Error: Number must be positive."));
    return false;
  }

  int i = 0;

  *valOut = 0;
  while (i < rawInputLen && rawInput[i] >= '0' && rawInput[i] <= '9') {
    *valOut = *valOut * 10 + (rawInput[i] - '0');
    ++i;
  }

  if (flags & QUARTERS) {
    /* In "quarters" mode, we allow numbers to end in .0, .25, .5, .75; the
    value we return is multiplied by 4. (We don't support floating point in
    general.) */
    *valOut *= 4;
    if (i < rawInputLen && rawInput[i] == '.') {
      ++i;
      if (i < rawInputLen && rawInput[i] == '0') {
        i += 1;
      } else if (i + 1 < rawInputLen &&
          rawInput[i] == '2' && rawInput[i+1] == '5') {
        *valOut += 1;
        i += 2;
      } else if (i < rawInputLen && rawInput[i] == '5') {
        *valOut += 2;
        i += 1;
      } else if (i < rawInputLen &&
          rawInput[i] == '7' && rawInput[i+1] == '5') {
        *valOut += 3;
        i += 2;
      } else {
        Serial.println(F("Error: Invalid number."));
        return false;
      }
      while (i < rawInputLen && rawInput[i] == '0') {
        ++i;
      }
    }
  }

  if (i != rawInputLen) {
    Serial.println(F("Error: Invalid number."));
    return false;
  }

  return true;
}

bool scanNumber(int16_t *valOut, int flags) {
  scanRawInput();
  return parseNumber(valOut, flags);
}

bool parseWord(const __FlashStringHelper *word) {
  PGM_P pgm_word = reinterpret_cast<PGM_P>(word);
  for (int i = 0; ; ++i) {
    char c = pgm_read_byte(pgm_word + i);
    if (c == '\0') {
      return (rawInputLen == i);
    }
    if (rawInputLen <= i) {
      return false;
    }
    if (tolower(c) != tolower(rawInput[i])) {
      return false;
    }
  }
}

bool scanYesNo() {
  scanRawInput();
  if (parseWord(F("y")) || parseWord(F("yes"))) {
    return true;
  } else if (rawInputLen == 0 || parseWord(F("n")) || parseWord(F("no"))) {
    return false;
  } else {
    Serial.println(F("Error: You did not enter \"y\" or \"n\"."));
    return false;
  }
}
