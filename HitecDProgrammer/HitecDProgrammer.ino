#include <HitecDServo.h>

#include "CommandLine.h"
#include "Common.h"
#include "RangeSettings.h"
#include "Settings.h"
#include "UnsupportedModel.h"

void printSettings() {
  printIdSetting();
  printDirectionSetting();
  printSpeedSetting();
  printDeadbandSetting();
  printSoftStartSetting();
  printRangeLeftAPVSetting();
  printRangeCenterAPVSetting();
  printRangeRightAPVSetting();
  printFailSafeSetting();
  printPowerLimitSetting();
  printOverloadProtectionSetting();
  printSmartSenseSetting();
  printSensitivityRatioSetting();
}

void resetSettingsToFactoryDefaults() {
  /* Print a copy of the servo settings, so the user has a backup copy of the
  previous settings if they change their mind after resetting it. */
  Serial.println(F("Current servo settings:"));
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

  Serial.println(F("New servo settings:"));
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
    printDiagnosticsForUnsupportedModel();
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
    Serial.println(F("Current servo settings:"));
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
