#include "Common.h"

#include "CommandLine.h"
#include "Move.h"
#include "UnsupportedModel.h"

HitecDServo servo;
int modelNumber;
HitecDSettings settings;

int16_t defaultRangeLeftAPV = -1;
int16_t defaultRangeRightAPV = -1;
int16_t defaultRangeCenterAPV = -1;

int16_t widestRangeLeftAPVClockwise = -1;
int16_t widestRangeRightAPVClockwise = -1;
int16_t widestRangeCenterAPVClockwise = -1;

int16_t widestRangeLeftAPV() {
  if (widestRangeLeftAPVClockwise == -1) {
    return -1;
  } else if (!settings.counterclockwise) {
    return widestRangeLeftAPVClockwise;
  } else {
    return 16383 - widestRangeRightAPVClockwise;
  }
}
int16_t widestRangeRightAPV() {
  if (widestRangeRightAPVClockwise == -1) {
    return -1;
  } else if (!settings.counterclockwise) {
    return widestRangeRightAPVClockwise;
  } else {
    return 16383 - widestRangeLeftAPVClockwise;
  }
}
int16_t widestRangeCenterAPV() {
  if (widestRangeCenterAPVClockwise == -1) {
    return -1;
  } else if (!settings.counterclockwise) {
    return widestRangeCenterAPVClockwise;
  } else {
    return 16383 - widestRangeCenterAPVClockwise;
  }
}

void fatalErr() {
  Serial.println(F("Please fix the problem and then reset your Arduino."));
  while (true) { }
}

void printErr(int res, bool fatal) {
  if (res == HITECD_OK) {
    return;
  }

  Serial.print(F("Error: "));
  Serial.println(hitecdErrToString(res));

  if (fatal) {
    fatalErr();
  }
}

void printValueWithDefault(int16_t value, int16_t defaultValue) {
  Serial.print(value, DEC);
  if (value == defaultValue) {
    Serial.println(F(" (default)"));
  } else if (defaultValue != -1) {
    Serial.print(F(" (default is "));
    Serial.print(defaultValue, DEC);
    Serial.println(')');
  } else {
    Serial.println(F(" (default unknown)"));
  }
}

void saveSettings() {
  int res;
  Serial.println(F("Saving new servo settings..."));

  /* Writing the settings starts by resetting the servo to factory settings,
  which will overwrite any gentle-movement settings. */
  usingGentleMovementSettings = false;

  res = servo.writeSettingsUnsupportedModelThisMightDamageTheServo(
    settings,
    allowUnsupportedModel
  );
  if (res != HITECD_OK) {
    printErr(res, true);
  }

  /* Wait for servo to reboot */
  delay(1000);

  /* Read back the settings to make sure we have the latest values. */
  if ((res = servo.readSettings(&settings)) != HITECD_OK) {
    printErr(res, true);
  }

  Serial.println(F("Done."));
}



