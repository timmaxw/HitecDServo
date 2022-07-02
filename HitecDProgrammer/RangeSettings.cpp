#include "RangeSettings.h"

#include "CommandLine.h"
#include "Common.h"

int16_t measuredMinRawAngle = -1;
int16_t measuredMaxRawAngle = -1;

int16_t tentativeRangeLeft;
int16_t tentativeRangeCenter;
int16_t tentativeRangeRight;

void changeSingleAngleSetting(int16_t *tentativeRawAngleInOut) {
  Serial.println(F("Enter new raw angle (or nothing to cancel)"));
  int16_t newRawAngle;
  if (!scanNumber(&newRawAngle)) {
    goto cancel;
  }
  if (newRawAngle == *tentativeRawAngleInOut) {
    goto cancel;
  }
  if (newRawAngle > 16383) {
    Serial.println(F("Error: Invalid raw angle."));
    goto cancel;
  }

  *tentativeRawAngleInOut = newRawAngle;
  Serial.println(F("Tentatively changed raw angle."));
  return;

cancel:
  Serial.println(F("Current raw angle will be kept."));
}

void changeAngleSettings850() {
  if (tentativeRangeLeft == settings.rangeLeftRawAngle) {
    Serial.print(F("Current raw angle for 850us PWM (left endpoint): "));
  } else {
    Serial.print(F("Tentative raw angle for 850us PWM (left endpoint): "));
  }
  printValueWithDefault(tentativeRangeLeft,
    HitecDSettings::defaultRangeLeftRawAngle(modelNumber));
  if (measuredMinRawAngle != -1) {
    Serial.print(F("Measured minimum for your servo: "));
    Serial.println(measuredMinRawAngle);
  } else {
    int16_t safeMinRawAngle = HitecDSettings::safeMinRawAngle(modelNumber);
    if (safeMinRawAngle != -1) {
      Serial.print(F("Safe minimum for your servo: "));
      Serial.println(safeMinRawAngle);
    }
  }

  changeSingleAngleSetting(&tentativeRangeLeft);
}

void changeAngleSettings1500() {
  if (tentativeRangeCenter == settings.rangeCenterRawAngle) {
    Serial.print(F("Current raw angle for 1500us PWM (center point): "));
  } else {
    Serial.print(F("Tentative raw angle for 1500us PWM (center point): "));
  }
  printValueWithDefault(tentativeRangeCenter,
    HitecDSettings::defaultRangeCenterRawAngle(modelNumber));

  changeSingleAngleSetting(&tentativeRangeCenter);
}

void changeAngleSettings2150() {
  if (tentativeRangeRight == settings.rangeRightRawAngle) {
    Serial.print(F("Current raw angle for 2150us PWM (right endpoint): "));
  } else {
    Serial.print(F("Tentative raw angle for 2150us PWM (right endpoint): "));
  }
  printValueWithDefault(tentativeRangeRight,
    HitecDSettings::defaultRangeRightRawAngle(modelNumber));
  if (measuredMaxRawAngle != -1) {
    Serial.print(F("Measured maximum for your servo: "));
    Serial.println(measuredMaxRawAngle);
  } else {
    int16_t safeMaxRawAngle = HitecDSettings::safeMaxRawAngle(modelNumber);
    if (safeMaxRawAngle != -1) {
      Serial.print(F("Safe maximum for your servo: "));
      Serial.println(safeMaxRawAngle);
    }
  }

  changeSingleAngleSetting(&tentativeRangeRight);
}

void changeAngleSettingsMove(int16_t targetRawAngle) {
  Serial.println(F("Moving..."));
  int16_t actualRawAngle;
  moveTempRawAngle(targetRawAngle, &actualRawAngle);
  Serial.print(F("The raw angle the servo actually reached was: "));
  Serial.println(actualRawAngle);
}

bool changeAngleSettingsDone() {
  if (tentativeRangeLeft > tentativeRangeCenter) {
    Serial.println(F(
      "Error: Left endpoint must be less than center point."));
    return false;
  }
  if (tentativeRangeRight < tentativeRangeCenter) {
    Serial.println(F(
      "Error: Right endpoint must be greater than center point."));
    return false;
  }

  settings.rangeLeftRawAngle = tentativeRangeLeft;
  settings.rangeCenterRawAngle = tentativeRangeCenter;
  settings.rangeRightRawAngle = tentativeRangeRight;
  writeSettings();
  return true;
}

void printAngleSettingsHelp() {
  Serial.println(F("Available commands for changing angle settings:"));
  Serial.println(F(
    "  left        - Change raw angle for 850us (left endpoint)"));
  Serial.println(F(
    "  center      - Change raw angle for 1500us (center point)"));
  Serial.println(F(
    "  right       - Change raw angle for 2150us (right endpoint)"));
  Serial.println(F(
    "  <number>    - Temporarily tell servo to move to arbitrary raw angle"));
  Serial.println(F(
    "  done        - Save changes and return to main menu"));
  Serial.println(F(
    "  cancel      - Cancel changes and return to main menu")); 
}

void changeRangeSettings() {
  if (!checkSupportedModel()) {
    goto cancel;
  }

  tentativeRangeLeft = settings.rangeLeftRawAngle;
  tentativeRangeCenter = settings.rangeCenterRawAngle;
  tentativeRangeRight = settings.rangeRightRawAngle;

  printAngleSettingsHelp();

  while (true) {
    Serial.println(F("Enter a command for changing angle settings:"));
    scanRawInput();
    if (parseWord("left")) {
      changeAngleSettings850();
    } else if (parseWord("center")) {
      changeAngleSettings1500();
    } else if (parseWord("right")) {
      changeAngleSettings2150();
    } else if (rawInput[0] >= '0' && rawInput[0] <= '9') {
      int16_t targetRawAngle;
      if (!parseNumber(&targetRawAngle)) {
        continue;
      }
      changeAngleSettingsMove(targetRawAngle);
    } else if (parseWord("done")) {
      if (changeAngleSettingsDone()) {
        return;
      }
    } else if (parseWord("cancel")) {
      goto cancel;
    } else {
      Serial.println(F("Error: What you entered is not a valid command."));
      printAngleSettingsHelp();
    }
  }

cancel:
  Serial.println(F("Previous angle left/center/right settings will be kept."));
  undoRangeMeasurementSettings();
}

