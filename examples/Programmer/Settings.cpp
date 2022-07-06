#include "Settings.h"

#include "CommandLine.h"
#include "ModelSpecs.h"
#include "Move.h"
#include "Programmer.h"
#include "RangeSettings.h"

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

void printAllSettings() {
  printIdSetting();
  printDirectionSetting();
  printSpeedSetting();
  printDeadbandSetting();
  printSoftStartSetting();
  printRangeLeftAPVSetting();
  printRangeRightAPVSetting();
  printRangeCenterAPVSetting();
  printFailSafeSetting();
  printPowerLimitSetting();
  printOverloadProtectionSetting();
  printSmartSenseSetting();
  printSensitivityRatioSetting();
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

void printIdSetting() {
  Serial.print(F("Current ID: "));
  printValueWithDefault(settings.id,
    HitecDSettings::defaultId);
}

void changeIdSetting() {
  printIdSetting();

  Serial.println(F("Enter new ID (or nothing to cancel):"));

  int16_t newId;
  if (!scanNumber(&newId) || newId == settings.id) {
    goto cancel;
  }
  if (newId < 0 || newId > 254) {
    Serial.println(F("Error: ID must be between 0 and 254."));
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.id = newId;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current ID will be kept."));
}

void printDirectionSetting() {
  Serial.print(F("Current direction: "));
  if (settings.counterclockwise) {
    Serial.println(F("Counterclockwise (default is clockwise)"));
  } else {
    Serial.println(F("Clockwise (default)"));
  }
}

void changeDirectionSetting() {
  printDirectionSetting();

  Serial.println(F(
    "Enter \"Clockwise\" or \"Counterclockwise\" (or nothing to cancel):"));
  scanRawInput();
  bool newCounterclockwise;
  if (parseWord(F("Clockwise"))) {
    newCounterclockwise = false;
  } else if (parseWord(F("Counterclockwise"))) {
    newCounterclockwise = true;
  } else if (rawInputLen == 0) {
    goto cancel;
  } else {
    Serial.println(F("Error: Invalid direction."));
    goto cancel;
  }
  if (newCounterclockwise == settings.counterclockwise) {
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.counterclockwise = newCounterclockwise;

  /* Nested block prevents compiler warnings about "goto cancel" crossing
  initialization of variables */
  {
    int16_t prevRangeLeftAPV = settings.rangeLeftAPV;
    int16_t prevRangeRightAPV = settings.rangeRightAPV;
    int16_t prevRangeCenterAPV = settings.rangeCenterAPV;
    settings.rangeLeftAPV = HITECD_APV_MAX - prevRangeRightAPV;
    settings.rangeRightAPV = HITECD_APV_MAX - prevRangeLeftAPV;
    settings.rangeCenterAPV = HITECD_APV_MAX - prevRangeCenterAPV;
  }

  saveSettings();
  return;

cancel:
  Serial.println(F("Current direction will be kept."));
}

void printSpeedSetting() {
  Serial.print(F("Current speed: "));
  printValueWithDefault(settings.speed,
    HitecDSettings::defaultSpeed);
}

void changeSpeedSetting() {
  printSpeedSetting();

  Serial.println(F(
    "Enter new speed from 10, 20, ... 100 (or nothing to cancel):"));
  int16_t newSpeed;
  if (!scanNumber(&newSpeed) || newSpeed == settings.speed) {
    goto cancel;
  }
  if (newSpeed < 10 || newSpeed > 100 || newSpeed % 10 != 0) {
    Serial.println(F("Error: Invalid speed."));
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.speed = newSpeed;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current speed will be kept."));
}

void printDeadbandSetting() {
  Serial.print(F("Current deadband: "));
  printValueWithDefault(settings.deadband,
    HitecDSettings::defaultDeadband);
}

void changeDeadbandSetting() {
  printDeadbandSetting();

  Serial.println(F(
    "Enter new deadband from 1, 2, ... 10 (or nothing to cancel):"));
  int16_t newDeadband;
  if (!scanNumber(&newDeadband) || newDeadband == settings.deadband) {
    goto cancel;
  }
  if (newDeadband < 1 || newDeadband > 10) {
    Serial.println(F("Error: Invalid deadband."));
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.deadband = newDeadband;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current deadband will be kept."));
}

void printSoftStartSetting() {
  Serial.print(F("Current soft start: "));
  printValueWithDefault(settings.softStart,
    HitecDSettings::defaultSoftStart);
}

void changeSoftStartSetting() {
  printSoftStartSetting();

  Serial.println(F(
    "Enter new soft start from 20, 40, ... 100 (or nothing to cancel):"));
  int16_t newSoftStart;
  if (!scanNumber(&newSoftStart) || newSoftStart == settings.softStart) {
    goto cancel;
  }
  if (newSoftStart < 20 || newSoftStart > 100 || newSoftStart % 20 != 0) {
    Serial.println(F("Error: Invalid soft start."));
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.softStart = newSoftStart;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current soft start will be kept."));
}

void printFailSafeSetting() {
  Serial.print(F("Current fail safe: "));
  if (settings.failSafe) {
    Serial.print(settings.failSafe);
    Serial.println(F(" (default is Off)"));
  } else if (settings.failSafeLimp) {
    Serial.println(F("Limp (default is Off)"));
  } else {
    Serial.println(F("Off (default)"));
  }
}

void changeFailSafeSetting() {
  printFailSafeSetting();

  Serial.println(F(
    "Enter new fail safe point in microseconds; or \"Off\" or \"Limp\" (or\r\n"
    "nothing to cancel):"));
  scanRawInput();
  int16_t newFailSafe;
  bool newFailSafeLimp;
  if (parseWord(F("Off"))) {
    newFailSafe = 0;
    newFailSafeLimp = false;
  } else if (parseWord(F("Limp"))) {
    newFailSafe = 0;
    newFailSafeLimp = true;
  } else if (parseNumber(&newFailSafe)) {
    newFailSafeLimp = false;
    if ((newFailSafe != 0 && newFailSafe < 850) || newFailSafe > 2150) {
      Serial.println(F("Error: Fail-safe should be between 850 and 2150"));
      goto cancel;
    }
  } else {
    goto cancel;
  }
  if (newFailSafe == settings.failSafe &&
      newFailSafeLimp == settings.failSafeLimp) {
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.failSafe = newFailSafe;
  settings.failSafeLimp = newFailSafeLimp;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current fail safe will be kept."));
}

void printPowerLimitSetting() {
  Serial.print(F("Current power limit: "));
  printValueWithDefault(settings.powerLimit,
    HitecDSettings::defaultPowerLimit);
}

void changePowerLimitSetting() {
  printPowerLimitSetting();

  Serial.println(F(
    "Warning: Power limit is an undocumented setting, not supported by\r\n"
    "Hitec's official programmers. Use at your own risk."));

  Serial.println(F(
    "Enter new power limit from 0 to 100 (or nothing to cancel):"));
  int16_t newPowerLimit;
  if (!scanNumber(&newPowerLimit) || newPowerLimit == settings.powerLimit) {
    goto cancel;
  }
  if (newPowerLimit < 0 || newPowerLimit > 100) {
    Serial.println(F("Error: Invalid power limit."));
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.powerLimit = newPowerLimit;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current power limit will be kept."));
}

void printOverloadProtectionSetting() {
  Serial.print(F("Current overload protection: "));
  if (settings.overloadProtection < 100) {
    Serial.print(settings.overloadProtection);
    Serial.println(F(" (default is Off)"));
  } else {
    Serial.println(F("Off (default)"));
  }
}

void changeOverloadProtectionSetting() {
  printOverloadProtectionSetting();

  Serial.println(F(
    "Enter new overload protection from 10, 20, ... 50; or \"Off\" (or\r\n"
    "nothing to cancel):"));
  scanRawInput();
  int16_t newOverloadProtection;
  if (parseWord(F("Off"))) {
    newOverloadProtection = 100;
  } else if (parseNumber(&newOverloadProtection)) {
    if (newOverloadProtection < 10 ||
        (newOverloadProtection > 50 && newOverloadProtection != 100) ||
        newOverloadProtection % 10 != 0) {
      Serial.println(F("Error: Invalid overload protection."));
      goto cancel;
    }
  } else {
    goto cancel;
  }
  if (newOverloadProtection == settings.overloadProtection) {
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.overloadProtection = newOverloadProtection;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current overload protection will be kept."));
}

void printSmartSenseSetting() {
  Serial.print(F("Current smart sense: "));
  if (settings.smartSense) {
    Serial.println(F("On (default)"));
  } else {
    Serial.println(F("Off (default is On)"));
  }
}

void changeSmartSenseSetting() {
  printSmartSenseSetting();

  Serial.println(F("Enter \"On\" or \"Off\" (or nothing to cancel):"));
  scanRawInput();
  bool newSmartSense;
  if (parseWord(F("On"))) {
    newSmartSense = true;
  } else if (parseWord(F("Off"))) {
    newSmartSense = false;
  } else if (rawInputLen == 0) {
    goto cancel;
  } else {
    Serial.println(F("Error: You did not enter \"On\" or \"Off\"."));
    goto cancel;
  }
  if (newSmartSense == settings.smartSense) {
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.smartSense = newSmartSense;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current smart sense will be kept."));
}

void printSensitivityRatioSetting() {
  Serial.print(F("Current sensitivity ratio: "));
  printValueWithDefault(settings.sensitivityRatio,
    HitecDSettings::defaultSensitivityRatio);
}

void changeSensitivityRatioSetting() {
  printSensitivityRatioSetting();

  if (settings.smartSense) {
    Serial.println(F(
      "Warning: Sensitivity ratio has no effect if smart sense is on."));
  }

  Serial.println(F(
    "Enter new sensitivity ratio from 819 to 4095 (or nothing to cancel):"));
  int16_t newSensitivityRatio;
  if (!scanNumber(&newSensitivityRatio) ||
      newSensitivityRatio == settings.sensitivityRatio) {
    goto cancel;
  }
  if (newSensitivityRatio < 819 || newSensitivityRatio > 4095) {
    Serial.println(F("Error: Invalid sensitivity ratio."));
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings.sensitivityRatio = newSensitivityRatio;
  saveSettings();
  return;

cancel:
  Serial.println(F("Current sensitivity ratio will be kept."));
}

void resetSettingsToFactoryDefaults() {
  /* Print a copy of the servo settings, so the user has a backup copy of the
  previous settings if they change their mind after resetting it. */
  Serial.println(F("Current servo settings:"));
  printAllSettings();

  Serial.println(F(
    "Reset all settings to factory defaults? Enter \"y\" or \"n\":"));
  if (!scanYesNo()) {
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings = HitecDSettings();
  saveSettings();

  if (!servo.isModelSupported()) {
    /* The servo library doesn't know the default values of rangeLeftAPV/etc.,
    but we just reset the servo, so we know the current values must be the
    default values. Record those values. */
    defaultRangeLeftAPV = settings.rangeLeftAPV;
    defaultRangeRightAPV = settings.rangeRightAPV;
    defaultRangeCenterAPV = settings.rangeCenterAPV;
  }

  Serial.println(F("New servo settings:"));
  printAllSettings();

  return;

cancel:
  Serial.println(F("Settings will not be reset to factory defaults."));
}
