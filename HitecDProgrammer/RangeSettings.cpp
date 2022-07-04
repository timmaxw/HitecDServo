#include "RangeSettings.h"

#include "CommandLine.h"
#include "Common.h"
#include "UnsupportedModel.h"

void printRangeLeftAPVSetting() {
  Serial.print(F("Current left endpoint of range, as APV: "));
  printValueWithDefault(settings.rangeLeftAPV, defaultRangeLeftAPV);
}

void printRangeRightAPVSetting() {
  Serial.print(F("Current right endpoint of range, as APV: "));
  printValueWithDefault(settings.rangeRightAPV, defaultRangeRightAPV);
}

void printRangeCenterAPVSetting() {
  Serial.print(F("Current center point of range, as APV: "));
  printValueWithDefault(settings.rangeCenterAPV, defaultRangeCenterAPV);
}

bool changeRangeSettingsDetect(); // forward declaration

bool changeRangeSettingsDefault() {
  printRangeLeftAPVSetting();
  printRangeRightAPVSetting();
  printRangeCenterAPVSetting();

  Serial.println(F(
    "Change range settings to factory defaults? Enter \"y\" or \"n\":"));
  if (!scanYesNo()) {
    return false;
  }

  settings.rangeLeftAPV = -1;
  settings.rangeRightAPV = -1;
  settings.rangeCenterAPV = -1;
  writeSettings();

  if (!servo.isModelSupported()) {
    /* The servo library doesn't know the default values of rangeLeftAPV/etc.,
    but we just reset the servo, so we know the current values must be the
    default values. Record those values. */
    defaultRangeLeftAPV = settings.rangeLeftAPV;
    defaultRangeRightAPV = settings.rangeRightAPV;
    defaultRangeCenterAPV = settings.rangeCenterAPV;
  }

  return true;
}

bool changeRangeSettingsWidest() {
  if (widestRangeLeftAPV == -1) {
    Serial.println(F(
      "Error: The HitecDServo library does not know the widest range for\r\n"
      "your servo model. Do you want to detect how far the servo can move\r\n"
      "and use that range? Enter \"y\" or \"n\":"));
    if (scanYesNo()) {
      return changeRangeSettingsDetect();
    } else {
      return false;
    }
  }

  printRangeLeftAPVSetting();
  Serial.print(F("Widest range left endpoint, as APV: "));
  Serial.println(widestRangeLeftAPV());

  printRangeRightAPVSetting();
  Serial.print(F("Widest range right endpoint, as APV: "));
  Serial.println(widestRangeRightAPV());

  printRangeCenterAPVSetting();
  Serial.print(F("Widest range center point, as APV: "));
  Serial.println(widestRangeCenterAPV());

  Serial.println(F("Change to widest range? Enter \"y\" or \"n\":"));
  if (!scanYesNo()) {
    return false;
  }

  settings.rangeLeftAPV = widestRangeLeftAPV();
  settings.rangeRightAPV = widestRangeRightAPV();
  settings.rangeCenterAPV = widestRangeCenterAPV();
  writeSettings();
  return true;
}

bool changeRangeSettingsDetect() {
  Serial.println(F(
    "Servo will move as far as possible in each direction. When you are\r\n"
    "ready, enter \"y\" to continue (or \"n\" to cancel):"));
  if (!scanYesNo()) {
    return false;
  }

  useRangeMeasurementSettings();
  int16_t left, right, center;
  Serial.println(F("Moving left as far as possible..."));
  temporarilyMoveToAPV(50, &left);
  Serial.println(F("Moving right as far as possible..."));
  temporarilyMoveToAPV(16333, &right);
  center = (left + right) / 2;

  printRangeLeftAPVSetting();
  Serial.print(F("Detected left limit, as APV: "));
  Serial.println(left);

  printRangeRightAPVSetting();
  Serial.print(F("Detected right limit, as APV: "));
  Serial.println(right);

  printRangeCenterAPVSetting();
  Serial.print(F("Center of detected limits, as APV: "));
  Serial.println(center);

  Serial.println(F(
    "Change range settings to detected limits? Enter \"y\" or \"n\":"));
  if (!scanYesNo()) {
    return false;
  }

  settings.rangeLeftAPV = left;
  settings.rangeRightAPV = right;
  settings.rangeCenterAPV = center;
  writeSettings();
  return true;
}

bool saveRangeSettings(int16_t left, int16_t right, int16_t center) {
  if ((center < left && center < right) ||
      (center > left && center > right)) {
    Serial.println(F(
      "Error: Center point must be between left and right endpoints."));
    return false;
  }
  if (left <= right) {
    settings.rangeLeftAPV = left;
    settings.rangeRightAPV = right;
  } else {
    Serial.println(F("Warning: Left and right endpoints will be swapped."));
    settings.rangeLeftAPV = right;
    settings.rangeRightAPV = left;
  }
  settings.rangeCenterAPV = center;
  writeSettings();
  return true;
}

void printRangeSettingsInteractiveHelp() {
  Serial.println(F(
    "Available commands for interactively setting range of motion:"));
  Serial.println(F(
    "  <number>    - Tell servo to move to arbitrary APV, from 0 to 16383"));
  Serial.println(F(
    "  left        - Change left endpoint to current servo position"));
  Serial.println(F(
    "  right       - Change right endpoint to current servo position"));
  Serial.println(F(
    "  center      - Change center point to current servo position"));
  Serial.println(F(
    "  save        - Save changes and exit"));
  Serial.println(F(
    "  cancel      - Cancel changes and exit"));
}

bool changeRangeSettingsInteractive() {
  printRangeLeftAPVSetting();
  printRangeRightAPVSetting();
  printRangeCenterAPVSetting();

  int16_t left = settings.rangeLeftAPV;
  int16_t right = settings.rangeCenterAPV;
  int16_t center = settings.rangeRightAPV;

  printRangeSettingsInteractiveHelp();

  int16_t actualAPV = servo.readCurrentAPV();

  while (true) {
    Serial.println(F("Enter a command for setting range of motion:"));
    scanRawInput();
    if (rawInput[0] >= '0' && rawInput[0] <= '9') {
      int16_t targetAPV;
      if (!parseNumber(&targetAPV)) {
        continue;
      }
      useRangeMeasurementSettings();
      Serial.println(F("Moving..."));
      temporarilyMoveToAPV(targetAPV, &actualAPV);
      Serial.print(F("The APV the servo actually reached was: "));
      Serial.println(actualAPV);

    } else if (parseWord(F("left"))) {
      left = actualAPV;
      Serial.print(F("New range left endpoint will be "));
      Serial.println(left);

    } else if (parseWord(F("right"))) {
      right = actualAPV;
      Serial.print(F("New range right endpoint will be "));
      Serial.println(right);

    } else if (parseWord(F("center"))) {
      center = actualAPV;
      Serial.print(F("New range center point will be "));
      Serial.println(center);

    } else if (parseWord(F("save"))) {
      if (saveRangeSettings(left, right, center)) {
        return true;
      }

    } else if (parseWord(F("cancel"))) {
      return false;

    } else {
      Serial.println(F("Error: What you entered is not a valid command."));
      printRangeSettingsInteractiveHelp();
    }
  }
}

bool changeRangeSettingsAPVHelper(int16_t *newAPVOut) {
  scanRawInput();
  if (rawInputLen == 0) {
    Serial.println(F("Current value will be kept."));
    return true;
  }
  if (!parseNumber(newAPVOut)) {
    return false;
  }
  if (*newAPVOut > 16383) {
    Serial.println(F("Error: Invalid APV."));
    return false;
  }

  if (widestRangeLeftAPV() != -1) {
    int16_t safeAPV = constrain(
      *newAPVOut, widestRangeLeftAPV(), widestRangeRightAPV());
    if (safeAPV != *newAPVOut) {
      Serial.print(F("Warning: "));
      Serial.print(*newAPVOut);
      Serial.print(F(" is beyond the recommended limit of "));
      Serial.print(safeAPV);
      Serial.println('.');
      Serial.println(F(
        "If you command the servo to move past the recommended limit, it\r\n"
        "may hit a physical end-stop and burn itself out. Use recommended\r\n"
        "limit instead? Enter \"y\" or \"n\":"));
      if (scanYesNo()) {
        *newAPVOut = safeAPV;
      }
    }
  }

  return true;
}

bool changeRangeSettingsAPV() {
  printRangeLeftAPVSetting();
  Serial.println(F(
    "Enter new left endpoint, as APV (or nothing to keep same):"));
  int16_t left = settings.rangeLeftAPV;
  if (!changeRangeSettingsAPVHelper(&left)) {
    return false;
  }

  printRangeRightAPVSetting();
  Serial.println(F(
    "Enter new right endpoint, as APV (or nothing to keep same):"));
  int16_t right = settings.rangeRightAPV;
  if (!changeRangeSettingsAPVHelper(&right)) {
    return false;
  }

  printRangeCenterAPVSetting();
  Serial.println(F(
    "Enter new center point, as APV (or nothing to keep same):"));
  int16_t center = settings.rangeCenterAPV;
  if (!changeRangeSettingsAPVHelper(&center)) {
    return false;
  }

  return saveRangeSettings(left, right, center);
}

void changeRangeSettings() {
  if (!checkSupportedModel()) {
    goto cancel;
  }

  Serial.println(F(
    "How do you want to choose the new range settings? Enter one of:"));
  Serial.println(F(
    "  default     - Change range to the factory default"));
  Serial.println(F(
    "  widest      - Change range to be as wide as possible"));
  Serial.println(F(
    "  detect      - Detect how far the servo can move, and use that range"));
  Serial.println(F(
    "  interactive - Change range by moving servo until it looks right"));
  Serial.println(F(
    "  apv         - Change range to specific APV numbers you type in"));

  scanRawInput();
  if (parseWord(F("default"))) {
    if (!changeRangeSettingsDefault()) {
      goto cancel;
    }
  } else if (parseWord(F("widest"))) {
    if (!changeRangeSettingsWidest()) {
      goto cancel;
    }
  } else if (parseWord(F("detect"))) {
    if (!changeRangeSettingsDetect()) {
      goto cancel;
    }
  } else if (parseWord(F("interactive"))) {
    if (!changeRangeSettingsInteractive()) {
      goto cancel;
    }
  } else if (parseWord(F("apv"))) {
    if (!changeRangeSettingsAPV()) {
      goto cancel;
    }
  } else if (rawInputLen == 0) {
    goto cancel;
  } else {
    Serial.println(F("Error: What you entered is not a valid option."));
    goto cancel;
  }
  return;

cancel:
  Serial.println(F("Current range settings will not be changed."));
  undoRangeMeasurementSettings();
}

