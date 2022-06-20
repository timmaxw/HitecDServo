#include <HitecDServo.h>

HitecDServo servo;
int modelNumber;
HitecDServoConfig servoConfig;

void printError(int res) {
  switch (res) {
  case HITECD_OK:
    return;
  case HITECD_ERR_NO_SERVO:
    Serial.println("Error: No servo detected.");
    break;
  case HITECD_ERR_NO_RESISTOR:
    Serial.println("Error: Missing 2k resistor between signal wire and +5V rail.");
    break;
  case HITECD_ERR_CORRUPT:
    Serial.println("Error: Corrupt response from servo.");
    break;
  case HITECD_ERR_UNSUPPORTED_MODEL:
    /* Should never happen; we check the model number on startup. */
  case HITECD_ERR_NOT_ATTACHED:
    /* Should never happen; we attach the HitecDServo in setup. */
  default:
    Serial.println("Error: Unknown error.");
    break;
  }
}

void debugUnknownModelRegistersToSerial() {
  static uint8_t registersToDebug[] = {
    /* Model number register */
    0x00,

    /* Settings registers. I want to know if the default values are different for
    other models. */
    0x32, 0x44, 0x4C, 0x4E, 0x54, 0x5E, 0x60, 0x64, 0x66, 0x68, 0x6C, 0x9C, 0xB0, 0xB2, 0xC2,

    /* Mystery registers that always seem to read a constant value on the D485HW.
    I want to know if they return a different value on other models. */
    0x04, 0x06, 0x8A, 0x8C, 0xC4, 0xD4, 0xD6,

    /* Mystery registers that the DPC-11 always writes to a constant value on the
    D485HW. I want to know if they are set to something else on other models. */
    0x50, 0x52, 0x56, 0x72, 0x98, 0x9A
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
    Serial.println("Warning: Ignoring unexpected input in serial buffer.");
  }

  rawInputLen = 0;
  while (true) {
    if (!Serial.available()) {
      continue;
    }
    char next = Serial.peek();
    if (next == '\r' || next == '\n') {
      if (rawInputLen < sizeof(rawInput)) {
        if (next == '\r') {
          /* Check for and discard a LF character (second half of CRLF) */
          delay(10);
          if (Serial.available() && Serial.peek() == '\n') Serial.read();
        }
        /* OK, we have a valid input line. */
        return;
      } else {
        /* We overflowed the buffer. Discard any remaining data, show an
        error, and restart. */
        delay(1000);
        while (Serial.available()) Serial.read();
        Serial.println("Error: Input was too long. Please try again:");
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
}

void printConfig() {
  Serial.print("ID: ");
  Serial.println(servoConfig.id, DEC);

  Serial.print("Direction: ");
  if (servoConfig.counterclockwise) {
    Serial.println("Counterclockwise");
  } else {
    Serial.println("Clockwise");
  }

  Serial.print("Speed: ");
  Serial.println(servoConfig.speed, DEC);

  Serial.print("Deadband: ");
  Serial.println(servoConfig.deadband, DEC);

  Serial.print("Soft start: ");
  Serial.println(servoConfig.softStart, DEC);

  Serial.print("Left point: ");
  Serial.println(servoConfig.leftPoint, DEC);
  Serial.print("Center point: ");
  Serial.println(servoConfig.centerPoint, DEC);
  Serial.print("Right point: ");
  Serial.println(servoConfig.rightPoint, DEC);

  Serial.print("Fail safe: ");
  if (servoConfig.failSafe) {
    Serial.println(servoConfig.failSafe, DEC);
  } else if (servoConfig.failSafeLimp) {
    Serial.println("Limp");
  } else {
    Serial.println("Off");
  }

  Serial.print("Overload protection: ");
  Serial.println(servoConfig.overloadProtection, DEC);

  Serial.print("Smart sense: ");
  if (servoConfig.smartSense) {
    Serial.println("On");
  } else {
    Serial.println("Off");
  }

  Serial.print("Sensitivity ratio: ");
  Serial.println(servoConfig.sensitivityRatio, DEC);
}

void setup() {
  Serial.begin(115200);

  servo.attach(2);

  Serial.println("Reading model number...");
  modelNumber = servo.readModelNumber();
  if (modelNumber < 0) {
    printError(modelNumber);
    while (true) { }
  }
  Serial.print("Model number: D");
  Serial.println(modelNumber, DEC);

  if (modelNumber != 485) {
    Serial.println("Warning: Your servo model is not fully supported. "
      "Currently, only the D485HW model is fully supported. To improve support "
      "for your servo model, please open a GitHub issue at "
      "https://github.com/timmaxw/HitecDServo/issues/new and include the "
      "following information:\n");
    debugUnknownModelRegistersToSerial();
    
  }

  Serial.println("Reading configuration...");
  int res = servo.readConfig(&servoConfig);
  if (res < 0) {
    printError(res);
    while (true) { }
  }
  printConfig();
}

void loop() {
  // put your main code here, to run repeatedly:

}
