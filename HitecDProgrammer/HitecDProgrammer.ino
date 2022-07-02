#include <HitecDServo.h>

#include "CommandLine.h"
#include "Common.h"
#include "RangeSettings.h"
#include "Settings.h"

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
  printValueWithDefault(settings.rangeLeftAPV,
    HitecDSettings::defaultRangeLeftAPV(modelNumber));

  Serial.print(F("  Raw angle for 1500us PWM (center point): "));
  printValueWithDefault(settings.rangeCenterAPV,
    HitecDSettings::defaultRangeCenterAPV(modelNumber));

  Serial.print(F("  Raw angle for 2150us PWM (right endpoint): "));
  printValueWithDefault(settings.rangeRightAPV,
    HitecDSettings::defaultRangeRightAPV(modelNumber));

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
