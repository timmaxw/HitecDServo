#include <HitecDServo.h>

HitecDServo servo;
int modelNumber;
HitecDServoConfig servoConfig;

void printErr(int res, bool needReset) {
  if (res == HITECD_OK) {
    return;
  }

  Serial.print(F("Error: "));
  Serial.println(hitecdErrToString(res));

  if (needReset) {
    Serial.println(F("Please fix the problem and then reset your Arduino."));
    while (true) { }
  }
}

void printRegisterDump() {
  static uint8_t registersToDebug[] = {
    /* Model number register */
    0x00,

    /* Settings registers. I want to know if the default values are different
    for other models. */
    0x32, 0x44, 0x4C, 0x4E, 0x54, 0x5E, 0x60, 0x64, 0x66, 0x68, 0x6C, 0x9C,
    0xB0, 0xB2, 0xC2,

    /* Mystery registers that always seem to read a constant value on the
    D485HW. I want to know if they return a different value on other models. */
    0x04, 0x06, 0x8A, 0x8C, 0xC4, 0xD4, 0xD6,

    /* Mystery registers that the DPC-11 always writes to a constant value on
    the D485HW. I want to know if they are set to something else on other
    models. */
    0x50, 0x52, 0x56, 0x72, 0x98, 0x9A
  };

  for (int i = 0; i < sizeof(registersToDebug); ++i) {
    uint8_t reg = registersToDebug[i];
    uint16_t temp;
    int res;
    if ((res = servo.readRawRegister(reg, &temp)) != HITECD_OK) {
      printErr(res, true);
    }
    Serial.print((reg >> 4) & 0x0F, HEX);
    Serial.print((reg >> 0) & 0x0F, HEX);
    Serial.print(':');
    Serial.print((temp >> 12) & 0x0F, HEX);
    Serial.print((temp >> 8) & 0x0F, HEX);
    Serial.print((temp >> 4) & 0x0F, HEX);
    Serial.print((temp >> 0) & 0x0F, HEX);
    if (i % 8 == 7 || i + 1 == sizeof(registersToDebug)) {
      Serial.println();
    } else {
      Serial.print(' ');
    }
  }
}

char rawInput[128];
int rawInputLen;
void scanRawInput() {
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
      if (rawInputLen < sizeof(rawInput)) {
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

  if (rawInputLen > 0) {
    Serial.print(F("You entered: "));
    Serial.write(rawInput, rawInputLen);
    Serial.println();
  } else {
    Serial.println(F("You entered nothing."));
  }
}

int16_t scanNumber(bool allowEmptyAsNegativeOne, bool quarters=false) {
  while (true) {
    scanRawInput();

    if (rawInputLen == 0) {
      if (allowEmptyAsNegativeOne) {
        return -1;
      } else {
        Serial.println(F("Error: Input was empty. Please try again:"));
        continue;
      }
    }

    bool negative = (rawInput[0] == '-');
    int i = (negative ? 1 : 0);
    if (rawInputLen == i) {
      Serial.println(F("Error: Invalid input. Please try again:"));
      continue;
    }

    int16_t number = 0;
    while (i < rawInputLen && rawInput[i] >= '0' && rawInput[i] <= '9') {
      number = number * 10 + (rawInput[i] - '0');
      ++i;
    }

    if (quarters) {
      /* In "quarters" mode, we allow numbers to end in .0, .25, .5, .75; the
      value we return is multiplied by 4. (We don't support floating point in
      general.) */
      number *= 4;
      if (i < rawInputLen && rawInput[i] == '.') {
        ++i;
        if (i < rawInputLen && rawInput[i] == '0') {
          i += 1;
        } else if (i + 1 < rawInputLen &&
            rawInput[i] == '2' && rawInput[i+1] == '5') {
          number += 1;
          i += 2;
        } else if (i < rawInputLen && rawInput[i] == '5') {
          number += 2;
          i += 1;
        } else if (i < rawInputLen &&
            rawInput[i] == '7' && rawInput[i+1] == '5') {
          number += 3;
          i += 2;
        } else {
          Serial.println(F("Error: Invalid input. Please try again:"));
          continue;
        }
        while (i < rawInputLen && rawInput[i] == '0') {
          ++i;
        }
      }
    }

    if (negative) number = -number;

    if (i != rawInputLen) {
      Serial.println(F("Error: Invalid input. Please try again:"));
      continue;
    }

    return number;
  }
}

void printValueWithDefault(int16_t value, int16_t defaultValue) {
  Serial.print(value, DEC);
  if (value != defaultValue) {
    Serial.println(F(" *"));
  } else {
    Serial.println();
  }
}

void printConfig() {
  Serial.println(F("Servo settings:"));

  Serial.print(F("  ID: "));
  printValueWithDefault(servoConfig.id,
    HitecDServoConfig::defaultId);

  Serial.print(F("  Direction: "));
  if (servoConfig.counterclockwise) {
    Serial.println(F("Counterclockwise *"));
  } else {
    Serial.println(F("Clockwise"));
  }

  Serial.print(F("  Speed: "));
  printValueWithDefault(servoConfig.speed,
    HitecDServoConfig::defaultSpeed);

  Serial.print(F("  Deadband: "));
  printValueWithDefault(servoConfig.deadband,
    HitecDServoConfig::defaultDeadband);

  Serial.print(F("  Soft start: "));
  printValueWithDefault(servoConfig.softStart,
    HitecDServoConfig::defaultSoftStart);

  Serial.print(F("  Raw angle for 850us PWM: "));
  printValueWithDefault(servoConfig.rawAngleFor850,
    HitecDServoConfig::defaultRawAngleFor850(modelNumber));

  Serial.print(F("  Raw angle for 1500us PWM: "));
  printValueWithDefault(servoConfig.rawAngleFor1500,
    HitecDServoConfig::defaultRawAngleFor1500(modelNumber));

  Serial.print(F("  Raw angle for 2150us PWM: "));
  printValueWithDefault(servoConfig.rawAngleFor2150,
    HitecDServoConfig::defaultRawAngleFor2150(modelNumber));

  Serial.print(F("  Fail safe: "));
  if (servoConfig.failSafe) {
    printValueWithDefault(servoConfig.failSafe,
      HitecDServoConfig::defaultFailSafe);
  } else if (servoConfig.failSafeLimp) {
    Serial.println(F("Limp *"));
  } else {
    Serial.println(F("Off"));
  }

  Serial.print(F("  Overload protection: "));
  if (servoConfig.overloadProtection < 100) {
    printValueWithDefault(servoConfig.overloadProtection,
      HitecDServoConfig::defaultOverloadProtection);
  } else {
    Serial.println(F("Off"));
  }

  Serial.print(F("  Smart sense: "));
  if (servoConfig.smartSense) {
    Serial.println(F("On"));
  } else {
    Serial.println(F("Off *"));
  }

  Serial.print(F("  Sensitivity ratio: "));
  printValueWithDefault(servoConfig.sensitivityRatio,
    HitecDServoConfig::defaultSensitivityRatio);

  Serial.println(F("  (* Indicates non-default value)"));
}

void printHelp() {
  Serial.println(F("Available commands:"));
  Serial.println(F("  V - View all servo settings"));
  Serial.println(F("  ? - Show this list of commands again"));
}

void setup() {
  int res;

  Serial.begin(115200);

  Serial.println(F("Enter the Arduino pin that the servo is attached to:"));
  int pin = scanNumber(false);
  Serial.println(F("Connecting to servo..."));
  if ((res = servo.attach(pin)) != HITECD_OK) {
    printErr(res, true);
  }

  if ((modelNumber = servo.readModelNumber()) < 0) {
    printErr(modelNumber, true);
  }

  Serial.print("Servo model: D");
  Serial.println(modelNumber, DEC);

  if (!servo.isModelSupported()) {
    Serial.println(F(
      "====================================================================\r\n"
      "Warning: Your servo model is not fully supported. Currently, only\r\n"
      "the D485HW model is fully supported. To improve support for your\r\n"
      "servo model, please open a GitHub issue at\r\n"
      "<https://github.com/timmaxw/HitecDServo/issues/new>\r\n"
      "Include the following following debug information:"
    ));
    printRegisterDump();
    Serial.println(F(
      "If you had already changed the values of any settings, please note\r\n"
      "them in the issue. Thanks!\r\n"
      "===================================================================="
    ));
  }

  if ((res = servo.readConfig(&servoConfig)) < 0) {
    printErr(res, true);
  }
  printConfig();

  printHelp();
}

void loop() {
  Serial.println(F("Enter a command:"));
  scanRawInput();
  if (rawInputLen != 1) {
    Serial.println(F("Error: Commands should be a single character."));
    return;
  }
  switch (rawInput[0]) {
  case 'V':
    printConfig();
    break;
  case '?':
    printHelp();
    break;
  default:
    Serial.print(F("Error: '"));
    Serial.print(rawInput[0]);
    Serial.println(F("' is not an available command."));
    printHelp();
    break;
  }
}
