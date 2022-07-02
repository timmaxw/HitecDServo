#include "Settings.h"

#include "CommandLine.h"
#include "Common.h"

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
  writeSettings();
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
  if (parseWord("Clockwise")) {
    newCounterclockwise = false;
  } else if (parseWord("Counterclockwise")) {
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
  int16_t prevAPVFor850 = settings.rangeLeftAPV;
  int16_t prevAPVFor1500 = settings.rangeCenterAPV;
  int16_t prevAPVFor2150 = settings.rangeRightAPV;
  settings.rangeLeftAPV = 16383 - prevAPVFor2150;
  settings.rangeCenterAPV = 16383 - prevAPVFor1500;
  settings.rangeRightAPV = 16383 - prevAPVFor850;
  // TODO: Print new raw angles
  writeSettings();
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
  writeSettings();
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
  writeSettings();
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
  writeSettings();
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
  if (parseWord("Off")) {
    newFailSafe = 0;
    newFailSafeLimp = false;
  } else if (parseWord("Limp")) {
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
  writeSettings();
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
  writeSettings();
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
  if (parseWord("Off")) {
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
  writeSettings();
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
  if (parseWord("On")) {
    newSmartSense = true;
  } else if (parseWord("Off")) {
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
  writeSettings();
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
  writeSettings();
  return;

cancel:
  Serial.println(F("Current sensitivity ratio will be kept."));
}

