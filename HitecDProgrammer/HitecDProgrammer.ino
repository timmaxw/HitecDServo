#include <HitecDServo.h>

#include "CommandLine.h"
#include "Common.h"
#include "RangeSettings.h"

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

void printSettings() {
  Serial.println(F("Servo settings:"));

  Serial.print(F("  ID: "));
  printValueWithDefault(settings.id,
    HitecDSettings::defaultId);

  Serial.print(F("  Direction: "));
  if (settings.counterclockwise) {
    Serial.println(F("Counterclockwise (default is clockwise)"));
  } else {
    Serial.println(F("Clockwise (default)"));
  }

  Serial.print(F("  Speed: "));
  printValueWithDefault(settings.speed,
    HitecDSettings::defaultSpeed);

  Serial.print(F("  Deadband: "));
  printValueWithDefault(settings.deadband,
    HitecDSettings::defaultDeadband);

  Serial.print(F("  Soft start: "));
  printValueWithDefault(settings.softStart,
    HitecDSettings::defaultSoftStart);

  Serial.print(F("  Raw angle for 850us PWM (left endpoint): "));
  printValueWithDefault(settings.rangeLeftRawAngle,
    HitecDSettings::defaultRangeLeftRawAngle(modelNumber));

  Serial.print(F("  Raw angle for 1500us PWM (center point): "));
  printValueWithDefault(settings.rangeCenterRawAngle,
    HitecDSettings::defaultRangeCenterRawAngle(modelNumber));

  Serial.print(F("  Raw angle for 2150us PWM (right endpoint): "));
  printValueWithDefault(settings.rangeRightRawAngle,
    HitecDSettings::defaultRangeRightRawAngle(modelNumber));

  Serial.print(F("  Fail safe: "));
  if (settings.failSafe) {
    Serial.print(settings.failSafe);
    Serial.println(F(" (default is Off)"));
  } else if (settings.failSafeLimp) {
    Serial.println(F("Limp (default is Off)"));
  } else {
    Serial.println(F("Off (default)"));
  }

  Serial.print(F("  Power limit: "));
  printValueWithDefault(settings.powerLimit,
    HitecDSettings::defaultPowerLimit);

  Serial.print(F("  Overload protection: "));
  if (settings.overloadProtection < 100) {
    Serial.print(settings.overloadProtection);
    Serial.println(F(" (default is Off)"));
  } else {
    Serial.println(F("Off (default)"));
  }

  Serial.print(F("  Smart sense: "));
  if (settings.smartSense) {
    Serial.println(F("On (default)"));
  } else {
    Serial.println(F("Off (default is On)"));
  }

  Serial.print(F("  Sensitivity ratio: "));
  printValueWithDefault(settings.sensitivityRatio,
    HitecDSettings::defaultSensitivityRatio);
}

void changeIdSetting() {
  Serial.print(F("Current ID: "));
  printValueWithDefault(settings.id,
    HitecDSettings::defaultId);

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

void changeDirectionSetting() {
  Serial.print(F("Current direction: "));
  if (settings.counterclockwise) {
    Serial.println(F("Counterclockwise (default is clockwise)"));
  } else {
    Serial.println(F("Clockwise (default)"));
  }

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
  int16_t prevRawAngleFor850 = settings.rangeLeftRawAngle;
  int16_t prevRawAngleFor1500 = settings.rangeCenterRawAngle;
  int16_t prevRawAngleFor2150 = settings.rangeRightRawAngle;
  settings.rangeLeftRawAngle = 16383 - prevRawAngleFor2150;
  settings.rangeCenterRawAngle = 16383 - prevRawAngleFor1500;
  settings.rangeRightRawAngle = 16383 - prevRawAngleFor850;
  // TODO: Print new raw angles
  writeSettings();
  return;

cancel:
  Serial.println(F("Current direction will be kept."));
}

void changeSpeedSetting() {
  Serial.print(F("Current speed: "));
  printValueWithDefault(settings.speed,
    HitecDSettings::defaultSpeed);

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

void changeDeadbandSetting() {
  Serial.print(F("Current deadband: "));
  printValueWithDefault(settings.deadband,
    HitecDSettings::defaultDeadband);

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

void changeSoftStartSetting() {
  Serial.print(F("Current soft start: "));
  printValueWithDefault(settings.softStart,
    HitecDSettings::defaultSoftStart);

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

void changeFailSafeSetting() {
  Serial.print(F("Current fail safe: "));
  if (settings.failSafe) {
    Serial.print(settings.failSafe);
    Serial.println(F(" (default is Off)"));
  } else if (settings.failSafeLimp) {
    Serial.println(F("Limp (default is Off)"));
  } else {
    Serial.println(F("Off (default)"));
  }

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

void changePowerLimitSetting() {
  Serial.print(F("Current power limit: "));
  printValueWithDefault(settings.powerLimit,
    HitecDSettings::defaultPowerLimit);

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

void changeOverloadProtectionSetting() {
  Serial.print(F("Current overload protection: "));
  if (settings.overloadProtection < 100) {
    Serial.print(settings.overloadProtection);
    Serial.println(F(" (default is Off)"));
  } else {
    Serial.println(F("Off (default)"));
  }

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

void changeSmartSenseSetting() {
  Serial.print(F("Current smart sense: "));
  if (settings.smartSense) {
    Serial.println(F("On (default)"));
  } else {
    Serial.println(F("Off (default is On)"));
  }

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

void changeSensitivityRatioSetting() {
  Serial.print(F("Current sensitivity ratio: "));
  printValueWithDefault(settings.sensitivityRatio,
    HitecDSettings::defaultSensitivityRatio);

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

void resetSettingsToFactoryDefaults() {
  /* Print a copy of the servo settings, so the user has a backup copy of the
  previous settings if they change their mind after resetting it. */
  printSettings();

  Serial.println(F(
    "Reset all settings to factory defaults? Enter \"Yes\" or \"No\":"));
  scanRawInput();
  if (parseWord("Yes")) {
    (void)0;
  } else if (parseWord("No") || rawInputLen == 0) {
    goto cancel;
  } else {
    Serial.println(F("Error: You did not enter \"Yes\" or \"No\"."));
    goto cancel;
  }
  if (!checkSupportedModel()) {
    goto cancel;
  }

  settings = HitecDSettings();
  writeSettings();
  printSettings();
  return;

cancel:
  Serial.println(F("Settings will not be reset to factory defaults."));
}

void setup() {
  int res;

  Serial.begin(115200);

  int16_t pin;
  do {
    Serial.println(F("Enter the Arduino pin that the servo is attached to:"));
  } while (!scanNumber(&pin, PRINT_IF_EMPTY));
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
      "them in the issue. Thanks!\r\n"));
    Serial.println(F("Press enter to continue..."));
    scanRawInput(NO_ECHO);
    Serial.println(F(
      "===================================================================="
    ));
  }

  if ((res = servo.readSettings(&settings)) < 0) {
    printErr(res, true);
  }
  printSettings();

  Serial.println(F(
    "===================================================================="));
  printHelp();
}

void printHelp() {
  Serial.println(F("Available commands:"));
  Serial.println(F(
    "  show        - Show current servo settings"));
  Serial.println(F(
    "  id          - Change ID setting"));
  Serial.println(F(
    "  direction   - Change direction setting"));
  Serial.println(F(
    "  speed       - Change speed setting"));
  Serial.println(F(
    "  deadband    - Change deadband setting"));
  Serial.println(F(
    "  softstart   - Change soft-start setting"));
  Serial.println(F(
    "  range       - Change range-of-motion settings"));
  Serial.println(F(
    "  failsafe    - Change fail-safe setting"));
  Serial.println(F(
    "  powerlimit  - Change power-limit setting"));
  Serial.println(F(
    "  overload    - Change overload-protection setting"));
  Serial.println(F(
    "  smartsense  - Change smart sense setting"));
  Serial.println(F(
    "  sensitivity - Change sensitivity ratio setting"));
  Serial.println(F(
    "  reset       - Reset all settings to factory defaults"));
  Serial.println(F(
    "  help        - Show this list of commands again"));
}

void loop() {
  Serial.println(F(
    "===================================================================="));
  Serial.println(F("Enter a command:"));
  scanRawInput();
  if (parseWord("show")) {
    printSettings();
  } else if (parseWord("id")) {
    changeIdSetting();
  } else if (parseWord("direction")) {
    changeDirectionSetting();
  } else if (parseWord("speed")) {
    changeSpeedSetting();
  } else if (parseWord("deadband")) {
    changeDeadbandSetting();
  } else if (parseWord("softstart")) {
    changeSoftStartSetting();
  } else if (parseWord("range")) {
    changeRangeSettings();
  } else if (parseWord("failsafe")) {
    changeFailSafeSetting();
  } else if (parseWord("powerlimit")) {
    changePowerLimitSetting();
  } else if (parseWord("overload")) {
    changeOverloadProtectionSetting();
  } else if (parseWord("smartsense")) {
    changeSmartSenseSetting();
  } else if (parseWord("sensitivity")) {
    changeSensitivityRatioSetting();
  } else if (parseWord("reset")) {
    resetSettingsToFactoryDefaults();
  } else if (parseWord("help")) {
    printHelp();
  } else {
    Serial.println(F("Error: What you entered is not a valid command."));
    printHelp();
  }
}
