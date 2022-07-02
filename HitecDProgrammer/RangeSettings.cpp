#include "RangeSettings.h"

#include "CommandLine.h"
#include "Common.h"

int16_t measuredMinAPV = -1;
int16_t measuredMaxAPV = -1;

int16_t tentativeRangeLeftAPV;
int16_t tentativeRangeCenterAPV;
int16_t tentativeRangeRightAPV;

void changeRangeAPVHelper(int16_t *tentativeAPVInOut) {
  Serial.println(F("Enter new point as APV (or nothing to cancel)"));
  int16_t newAPV;
  if (!scanNumber(&newAPV)) {
    goto cancel;
  }
  if (newAPV == *tentativeAPVInOut) {
    goto cancel;
  }
  if (newAPV > 16383) {
    Serial.println(F("Error: Invalid APV."));
    goto cancel;
  }

  *tentativeAPVInOut = newAPV;
  Serial.println(F(
    "Tentatively changed range of motion. Use 'save' command to make this\r\n"
    "change permanent."));
  return;

cancel:
  Serial.println(F("Current range of motion will be kept."));
}

void printRangeLeftAPVSetting() {
  Serial.print(F("Current left endpoint of range, as APV: "));
  printValueWithDefault(settings.rangeLeftAPV,
    HitecDSettings::defaultRangeLeftAPV(modelNumber));
}

void changeRangeLeftAPVSetting() {
  if (tentativeRangeLeftAPV == settings.rangeLeftAPV) {
    Serial.print(F("Current left endpoint of range, as APV: "));
  } else {
    Serial.print(F("Tentative left endpoint of range, as APV: "));
  }
  printValueWithDefault(tentativeRangeLeftAPV,
    HitecDSettings::defaultRangeLeftAPV(modelNumber));
  if (measuredMinAPV != -1) {
    Serial.print(F("Measured minimum APV for your servo: "));
    Serial.println(measuredMinAPV);
  } else {
    int16_t safeMinAPV = HitecDSettings::safeMinAPV(modelNumber);
    if (safeMinAPV != -1) {
      Serial.print(F("Safe minimum APV for your servo: "));
      Serial.println(safeMinAPV);
    }
  }

  changeRangeAPVHelper(&tentativeRangeLeftAPV);
}

void printRangeCenterAPVSetting() {
  Serial.print(F("Current center point of range, as APV: "));
  printValueWithDefault(settings.rangeCenterAPV,
    HitecDSettings::defaultRangeCenterAPV(modelNumber));
}

void changeRangeCenterAPVSetting() {
  if (tentativeRangeCenterAPV == settings.rangeCenterAPV) {
    Serial.print(F("Current center point of range, as APV: "));
  } else {
    Serial.print(F("Tentative center point of range, as APV: "));
  }
  printValueWithDefault(tentativeRangeCenterAPV,
    HitecDSettings::defaultRangeCenterAPV(modelNumber));

  changeRangeAPVHelper(&tentativeRangeCenterAPV);
}

void printRangeRightAPVSetting() {
  Serial.print(F("Current right endpoint of range, as APV: "));
  printValueWithDefault(settings.rangeRightAPV,
    HitecDSettings::defaultRangeRightAPV(modelNumber));
}

void changeRangeRightAPVSetting() {
  if (tentativeRangeRightAPV == settings.rangeRightAPV) {
    Serial.print(F("Current right endpoint of range, as APV: "));
  } else {
    Serial.print(F("Tentative right endpoint of range, as APV: "));
  }
  printValueWithDefault(tentativeRangeRightAPV,
    HitecDSettings::defaultRangeRightAPV(modelNumber));
  if (measuredMaxAPV != -1) {
    Serial.print(F("Measured maximum for your servo: "));
    Serial.println(measuredMaxAPV);
  } else {
    int16_t safeMaxAPV = HitecDSettings::safeMaxAPV(modelNumber);
    if (safeMaxAPV != -1) {
      Serial.print(F("Safe maximum for your servo: "));
      Serial.println(safeMaxAPV);
    }
  }

  changeRangeAPVHelper(&tentativeRangeRightAPV);
}

void temporarilyMoveToAPVAndPrint(int16_t targetAPV) {
  Serial.println(F("Moving..."));
  int16_t actualAPV;
  temporarilyMoveToAPV(targetAPV, &actualAPV);
  Serial.print(F("The APV the servo actually reached was: "));
  Serial.println(actualAPV);
}

bool saveRangeSettings() {
  if (tentativeRangeLeftAPV > tentativeRangeCenterAPV) {
    Serial.println(F(
      "Error: Left endpoint must be less than center point."));
    return false;
  }
  if (tentativeRangeRightAPV < tentativeRangeCenterAPV) {
    Serial.println(F(
      "Error: Right endpoint must be greater than center point."));
    return false;
  }

  settings.rangeLeftAPV = tentativeRangeLeftAPV;
  settings.rangeCenterAPV = tentativeRangeCenterAPV;
  settings.rangeRightAPV = tentativeRangeRightAPV;
  writeSettings();
  return true;
}

void printRangeSettingsHelp() {
  Serial.println(F("Available commands for setting range of motion:"));
  Serial.println(F(
    "  left        - Change left endpoint setting"));
  Serial.println(F(
    "  center      - Change center point setting"));
  Serial.println(F(
    "  right       - Change right endpoint setting"));
  Serial.println(F(
    "  <number>    - Temporarily tell servo to move to arbitrary APV"));
  Serial.println(F(
    "  save        - Save changes and return to main menu"));
  Serial.println(F(
    "  cancel      - Cancel changes and return to main menu")); 
}

void changeRangeSettings() {
  if (!checkSupportedModel()) {
    goto cancel;
  }

  tentativeRangeLeftAPV = settings.rangeLeftAPV;
  tentativeRangeCenterAPV = settings.rangeCenterAPV;
  tentativeRangeRightAPV = settings.rangeRightAPV;

  printRangeSettingsHelp();

  while (true) {
    Serial.println(F("Enter a command for setting range of motion:"));
    scanRawInput();
    if (parseWord("left")) {
      changeRangeLeftAPVSetting();
    } else if (parseWord("center")) {
      changeRangeCenterAPVSetting();
    } else if (parseWord("right")) {
      changeRangeRightAPVSetting();
    } else if (rawInput[0] >= '0' && rawInput[0] <= '9') {
      int16_t targetAPV;
      if (!parseNumber(&targetAPV)) {
        continue;
      }
      temporarilyMoveToAPVAndPrint(targetAPV);
    } else if (parseWord("save")) {
      if (saveRangeSettings()) {
        return;
      }
    } else if (parseWord("cancel")) {
      goto cancel;
    } else {
      Serial.println(F("Error: What you entered is not a valid command."));
      printRangeSettingsHelp();
    }
  }

cancel:
  Serial.println(F("Previous range of motion settings will be kept."));
  undoRangeMeasurementSettings();
}

